#include "./ssu_backup.h"

void ssu_backup_start(void)
{
	int command;
	char buf[BUF_SIZE];

	system("clear");
	
	if(pthread_mutex_init(&mutex,NULL) != 0)
		fprintf(stderr,"mutex init error\n");

	list_init(); // 리스트 초기화

	while(1)
	{
		memset((char *)buf,0,BUF_SIZE);
		printf("20152437>");
		fgets(buf,BUF_SIZE,stdin);

		if(buf[0] == '\n') // 엔터 입력 시
			continue;

		buf[strlen(buf)-1] = 0; // 개행 제거
		command = input_token(buf);

		if(command == ADD) // add
		{
			ssu_backup_add(buf);
		}
		else if(command == REMOVE) // remove
		{
			ssu_backup_remove(buf);
		}
		else if(command == COMPARE) // compare
		{
			ssu_backup_compare(buf);
		}
		else if(command == RECOVER) // recover
		{
			ssu_backup_recover(buf);
		}
		else if(command == LIST) // list
		{
			ssu_backup_list(buf);
		}
		else if(command == LS) // ls
		{
			ssu_backup_ls(buf);
		}
		else if(command == VI) // vi
		{
			ssu_backup_vi(buf);
		}
		else if(command == EXIT) // exit
		{
			if(pthread_mutex_destroy(&mutex) != 0)
				fprintf(stderr,"mutex destroy error\n");
			exit(EXIT_SUCCESS);
		}
		else
		{
			fprintf(stderr,"INVALID command!\n");
			continue;
		}
	}
}

