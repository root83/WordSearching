/**********************************************************************************************
2016.03.23 개발 완료
source 파일 정보
  structure.h : B-tree, 색인, Key 정보 등에 대한 구조 정의
  btree.h btree.c : btree 처리
  wordsearch.h wordsearch.c : 문서 색인 구축 및 검색 기능
***********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "structure.h"
#include "wordsearch.h"
#include "btree.h"

Bnode *head;

/**************
초기화
**************/
void init()
	{
	keyCount  =  0;
	head  =  (Bnode*)malloc(sizeof(Bnode));
	head->ptr[0]  =  NULL;
	head->n  =  0;
}

/********************
알파벳, 숫자를 제외한
특수문자 제거
********************/

int _ascii_check(char *input)
{
	int i, j, cnt=0;
	for(i  =  0; i < strlen(input); i++)
	{
		if((int)input[i] < 48 || (int)input[i] > 122 || ((int)input[i] < 65 && (int)input[i] >57) || ((int)input[i] < 97 && (int)input[i] >90))
		{
			for(j  =  i; j < strlen(input)-1; j++)
			{
				input[j]  =  input[j+1];
			}
			
			input[strlen(input)-1]  =  '\0';
			i--;
			cnt++;
		}
	}
	return cnt;
}

/******************************************
입력된 문자열에서 Keyword 추출

입력값
inputString : 입력 문자열
docNum      : 문서 번호
base				: B-tree 삽입 전 Key 정보 리스트
*******************************************/

void _get_keylist(char *inputString, int docNum, List *base)
{
	char *token;
	int i, j, keyCheck, docCheck;
	Keyvalue *k,*key;
	Docidx *d, *doc;
	token  =  strtok(inputString, " ");
	while(token !=  NULL)
	{
		_ascii_check(token);
		k = base->head;
		keyCheck  =  0; 
		

		while(k!= NULL)	
		{
			if(!strcmp(k->key, token))
			{
				if(k->count == 0)
				{
					doc = (Docidx*)malloc(sizeof(Docidx));
					doc->documentNum  =  docNum;
					doc->next = NULL;
					k->DocuVector = doc;
				}
				else
				{
					d  =  k->DocuVector;
					docCheck  =  0;

					while(d!= NULL)
					{
						if(d->documentNum  ==  docNum)
						{
							docCheck  =  1;
							break;
						}
						d  =  d->next;
					}

					if(docCheck  ==  0)
					{
						d  =  k->DocuVector;
						for(j  =  0;d->next!= NULL; j++)
							d  =  d->next;
						doc = (Docidx*)malloc(sizeof(Docidx));
						doc->documentNum  =  docNum;
						doc->next = NULL;
						d->next  =  doc;
					}
				}
				k->count++;
				keyCheck  =  1;
				break;
			}
			k = k->next;
		}
		
		if(keyCheck  ==  0)
		{
			key  =  (Keyvalue *)malloc(sizeof(Keyvalue));
			key->key = (char*)malloc(sizeof(char)*strlen(token));
			sprintf(key->key, token);
			key->count  =  1;
			key->next = NULL;

			doc = (Docidx*)malloc(sizeof(Docidx));
			doc->documentNum  =  docNum;
			doc->next = NULL;

			key->DocuVector  =  doc;

			if(keyCount == 0 )
			{
				base->head = key;
			}

			else
			{
				k = base->head;
				for(i = 0; k->next!= NULL; i++)
					k = k->next;
				k->next = key;
			}			
			keyCount++;
		}
		token  =  strtok(NULL," ");
	}
}

/***************************
입력된 파일에서 key값을 추출
파일명이 문서색인 번호
***************************/

void get_words(char *inputFile)
{
	FILE *ifp;
	char inputBuffer[8192], *ptr;
	int	i,j,docNum = atoi(inputFile);
	Keyvalue *k, *tempK;
	Docidx *d;
	listInfo  =  (List*)malloc(sizeof(List));	
	listInfo->head = NULL;	
	if((ifp  =  fopen(inputFile,"r"))  == NULL)
	{
		printf("입력파일 읽기 실패\n");
		return;
	}
	if((ptr = strtok(inputFile,"."))!=NULL)
		docNum = atoi(ptr);
	while(fgets(inputBuffer, sizeof(inputBuffer), ifp) !=  NULL)
	{
		inputBuffer[strlen(inputBuffer)-1] = '\0';
		
		_get_keylist(inputBuffer, docNum, listInfo);
	}
	fclose(ifp);
	
	
	k = listInfo->head;
	while(k!= NULL)
	{
		if (insert_btree(k,head)  ==  NULL)   /* 추출된 key값 및 색인 정보를 B-tree에 삽입 */
			;
		k = k->next;
	}
	
	k = listInfo->head;
	while(k!= NULL)     /* 리스트 Free */
	{
		tempK  =  k;
		k  =  k->next;
		free(tempK);
	}	
	listInfo->head  =  NULL;
	free(listInfo);
	keyCount = 0;
	return ;
}

