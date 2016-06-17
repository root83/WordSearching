#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "structure.h"
#include "liblog.h"


#define CLOCKS_PER_SEC ((clock_t)1000000)

#define MEM_SIZE    1048576
#define DEFAULT_KEY_NUM		149;
int KEY_NUM[13];
char input_index[13][2] = {"aq", "bn", "cz", "dv", "eg", "fo", "hl", "iw", "jm", "kt", "py", "ru", "sx"};

int WORD_COUNT=1000;

typedef struct input_key{
	char *key;
	int	 type;
	int	 row_num;
	char *input_file;
	int	 shm_num;
}INPUT_KEY;
typedef struct result_info{
	char *key;
	int key_cnt;
	int *doc;
	int	doc_cnt;
	int *term;
	int *use;
	int and_doc_cnt;
}RESULT_INFO;
union semun{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
};

RESULT_INFO result_info;
int result_info_cnt=0;
RESULT_INFO and_result_info;
int and_result_info_cnt=0;
char *input_string;
clock_t startTime, endTime;


int _is_space_work(unsigned char ch)
{
	if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
		return 1;
	return 0;
}


int str_LRTrim_work(char *pszString)
{
	int len = 0;
	if( pszString && *pszString )
	{
		register char * pszStart = pszString;
		register char * pszEnd = pszString;
		register char *pszFirstNonSpace = NULL;
		register short int	 bAllSpace = 1;

		while( *pszEnd )
		{
			if( bAllSpace && !_is_space_work( *pszEnd ) )
			{
				pszFirstNonSpace = pszEnd;
				bAllSpace = 0;
			}
			pszEnd++;
		}

		if( bAllSpace )
		{
			*pszString = '\0';
			return 0;
		}
		else
		{
			pszEnd--;

			while( ( _is_space_work( *pszEnd ) ) && pszEnd >= pszString )
			{
				*pszEnd-- = '\0';
			}
			if(pszFirstNonSpace)
			{
				while( (*pszStart++ = *pszFirstNonSpace++) != NULL );
				len = pszStart - pszString - 1;
			}
			else
			{
				len = pszEnd - pszString + 1;
			}
		}
	}
	return len;
}


int ascii_check(char *input)
{
    int i, j, cnt=0;
    for(i = 0; i < strlen(input); i++)
    {
		if((int)input[i]==94)
			input[i] = 34;
    }
    str_LRTrim_work(input);
    return cnt;
}

int main(int argc, char **argv)
{
	int pid, i, j;

	input_string=(char*)malloc(sizeof(char)*strlen(argv[1]));
	sprintf(input_string,argv[1]);

    pid = 0;
    if((pid = fork()) < 0) {
        exit(0);
    }
    else if(pid != 0) {
        exit(0);
    }
    setsid();
	
	/*
	if ( -1 == ( shm_id = shmget( (key_t)DEFAULT_KEY_NUM, MEM_SIZE, IPC_CREAT|0666)))
   {
	  printf( "공유 메모리 생성 실패 %d/%d\n", i, DEFAULT_KEY_NUM);
	  return -1;
   }

   if ( ( void *)-1 == ( shm_addr = shmat( shm_id, ( void *)0, 0)))
   {
	  printf( "공유 메모리 첨부 실패\n");
	  return -1;
   }
	KEY_NUM = atoi((char *)shm_addr);
	if(KEY_NUM >=(150+(13*12))){
		KEY_NUM=150;
	}
	sprintf( (char *)shm_addr, "%d", KEY_NUM+13);
	shmdt(shm_addr);
	*/
	search_keyword(input_string);


	return 0;
}

