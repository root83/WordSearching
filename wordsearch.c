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
	keyCount  =  0;
	head  =  (Bnode*)malloc(sizeof(Bnode));
	head->ptr[0]  =  NULL;
	head->n  =  0;
}

/********************
���ĺ�, ���ڸ� ������
Ư������ ����
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
�Էµ� ���ڿ����� Keyword ����

�Է°�
inputString : �Է� ���ڿ�
docNum      : ���� ��ȣ
base				: B-tree ���� �� Key ���� ����Ʈ
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
�Էµ� ���Ͽ��� key���� ����
���ϸ��� �������� ��ȣ
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
		printf("�Է����� �б� ����\n");
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
		if (insert_btree(k,head)  ==  NULL)   /* ����� key�� �� ���� ������ B-tree�� ���� */
			;
		k = k->next;
	}
	
	k = listInfo->head;
	while(k!= NULL)     /* ����Ʈ Free */
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
�Էµ� �˻��� ���ڿ����� �˻� Key ����
�ֵ���ǥ�� �پ��ִ� ���ڿ��� AND �˻�,
�ƴϸ� OR �˻� Keyword�� ����

�Է°�
input  : �Էµ� ���ڿ�
andKey : AND �˻� keyword ����Ʈ head
orKey  : OR �˻� keyword ����Ʈ head
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
		'"' ��ȣ�� �پ� ���� ���
		AND ���� keyword�� �з�
		*************************/
		if(strchr(token,'\"')!= NULL)
		{												/* Ư������ ���� */
			_ascii_check(token);	
			akN = (Keyvalue*)malloc(sizeof(Keyvalue));
			akN->key = (char*)malloc(sizeof(char)*strlen(token));
			sprintf(akN->key, token);
			akN->next = NULL;
			if(totalAndKey == 0)													/* ���� �Էµ� AND �˻� Key�� ���� ��� */
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
		OR ���� keyword �з�
		*******************/
		
		else
		{
			_ascii_check(token);
			okN = (Keyvalue*)malloc(sizeof(Keyvalue));
			okN->key = (char*)malloc(sizeof(char)*strlen(token));
			sprintf(okN->key, token);
			okN->next = NULL;
			if(totalOrKey == 0)														/* ���� �Էµ� OR �˻� Key�� ���� ��� */
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
���� ���� �˻�� ���Ե� ��������
�˻� ����� ����

�Է°�
base : �˻���� ����Ʈ head
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
�˻�� �Է¹ް� B-tree���� key�� �˻��Ͽ�
����� ���
******************************************/
void search_prompt(Bnode *base)
{
	char keyword[1024];
	int i, docCheck, documentListCnt = 0, andCheck, ret;
	int *documentNumList;

	Bnodekey *keyR;	// keyword �˻� ��� ����
	Docidx *d, *doc, *resultDoc, *tempD; 
	Keyvalue *k, *tempK;
	List *inputKeyAndList, *inputKeyOrList;
	Doclist *resultList;

	inputKeyAndList  =  (List*)malloc(sizeof(List));	
	inputKeyOrList  =  (List*)malloc(sizeof(List));	
	
	resultList  =  (Doclist*)malloc(sizeof(Doclist));
	
	while (1)
	{
		printf("\n�˻��� Ű���带 �Է��ϼ��� (����� q!)->");
		
		gets(keyword);
		if(strstr(keyword,"q!")!= NULL)
			break;
		inputKeyAndList->head = NULL;	
		inputKeyOrList->head = NULL;
		resultList->head  =  NULL;
		documentListCnt = 0;
		documentNumList  =  (int*)malloc(sizeof(int)*(documentListCnt+1));
		andCheck = 0;
		if((ret = input_command(keyword, inputKeyAndList, inputKeyOrList))>0) 					/* �Էµ� ������ AND, OR �˻� ���ǿ� ���� ����Ʈ ���� */
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
			OR ���� �˻� keyword ó��
			************************/
			
			k  =  inputKeyOrList->head; 
			while(k!= NULL)
			{
				if((keyR = serch_btree(k->key,base))!= NULL) /* B-tree���� Keyword �˻� */
				{
					d  =  keyR->DocuVector;
					while(d !=  NULL)
					{
						docCheck = 0;
						resultDoc  =  resultList->head;
						while(resultDoc!= NULL)
						{
							if(resultDoc->documentNum  ==  d->documentNum)
							{ /* ������ �˻��� index Ȯ�� */
								docCheck = 1;
								resultDoc->termFreq++;
								break;
							}
							resultDoc  =  resultDoc->next;
						}
						if(docCheck  ==  0)
						{                     /* ���� �˻��� index �߰� */
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
			AND ���� �˻� keyword ó��
			*************************/
			
			k  =  inputKeyAndList->head; 
			while(k!= NULL)
			{
				if((keyR = serch_btree(k->key,base))!= NULL) /* B-tree���� Keyword �˻� */
				{
					resultDoc  =  resultList->head;
					if(resultDoc  ==  NULL)
					{																		/* ���� Index�� �������� ���� ��� */
						d  =  keyR->DocuVector;
						if(documentListCnt>0)								/* AND ���ǿ� ���� �˻� ����� ���ܵǾ� Index�� �������� �ʴ� ��� ó������ ����*/
							;
						else																/* ������ �˻��� ����� ���� ��� */
						{
							while(d !=  NULL)
							{
								docCheck = 0;
								resultDoc  =  resultList->head;
								while(resultDoc!= NULL)
								{
									if(resultDoc->documentNum  ==  d->documentNum)                /* ���� �˻� ����� �ش� ���� ������ ������ ��� �� ���� */
									{
										docCheck = 1;
										resultDoc->termFreq++;
										break;
									}
									resultDoc  =  resultDoc->next;
								}
								if(docCheck  ==  0)																						/* ���� �˻������ �ش� ���� ������ �������� ���� ��� ������ �˻������ �߰� */
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
					else {																												/* OR ���� �˻� ����� ������ ��� */
						d  =  keyR->DocuVector;
						while(d !=  NULL)
						{
							docCheck = 0;
							for(i = 0; i < documentListCnt; i++)
							{
								if(documentNumList[i]  ==  d->documentNum)							/* documentNumList : AND ���ǿ� ���� �˻��� ������� ����Ʈ */
								{
									docCheck = 1;
								}
							}
							if(docCheck == 0)																					/* ���� �˻��� AND ���� ����� �ش� ���������� ���� �� ����Ʈ�� ��� */
							{
								documentNumList[documentListCnt]  =  d->documentNum;
								documentListCnt++;
								documentNumList = (int*)realloc(documentNumList, sizeof(int)*(documentListCnt+1));
							}
							resultDoc  =  resultList->head;
							while(resultDoc!= NULL)
							{
								if(resultDoc->documentNum  ==  d->documentNum)					/* ���� �˻� ����� �ش� ���� ������ ������ ��� �� ���� */
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
								if(documentNumList[i]  ==  resultDoc->documentNum)				/* ��� �˻� ����� �� AND ���� �˻� ��� ���� ������ ���Ե��� �ʴ� ��� �ش� ��� ���� */
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
					andCheck = 1;																	/* AND Keyword �˻������ �����Ƿ� �˻� �ߴ� */
					break;
				}
				k  =  k->next;
			}
			
			if(resultList->head == NULL || andCheck == 1)
			{
				printf("�˻� ����� �����ϴ�\n");
			}
			else
			{
				resultList  =  _sort_list(resultList);
				printf("���� ��ġ/�˻�Ű���� ranking : ");
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
			printf("�˻�� �ٽ� �Է����ּ���.\n");
		}
		else
		{
			printf("�Էµ� �˻�� �����ϴ�.\n");
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
		printf("���� ��ȣ ������ �Է��ϼ���.(���� : q!, �˻� : s!)");
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
	
	delete_btree(head);                           /* B-tree ���� */
	free(head);
}
 