int _input_check(char *input)
{
	int i;
	for(i=0; i<strlen(input); i++){
 		if(!((int)input[i] < 48 || (int)input[i] > 122 || ((int)input[i] < 65 && (int)input[i] >57) || ((int)input[i] < 97 && (int)input[i] >90)))
			return 1;
	}
	return -1;
}
/*****************************************
입력된 검색어 문자열에서 검색 Key 추출
쌍따옴표가 붙어있는 문자열은 AND 검색,
아니면 OR 검색 Keyword로 구분

입력값
input  : 입력된 문자열
andKey : AND 검색 keyword 리스트 head
orKey  : OR 검색 keyword 리스트 head
*****************************************/
int input_command(char *input, List *andKey, List *orKey)
{
	char *token, buf[1024];
	int	totalAndKey = 0, totalOrKey = 0;
	Keyvalue *ak, *akN, *ok, *okN;
	token  =  strtok(input, " ");
	while(token != NULL)
	{
		sprintf(buf, token);
		if(_input_check(buf)<0)
			return -1;
		/*************************
		'"' 기호가 붙어 있을 경우
		AND 조건 keyword로 분류
		*************************/
		if(strchr(token,'\"')!= NULL)
		{												/* 특수문자 제외 */
			_ascii_check(token);	
			akN = (Keyvalue*)malloc(sizeof(Keyvalue));
			akN->key = (char*)malloc(sizeof(char)*strlen(token));
			sprintf(akN->key, token);
			akN->next = NULL;
			if(totalAndKey == 0)													/* 먼저 입력된 AND 검색 Key가 없을 경우 */
			{
				andKey->head  =  akN;
			}
			else
			{
				ak  =  andKey->head;
				while(ak->next!= NULL)
					ak  =  ak->next;
				ak->next  =  akN;
			}
			totalAndKey++;
		}
		
		/*******************
		OR 조건 keyword 분류
		*******************/
		
		else
		{
			_ascii_check(token);
			okN = (Keyvalue*)malloc(sizeof(Keyvalue));
			okN->key = (char*)malloc(sizeof(char)*strlen(token));
			sprintf(okN->key, token);
			okN->next = NULL;
			if(totalOrKey == 0)														/* 먼저 입력된 OR 검색 Key가 없을 경우 */
			{
				orKey->head  =  okN;
			}
			else
			{
				ok  =  orKey->head;
				while(ok->next!= NULL)
					ok  =  ok->next;
				ok->next  =  okN;
			}
			totalOrKey++;
		}
		token  =  strtok(NULL, " ");
	}
	
	return totalAndKey + totalOrKey;
}

/*********************************
가장 많은 검색어가 포함된 기준으로
검색 결과를 정렬

입력값
base : 검색결과 리스트 head
*********************************/

Doclist *_sort_list(Doclist *base)
{
	Doclist *returnList;
	returnList  =  (Doclist*)malloc(sizeof(Doclist));
	returnList->head  =  NULL;
	Docidx *a, *b;
	Docidx *doc, *tmp, *tmp2;


	returnList->head  =  NULL;
	a  =  base->head;

	while(a !=  NULL)
	{
		doc  =  (Docidx*)malloc(sizeof(Docidx));
		doc->documentNum  =  a->documentNum;
		doc->termFreq  =  a->termFreq;
		doc->next  =  NULL;
		if(returnList->head == NULL)
		{
			returnList->head  =  doc;
		}
		else
		{
			tmp  =  returnList->head;
			if(tmp->termFreq < doc->termFreq)
			{
				doc->next  = tmp;
				returnList->head  =  doc;
			}
			else
			{
				while(tmp!= NULL)
					{
					if(tmp->termFreq >=  doc->termFreq)
						{
						if(tmp->next == NULL)
						{
							tmp->next  =  doc;
							break;
						}
					}
					else
					{
						doc->next  =  tmp;
						tmp2->next  =  doc;
						break;
					}
					tmp2  =  tmp;
					tmp  =  tmp->next;
				}
			}
		}
		a  =  a->next;
	}
	return returnList;

}
	