int parseOrResult(char *input){
	char *ptr, *ptr2, *tempBuf, tmpBuf[1024];
	int i,j,k,doc_cnt=0,key_cnt=0, kc;

	ptr=strtok(input,"\n");
	while(ptr!=NULL){
		if(strstr(ptr,"/OR")!=NULL){
			tempBuf=(char*)malloc(sizeof(char) * strlen(ptr));
			sprintf(tempBuf, ptr);
			ptr2=strtok(tempBuf,"\t");
			i=0;
			while((ptr2=strtok(NULL,"\t"))!=NULL){
				doc_cnt++;
				if(result_info.key_cnt==0){
					result_info.doc[result_info.doc_cnt] = atoi(ptr2); 
					result_info.use[result_info.doc_cnt] = 1;
					result_info.doc_cnt++;
				}else{
					if(result_info.and_doc_cnt==0){
						k=0;
						for(j=0;j<result_info.doc_cnt;j++){
							if(result_info.doc[j] == atoi(ptr2)){
								result_info.term[j]++;
								k=1;
								break;
							}
						}
						if(k==0){	
							result_info.doc[result_info.doc_cnt] = atoi(ptr2); 
							result_info.use[result_info.doc_cnt] = 1;
							result_info.doc_cnt++;
						}
					}
				}
				i++;
			}
			result_info.key_cnt++;
			
		}
		ptr=strtok(NULL,"\n");
	}
	return doc_cnt;
}
int parseAndResult(char *input){
	char *ptr, *ptr2, *tempBuf, tmpBuf[1024], **outBuf;
	int i,j,k;
	int *doc;
	int doc_cnt, doc_cnt2=0;
	int key_cnt=0;
	doc_cnt=0;
	doc = (int *)malloc(sizeof(int)*(doc_cnt+1));
	ptr=strtok(input,"\n");
	
	while(ptr!=NULL){
		ptr = strtok(NULL,"\n");
		if(strstr(ptr,"/AND")!=NULL){			
			doc_cnt=0;
			tempBuf=(char*)malloc(sizeof(char) * strlen(ptr));
			sprintf(tempBuf, ptr);
			ptr2=strtok(tempBuf,"\t");
			while((ptr2=strtok(NULL,"\t"))!=NULL){
				doc_cnt2++;
				k=0;
				if(and_result_info.doc_cnt==0){							// 처음 검색된 Key일 경우 색인 정보 바로 추가
					result_info.doc[result_info.doc_cnt] = atoi(ptr2); 
					result_info.use[result_info.doc_cnt] = 1;
					result_info.doc_cnt++;
				}

				k=0;
				if(result_info.key_cnt>0){								// AND 조건 연산을 위한 색인 리스트
					for(j=0;j<doc_cnt;j++){
						if(atoi(ptr2) == doc[j]){
							k=1;
							break;
						}
					}
				}
				if(k==0){
					doc[doc_cnt] = atoi(ptr2);
					doc_cnt++;
					doc = (int *)realloc(doc,sizeof(int)*(doc_cnt+1));
				}
			}
			result_info.key_cnt++;
			result_info.and_doc_cnt++;
			if(result_info.key_cnt>1){									// 검색된 색인 리스트와 비교하여 AND 조건에 만족되지 않을 경우 결과에서 누락
        		for(j=0;j<result_info.doc_cnt;j++){
					if(result_info.use[j]==1){
						k=0;
						for(i=0;i<doc_cnt;i++){
							if(doc[i] == result_info.doc[j]){
								k=1;
								break;
							}
						}
						if(k==0){
							result_info.use[j]=0;
						}
					}
        		}
			}
		}
	}
	free(doc);
	return doc_cnt2;
	
}