int ssu_backup_add(char * str) // add <FILENAME> [PERIOD] [OPTION]
{
	Node * newNode;
	Opt option;
	pthread_t t_id; // thread id
	struct tm * t;
	int ttime,len,count,number; // number -n option
	int i,j,index,period; // period 
	char curwdir[FILE_NAME];
	char dirname[FILE_NAME];
	char tempname[FILE_NAME];
	char filename[FILE_NAME*2]; // filename
	char per[TMP_SIZE]; 
	char opt[TMP_SIZE];
	char timestr[TMP_SIZE];
	struct stat statbuf;
	struct dirent ** flist; // file lists of directory -d option
	bool check = TRUE;

	/* 초기화 */

	newNode = (Node *)malloc(sizeof(Node));
	memset((char *)curwdir,0,FILE_NAME);
	memset((char *)dirname,0,FILE_NAME);
	memset((char *)tempname,0,FILE_NAME);
	memset((char *)filename,0,FILE_NAME*2);
	memset((char *)per,0,TMP_SIZE);
	memset((char *)opt,0,BUF_SIZE);
	option.m = option.n = option.t = option.d = FALSE; // option 초기화

	count = 0;
	len = strlen(str);
	index = get_index(str);

	for(;index < len && str[index] == ' ';index++);

	j = 0;
	for(i = index; str[i] != ' ' && i < len; i++) // filename 추출
	{
		tempname[j] = str[i];
		j++;
	}
	index = i;

	if(access(tempname,F_OK) != 0) // file이 존재하지 않음
	{
		fprintf(stderr,"error : there is no matching file\n");
		return -1;
	}

	if(realpath(tempname,filename) == NULL) // 절대경로 가져오는 함수(3)
	{
		fprintf(stderr,"error : realpath error\n");
		return -1;
	}
	if(strlen(filename) >= 256) // 길이 검사
	{
		fprintf(stderr,"error : the length of filename must be not over 255bytes.\n");
		return -1;
	}
	for(j = 0; j < strlen(filename); j++) // 아스키 값인지 검사
	{
		if(!IS_ASCII(filename[j]))
		{
			fprintf(stderr,"error : filename has to consist of ascii character.\n");
			return -1;
		}
	}

	if(lstat(filename,&statbuf) < 0)
	{
		fprintf(stderr,"lstat error for %s\n", filename);
		return -1;
	}

	if(!S_ISREG(statbuf.st_mode))
	{
		if(!S_ISDIR(statbuf.st_mode))
		{
			fprintf(stderr,"error : %s is not regular file.\n", filename);
			return -1;
		}
		else
			check = FALSE;
	}

	for(;index < len && str[index] == ' ';index++);

	if(index >= len)
	{
		fprintf(stderr,"error : please input a period.\n");
		return -1;
	}

	j = 0;
	for(i = index; str[i] != ' ' && i < len; i++) // period 추출
	{
		per[j] = str[i];
		j++;
	}
	index = i;

	if(!is_integer(per))
		return -1;
	period = atoi(per);
	if(!(5 <= period && period <= 10))
	{
		fprintf(stderr,"error : period range is 5 <= period <= 10\n");
		return -1;
	}

	for(;index < len && str[index] == ' ';index++);

	for(i = index; i < len; i++)
	{
		if(str[i] != ' ') // 공백이 아닌 문자가 등장
		{
			if(str[i] = '-')
			{
				if(str[i+1] == 'm') // mtime
				{
					option.m = TRUE;
					i = i+1;
				}
				else if(str[i+1] == 'n') // number
				{
					option.n = TRUE;
					i = i+2;

					if((i = get_option_argument(str,opt,i,&number)) < 0)
						return -1;
					if(!(1 <= number && number <= 100))
					{
						fprintf(stderr,"error : number range is 1<= number <= 100\n");
						return -1;
					}
				}
				else if(str[i+1] == 't') // 보관 time
				{
					option.t = TRUE;
					i = i+2;

					if((i = get_option_argument(str,opt,i,&ttime)) < 0)
						return -1;
					if(!(60 <= ttime && ttime <= 1200))
					{
						fprintf(stderr,"error : time range is 60 <= time <= 1200(sec)\n");
						return -1;
					}
				}
				else if(str[i+1] == 'd') // 해당 디렉토리의 모든 파일들을 리스트 추가
				{
					check = TRUE;
					option.d = TRUE;
					
					if(!S_ISDIR(statbuf.st_mode))
					{
						fprintf(stderr,"error : %s is not a directory.\n", filename);
						return -1;
					}

					if((count = scandir(filename,&flist,0,versionsort)) < 0)
					{
						fprintf(stderr,"scandir error for %s\n", tempname);
						return -1;
					}
					realpath(tempname,curwdir);
					strcpy(dirname,get_current_dir_name());

					i = i + 2;
				}
				else
				{
					fprintf(stderr,"error : using an invalid option\n");
					return -1;
				}
			}
		}
	} // end of 'for'

	if(!check)
	{
		fprintf(stderr,"error : input file is a directory without d option\n");
		return -1;
	}

	newNode->t_id = t_id;
	memset((char *)newNode->filename,0,FILE_NAME);
	newNode->period = period;

	if(option.n)
		newNode->number = number;
	else
		newNode->number = 0;

	if(option.t)
		newNode->ttime = ttime;
	else
		newNode->ttime = 0;
	
	newNode->option = option;
	newNode->t_id = t_id;

	if(!(newNode->option.d)) // d option이 없을 때
	{
		strcpy(newNode->filename,filename);

		if(!list_compare(newNode->filename))
		{
			fprintf(stderr,"error : same filename is in a backup_list\n");
			return -1;
		}
	
		if(pthread_create(&(newNode->t_id),NULL,thread_handler,(void *)newNode) != 0)
		{
			fprintf(stderr,"pthread_create() error\n");
			return -1;
		}
		if(lstat(newNode->filename,&statbuf) < 0)
		{
			fprintf(stderr,"lstat() error for %s\n", newNode -> filename);
			return -1;
		}

		newNode -> mtime = statbuf.st_mtime;
		newNode -> head = NULL;
		pthread_detach(newNode->t_id);
		mutex_log(newNode->filename,"added");
		list_insert(newNode); // list add
	}

	if(option.d) // d option exists
	{
		i = 0;

		chdir(curwdir); // 작업 디렉토리를 -d 의 디렉토리로 이동

		while(i < count)
		{
			if(!strcmp(flist[i]->d_name,".") || !strcmp(flist[i]->d_name,".."))
			{
				i++;
				continue;
			}
			pthread_t new_id;
			struct stat tempstat;

			memset((char *)tempname,0,FILE_NAME);

			if(realpath(flist[i]->d_name,tempname) == NULL)
			{
				fprintf(stderr,"realpath() error for %s\n", flist[i]->d_name);
				return -1;
			}
			if(lstat(tempname,&tempstat) < 0)
			{
				fprintf(stderr,"lstat error for %s\n", tempname);
				return -1;
			}

			if(S_ISDIR(tempstat.st_mode)) // 파일이 디렉토리
			{
				if(!get_dirfile(tempname,newNode))
					return -1;
				chdir(curwdir);
			}
			else if(S_ISREG(tempstat.st_mode)) // 일반 파일
			{
				Node * node = (Node *)malloc(sizeof(Node));
				memset(node, 0, sizeof(Node));
				strcpy(node -> filename,tempname);

				node->t_id = new_id;
				node->period = newNode->period;
				node->head = NULL;
				node->ttime = newNode->ttime;
				node->number = newNode->number;
				node->mtime = tempstat.st_mtime;
				node->option.m = newNode->option.m;
				node->option.n = newNode->option.n;
				node->option.t = newNode->option.t;
				node->option.d = newNode->option.d;
				
				if(!list_compare(node->filename)) // 같은 파일 있는지 검사
				{
					fprintf(stderr,"error : same filename is in a backup_list\n");
					return -1;
				}
			
				if(pthread_create(&(node->t_id),NULL,thread_handler,(void *)node) != 0)
				{
					fprintf(stderr,"pthread_create() error\n");
					return -1;
				}
				pthread_detach(node->t_id);
				mutex_log(node->filename,"added");
				list_insert(node);
			}
			else
			{
				fprintf(stderr,"error : %s is not a regular file.\n", flist[i]->d_name);
				return -1;
			}
			i++;
		}
		chdir(dirname);
	}
}

