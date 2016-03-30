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
	int i;
	head = (Bnode*)malloc(sizeof(Bnode));
	head->ptr[0] = NULL;
	head->n = 0;
}

/*****************
문서 정보 초기화
*****************/
Docidx *init_doc(int docNum, int termFreq){
	Docidx *doc = (Docidx*)malloc(sizeof(Docidx));
	doc->documentNum = docNum;
	doc->termFreq = termFreq;
	doc->next = NULL;
	
	return doc;	
}

/**************
Key 정보 초기화
**************/
Keyvalue *init_key(char *keyword, Docidx *doc){
	Keyvalue *key = (Keyvalue *)malloc(sizeof(Keyvalue));
	key->key = (char*)malloc(sizeof(char)*strlen(keyword));
	sprintf(key->key, keyword);
	key->count = 1;
	key->next = NULL;

	key->DocuVector = doc;	
	return key;
}

/********************
알파벳, 숫자를 제외한
특수문자 제거
********************/

int _ascii_check(char *input)
{
	int i, j, cnt=0;
	for(i = 0; i < strlen(input); i++)
	{
		if((int)input[i] < 48 || (int)input[i] > 122 || ((int)input[i] < 65 && (int)input[i] >57) || ((int)input[i] < 97 && (int)input[i] >90))
		{
			for(j = i; j < strlen(input)-1; j++)
			{
				input[j] = input[j+1];
			}
			
			input[strlen(input)-1] = '\0';
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
listInfo		: B-tree 삽입 전 Key 정보 리스트
*******************************************/

void _get_keylist(char *inputString, int docNum, List *listInfo)
{
	char *token;
	int i, j, keyCheck, docCheck;
	Keyvalue *k;													// Key List 처리를 위한 Temp Keyvalue
	Keyvalue *key;												// 신규 Key 삽입을 위한 Buffer Docidx
	Docidx *d;														// Document Vector List 처리를 위한 temp Docidx
	Docidx *doc;													// 신규 Document Vector 삽입을 위한 Buffer Docidx
	token = strtok(inputString, " ");  		// 입력값에 대하여 공백을 기준으로 형태소를 추출
	while(token !=  NULL)
	{
		_ascii_check(token);										/* 특수문자 제거 */

		keyCheck = 0; 
		k = listInfo->head;
		while(k!= NULL)													/* 추출된 기존 형태소 리스트와 비교 */
		{
			if(!strcmp(k->key, token))						/* 추출된 동일한 형태소가 존재하는 경우 */
			{
				d = k->DocuVector;
				docCheck = 0;
				while(d!= NULL)											/* 추출된 형태소의 Document vector와 비교 */
				{
					if(d->documentNum  ==  docNum)		/* 동일한 문서에 대해서 추출된 형태소 일 경우 */
					{
						docCheck = 1;
						break;
					}
					d = d->next;
				}

				if(docCheck  ==  0)									/* 추출된 형태소에 대해 신규 문서 색인 정보 삽입 */
				{
					d = k->DocuVector;
					for(j = 0;d->next!= NULL; j++)
						d = d->next;
						
					doc = init_doc(docNum, 0);
					d->next = doc;
				}
				k->count++;													/* 형태소 검출 횟수 증가 */
				keyCheck = 1;
				break;
			}
			k = k->next;
		}
	
		if(keyCheck  ==  0)											/* 신규 검출 형태소 등록 */
		{
			doc = init_doc(docNum, 0);
			key = init_key(token, doc);

			if(listInfo->head == NULL )
			{
				listInfo->head = key;
			}

			else
			{
				k = listInfo->head;
				for(i = 0; k->next!= NULL; i++)
					k = k->next;
				k->next = key;
			}			
		}
		token = strtok(NULL," ");
	}
}

/***************************
입력된 파일에서 key값을 추출
파일명이 문서색인 번호
***************************/

void get_words(char *inputFile, Bnode *base)
{
	FILE *ifp;										// 입력 파일 포인터
	char inputBuffer[8192], *ptr;
	int	i,j,docNum;
	Keyvalue *k, *tempK;					// Keyword 처리 및 메모리 free buffer
	Docidx *d;										// Document 정보 처리 buffer
	List* listInfo;								// Keyword 리스트
	
	
	listInfo = (List*)malloc(sizeof(List));	
	listInfo->head = NULL;	
	
	if((ifp = fopen(inputFile,"r"))  == NULL)
	{
		printf("입력파일 읽기 실패\n");
		return;
	}
	
	if((ptr = strtok(inputFile,"."))!=NULL)										/* 파일명에서 확장자 제거 */
		docNum = atoi(ptr);
		
	while(fgets(inputBuffer, sizeof(inputBuffer), ifp) !=  NULL)
	{
		inputBuffer[strlen(inputBuffer)-1] = '\0';
		
		_get_keylist(inputBuffer, docNum, listInfo);						/* 파일에서 행단위 값에 대해 Keyword 검출 */
	}
	fclose(ifp);

	k = listInfo->head;
	while(k!= NULL)
	{
		if (insert_btree(k, base)  ==  NULL)   									/* 추출된 key값 및 색인 정보를 B-tree에 삽입 */
			;
		k = k->next;
	}
	
	k = listInfo->head;
	while(k!= NULL)     																			/* 리스트 Free */
	{
		tempK = k;
		k = k->next;
		free(tempK);
	}	
	listInfo->head = NULL;
	free(listInfo);
	return ;
}

/********************
검색어 입력 조건 확인
********************/
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
입력된 검색어 문자열에서 검색어 추출
쌍따옴표가 붙어있는 문자열은 AND 검색,
아니면 OR 검색 Keyword로 구분
입력값
input  : 입력된 문자열
andKey : AND 검색 검색어 리스트 head
orKey  : OR 검색 검색어 리스트 head
*****************************************/
int input_command(char *input, List *andKey, List *orKey)
{
	char *token, buf[1024];
	int	totalAndKey = 0, totalOrKey = 0;
	Keyvalue *ak, *akN;					// AND 검색 조건 검색어 추출을 위한 buffer
	Keyvalue *ok, *okN;					// OR 검색 조건 검색어 추출을 위한 buffer
	token = strtok(input, " ");
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
		{												
			_ascii_check(token);							/* 특수문자 제외 */
			akN = init_key(token, NULL);

			if(andKey->head == NULL)					/* 먼저 입력된 AND 검색 검색어가 없을 경우 검색어 정보 추가 */
			{
				andKey->head = akN;
			}
			else
			{
				ak = andKey->head;
				while(ak->next!= NULL)
					ak = ak->next;
				ak->next = akN;
			}
			totalAndKey++;
		}
		
		/*******************
		OR 조건 keyword 분류
		*******************/
		
		else
		{
			_ascii_check(token);
			okN = init_key(token, NULL);
			if(orKey->head == NULL)						/* 먼저 입력된 OR 검색 검색어가 없을 경우 검색어 정보 추가 */
			{
				orKey->head = okN;
			}
			else
			{
				ok = orKey->head;
				while(ok->next!= NULL)
					ok = ok->next;
				ok->next = okN;
			}
			totalOrKey++;
		}
		token = strtok(NULL, " ");
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
	Doclist *returnList;														/* return할 List */
	Docidx *a;
	Docidx *doc, *tmp, *tmp2;

	returnList = (Doclist*)malloc(sizeof(Doclist));
	returnList->head = NULL;

	a = base->head;
	while(a !=  NULL)
	{
		doc = init_doc(a->documentNum, a->termFreq);
		if(returnList->head == NULL)									/* 첫 문서 색인 정보 */
		{
			returnList->head = doc;
		}
		else
		{
			tmp = returnList->head;
			if(tmp->termFreq < doc->termFreq)						/* 리스트 Head 문서색인의 검색빈도가 더 낮을 경우 head 교체 */
			{
				doc->next  = tmp;
				returnList->head = doc;
			}
			else
			{
				while(tmp!= NULL)
				{
					if(tmp->termFreq >=  doc->termFreq)			
						{
						if(tmp->next == NULL)									/* 리스트의 마지막에 문서색인 정보 삽입 */
						{
							tmp->next = doc;
							break;
						}
					}
					else
					{
						doc->next = tmp;											/* 리스트의 적당한 위치에 문서 색인 정보 삽입 */
						tmp2->next = doc;
						break;
					}
					tmp2 = tmp;
					tmp = tmp->next;
				}
			}
		}
		a = a->next;
	}
	return returnList;

}


/**************************************
문서 색인 정보 리스트에서
문서 색인 정보 찾기
입력값
listDoc  : 기존 검색 결과 문서색인 List
inputDoc : 검색할 문서색인 정보
**************************************/

Docidx *_search_document(Doclist *listDoc, Docidx *inputDoc)
{
	Docidx *d;
	d = listDoc->head;
	while(d!= NULL)
	{	
		if(d->documentNum  ==  inputDoc->documentNum)
		{ 
			return d;
		}
		d = d->next;
	}
	return NULL;
}


/**************************************
문서 색인 정보 리스트에
문서 색인 정보 삽입
입력값
listDoc  : 기존 검색 결과 문서색인 List
inputDoc : 삽입할 문서색인 정보
**************************************/

int _insert_document(Doclist *listDoc, Docidx *inputDoc)
{
	Docidx *d;
	Docidx *doc;
	doc = init_doc(inputDoc->documentNum, 1);
	if(listDoc->head == NULL){
		listDoc->head = doc;
	}
	else{
		d = listDoc->head;
		while(d->next != NULL)
		{	
			d = d->next;
		}
		d->next = doc;
	}
	return 0;
}
	

/**************************************
문서 색인 정보 리스트에
문서 색인 정보 삭제
입력값
listDoc  : 기존 검색 결과 문서색인 List
inputDoc : 삭제할 문서색인 정보
**************************************/
	
int _delete_document(Doclist *listDoc, Docidx *inputDoc)
{
	Docidx *d;

	if(inputDoc  ==  listDoc->head)
		listDoc->head = inputDoc->next;
	else
	{
		d = listDoc->head;
		while(d->next !=  inputDoc)
			d = d->next;
		d->next = inputDoc->next;
	}
	return 0;
}


/******************************************
B-tree에서 입력된 OR 검색조건 검색어를
검색하여 결과 결과 list에 저장
입력값
inputList  : 입력된 OR 검색조건 검색어 List
resultList : 결과 문서 색인 정보 List
base			 : B-tree 루트 노드
******************************************/

void _or_search(List* inputList, Doclist* resultList, Bnode *base)
{
	Keyvalue *key;
	Bnodekey *keyR;	// keyword 검색 결과 저장
	Docidx *d, *doc;
	
	key = inputList->head; 

	while(key!= NULL)
	{
		if((keyR = search_btree(key->key, base))!= NULL) 					/* B-tree에서 Keyword 검색 */
		{
			d = keyR->DocuVector;
			while(d !=  NULL)
			{
				if((doc = _search_document(resultList, d))>0)					/* 기존 검색결과 리스트에서 검색된 문서 색인의 정보가 있을 경우 빈도 증가 */
					doc->termFreq++;
				else
					_insert_document(resultList, d);                    /* 새로 검색된 index 추가 */
				d = d->next;
			}
		}
		key =  key->next;
	}
	
}

/*********************************************
B-tree에서 입력된 AND 검색조건 검색어를
검색하여 결과 결과 list에 저장
AND 검색 결과에 해당하지 않는 기존 결과는 삭제
입력값
inputList  : 입력된 AND 검색조건 검색어 List
resultList : 결과 문서 색인 정보 List
base			 : B-tree 루트 노드
*********************************************/
int _and_search(List* inputList, Doclist* resultList, Bnode *base)
{
	Keyvalue *key;
	Bnodekey *keyR;	// keyword 검색 결과 저장
	Docidx *d, *doc;
	int i, documentListCnt, docCheck;
	int *documentNumList;							// AND 조건 검색에 의해 검색된 문서 색인 리스트, 기존/신규 검색 결과에서 해당 리스트에 존재하지 않을 경우 결과 삭제
		
	documentListCnt = 0;
	documentNumList = (int*)malloc(sizeof(int)*(documentListCnt+1));
	
	key = inputList->head; 

	while(key!= NULL)
	{
		if((keyR = search_btree(key->key,base))!= NULL) /* B-tree에서 검색어 검색 */
		{
			
			if(resultList->head  ==  NULL)
			{																							/* 기존 문서 색인 정보가 존재하지 않을 경우 */
				d = keyR->DocuVector;
				if(documentListCnt>0)												/* AND 조건에 의해 검색 결과가 제외되어 문서색인정보가 존재하지 않는 경우 처리하지 않음*/
					;	
				else																				/* 기존에 검색된 결과가 없는 경우 */
				{
					while(d !=  NULL)
					{
						if((doc = _search_document(resultList, d))>0)		/* 기존 검색결과에 해당 문서 색인 정보가 존재할 경우 검색 빈도 증가 */
							doc->termFreq++;
						else																			
							_insert_document(resultList, d); 							/* 기존 검색결과에 해당 문서 색인이 존재하지 않을 경우 문서 색인 정보를 검색결과에 추가 */
						d = d->next;
					}
				}
			}
			else {																												/* 기존 검색 결과가 존재할 경우 */
				d = keyR->DocuVector;
				while(d !=  NULL)
				{
					docCheck = 0;
					for(i = 0; i < documentListCnt; i++)
					{
						if(documentNumList[i]  ==  d->documentNum)							/* documentNumList : AND 조건에 의해 검색된 결과들의 리스트 */
						{
							docCheck = 1;
							break;
						}
					}
					if(docCheck == 0)																					/* 기존 검색된 AND 조건 결과에 해당 문서색인이 없을 때 리스트에 등록 */
					{
						documentNumList[documentListCnt] = d->documentNum;
						documentListCnt++;
						documentNumList = (int*)realloc(documentNumList, sizeof(int)*(documentListCnt+1));
					}
					if((doc = _search_document(resultList, d))>0)							/* 검색된 문서 색인 정보가 기존 결과 리스트에 존재할 경우 빈도 증가 */
						doc->termFreq++;
					d = d->next;
				}
				
				d = resultList->head;
				while(d !=  NULL)																						/* 모든 검색 결과들 중 AND 조건에 의해 검색된 문서 색인이 포함되지 않는 경우 해당 정보 삭제 */
				{
					docCheck = 0;
					for(i = 0; i < documentListCnt; i++)
					{
						if(documentNumList[i]  ==  d->documentNum)				
						{
							docCheck = 1;
							break;
						}
					}
					if(docCheck  ==  0)																				
					{
						_delete_document(resultList, d);									
					}
					d = d->next;
				}
			}
		}
		else
		{
			return 1;																											/* AND Keyword 검색결과가 없으므로 검색 중단 */
			break;
		}
		key = key->next;
	}
	return 0;
}
	
/******************************************
검색어를 입력받고 B-tree에서 key를 검색하여
결과를 출력
******************************************/
void search_prompt(Bnode *base)
{
	char keyword[1024];
	int i, docCheck, ret;
	int andCheck;					


	Docidx *d;
	Docidx *resultDoc, *tempD; 
	Keyvalue *k, *tempK;
	List *inputKeyAndList, *inputKeyOrList;
	Doclist *resultList;


	
	while (1)
	{
		printf("\n검색할 키워드를 입력하세요 (종료시 q!)->");
		
		gets(keyword);
		if(strstr(keyword,"q!")!= NULL)																				/* "q!" 입력시 loop 종료 */
			break;

		inputKeyAndList = (List*)malloc(sizeof(List));	
		inputKeyAndList->head = NULL;	
		inputKeyOrList = (List*)malloc(sizeof(List));	
		inputKeyOrList->head = NULL;
		resultList = (Doclist*)malloc(sizeof(Doclist));
		resultList->head = NULL;
		andCheck = 0;
		
		
		if((ret = input_command(keyword, inputKeyAndList, inputKeyOrList))>0) /* 입력된 문장을 AND, OR 검색 조건에 맞춰 리스트 저장 */
		{
			k = inputKeyOrList->head;
			printf("OR key :\n");
			while(k != NULL)																										/* OR 검색조건 검색어 List 출력 */
			{
				printf("\t%s", k->key);
				k = k->next;
			}
			printf("\n");
			k = inputKeyAndList->head;
			printf("AND key :\n");																								/* AND 검색조건 검색어 List 출력 */
			while(k != NULL)
			{
				printf("\n%s", k->key);
				k = k->next;
			}
			printf("\n");
			
			/************************ 
			OR 조건 검색 keyword 처리
			************************/
			if(inputKeyOrList->head !=NULL)
				_or_search(inputKeyOrList, resultList, base);												/* OR 조건 검색어부터 검색 */
			

			/************************* 
			AND 조건 검색 keyword 처리
			*************************/
			if(inputKeyAndList->head !=NULL)
				andCheck = _and_search(inputKeyAndList, resultList, base);					/* AND 조건 검색어 검색 */
																																						/* andCheck = 1 일 경우 AND 검색 조건에 의해 모든 결과가 무효 */
																																						/* 해당 경우에 대해 따로 처리하기 위해 삽입하였으나 기능 미구현. */
			
			if(resultList->head == NULL || andCheck == 1)
			{
				printf("검색 결과가 없습니다\n");
			}
			else
			{
				resultList = _sort_list(resultList);																/* 검색 결과를 문서 내에서 가장 많은 검색으롤 포함한 순으로 정렬 */
				printf("문서 위치/문서 포함 키워드 수 : ");
				resultDoc = resultList->head;
				while(resultDoc!= NULL)
				{
					printf("%d/%d\t", resultDoc->documentNum, resultDoc->termFreq);
					resultDoc = resultDoc->next;
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
	
		/*************
			List Free
		*************/
		k = inputKeyAndList->head;
		while(k!= NULL)
		{
			tempK = k;		k = k->next;		free(tempK);
		}
		k = inputKeyOrList->head;
		while(k!= NULL)
		{
			tempK = k;		k = k->next;		free(tempK);
		}
		resultDoc = resultList->head;
		while(resultDoc!= NULL)
		{
			d = resultDoc;		resultDoc = resultDoc->next;		free(d);
		}
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
		get_words(input, head);													/* 입력된 파일에 대하여 역파일 처리 */
	}
	
	search_prompt(head);												/* 검색어 입력모드로 전환 */
	
	delete_btree(head);                           /* B-tree 삭제 */
	free(head);
}

 
