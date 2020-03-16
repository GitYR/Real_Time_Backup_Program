#ifndef _SSU_BACKUP_FOR_PROJECT_3_
#define _SSU_BACKUP_FOR_PROJECT_3_
#define _GNU_SOURCE

#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

#define ADD 0
#define REMOVE 1
#define COMPARE 2
#define RECOVER 3
#define LIST 4
#define LS 5
#define VI 6
#define EXIT 7
#define INVALID -1

#define TMP_SIZE 32 // 임시로 배열 쓸 때 사용
#define BUF_SIZE 512 // buf 크기
#define FILE_NAME 256 // 백업해야할 파일 경로 길이 - 절대 경로
#define BACKUP_DIR 256 // 백업파일을 저장할 디렉토리 이름

#define IS_ASCII(ch) (0 <= ch && ch <= 127)
#define S_MODE 0644

/* FOR LINKED LIST */

typedef _Bool bool;

/* PROMPT COMMAND OPTION */

typedef struct _option
{
	bool m;
	bool n;
	bool t;
	bool d;
}Opt;

typedef struct _select
{
	char filename[FILE_NAME];
	int number;
}Select;

typedef struct _list // 백업된 파일들의 리스트
{
	char filename[BUF_SIZE]; // 백업된 파일 이름
	char timestr[TMP_SIZE]; // 백업 시간
	int size; // 크기
	int sec; // 시간
	struct _list * next;
}List;

typedef struct _node // add를 통해 추가되는 노드
{
	pthread_t t_id; // thread id
	char filename[FILE_NAME]; // 파일의 절대경로
	int mtime; // m option을 위한 변수
	int ttime; // 보관할 시간(초)
	int number; // 유지할 백업 파일 수
	int period; // 백업 주기(초)
	Opt option; // 백업 옵션
	List * head; // 백업 리스트
	struct _node * next; // 다음 노드
}Node;

Node * head; // 백업 파일 리스트의 시작 부분
char log_file[FILE_NAME]; // log 파일
char dir_backup[BACKUP_DIR]; // 백업할 디렉토리 경로
pthread_mutex_t mutex; // mutex

/* 백업 함수들이 사용하는 부속 함수 */ // ssu_support.c에 정의

int input_token(const char * str); // 명령어 판단 함수

int get_index(const char * str); // 명령어 제외한 나머지 문자열

bool is_integer(const char * str); // 정수 확인

bool is_ascii(const char * str); // ascii 값인지 확인

int get_option_argument(const char * str,char * option,int index,int * number); // 숫자 파싱

bool get_dirfile(const char * str,Node * newNode); // 디렉토리 재귀함수

void get_time_string(char * str);

void get_file_name(char * str,char * filename); // 파일의 이름 파싱

void mutex_log(char * filename,char * messsage); // mutex를 사용한 log 작성

void set_backup_file(char * str,char * gentime,char * backup); // 백업 파일 생성

/* 백업 함수 */ // ssu_backup.c 에 정의

void ssu_backup_start(void); // ssu_backup 시작

int ssu_backup_add(char * str); // add 정의

int ssu_backup_remove(char * str); // remove 정의

int ssu_backup_compare(char * str); // compare 정의

int ssu_backup_recover(char * str); // recover 정의

int ssu_backup_list(char * str); // list 정의

int ssu_backup_ls(char * str); // ls 정의

int ssu_backup_vi(char * str); // vi 정의

int ssu_backup_exit(char * str); // exit 정의

void * thread_handler(void * arg); // thread handler

/* 연결리스트 함수 */ // linkedlist.c 에 정의

void list_init(void); // 초기화

void list_insert(Node * newNode); // 리스트 추가

bool list_delete(char * filename); // 리시트 삭제

Node * list_search(char * filename); // 리스트 검색

bool list_compare(char * filename); // 리스트에 같은 백업 리스트가 있는지 검사

void list_print(void); // 리스트 출력

bool list_empty(void); // 리스트에 내용 존재 여부 검사

/* 백업 리스트 함수 */ // backuplist.c에 정의

void backup_insert(Node * node,List * newNode); 

bool backup_delete(Node * node); 

bool backup_empty(Node * node);

int backup_count(Node * node);

bool backup_print(Node * node);

void backup_time(Node * node);

void backup_delete_random(Node * node,List * list);

List * backup_search(Node * node,char * filename);

void backup_to_select(Node * node,Select * select,int count);

#endif