int ssu_backup_remove(char * str) // remove <FILENAME> [OPTION]
{
	Node * node;
	int index,len,i,j;
	char filename[FILE_NAME];
	char tempname[FILE_NAME];

	memset((char *)filename,0,FILE_NAME);
	memset((char *)tempname,0,FILE_NAME);

	len = strlen(str);
	index = get_index(str); // remove 명령어 부분 뛰어넘기
	
	for(;index < len && str[index] == ' ';index++);

	j = 0;
	for(i = index; str[i] != ' ' && i < len; i++) // filename 추출
	{
		tempname[j] = str[i];
		j++;
	}

	index = i;

	if(!strcmp(tempname,"-a")) // remove all option
	{
		for(i = index; i < len; i++)
		{
			if(str[i] != ' ')
			{
				fprintf(stderr,"error : do not use any arguments with 'a' option\n");
				return -1;
			}
		}
		while(head)
		{
			if(pthread_cancel(head->t_id) != 0)
			{
				fprintf(stderr,"pthread_cancel error\n");
				return -1;
			}
			mutex_log(head->filename,"deleted");
			list_delete(head->filename);
		}
		return 0; // exit_success
	}

	for(i = index; i < len && str[i] == ' '; i++);

	if(i < len && str[i] != ' ')
	{
		fprintf(stderr,"error : invalid input!\n");
		return -1;
	}

	if(access(tempname,F_OK) != 0) // file이 존재하지 않음
	{
		fprintf(stderr,"error : there is no matching file\n");
		return -1;
	}

	if(realpath(tempname,filename) == NULL) // 절대경로 가져오는 함수(3)
	{
		fprintf(stderr,"error : realpath error\n");
		return -1;
	}
	if(strlen(filename) >= 256) // 길이 검사
	{
		fprintf(stderr,"error : the length of filename must be not over 255bytes.\n");
		return -1;
	}

	if((node = list_search(filename)) == NULL)
	{
		fprintf(stderr,"error : %s is not in a backup_list\n", filename);
		return -1;
	}

	if(pthread_cancel(node->t_id) != 0)
	{
		fprintf(stderr,"pthread_cancel error\n");
		return -1;
	}

	if(!list_delete(node->filename))
	{
		fprintf(stderr,"error : delete error\n");
		return -1;
	}
	mutex_log(node->filename,"deleted");
	return 0;
}

