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
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include "structure.h"
#include "wordsearch.h"
#include "btree.h"
#include <sys/shm.h>

#define	MEM_SIZE	104800

char input_index[13][2] = {"aq", "bn", "cz", "dv", "eg", "fo", "hl", "iw", "jm", "kt", "py", "ru", "sx"};

int KEY_NUM;

int word_cnt=0;
int btree_word_cnt=0;


Bnode *head;
int input_idx;
int shm_key;


/**************
�ʱ�ȭ
**************/
Bnode* init_head()
{
	Bnode *head;
	head = (Bnode*)malloc(sizeof(Bnode));
	head->ptr[0] = NULL;
	head->n = 0;
	return head;
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



int IsSpaceWork(unsigned char ch)
{
	if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
		return 1;
	return 0;
}

int StrLRTrimWork(char *pszString)
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
			if( bAllSpace && !IsSpaceWork( *pszEnd ) )
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

			while( ( IsSpaceWork( *pszEnd ) ) && pszEnd >= pszString )
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

/********************
���ĺ�, ���ڸ� ������
Ư������ ����
********************/

int _ascii_check(char *input)
{
	int i, j, cnt=0;
	for(i = 0; i < strlen(input); i++)
	{
		if((int)input[i]>64 && (int)input[i]<91){
			cnt++;
			input[i] = (int)input[i]+32;
		}
		else if((int)input[i]>96 &&(int)input[i]<123)
			cnt++;
		else if((int)input[i] < 48 || (int)input[i] > 122 || ((int)input[i] < 65 && (int)input[i] >57) || ((int)input[i] < 97 && (int)input[i] >90))
		{
/*
			for(j = i; j < strlen(input)-1; j++)
			{
				input[j] = input[j+1];
			}
			
			input[strlen(input)-1] = '\0';
			i--;
*/			
			input[i]=' ';
		}
	}
	StrLRTrimWork(input);
	if((int)input[0]>47 && (int)input[0]<58)
		cnt=0;
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
	char *token, tempBuf[1024];
	FILE *fp, *logFile;
	int i, j, keyCheck, docCheck,l=0;
	Keyvalue *k;													// Key List ó���� ���� Temp Keyvalue
	Keyvalue *key;												// �ű� Key ������ ���� Buffer Docidx
	Docidx *d;														// Document Vector List ó���� ���� temp Docidx
	Docidx *doc;													// �ű� Document Vector ������ ���� Buffer Docidx
	_ascii_check(inputString);										/* Ư������ ���� */
	StrLRTrimWork(inputString);
	token = strtok(inputString, " ");  		// �Է°��� ���Ͽ� ������ �������� ���¼Ҹ� ����
	while(token !=  NULL)
	{
		if(_ascii_check(token)>0){
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
			word_cnt++;;
		}
		}
		l++;
		token = strtok(NULL," ");
	}
}


