/**********************************************************************************************
2016.03.23 ���� �Ϸ�
source ���� ����
  structure.h : B-tree, ����, Key ���� � ���� ���� ����
  btree.h btree.c : btree ó��
  wordsearch.h wordsearch.c : ���� ���� ���� �� �˻� ���
***********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "structure.h"
#include "wordsearch.h"
#include "btree.h"

Bnode *head;

/**************
�ʱ�ȭ
**************/
void init()
{
	int i;
	head = (Bnode*)malloc(sizeof(Bnode));
	head->ptr[0] = NULL;
	head->n = 0;
}

/*****************
���� ���� �ʱ�ȭ
*****************/
Docidx *init_doc(int docNum, int termFreq){
	Docidx *doc = (Docidx*)malloc(sizeof(Docidx));
	doc->documentNum = docNum;
	doc->termFreq = termFreq;
	doc->next = NULL;
	
	return doc;	
}

/**************
Key ���� �ʱ�ȭ
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
���ĺ�, ���ڸ� ������
Ư������ ����
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
�Էµ� ���ڿ����� Keyword ����
�Է°�
inputString : �Է� ���ڿ�
docNum      : ���� ��ȣ
listInfo		: B-tree ���� �� Key ���� ����Ʈ
*******************************************/

void _get_keylist(char *inputString, int docNum, List *listInfo)
{
	char *token;
	int i, j, keyCheck, docCheck;
	Keyvalue *k;													// Key List ó���� ���� Temp Keyvalue
	Keyvalue *key;												// �ű� Key ������ ���� Buffer Docidx
	Docidx *d;														// Document Vector List ó���� ���� temp Docidx
	Docidx *doc;													// �ű� Document Vector ������ ���� Buffer Docidx
	token = strtok(inputString, " ");  		// �Է°��� ���Ͽ� ������ �������� ���¼Ҹ� ����
	while(token !=  NULL)
	{
		_ascii_check(token);										/* Ư������ ���� */

		keyCheck = 0; 
		k = listInfo->head;
		while(k!= NULL)													/* ����� ���� ���¼� ����Ʈ�� �� */
		{
			if(!strcmp(k->key, token))						/* ����� ������ ���¼Ұ� �����ϴ� ��� */
			{
				d = k->DocuVector;
				docCheck = 0;
				while(d!= NULL)											/* ����� ���¼��� Document vector�� �� */
				{
					if(d->documentNum  ==  docNum)		/* ������ ������ ���ؼ� ����� ���¼� �� ��� */
					{
						docCheck = 1;
						break;
					}
					d = d->next;
				}

				if(docCheck  ==  0)									/* ����� ���¼ҿ� ���� �ű� ���� ���� ���� ���� */
				{
					d = k->DocuVector;
					for(j = 0;d->next!= NULL; j++)
						d = d->next;
						
					doc = init_doc(docNum, 0);
					d->next = doc;
				}
				k->count++;													/* ���¼� ���� Ƚ�� ���� */
				keyCheck = 1;
				break;
			}
			k = k->next;
		}
	
		if(keyCheck  ==  0)											/* �ű� ���� ���¼� ��� */
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
�Էµ� ���Ͽ��� key���� ����
���ϸ��� �������� ��ȣ
***************************/

void get_words(char *inputFile, Bnode *base)
{
	FILE *ifp;										// �Է� ���� ������
	char inputBuffer[8192], *ptr;
	int	i,j,docNum;
	Keyvalue *k, *tempK;					// Keyword ó�� �� �޸� free buffer
	Docidx *d;										// Document ���� ó�� buffer
	List* listInfo;								// Keyword ����Ʈ
	
	
	listInfo = (List*)malloc(sizeof(List));	
	listInfo->head = NULL;	
	
	if((ifp = fopen(inputFile,"r"))  == NULL)
	{
		printf("�Է����� �б� ����\n");
		return;
	}
	
	if((ptr = strtok(inputFile,"."))!=NULL)										/* ���ϸ��� Ȯ���� ���� */
		docNum = atoi(ptr);
		
	while(fgets(inputBuffer, sizeof(inputBuffer), ifp) !=  NULL)
	{
		inputBuffer[strlen(inputBuffer)-1] = '\0';
		
		_get_keylist(inputBuffer, docNum, listInfo);						/* ���Ͽ��� ����� ���� ���� Keyword ���� */
	}
	fclose(ifp);

	k = listInfo->head;
	while(k!= NULL)
	{
		if (insert_btree(k, base)  ==  NULL)   									/* ����� key�� �� ���� ������ B-tree�� ���� */
			;
		k = k->next;
	}
	
	k = listInfo->head;
	while(k!= NULL)     																			/* ����Ʈ Free */
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
�˻��� �Է� ���� Ȯ��
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
�Էµ� �˻��� ���ڿ����� �˻��� ����
�ֵ���ǥ�� �پ��ִ� ���ڿ��� AND �˻�,
�ƴϸ� OR �˻� Keyword�� ����
�Է°�
input  : �Էµ� ���ڿ�
andKey : AND �˻� �˻��� ����Ʈ head
orKey  : OR �˻� �˻��� ����Ʈ head
*****************************************/
int input_command(char *input, List *andKey, List *orKey)
{
	char *token, buf[1024];
	int	totalAndKey = 0, totalOrKey = 0;
	Keyvalue *ak, *akN;					// AND �˻� ���� �˻��� ������ ���� buffer
	Keyvalue *ok, *okN;					// OR �˻� ���� �˻��� ������ ���� buffer
	token = strtok(input, " ");
	while(token != NULL)
	{
		sprintf(buf, token);
		if(_input_check(buf)<0)
			return -1;
		/*************************
		'"' ��ȣ�� �پ� ���� ���
		AND ���� keyword�� �з�
		*************************/
		if(strchr(token,'\"')!= NULL)
		{												
			_ascii_check(token);							/* Ư������ ���� */
			akN = init_key(token, NULL);

			if(andKey->head == NULL)					/* ���� �Էµ� AND �˻� �˻�� ���� ��� �˻��� ���� �߰� */
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
		OR ���� keyword �з�
		*******************/
		
		else
		{
			_ascii_check(token);
			okN = init_key(token, NULL);
			if(orKey->head == NULL)						/* ���� �Էµ� OR �˻� �˻�� ���� ��� �˻��� ���� �߰� */
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
���� ���� �˻�� ���Ե� ��������
�˻� ����� ����
�Է°�
base : �˻���� ����Ʈ head
*********************************/

Doclist *_sort_list(Doclist *base)
{
	Doclist *returnList;														/* return�� List */
	Docidx *a;
	Docidx *doc, *tmp, *tmp2;

	returnList = (Doclist*)malloc(sizeof(Doclist));
	returnList->head = NULL;

	a = base->head;
	while(a !=  NULL)
	{
		doc = init_doc(a->documentNum, a->termFreq);
		if(returnList->head == NULL)									/* ù ���� ���� ���� */
		{
			returnList->head = doc;
		}
		else
		{
			tmp = returnList->head;
			if(tmp->termFreq < doc->termFreq)						/* ����Ʈ Head ���������� �˻��󵵰� �� ���� ��� head ��ü */
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
						if(tmp->next == NULL)									/* ����Ʈ�� �������� �������� ���� ���� */
						{
							tmp->next = doc;
							break;
						}
					}
					else
					{
						doc->next = tmp;											/* ����Ʈ�� ������ ��ġ�� ���� ���� ���� ���� */
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
���� ���� ���� ����Ʈ����
���� ���� ���� ã��
�Է°�
listDoc  : ���� �˻� ��� �������� List
inputDoc : �˻��� �������� ����
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
���� ���� ���� ����Ʈ��
���� ���� ���� ����
�Է°�
listDoc  : ���� �˻� ��� �������� List
inputDoc : ������ �������� ����
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
���� ���� ���� ����Ʈ��
���� ���� ���� ����
�Է°�
listDoc  : ���� �˻� ��� �������� List
inputDoc : ������ �������� ����
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
B-tree���� �Էµ� OR �˻����� �˻��
�˻��Ͽ� ��� ��� list�� ����
�Է°�
inputList  : �Էµ� OR �˻����� �˻��� List
resultList : ��� ���� ���� ���� List
base			 : B-tree ��Ʈ ���
******************************************/

void _or_search(List* inputList, Doclist* resultList, Bnode *base)
{
	Keyvalue *key;
	Bnodekey *keyR;	// keyword �˻� ��� ����
	Docidx *d, *doc;
	
	key = inputList->head; 

	while(key!= NULL)
	{
		if((keyR = search_btree(key->key, base))!= NULL) 					/* B-tree���� Keyword �˻� */
		{
			d = keyR->DocuVector;
			while(d !=  NULL)
			{
				if((doc = _search_document(resultList, d))>0)					/* ���� �˻���� ����Ʈ���� �˻��� ���� ������ ������ ���� ��� �� ���� */
					doc->termFreq++;
				else
					_insert_document(resultList, d);                    /* ���� �˻��� index �߰� */
				d = d->next;
			}
		}
		key =  key->next;
	}
	
}

/*********************************************
B-tree���� �Էµ� AND �˻����� �˻��
�˻��Ͽ� ��� ��� list�� ����
AND �˻� ����� �ش����� �ʴ� ���� ����� ����
�Է°�
inputList  : �Էµ� AND �˻����� �˻��� List
resultList : ��� ���� ���� ���� List
base			 : B-tree ��Ʈ ���
*********************************************/
int _and_search(List* inputList, Doclist* resultList, Bnode *base)
{
	Keyvalue *key;
	Bnodekey *keyR;	// keyword �˻� ��� ����
	Docidx *d, *doc;
	int i, documentListCnt, docCheck;
	int *documentNumList;							// AND ���� �˻��� ���� �˻��� ���� ���� ����Ʈ, ����/�ű� �˻� ������� �ش� ����Ʈ�� �������� ���� ��� ��� ����
		
	documentListCnt = 0;
	documentNumList = (int*)malloc(sizeof(int)*(documentListCnt+1));
	
	key = inputList->head; 

	while(key!= NULL)
	{
		if((keyR = search_btree(key->key,base))!= NULL) /* B-tree���� �˻��� �˻� */
		{
			
			if(resultList->head  ==  NULL)
			{																							/* ���� ���� ���� ������ �������� ���� ��� */
				d = keyR->DocuVector;
				if(documentListCnt>0)												/* AND ���ǿ� ���� �˻� ����� ���ܵǾ� �������������� �������� �ʴ� ��� ó������ ����*/
					;	
				else																				/* ������ �˻��� ����� ���� ��� */
				{
					while(d !=  NULL)
					{
						if((doc = _search_document(resultList, d))>0)		/* ���� �˻������ �ش� ���� ���� ������ ������ ��� �˻� �� ���� */
							doc->termFreq++;
						else																			
							_insert_document(resultList, d); 							/* ���� �˻������ �ش� ���� ������ �������� ���� ��� ���� ���� ������ �˻������ �߰� */
						d = d->next;
					}
				}
			}
			else {																												/* ���� �˻� ����� ������ ��� */
				d = keyR->DocuVector;
				while(d !=  NULL)
				{
					docCheck = 0;
					for(i = 0; i < documentListCnt; i++)
					{
						if(documentNumList[i]  ==  d->documentNum)							/* documentNumList : AND ���ǿ� ���� �˻��� ������� ����Ʈ */
						{
							docCheck = 1;
							break;
						}
					}
					if(docCheck == 0)																					/* ���� �˻��� AND ���� ����� �ش� ���������� ���� �� ����Ʈ�� ��� */
					{
						documentNumList[documentListCnt] = d->documentNum;
						documentListCnt++;
						documentNumList = (int*)realloc(documentNumList, sizeof(int)*(documentListCnt+1));
					}
					if((doc = _search_document(resultList, d))>0)							/* �˻��� ���� ���� ������ ���� ��� ����Ʈ�� ������ ��� �� ���� */
						doc->termFreq++;
					d = d->next;
				}
				
				d = resultList->head;
				while(d !=  NULL)																						/* ��� �˻� ����� �� AND ���ǿ� ���� �˻��� ���� ������ ���Ե��� �ʴ� ��� �ش� ���� ���� */
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
			return 1;																											/* AND Keyword �˻������ �����Ƿ� �˻� �ߴ� */
			break;
		}
		key = key->next;
	}
	return 0;
}
	
/******************************************
�˻�� �Է¹ް� B-tree���� key�� �˻��Ͽ�
����� ���
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
		printf("\n�˻��� Ű���带 �Է��ϼ��� (����� q!)->");
		
		gets(keyword);
		if(strstr(keyword,"q!")!= NULL)																				/* "q!" �Է½� loop ���� */
			break;

		inputKeyAndList = (List*)malloc(sizeof(List));	
		inputKeyAndList->head = NULL;	
		inputKeyOrList = (List*)malloc(sizeof(List));	
		inputKeyOrList->head = NULL;
		resultList = (Doclist*)malloc(sizeof(Doclist));
		resultList->head = NULL;
		andCheck = 0;
		
		
		if((ret = input_command(keyword, inputKeyAndList, inputKeyOrList))>0) /* �Էµ� ������ AND, OR �˻� ���ǿ� ���� ����Ʈ ���� */
		{
			k = inputKeyOrList->head;
			printf("OR key :\n");
			while(k != NULL)																										/* OR �˻����� �˻��� List ��� */
			{
				printf("\t%s", k->key);
				k = k->next;
			}
			printf("\n");
			k = inputKeyAndList->head;
			printf("AND key :\n");																								/* AND �˻����� �˻��� List ��� */
			while(k != NULL)
			{
				printf("\n%s", k->key);
				k = k->next;
			}
			printf("\n");
			
			/************************ 
			OR ���� �˻� keyword ó��
			************************/
			if(inputKeyOrList->head !=NULL)
				_or_search(inputKeyOrList, resultList, base);												/* OR ���� �˻������ �˻� */
			

			/************************* 
			AND ���� �˻� keyword ó��
			*************************/
			if(inputKeyAndList->head !=NULL)
				andCheck = _and_search(inputKeyAndList, resultList, base);					/* AND ���� �˻��� �˻� */
																																						/* andCheck = 1 �� ��� AND �˻� ���ǿ� ���� ��� ����� ��ȿ */
																																						/* �ش� ��쿡 ���� ���� ó���ϱ� ���� �����Ͽ����� ��� �̱���. */
			
			if(resultList->head == NULL || andCheck == 1)
			{
				printf("�˻� ����� �����ϴ�\n");
			}
			else
			{
				resultList = _sort_list(resultList);																/* �˻� ����� ���� ������ ���� ���� �˻����� ������ ������ ���� */
				printf("���� ��ġ/���� ���� Ű���� �� : ");
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
			printf("�˻�� �ٽ� �Է����ּ���.\n");
		}
		else
		{
			printf("�Էµ� �˻�� �����ϴ�.\n");
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
		printf("���� ��ȣ ������ �Է��ϼ���.(���� : q!, �˻� : s!)");
		gets(input);
		if(strstr(input,"s!")!= NULL)
			break;
		else if(strstr(input,"q!")!= NULL)
		{
			exit(0);
		}
		get_words(input, head);													/* �Էµ� ���Ͽ� ���Ͽ� ������ ó�� */
	}
	
	search_prompt(head);												/* �˻��� �Է¸��� ��ȯ */
	
	delete_btree(head);                           /* B-tree ���� */
	free(head);
}

 