int ssu_backup_compare(char * str) // compare <FILENAME1> <FILENAME2>
{
	int i,j;
	int index,len;
	char tempname1[FILE_NAME];
	char tempname2[FILE_NAME];
	char filename1[FILE_NAME];
	char filename2[FILE_NAME];
	struct stat statbuf1,statbuf2;

	memset((char *)tempname1,0,FILE_NAME);
	memset((char *)tempname2,0,FILE_NAME);
	memset((char *)filename1,0,FILE_NAME);
	memset((char *)filename2,0,FILE_NAME);

	len = strlen(str);
	index = get_index(str);
	
	for(;index < len && str[index] == ' ';index++);
	i = index;

	j = 0;
	for(i = index; i < len && str[i] != ' '; i++)
	{
		tempname1[j] = str[i];
		j++;
	}
	index = i;
	
	for(;index < len && str[index] == ' ';index++);
	i = index;

	j = 0;
	for(; i < len && str[i] != ' '; i++)
	{
		tempname2[j] = str[i];
		j++;
	}

	if(i < len && str[i] != ' ')
	{
		fprintf(stderr,"error : invalid input!\n");
		return -1;
	}

	if(access(tempname1,F_OK) != 0)
	{
		fprintf(stderr,"error : %s does not exist.\n",tempname1);
		return -1;
	}

	if(access(tempname2,F_OK) != 0)
	{
		fprintf(stderr,"error : %s does not exist.\n",tempname2);
		return -1;
	}

	if(realpath(tempname1,filename1) == NULL) 
	{
		fprintf(stderr,"realpath() error\n");
		return -1;
	}

	if(realpath(tempname2,filename2) == NULL)
	{
		fprintf(stderr,"realpath() error\n");
		return -1;
	}

	if(lstat(filename1,&statbuf1) < 0)
	{
		fprintf(stderr,"lstat() error\n");
		return -1;
	}

	if(lstat(filename2,&statbuf2) < 0)
	{
		fprintf(stderr,"lstat() error\n");
		return -1;
	}

	if(statbuf1.st_mtime == statbuf2.st_mtime && statbuf1.st_size == statbuf2.st_size)
	{
		printf("result : same file\n");
		return 0;
	}
	else
	{
		printf("result : different file\n");
		printf("%s's mtime : %ld, size : %ldbytes\n", filename1, statbuf1.st_mtime, statbuf1.st_size);
		printf("%s's mtime : %ld, size : %ldbytes\n", filename2, statbuf2.st_mtime, statbuf2.st_size);
		return 0;
	}
}

int ssu_backup_recover(char * str) // recover <FILENAME> [OPTION]
{
	int i,j;
	int index,len;
	char tempname[FILE_NAME];
	char filename[FILE_NAME];
	char newname[FILE_NAME];
	char option[TMP_SIZE];
	bool n = FALSE;
	Node * node;

	memset((char *)tempname,0,FILE_NAME);
	memset((char *)filename,0,FILE_NAME);
	memset((char *)newname,0,FILE_NAME);
	memset((char *)option,0,TMP_SIZE);

	len = strlen(str);
	index = get_index(str);

	for(; index < len && str[index] == ' '; index++);
	i = index;

	j = 0;
	for(; i < len && str[i] != ' '; i++)
	{
		tempname[j] = str[i];
		j++;
	}

	index = i;
	for(; index < len && str[index] == ' '; index++);
	i = index;

	j = 0;

	if(i < len && str[i] != ' ') // -n option?
	{
		for(; i < len && str[i] != ' '; i++)
		{
			option[j] = str[i];
			j++;
		}
		if(!strcmp(option,"-n"))
		{
			n = TRUE;

			for(; i < len && str[i] == ' '; i++);

			if(i >= len)
			{
				fprintf(stderr,"error : invalid input!\n");
				return -1;
			}

			j = 0;
			for(; i < len && str[i] != ' '; i++)
			{
				newname[j] = str[i];
				j++;
			}
			if(access(newname,F_OK) == 0)
			{
				fprintf(stderr,"error : %s already exists\n", newname);
				return -1;
			}
		}
		else
		{
			fprintf(stderr,"error : invalid input!\n");
			return -1;
		}
	}

	if(realpath(tempname,filename) == NULL) // 절대경로 가져오기
	{
		fprintf(stderr,"realpath() error\n");
		return -1;
	}

	if(access(filename,F_OK) != 0) // 파일이 존재하는가?
	{
		fprintf(stderr,"error : %s does not exist.\n",filename);
		return -1;
	}

	if((node = list_search(filename)) == NULL) // 리스트에 파일이 없음
	{
		fprintf(stderr,"error : %s does not exist in backup_list.\n", filename);
		return -1;
	}
	else
	{
		int input;
		List * select;
		int count = backup_count(node);
		char command[BUF_SIZE*2];
		Select * array = (Select *)malloc(sizeof(Select)*count);

		memset((Select *)array,0,sizeof(array));
		memset((char *)command,0,BUF_SIZE*2);
		memset((char *)tempname,0,FILE_NAME);

		if(pthread_cancel(node->t_id) != 0) // 백업 중단
		{
			fprintf(stderr,"pthread_cancel() error\n");
			return -1;
		}

		backup_to_select(node,array,count);	

		if(!backup_print(node))
		{
			fprintf(stderr,"backup list is empty\n");
			list_delete(node->filename);	
			return 0;
		}
		printf("%d : not select any file.\n", count+1);
		printf("input : ");
		scanf("%d", &input);
		getchar(); // 개행 잡기
		if(input == count+1)
		{
			mutex_log(node->filename,"deleted");
			list_delete(node->filename);
			return 0;
		}

		if((select = backup_search(node,array[input-1].filename)) == NULL)
		{
			fprintf(stderr,"error : searching failed.\n");
			return -1;
		}

		strcpy(tempname,select->filename);

		if(n == TRUE)
		{
			sprintf(command,"cp %s %s", tempname, newname);
			system(command);
			mutex_log(newname,"recoverd");
			memset((char *)command,0,BUF_SIZE*2);
			sprintf(command,"cat %s", newname);
			system(command);
		}
		else // FALSE
		{
			sprintf(command,"cp %s %s", tempname, filename);
			system(command);
			mutex_log(filename,"recoverd");
			memset((char *)command,0,BUF_SIZE*2);
			sprintf(command,"cat %s", filename);
			system(command);
		}

		list_delete(node->filename);
	}
}

