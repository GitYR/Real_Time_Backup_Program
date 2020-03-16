#include "./ssu_backup.h"

int input_token(const char * str) // 명령어 확인 함수
{
	int i,len;
	char buf[TMP_SIZE];

	memset((char *)buf,0,TMP_SIZE);
	len = strlen(str);

	for(i = 0; i < len && str[i] != ' '; i++)
		buf[i] = str[i];

	if(!strcmp(buf,"add"))
		return ADD;
	else if(!strcmp(buf,"remove"))
		return REMOVE;
	else if(!strcmp(buf,"compare"))
		return COMPARE;
	else if(!strcmp(buf,"recover"))
		return RECOVER;
	else if(!strcmp(buf,"list"))
		return LIST;
	else if(!strcmp(buf,"ls"))
		return LS;
	else if(!strcmp(buf,"vi") || !strcmp(buf,"vim"))
		return VI;
	else if(!strcmp(buf,"exit"))
		return EXIT;
	else
		return INVALID;
}

int get_index(const char * str) // 명령어 부분 건너 뛰기
{
	int index = 0;

	for(index = 0; index < strlen(str); index++)
		if(str[index] == ' ')
			return index;
}

bool is_integer(const char * str) // 정수 판독
{
	int i;

	for(i = 0; i < strlen(str); i++) // period 검사
	{
		if(str[i] == '.' || !('0' <= str[i] && str[i] <= '9'))
		{
			fprintf(stderr,"error : please input an integer number.\n");
			return FALSE;
		}
	}
	return TRUE;
}

int get_option_argument(const char * str,char * option,int index,int * number) // option 파싱
{
	int i,len;

	len = strlen(str);

	for(i = index; i < len && str[i] == ' ' && str[i] != '-'; i++);

	if(str[i] == '-')
	{
		fprintf(stderr,"error : using an invalid option\n");
		return -1;
	}

	if(i >= len)
	{
		fprintf(stderr,"error : using an invalid option\n");
		return -1;
	}

	index = i;
	memset((char *)option,0,TMP_SIZE);
	for(; i < len && str[i] != ' '; i++)
		option[i-index] = str[i];
	if(!is_integer(option))
		return -1;
	*number = atoi(option);

	return i;
}

bool get_dirfile(const char * str,Node * newNode) // 디렉토리 재귀함수
{
	int i,count;
	struct stat statbuf;
	struct dirent ** flist;
	char curwdir[FILE_NAME];
	char filename[FILE_NAME];

	memset((char *)curwdir,0,FILE_NAME);
	memset((char *)filename,0,FILE_NAME);

	strcpy(curwdir,get_current_dir_name());

	if((count = scandir(str,&flist,0,versionsort)) < 0)
	{
		fprintf(stderr,"scandir error for %s\n", str);
		return FALSE;
	}

	chdir(str);

	i = 0; 
	while(i < count) 
	{ 
		if(!strcmp(flist[i]->d_name,".") || !strcmp(flist[i]->d_name,".."))
		{
			i++;
			continue;
		}

		pthread_t new_id; 
		
		Node * node = (Node *)malloc(sizeof(Node));
		memset(node, 0, sizeof(Node));
		
		node->t_id = newNode->t_id;
		node->ttime = newNode->ttime;
		node->period = newNode->period;
		node->number = newNode->number;
		node->option.m = newNode->option.m;
		node->option.n = newNode->option.n;
		node->option.t = newNode->option.t;
		node->option.d = newNode->option.d;

		if(lstat(flist[i]->d_name,&statbuf) < 0)
		{
			fprintf(stderr,"lstat() error for %s\n", flist[i]->d_name);
			return FALSE;
		}
		node->mtime = statbuf.st_mtime;
		node->head = NULL;

		if(realpath(flist[i]->d_name,filename) == NULL)
		{
			fprintf(stderr,"realpath() error\n");
			return FALSE;
		}	
		strcpy(node -> filename,filename);

		if(S_ISREG(statbuf.st_mode))
		{
			if(!list_compare(node->filename))
			{
				fprintf(stderr,"error : same filename is in a backup_list\n");
				return -1;
			}

			if(pthread_create(&(node->t_id),NULL,thread_handler,(void *)node) != 0)
			{
				fprintf(stderr,"pthread_create() error\n");
				return FALSE;
			}
			pthread_detach(node->t_id);
			mutex_log(node->filename,"added"); // log 쓰기
			list_insert(node);
		}
		else if(S_ISDIR(statbuf.st_mode))
		{
			if(!get_dirfile(flist[i]->d_name,node))
				return FALSE;
			chdir(str);
		}
		else
		{
			fprintf(stderr,"error : can add only regular file.\n");
			return FALSE;
		}
		i++;
	}

	return TRUE;
}

