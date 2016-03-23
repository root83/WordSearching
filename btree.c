#include <stdlib.h> 
#include <stdio.h> 
#include "structure.h"
#include "btree.h"

/**************************************
Bnode Key 초기화

입력값 
node  : 초기화 대상 key 노드
key   : keyword
count : key가 검색된 횟수
docVector : Key가 검색된 문서 색인 정보

**************************************/

Bnodekey *init_Bnodekey(Bnodekey* node, char* key, int count, Docidx* docVector)
{
	if((node = (Bnodekey*)malloc(sizeof(Bnodekey))) == NULL)
		return NULL;
	node->key = (char*)malloc(sizeof(char)*strlen(key));
	sprintf(node->key, key);
	node->count = count;
	node->DocuVector = docVector;
	return node;
}

/*******************
node에서 key 값 찾기

입력값
key : keyword
t   : 검색 대상 node
*******************/

int in_Bnode(char *key, Bnode *t)
{
	int i;
	Bnodekey *k;
	for (i = 0; i < t->n; i ++)
	{
		k = t->key[i];
		if (!strcmp(key,k->key)) {
			return i;
		}
	}
	return - 1;
}

/************************
B-tree에서 Key value 찾기

입력값
key  : keword
base : B-tree의 root node
************************/

Bnodekey *serch_btree(char *key, Bnode *base)
{
	Bnode *t;
	Bnodekey *k;
	int i, ptr, result, loopCheck;
	t = base->ptr[0];
	while (t != NULL && (ptr = in_Bnode(key, t)) == - 1)
	{
		for (i = 0; i < t->n; i ++)
		{
			k = t->key[i];
			result = strcmp(key, k->key);
			if(result <= 0)
			{
				break;
			}
		}
		t = t->ptr[i];
	}
	if(t == NULL)
		return NULL;	
	k = t->key[ptr];
	return k;
}

/*****************************
node에 Key 삽입
알파벳 순서로 위치 결정

t     : 삽입 대상 node
key   : 삽입 key 정보
left  : key의 왼쪽 자식 node
right : key의 오른쪽 자식 node
*****************************/

void insert_key(Bnode *t, Bnodekey *key, Bnode *left, Bnode *right) 
{
	int i;
	Bnodekey *k;
	i = t->n;
	while (i > 0)
	{
		k = t->key[i - 1];
		if(strcmp(k->key, key->key) <= 0)
			break;
		t->key[i] = t->key[i - 1];
		t->ptr[i + 1] = t->ptr[i];
		i -- ;
	}
	t->n ++ ;
	t->key[i] = key;
	t->ptr[i] = left;
	t->ptr[i + 1] = right;
	
}

/*************************************
node의 key가 MAXIMUM보다 많아질 경우
MINIMUM을 기준으로(중간값) 노드를 분할

입력값
key    : 삽입하려고 하는 Key 정보
Bnode  : Pivot 기준 node
bas    : B-tree의 root node
*************************************/

Bnode *split(Bnodekey *key, Bnode *pivot, Bnode *base) /* 노드 분할 */
{
	Bnode *left, *right;
	Bnode *child;
	Bnodekey *k, *k2;
	int i;
	if ((right = (Bnode*)malloc(sizeof(Bnode))) == NULL) 
		return NULL;
	if (pivot == base)						/* root node를 분할할 경우 */
	{
		child = pivot->ptr[0];
		if ((left = (Bnode*)malloc(sizeof(Bnode))) == NULL) 
			return NULL;
		for (i = 0; i < MINIMUM; i ++) /* 좌측 node 생성 */
		{
			left->key[i] = child->key[i];
			left->ptr[i] = child->ptr[i];
		}
		left->ptr[i] = child->ptr[i];
		left->n = MINIMUM;
		for (i = MINIMUM + 1; i < MAXIMUM; i ++) /* 우측 node 생성 */
		{
			right->key[i - MINIMUM - 1] = child->key[i];
			right->ptr[i - MINIMUM - 1] = child->ptr[i];
		}
		right->ptr[i - MINIMUM - 1] = child->ptr[i];
		right->n = MINIMUM;
		child->n = 0; 
		insert_key(child, child->key[MINIMUM], left, right);
	}
	else
	{
		for (i = 0; i < pivot->n ; i ++)
		{
			k = pivot->key[i];
			if(strcmp(key->key, k->key) < 0)
				break;
		}
		left = pivot->ptr[i];
		left->n = MINIMUM; 									/* 좌측 node는 이미 있으므로 우측 node만 생성 */
		for (i = MINIMUM + 1; i < MAXIMUM; i ++)
		{
			right->key[i - MINIMUM - 1] = left->key[i];
			right->ptr[i - MINIMUM - 1] = left->ptr[i];
		}
		right->ptr[i - MINIMUM - 1] = left->ptr[i];
		right->n = MINIMUM;
		insert_key(pivot, left->key[MINIMUM], left, right); /* pivot update */
		child = pivot;
	}
	return child;
}