int ssu_backup_list(char * str) // list
{
	list_print();
	return 0;
}

int ssu_backup_ls(char * str) // ls
{
	system(str);
	return 0;
}

int ssu_backup_vi(char * str) // vi(m)
{
	int i,j,index,len;
	char filename[FILE_NAME];
	char command[BUF_SIZE];

	memset((char *)filename,0,FILE_NAME);
	memset((char *)command,0,BUF_SIZE);

	len = strlen(str);
	index = get_index(str);

	for(; index < len && str[index] == ' '; index++);
	i = index;

	j = 0;
	for(; i < len && str[i] != ' '; i++)
	{
		filename[j] = str[i];
		j++;
	}

	sprintf(command,"vi %s", filename);
	system(command);
	return 0;
}

int ssu_backup_exit(char * str) // exit
{
}

void * thread_handler(void * arg) // thread handler
{
	Node * node = (Node *)arg;
	struct stat statbuf;
	time_t timer;
	struct tm * t;
	char timestr[TMP_SIZE];
	char filename[BUF_SIZE]; // 백업 노드의 생성된 파일 리스트에 들어갈 변수
	pthread_t t_id = pthread_self();

	if(node -> t_id != t_id)
	{
		fprintf(stderr,"error : thread id is different!\n");
		node -> t_id = t_id;
	}

	while(1)
	{
		sleep(node->period); // 주기만큼 잠들기
		backup_time(node); // 주기만큼 더해주기	
		memset((char *)timestr,0,TMP_SIZE);
		memset((char *)filename,0,BUF_SIZE);
		timer = time(NULL);
		t = localtime(&timer);
		sprintf(timestr,"%02d%02d%02d%02d%02d%02d", t->tm_year-100, t->tm_mon + 1, t->tm_mday,
				t->tm_hour, t->tm_min, t->tm_sec);

		List * newList = (List *)malloc(sizeof(List));
		memset((char *)newList->filename,0,BUF_SIZE);
		memset((char *)newList->timestr,0,TMP_SIZE);
		strcpy(newList->timestr,timestr);

		if(lstat(node->filename,&statbuf) < 0)
		{
			fprintf(stderr,"lstat() error for %s\n", node->filename);
			break;
		}
		newList->sec = 0;
		newList->size = statbuf.st_size;

		if(node->option.m) // m otion
		{
			if(statbuf.st_mtime != node->mtime)
			{
				set_backup_file(node->filename,timestr,filename); // 백업 파일 만들기
				node->mtime = statbuf.st_mtime; // mtime 변경
				strcpy(newList->filename,filename);
				backup_insert(node,newList);
			}
		}
		else // without m option
		{
			set_backup_file(node->filename,timestr,filename); // 백업 파일 만들기
			strcpy(newList->filename,filename);
			backup_insert(node,newList);
		}

		if(node->option.n) // n option
		{
			if(backup_count(node) > node->number) // number option 개수보다 많다면
				if(!backup_delete(node))
					fprintf(stderr,"backup_delete error\n");
		}
	}
}