void get_time_string(char * str) // 현재 시각 구하기
{
	char timestr[TMP_SIZE];
	time_t timer = time(NULL);
	struct tm * t = localtime(&timer);

	memset((char *)str,0,TMP_SIZE);	
	memset((char *)timestr,0,TMP_SIZE);

	sprintf(timestr,"%02d%02d%02d%02d%02d%02d", t->tm_year - 100, t->tm_mon + 1, t->tm_mday,
			t->tm_hour, t->tm_min, t->tm_sec);
	strcpy(str,timestr);
}

void get_file_name(char * str,char * filename) // 파일의 상대경로 이름 따오기
{
	int i,index;

	index = strlen(str);
	while(index >= 0)
	{
		if(str[index] == '/')
			break;
		index--;
	}
	index++;

	for(i = index; i < strlen(str); i++)
		filename[i-index] = str[i];
}

void set_backup_file(char * str,char * gentime,char * backup) // backup file 생성 함수
{
	int fd1,fd2,count;
	char backup_file[BUF_SIZE];
	char filename[FILE_NAME];
	char buf[BUF_SIZE];
	struct stat statbuf1,statbuf2;

	memset((char *)backup_file,0,BUF_SIZE);
	memset((char *)filename,0,FILE_NAME);
	memset((char *)buf,0,BUF_SIZE);
	memset((char *)backup,0,BUF_SIZE);

	get_file_name(str,filename); // 파일의 상대경로 이름 받기
	sprintf(backup_file,"%s/%s_%s", dir_backup, filename, gentime);

	if((fd1 = open(str,O_RDONLY)) < 0) // backup할 파일
	{
		fprintf(stderr,"open error for %s\n", str);
		return;
	}

	if(lstat(str,&statbuf1) < 0)
	{
		fprintf(stderr,"lstat() error\n");
		return;
	}

	if((fd2 = open(backup_file,O_WRONLY | O_CREAT | O_EXCL, S_MODE)) < 0) // 생성할 파일
	{
		fprintf(stderr,"open error for %s\n", backup_file);
		return;
	}

	if(lstat(backup_file,&statbuf2) < 0)
	{
		fprintf(stderr,"lstat() error\n");
		return;
	}

	while((count = read(fd1,buf,BUF_SIZE)) > 0)
	{
		write(fd2,buf,count);
		memset((char *)buf,0,BUF_SIZE);
	}
	
	if(chmod(backup_file,statbuf1.st_mode | statbuf2.st_mode) < 0) // 실행권한 동기화
	{
		fprintf(stderr,"chmod() error\n");
		return;
	}
	fsync(fd2); // 바로 쓰기

	mutex_log(backup_file,"generated"); 
	strcpy(backup,backup_file);

	close(fd1);
	close(fd2);
}

void mutex_log(char * filename,char * message) // log 쓰는 함수
{
	int i;
	FILE * fp;
	char timestr[TMP_SIZE],settime[TMP_SIZE];
	char buf[BUF_SIZE];

	memset((char *)timestr,0,TMP_SIZE);
	memset((char *)settime,0,TMP_SIZE);
	memset((char *)buf,0,BUF_SIZE);

	get_time_string(timestr); // 시간 값 구해오기

	settime[0] = '[';
	for(i = 0; i < 6; i++)
		settime[i+1] = timestr[i];
	settime[i+1] = ' ';
	for(i = 6; i < strlen(timestr); i++)
		settime[i+2] = timestr[i];
	settime[i+2] = ']'; // log에 맞게 시간 분리

	pthread_mutex_lock(&mutex); // 상호 배제 시작

	if((fp = fopen(log_file,"r+")) == NULL) // log file write
	{
		fprintf(stderr,"open error for %s\n", log_file);
		exit(EXIT_FAILURE);
	}

	fseek(fp,0,SEEK_END);
	sprintf(buf,"%s %s %s\n",settime, filename, message);
	fputs(buf,fp);

	fclose(fp);
	pthread_mutex_unlock(&mutex); // 상호 배제 끝
}