/************************************
문서를 추가할 경우 
동일한 Key에 대해
문서 색인을 추가함

key : node에 이미 추가된 Key 정보
Bk  : 추가할 문서색인이 담긴 key 정보
************************************/

void _insert_documnet(Bnodekey *key, Bnodekey *Bk)
{
	Docidx *d, *doc, *buf;
	int docCheck;
	doc = Bk->DocuVector;
	docCheck = 0;
	while(doc != NULL)
	{
		d = key->DocuVector;
		while(d != NULL)
		{
			if(doc->documentNum == d->documentNum)
			{
				docCheck = 1;
				break;
			}
			d = d->next;
		}
		if(docCheck == 0)
		{
			buf = (Docidx*)malloc(sizeof(Docidx));
			buf->documentNum = doc->documentNum;
			buf->termFreq = doc->termFreq;
			buf->next = NULL;
			d = key->DocuVector;
			while(d->next != NULL)
			{
				d = d->next;
			}
			d->next = buf;
		}
		doc = doc->next;
	}
	
}
/***************************
B-tree에 Key, 색인 정보 삽입

kHead : Key값의 Head 
base : B-tree의 root node
Bk : 삽입될 키 정보
***************************/

Bnode *insert_btree(Keyvalue *kHead, Bnode *base)					
{
	Bnode *t, *parent;
	Bnodekey *Bk, *tk;
	Keyvalue *k;
	int i, ret, insertCheck;
	insertCheck = 0;

	k = kHead;
	if((Bk = init_Bnodekey(Bk, k->key, k->count, k->DocuVector)) == NULL) /* 삽입할 Key 정보 초기화 */
	{
		return;
	}
	parent = base;
	t = base->ptr[0];
	if (t == NULL)
	{
		if ((t = (Bnode*)malloc(sizeof(Bnode))) == NULL)
		{
			return NULL;
		}
		t->n = 0;
		t->ptr[0] = NULL;
		base->ptr[0] = t;
	}
	while (t != NULL)
	{
		if ((ret = in_Bnode(Bk->key, t)) >= 0) { /* 동일한 key가 존재할 경우 */
			
			_insert_documnet(t->key[ret], Bk);
			return parent; 
		}
		if (t->n == MAXIMUM)    /* 해당 node에 key가 최대값만큼 저장된 경우 분할 */
		{
			t = split(Bk, parent, base);
			if (t == NULL) {
				return NULL;
			}
		}
		parent = t;
		for (i = 0; i < t->n ; i ++)
		{
			tk = t->key[i];
			if(strcmp(Bk->key, tk->key) < 0) /* 알파벳 기준으로 key를 저장할 리프node를 탐색 */
			{
				break;
			}
		}
		t = t->ptr[i];
	}
	insert_key(parent, Bk, NULL, NULL);		/* 찾은 노드에 키 삽입 */
	
	return parent;
}

void _delete(Bnode *t)
{
	int i;
	if (t != NULL)
	{
		for (i = 0; i < t->n; i ++)
{
			_delete(t->ptr[i]);
		}
		free(t);
	}
}

/**********
B-tree 삭제
**********/

Bnode *delete_btree(Bnode *base)
{
	_delete(base->ptr[0]);
	base->ptr[0] = NULL;
	return base;
}


/**********
개발 test용
B-tree 출력
**********/

void _list(Bnode *t)
{
	int i;
	static int x = 0;
	Bnodekey *k;
	if (t != NULL)
	{
		printf("count :%d\n", t->n);
		x += 2;
		for (i = 2; i < x; i ++) 
			printf(" ");
		for (i = 0; i < t->n; i ++) {
			k = t->key[i];
			printf("%s ", k->key);
		}
		for (i = 0; i < t->n + 1; i ++)
{
			printf("ptr[%d]\t",i);
			_list(t->ptr[i]);
		}
		printf("\n");
		x -= 2;
	}
}
void list_btree(Bnode *base)
{
	_list(base->ptr[0]);
}