int search_keyword(char *input_string)
{
	char tempBuf[10000],tempBuf2[10000], inputBuf[10000];
	char input_cmd[13][10000];
	char **output_cmd;
	int shm_id[13], input_shm[13];
	int key_shm_id, key_cnt=0;
	int result_cnt, result_total_cnt;

	INPUT_KEY *input_key_list;
	int i, j, k, l;
	struct timeb ta;
	struct timeb tb;
	struct tm t;
	void *key_shm_addr;
	void **shm_addr;
	
	union semun sem_union;
	int semid;
	int sem_num = 1;
	struct sembuf sem_open = {0,-1,SEM_UNDO};
	struct sembuf sem_close = {0,1,SEM_UNDO};


	shm_addr=(void *)malloc(sizeof(void *) * 13);

	i=9;
	j=0;
	k=0;
	input_key_list = (INPUT_KEY*)malloc(sizeof(INPUT_KEY) * (key_cnt+1));
	output_cmd = (char **)malloc(sizeof(char*) * 13);
	result_info.key_cnt=0;
	result_info.doc_cnt=0;
	result_info.doc = (int*)malloc( sizeof(int)* (WORD_COUNT)) ;
	result_info.term = (int*)malloc(sizeof(int)* (WORD_COUNT)) ;
	result_info.use = (int*)malloc(sizeof(int)* (WORD_COUNT)) ;
	and_result_info.doc_cnt=0;

	sprintf(inputBuf, input_string);
	ascii_check(inputBuf);
	str_LRTrim_work(inputBuf);
	sprintf(tempBuf,"/wndi/temp/result/%s.log",inputBuf);
	LogStart(tempBuf);
	
	for(i=0;i<13;i++){
		memset(input_cmd[i],0x00,10000);
		input_shm[i]=0;
	}
	ptr=strtok(inputBuf," ");
	/* 검색어 형태소 추출 */
	while(ptr!=NULL){
		if(strchr(ptr,'^')!=NULL)
			input_key_list[key_cnt].type=1;
		else
			input_key_list[key_cnt].type=0;
		ascii_check(ptr);
		str_LRTrim_work(ptr);
		if(strlen(ptr)>0){
			input_key_list[key_cnt].key = (char*)malloc(sizeof(char)*strlen(ptr));
			sprintf(input_key_list[key_cnt].key,ptr);
			k=0;
			for(i=0;i<13;i++){
				if(input_key_list[key_cnt].key[0] == input_index[i][0] || input_key_list[key_cnt].key[0] == input_index[i][1] ){
					k=1;
					break;
				}
			}

			if(k==1){
				input_shm[i]=1;
				if(input_key_list[key_cnt].type==0)
					sprintf(tempBuf,"%s ",input_key_list[key_cnt].key);
				else
					sprintf(tempBuf,"\"%s\" ",input_key_list[key_cnt].key);
				strcat(input_cmd[i], tempBuf);
			}
			key_cnt++;
			input_key_list = (INPUT_KEY*)realloc(input_key_list,sizeof(INPUT_KEY) * (key_cnt+1));
		}
		ptr=strtok(NULL," ");
	}
	
	/* 공유메모리 KEY를 받아오기 위한 세마포어 Lock */
	if((semid = semget((key_t)DEFAULT_KEY_NUM, sem_num, IPC_CREAT|0666))! == -1){
		perror("semget error");
		exit(0);
	}
	sem_union.val=1;
	if (semctl(semid, 0, SETVAL, sem_union)==-1){
		perror("semctl error");
		exit(0);
	}
	if(semop(semid,&sem_open,1) == -1){
		perror("semop error");
		exit(0);
	}
	
	/* 13개 공유메모리 Key set을 받아옴 */
	if (( key_shm_id = shmget( (key_t)DEFAULT_KEY_NUM, MEM_SIZE, IPC_CREAT|0666))  == -1)
	{
		printf( "공유 메모리 생성 실패 %d/%d\n", i, DEFAULT_KEY_NUM);
		return -1;
	}

	if ( key_shm_addr = shmat( key_shm_id, ( void *)0, 0)) ==  ( void *)-1 )
	{
		printf( "공유 메모리 첨부 실패\n");
		return -1;
	}
	ptr=strtok((char *)key_shm_addr, "\t");
	i=0;
	while(ptr!=NULL, i<13){
		KEY_NUM[i] = atoi(ptr);
		i++;
		ptr=strtok(NULL,"\t");
	}
	
	/* 공유메모리에 KEY값으로 검색어 전달 */
	for(i=0;i<13;i++){
		if(input_shm[i]>0){

			if (( shm_id[i] = shmget( (key_t)(KEY_NUM+i), MEM_SIZE, IPC_CREAT|0666)) == -1)
		   {
			  printf( "공유 메모리 생성 실패 %d/%d\n", i, KEY_NUM+i);
			  return -1;
		   }

		   if (( shm_addr[i] = shmat( shm_id[i], ( void *)0, 0)) == ( void *)-1)
		   {
			  printf( "공유 메모리 첨부 실패\n");
			  return -1;
		   }
			while(strstr((char *)shm_addr[i],"[ready]")==NULL){
				sleep(1);
			}
			sprintf( (char *)shm_addr[i], "%s %d [input]", input_cmd[i], process_cnt[i]);
			l++;
		}
	}
	startTime = clock();	
	
	/* Key 리스트 update*/
	
	sprintf(tempBuf,"");
	sprintf(tempBuf2,"");
	for(i=0;i<13;i++){
		if(input_shm[i]==2){
			if((KEY_NUM[i]+=13)>(150+(13*12))){
				KEY_NUM[i]=150+i;
			}
		}
		sprintf(tempBuf2, "%d\t", KEY_NUM[i]);
		strcat(tempBuf,tempBuf2);
	}
	sprintf((char *)key_shm_addr, tempBuf);
	shmdt(key_shm_addr);
	
	/*세마포어 Unlock*/
	semop(semid,&sem_close,1);
	
	
	
	/*검색결과 받아오기 */

	while(l==1){
		l=0;
		for(i=0;i<13;i++){
			if(input_shm[i]==1){
				if(strlen((char *)shm_addr[i])>0 && strstr((char *)shm_addr[i],"[output]")!=NULL){
						output_cmd[i]=(char *)malloc(sizeof(char)*strlen((char *)shm_addr[i]));
						sprintf(output_cmd[i], (char *)shm_addr[i]);
						sprintf((char *)shm_addr[i],"[ok]");
						shmdt(shm_addr[i]);
						input_shm[i]=2;
				}
				else{
					l=1;
				}
			}
		}
	}
	

	endTime =clock();
	logprint("%s search time : %.3lf\n", inputBuf, (double)(endTime - startTime)/CLOCKS_PER_SEC);
	
	/*검색 결과 연산 */
	result_cnt=0;
	result_total_cnt=0;
	for(i=0;i<13;i++){
		if(input_shm[i]==2){
			outBuf=(char*)malloc(sizeof(char)*strlen(output_cmd[i]));
			sprintf(outBuf,output_cmd[i]);
			result_total_cnt+=parseAndResult(outBuf);
		}
	}
	for(i=0;i<13;i++){
		if(input_shm[i]==2){
			outBuf=(char*)malloc(sizeof(char)*strlen(output_cmd[i]));
			sprintf(outBuf,output_cmd[i]);
			result_total_cnt+=parseOrResult(outBuf);
		}
	}
	sprintf(tempBuf,"");
	for(i=0;i<result_info.doc_cnt;i++){
		if(result_info.use[i]==1){
			result_cnt++;
		}
	}	
	endTime = clock();
	
	/* 결과 출력 */
	logprint("%s total time : %.3lf / %d/%d\n", inputBuf, (double)(endTime - startTime)/CLOCKS_PER_SEC, result_cnt, result_total_cnt);
	result_info.doc_cnt=0;
	and_result_info.doc_cnt=0;

	return 0;
}