/******************************************
검색어를 입력받고 B-tree에서 key를 검색하여
결과를 출력
******************************************/
void search_prompt(Bnode *base)
{
	char keyword[1024];
	int i, docCheck, documentListCnt = 0, andCheck, ret;
	int *documentNumList;

	Bnodekey *keyR;	// keyword 검색 결과 저장
	Docidx *d, *doc, *resultDoc, *tempD; 
	Keyvalue *k, *tempK;
	List *inputKeyAndList, *inputKeyOrList;
	Doclist *resultList;

	inputKeyAndList  =  (List*)malloc(sizeof(List));	
	inputKeyOrList  =  (List*)malloc(sizeof(List));	
	
	resultList  =  (Doclist*)malloc(sizeof(Doclist));
	
	while (1)
	{
		printf("\n검색할 키워드를 입력하세요 (종료시 q!)->");
		
		gets(keyword);
		if(strstr(keyword,"q!")!= NULL)
			break;
		inputKeyAndList->head = NULL;	
		inputKeyOrList->head = NULL;
		resultList->head  =  NULL;
		documentListCnt = 0;
		documentNumList  =  (int*)malloc(sizeof(int)*(documentListCnt+1));
		andCheck = 0;
		if((ret = input_command(keyword, inputKeyAndList, inputKeyOrList))>0) 					/* 입력된 문장을 AND, OR 검색 조건에 맞춰 리스트 저장 */
		{
			k  =  inputKeyOrList->head;
			printf("OR key : ");
			while(k != NULL)
			{
				printf("%s ", k->key);
				k  =  k->next;
			}
			printf("\n");
			k  =  inputKeyAndList->head;
			printf("AND key : ");
			while(k != NULL)
			{
				printf("%s ", k->key);
				k  =  k->next;
			}
			printf("\n");
			
			/************************ 
			OR 조건 검색 keyword 처리
			************************/
			
			k  =  inputKeyOrList->head; 
			while(k!= NULL)
			{
				if((keyR = serch_btree(k->key,base))!= NULL) /* B-tree에서 Keyword 검색 */
				{
					d  =  keyR->DocuVector;
					while(d !=  NULL)
					{
						docCheck = 0;
						resultDoc  =  resultList->head;
						while(resultDoc!= NULL)
						{
							if(resultDoc->documentNum  ==  d->documentNum)
							{ /* 기존에 검색된 index 확인 */
								docCheck = 1;
								resultDoc->termFreq++;
								break;
							}
							resultDoc  =  resultDoc->next;
						}
						if(docCheck  ==  0)
						{                     /* 새로 검색된 index 추가 */
							doc  =  (Docidx*)malloc(sizeof(Docidx));
							doc->documentNum  =  d->documentNum;
							doc->termFreq  =  1;
							doc->next  =  NULL;
							if(resultList->head  ==  NULL)
							{
								resultList->head  =  doc;
							}
							else
							{
								resultDoc  =  resultList->head;
								while(resultDoc->next!= NULL)
									resultDoc  =  resultDoc->next;
								resultDoc->next  =  doc;
							}
						}
						d  =  d->next;
					}
				}
				k  =  k->next;
			}
			
			/************************* 
			AND 조건 검색 keyword 처리
			*************************/
			
			k  =  inputKeyAndList->head; 
			while(k!= NULL)
			{
				if((keyR = serch_btree(k->key,base))!= NULL) /* B-tree에서 Keyword 검색 */
				{
					resultDoc  =  resultList->head;
					if(resultDoc  ==  NULL)
					{																		/* 기존 Index가 존재하지 않을 경우 */
						d  =  keyR->DocuVector;
						if(documentListCnt>0)								/* AND 조건에 의해 검색 결과가 제외되어 Index가 존재하지 않는 경우 처리하지 않음*/
							;
						else																/* 기존에 검색된 결과가 없는 경우 */
						{
							while(d !=  NULL)
							{
								docCheck = 0;
								resultDoc  =  resultList->head;
								while(resultDoc!= NULL)
								{
									if(resultDoc->documentNum  ==  d->documentNum)                /* 기존 검색 결과에 해당 문서 색인이 존재할 경우 빈도 증가 */
									{
										docCheck = 1;
										resultDoc->termFreq++;
										break;
									}
									resultDoc  =  resultDoc->next;
								}
								if(docCheck  ==  0)																						/* 기존 검색결과에 해당 문서 색인이 존재하지 않을 경우 색인을 검색결과에 추가 */
								{
									doc  =  (Docidx*)malloc(sizeof(Docidx));
									doc->documentNum  =  d->documentNum;
									doc->termFreq  =  1;
									doc->next  =  NULL;
									resultDoc  =  resultList->head;
									if(resultDoc  ==  NULL)
										resultList->head  =  doc;
									else
									{
										while(resultDoc->next!= NULL)
											resultDoc  =  resultDoc->next;
										resultDoc->next  =  doc;
									}
								}
/*								for(i = 0; i < documentListCnt; i++)
								{
									if(documentNumList[i]  ==  d->documentNum)
									{
										docCheck = 1;
									}
								}
								if(docCheck == 0)
								{
									documentNumList[documentListCnt]  =  d->documentNum;
									documentListCnt++;
									documentNumList = (int*)realloc(documentNumList, sizeof(int)*(documentListCnt+1));
								}*/
								d  =  d->next;
							}
						}
					}
					else {																												/* OR 조건 검색 결과가 존재할 경우 */
						d  =  keyR->DocuVector;
						while(d !=  NULL)
						{
							docCheck = 0;
							for(i = 0; i < documentListCnt; i++)
							{
								if(documentNumList[i]  ==  d->documentNum)							/* documentNumList : AND 조건에 의해 검색된 결과들의 리스트 */
								{
									docCheck = 1;
								}
							}
							if(docCheck == 0)																					/* 기존 검색된 AND 조건 결과에 해당 문서색인이 없을 때 리스트에 등록 */
							{
								documentNumList[documentListCnt]  =  d->documentNum;
								documentListCnt++;
								documentNumList = (int*)realloc(documentNumList, sizeof(int)*(documentListCnt+1));
							}
							resultDoc  =  resultList->head;
							while(resultDoc!= NULL)
							{
								if(resultDoc->documentNum  ==  d->documentNum)					/* 기존 검색 결과에 해당 문서 색인이 존재할 경우 빈도 증가 */
								{
									resultDoc->termFreq++;
									break;
								}
								resultDoc  =  resultDoc->next;
							}
							
							d  =  d->next;
						}
						resultDoc  =  resultList->head;
						while(resultDoc !=  NULL)
						{
							docCheck = 0;
							for(i  =  0; i < documentListCnt; i++)
							{
								if(documentNumList[i]  ==  resultDoc->documentNum)				/* 모든 검색 결과들 중 AND 조건 검색 결과 문서 색인이 포함되지 않는 경우 해당 결과 삭제 */
								{
									docCheck = 1;
									break;
								}
							}
							if(docCheck  ==  0)																				
							{
								if(resultDoc  ==  resultList->head)
									resultList->head  =  resultDoc->next;
								else
								{
									d  =  resultList->head;
									while(d->next !=  resultDoc)
										d  =  d->next;
									d->next  =  resultDoc->next;
								}
							}
							resultDoc  =  resultDoc->next;
						}
					}
				}
				else
				{
					andCheck = 1;																	/* AND Keyword 검색결과가 없으므로 검색 중단 */
					break;
				}
				k  =  k->next;
			}
			
			if(resultList->head == NULL || andCheck == 1)
			{
				printf("검색 결과가 없습니다\n");
			}
			else
			{
				resultList  =  _sort_list(resultList);
				printf("문서 위치/검색키워드 ranking : ");
				resultDoc  =  resultList->head;
				while(resultDoc!= NULL)
				{
					printf("%d/%d\t", resultDoc->documentNum, resultDoc->termFreq);
					resultDoc  =  resultDoc->next;
				}
				printf("\n");
			}

		}
		else if(ret < 0)
		{
			printf("검색어를 다시 입력해주세요.\n");
		}
		else
		{
			printf("입력된 검색어가 없습니다.\n");
		}
	}
	
	
	/*************
		List Free
	*************/
	k  =  inputKeyAndList->head;
	while(k!= NULL)
	{
		tempK  =  k;		k  =  k->next;		free(tempK);
	}
	k  =  inputKeyOrList->head;
	while(k!= NULL)
	{
		tempK  =  k;		k  =  k->next;		free(tempK);
	}
	
	
	resultDoc  =  resultList->head;
	while(resultDoc!= NULL)
	{
		d  =  resultDoc;		resultDoc  =  resultDoc->next;		free(d);
	}

}


void main(int argc, char *argv[])
{
	char input[1024] ;
	init();
	while(1)
	{
		printf("문서 번호 파일을 입력하세요.(종료 : q!, 검색 : s!)");
		gets(input);
		if(strstr(input,"s!")!= NULL)
			break;
		else if(strstr(input,"q!")!= NULL)
		{
			exit(0);
		}
		get_words(input);
	}
	
	search_prompt(head);
	
	delete_btree(head);                           /* B-tree 삭제 */
	free(head);
}
 
