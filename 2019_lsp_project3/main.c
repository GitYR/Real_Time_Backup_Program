#include "./ssu_backup.h"

int main(int argc,char * argv[])
{
	int fd;
	struct stat statbuf;

	memset((char *)log_file,0,FILE_NAME);

	if(argc > 2) 
	{
		fprintf(stderr,"usage : %s <DIRECTORY PATH>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	memset((char *)dir_backup,0,BACKUP_DIR);

	if(argc == 2)
	{
		if(access(argv[1],F_OK) != 0) // 인자가 있으면 지정된 디렉토리 경로
		{
			fprintf(stderr,"usage : %s <EXISTING DIRECTORY PATH>\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		if(access(argv[1],R_OK | X_OK | W_OK) != 0)
		{
			fprintf(stderr,"usage : %s <ACCESSABLE DIRECTORY PATH>\n", argv[0]);
			exit(EXIT_FAILURE);
		} // 디렉토리 접근 권한 판정
		
		if(realpath(argv[1],dir_backup) == NULL)
		{
			fprintf(stderr,"realpath() error\n");
			exit(EXIT_FAILURE);
		}

		if(lstat(dir_backup,&statbuf) < 0)
		{
			fprintf(stderr,"lstat() error for %s\n", dir_backup);
			exit(EXIT_FAILURE);
		}

		if(!S_ISDIR(statbuf.st_mode)) // 디렉토리 X
		{
			fprintf(stderr,"usage : %s <DIRECTORY_PATH>\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		sprintf(dir_backup,"%s/backup", dir_backup);
		mkdir(dir_backup,0744);
	}

	if(argc == 1) // 인자가 없으면 현재 디렉토리경로에 백업 디렉토리 생성
	{
		if(getcwd(dir_backup,BACKUP_DIR) == NULL) // 현재 경로 가져오기 
		{
			fprintf(stderr,"fail to get a directory path.\n");
			exit(EXIT_FAILURE);
		}
		sprintf(dir_backup,"%s/backup", dir_backup);
		mkdir(dir_backup,0744); // rwxrw-rw-
	}

	sprintf(log_file,"%s/log_file.txt", dir_backup);
	if((fd = open(log_file,O_RDWR | O_CREAT | O_TRUNC, S_MODE)) < 0)
	{
		fprintf(stderr,"open error for %s\n", log_file);
		exit(EXIT_FAILURE);
	}
	close(fd);
	ssu_backup_start(); // ssu_backup 실행
	exit(EXIT_SUCCESS);
}