void _get_keylist_index(char *inputString, Docidx *doc, List *listInfo)
{
	char *token;
	int i, j, keyCheck, docCheck;
	Keyvalue *k;													// Key List ó���� ���� Temp Keyvalue
	Keyvalue *key;												// �ű� Key ������ ���� Buffer Docidx
	Docidx *d;														// Document Vector List ó���� ���� temp Docidx


	keyCheck = 0; 
	k = listInfo->head;
	while(k!= NULL)													/* ����� ���� ���¼� ����Ʈ�� �� */
	{
		if(!strcmp(k->key, inputString))						/* ����� ������ ���¼Ұ� �����ϴ� ��� */
		{
			d = k->DocuVector;
			docCheck = 0;
			while(d!= NULL)											/* ����� ���¼��� Document vector�� �� */
			{
				if(d->documentNum  ==  doc->documentNum)		/* ������ ������ ���ؼ� ����� ���¼� �� ��� */
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
		key = init_key(inputString, doc);

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

}

void get_words_file(char *inputFile, Bnode *base)
{
	char inputBuffer[999999], *ptr, getBuffer[999999], tempBuf[999999], tempBuf2[999999], logFileName[1024];;
	int	i,j,docNum;
	Keyvalue *k, *tempK;					// Keyword ó�� �� �޸� free buffer
	Docidx *d, *doc;										// Document ���� ó�� buffer
	List* listInfo;								// Keyword ����Ʈ
	FILE *ifp;
	
	listInfo = (List*)malloc(sizeof(List));	
	listInfo->head = NULL;	
	if((ifp = fopen(inputFile,"r"))  == NULL)
	{
		
		printf("�Է����� �б� ����\n");
		return;
	}

	while(fgets(inputBuffer, sizeof(inputBuffer), ifp) !=  NULL)
	{
		sprintf(tempBuf,inputBuffer);
		ptr = strtok(tempBuf," ");
		sprintf(getBuffer, ptr);
		while((ptr=strtok(NULL," "))!=NULL){
			sprintf(tempBuf2, ptr);
			StrLRTrimWork(tempBuf2);
			if(strlen(tempBuf2)>0){
				docNum=atoi(tempBuf2);
				doc=init_doc(docNum, 0);
				_get_keylist_index(getBuffer, doc, listInfo);						/* ���Ͽ��� ����� ���� ���� Keyword ���� */
			}
		}
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


/***************************
�Էµ� ���Ͽ��� key���� ����
���ϸ��� �������� ��ȣ
***************************/

void get_words(char *inputFile, Bnode *base)
{
	char inputBuffer[1048576], *ptr, getBuffer[1048576], tempBuf[1048576], tempBuf2[1048576];
	int	i,j,docNum, ret;
	Keyvalue *k, *tempK;					// Keyword ó�� �� �޸� free buffer
	Docidx *d;										// Document ���� ó�� buffer
	List* listInfo;								// Keyword ����Ʈ
	FILE *lfp, *ifp;

	listInfo = (List*)malloc(sizeof(List));	
	listInfo->head = NULL;
	if((ifp = fopen(inputFile,"r"))  == NULL)
	{
		printf("�Է����� �б� ����\n");
		return;
	}
		

//	if((ptr = strtok(inputFile,","))!=NULL)										/* ���ϸ��� Ȯ���� ���� */
//		docNum = atoi(ptr);

		
	while(fgets(inputBuffer, sizeof(inputBuffer), ifp) !=  NULL)
	{
		inputBuffer[strlen(inputBuffer)] = '\0';
		strcpy(tempBuf,inputBuffer);
		if(strstr(tempBuf,"INSERT")!=NULL){
			ptr=strtok(tempBuf,"(");
			ptr=strtok(NULL,",");
		}
		else{
			for(i=0;i<strlen(tempBuf)-1;i++){
				tempBuf[i]=tempBuf[i+1];
			}
			ptr=strtok(tempBuf,",");
		}

		docNum=atoi(ptr);
		if(docNum==0)
			continue;
		ptr=strtok(NULL,",");
		ptr=strtok(NULL,",");
		sprintf(getBuffer,"");
		while((ptr=strtok(NULL,")"))!=NULL){
			sprintf(tempBuf2,ptr);
			_ascii_check(tempBuf2);										/* Ư������ ���� */
			strcat(getBuffer,tempBuf2);
		}
		getBuffer[strlen(getBuffer)] = '\0';

		_get_keylist(getBuffer, docNum, listInfo);						/* ���Ͽ��� ����� ���� ���� Keyword ���� */

	}
	fclose(ifp);
	k = listInfo->head;
	while(k!= NULL)
	{
		ret = insert_btree(k, base);   									/* ����� key�� �� ���� ������ B-tree�� ���� */
		if(ret<0){
			sprintf(tempBuf,"/wndi/temp/btree_fail.log");
		}
        else if(ret==0){
            sprintf(tempBuf,"/wndi/temp/btree_succ.log");
			btree_word_cnt++;;
        }

		k = k->next;
	}

	return base;
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
			StrLRTrimWork(token);
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


Doclist *_sort_list_all(List *base)
{
    List *returnList;                                                        /* return�� List */
    Docidx *a;
    Docidx *doc, *tmp, *tmp2;
	Doclist *dList;
	Keyvalue *k, *key, *keyR;
    returnList = (List*)malloc(sizeof(List));
    returnList->head = NULL;
	dList = (Doclist*)malloc(sizeof(Doclist));
	dList->head = NULL;
	k = base->head;
	while(k!=NULL){
    	a = k->DocuVector;
    	while(a !=  NULL)
    	{
	        doc = init_doc(a->documentNum, a->termFreq);
	    	if(dList->head == NULL)                                    /* ù ���� ���� ���� */
	    	{
	        	dList->head = doc;
	    	}
	       else
	       {
           		tmp = dList->head;
           		if(tmp->termFreq < doc->termFreq)                       /* ����Ʈ Head ���������� �˻��󵵰� �� ���� ��� head ��ü */
           		{
               		doc->next  = tmp;
               		dList->head = doc;
           		}
           		else
   	       		{
               		while(tmp!= NULL)
               		{
                   		if(tmp->termFreq >=  doc->termFreq)
                       	{
                       		if(tmp->next == NULL)                                   /* ����Ʈ�� �������� �������� ���� ���� */
                       		{
                           		tmp->next = doc;
                           		break;
                       		}
   	               		}
                   		else
	                   	{
                      		doc->next = tmp;                                            /* ����Ʈ�� ������ ��ġ�� ���� ���� ���� ���� */
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
		key = init_key(k->key, dList->head);
		if(returnList->head==NULL){
			returnList->head = key;
		}
		else{
			keyR=returnList->head;
			while(keyR->next!=NULL)
				keyR=keyR->next;
			keyR->next=key;
		}
	
		k=k->next;
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
Docidx *_search_document_all(Doclist *listDoc, Docidx *inputDoc)
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

int _insert_document_all(Doclist *listDoc, Docidx *inputDoc)
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
void _search_all(List* inputList, List* resultList, Bnode *base)
{
    Keyvalue *key, *keyResult, *k;
    Bnodekey *keyR; // keyword �˻� ��� ����
    Docidx *d, *doc;
	Doclist *kdList;
	int chk;

    key = inputList->head;
	kdList = (Doclist*) malloc(sizeof(Doclist));

    while(key!= NULL)
    {
		printf("key : %s \n", key->key);
        if((keyR = search_btree(key->key, base))!= NULL)                    /* B-tree���� Keyword �˻� */
        {
			keyResult = resultList->head;
			chk=0;
			while(keyResult != NULL){
				if(!strcmp(keyResult->key, keyR->key)){
					chk=1;
					break;
				}
				keyResult = keyResult->next;
            }
			if(chk==0){
				k = init_key(key->key, keyR->DocuVector);
				printf("insert key : %s\n", key->key);
            	if(resultList->head == NULL )
            	{
                	resultList->head = k;
            	}
            	else
            	{
                	keyResult = resultList->head;
                	while(keyResult->next!= NULL)
                    	keyResult = keyResult->next;
                	keyResult->next = k;
	            }
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
void search_index(char *inputBuffer, Bnode *base)
{
    char keyword[1024], tempBuf[1024], resultSet[1000000];
    int i, docCheck, ret, index, count;
    int andCheck;
	int	shm_id;
	void *shm_addr;
    FILE *ifp;
	char *ptr_1;


    Docidx *d;
    Docidx *resultDoc, *tempD;
    Keyvalue *k, *tempK, *key;
    List *inputKeyAndList, *inputKeyOrList;
    Doclist *resultList;

    List *resultOrList, *resultAndList;
	if((ptr_1=strtok(inputBuffer,"["))==NULL)
		return -1;
	sprintf(keyword,ptr_1);


	inputKeyAndList = (List*)malloc(sizeof(List));
	inputKeyAndList->head = NULL;
	inputKeyOrList = (List*)malloc(sizeof(List));
	inputKeyOrList->head = NULL;
	resultList = (Doclist*)malloc(sizeof(Doclist));
	resultList->head = NULL;
	resultOrList = (List*)malloc(sizeof(List));
	resultOrList->head = NULL;
	resultAndList = (List*)malloc(sizeof(List));
	resultAndList->head = NULL;
	andCheck = 0;
	sprintf(resultSet,"");
	
	if((ret = input_command(keyword, inputKeyAndList, inputKeyOrList))>0) /* �Էµ� ������ AND, OR �˻� ���ǿ� ���� ����Ʈ ���� */
	{
		/************************
		OR ���� �˻� keyword ó��
		************************/
		if(inputKeyOrList->head !=NULL)
			_search_all(inputKeyOrList, resultOrList, base);


		/*************************
		AND ���� �˻� keyword ó��
		*************************/
		if(inputKeyAndList->head !=NULL)
			_search_all(inputKeyAndList, resultAndList, base);
		if(resultOrList->head == NULL && resultAndList == NULL)
		{
		}
		else
		{
			count=0;
			k = inputKeyOrList->head;
			while(k!=NULL){
				key = resultOrList->head;
				i=0;
				while(key!=NULL){
					if(!strcmp(k->key, key->key)){
						
						sprintf(tempBuf,"%s/OR", key->key);
						strcat(resultSet,tempBuf);
						resultDoc = key->DocuVector;
						while(resultDoc!= NULL)
						{
							sprintf(tempBuf,"\t%d", resultDoc->documentNum);
							strcat(resultSet, tempBuf);
							resultDoc = resultDoc->next;
						}
						strcat(resultSet, "\n");
						i=1;
						break;
					}
					key = key->next;
				}
				if(i==0){
					sprintf(tempBuf, "%s/OR\n", k->key);
					strcat(resultSet, tempBuf);
				}
				k=k->next;
			}
			i=0;
			k = inputKeyAndList->head;
			while(k!=NULL){
				key = resultAndList->head;
				i=0;
				while(key!=NULL){
					if(!strcmp(k->key, key->key)){
						resultDoc = key->DocuVector;
						sprintf(tempBuf,"%s/AND",  key->key);
						strcat(resultSet,tempBuf);
						while(resultDoc!= NULL)
						{
							sprintf(tempBuf,"\t%d", resultDoc->documentNum);
							resultDoc = resultDoc->next;
							strcat(resultSet, tempBuf);
						}
						strcat(resultSet, "\n");
						i=1;
						break;
					}
					key = key->next;
				}
				if(i==0){
					sprintf(tempBuf, "%s/AND\n", k->key);
					strcat(resultSet, tempBuf);
				}
				k=k->next;
			}
			strcat(resultSet,"\n");
		}
	}
	else if(ret < 0)
	{
		;
	}
	else
	{
		;
	}
	if((shm_id = shmget( (key_t)KEY_NUM, MEM_SIZE, IPC_CREAT|0666)) == -1){
		return -1;
	}
	if( ( shm_addr = shmat( shm_id, ( void *)0, 0)) == (void *)-1){
		return -1;
	}
	sprintf( (char *)shm_addr, "%s[output]", resultSet); 
	shmdt( shm_addr);

	/*************
		List Free
	*************/
	k = inputKeyAndList->head;
	while(k!= NULL)
	{
		tempK = k;      k = k->next;        free(tempK);
	}
	k = inputKeyOrList->head;
	while(k!= NULL)
	{
		tempK = k;      k = k->next;        free(tempK);
	}
	resultDoc = resultList->head;
	while(resultDoc!= NULL)
	{
		d = resultDoc;      resultDoc = resultDoc->next;        free(d);
	}
}

void main(int argc, char *argv[])
{
	char input[1024], tempBuf[1024], tmpBuf[1024], inputBuffer[1024];
	FILE *fp, *lfp;
	int i, j,  pid, k, check=0 ;
	DIR *dirp;
	struct dirent *dp;

	int   shm_id;
	void *shm_addr;

	input_idx = atoi(argv[1]);
	shm_key = atoi(argv[2]);

    pid = 0;
    if((pid = fork()) < 0) {
        exit(0);
    }
    else if(pid != 0) {
        exit(0);
    }
    setsid();
	KEY_NUM = 150+(shm_key*13 + input_idx);
	
	
    head = init_head();
	sprintf(tempBuf,"/wndi/temp/index_final/%c", input_index[input_idx][0]);
    get_words_file(tempBuf, head);
	sprintf(tempBuf,"/wndi/temp/index_final/%c", input_index[input_idx][1]);
    get_words_file(tempBuf, head);
	if ( ( shm_id = shmget( (key_t)KEY_NUM, MEM_SIZE, IPC_CREAT|0666)) == -1 )
	{
		printf( "���� �޸� ���� ����\n");
	   return -1;
	}

	if (( shm_addr = shmat( shm_id, ( void *)0, 0)) == ( void *)-1 )
	{
		   printf( "���� �޸� ÷�� ����\n");
		   return -1;
	}
	sprintf((char *)shm_addr, "[ready]");
    while(1){
		if(strlen((char *)shm_addr)>0 && strstr((char *)shm_addr,"[quit]")!=NULL)
		{		
			break;

		}
		if(strlen((char *)shm_addr)>0 && strstr((char *)shm_addr,"[input]")!=NULL){
			sprintf(inputBuffer, (char *)shm_addr);
			sprintf((char *)shm_addr,"");

            search_index(inputBuffer, head);                                              /* �˻��� �Է¸��� ��ȯ */
			while(strstr((char *)shm_addr,"[ok]")==NULL);
			sprintf((char *)shm_addr, "[ready]");

		}
    }
	if ( shmdt( shm_addr) == -1)
	{
	}

    delete_btree(head);                           /* B-tree ���� */
    free(head);
}

