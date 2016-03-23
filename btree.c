#include <stdlib.h> 
#include <stdio.h> 
#include "structure.h"
#include "btree.h"

/**************************************
Bnode Key �ʱ�ȭ

�Է°� 
node  : �ʱ�ȭ ��� key ���
key   : keyword
count : key�� �˻��� Ƚ��
docVector : Key�� �˻��� ���� ���� ����

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
node���� key �� ã��

�Է°�
key : keyword
t   : �˻� ��� node
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
B-tree���� Key value ã��

�Է°�
key  : keword
base : B-tree�� root node
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
node�� Key ����
���ĺ� ������ ��ġ ����

t     : ���� ��� node
key   : ���� key ����
left  : key�� ���� �ڽ� node
right : key�� ������ �ڽ� node
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
node�� key�� MAXIMUM���� ������ ���
MINIMUM�� ��������(�߰���) ��带 ����

�Է°�
key    : �����Ϸ��� �ϴ� Key ����
Bnode  : Pivot ���� node
bas    : B-tree�� root node
*************************************/

Bnode *split(Bnodekey *key, Bnode *pivot, Bnode *base) /* ��� ���� */
{
	Bnode *left, *right;
	Bnode *child;
	Bnodekey *k, *k2;
	int i;
	if ((right = (Bnode*)malloc(sizeof(Bnode))) == NULL) 
		return NULL;
	if (pivot == base)						/* root node�� ������ ��� */
	{
		child = pivot->ptr[0];
		if ((left = (Bnode*)malloc(sizeof(Bnode))) == NULL) 
			return NULL;
		for (i = 0; i < MINIMUM; i ++) /* ���� node ���� */
		{
			left->key[i] = child->key[i];
			left->ptr[i] = child->ptr[i];
		}
		left->ptr[i] = child->ptr[i];
		left->n = MINIMUM;
		for (i = MINIMUM + 1; i < MAXIMUM; i ++) /* ���� node ���� */
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
		left->n = MINIMUM; 									/* ���� node�� �̹� �����Ƿ� ���� node�� ���� */
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
������ �߰��� ��� 
������ Key�� ����
���� ������ �߰���

key : node�� �̹� �߰��� Key ����
Bk  : �߰��� ���������� ��� key ����
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
B-tree�� Key, ���� ���� ����

kHead : Key���� Head 
base : B-tree�� root node
Bk : ���Ե� Ű ����
***************************/

Bnode *insert_btree(Keyvalue *kHead, Bnode *base)					
{
	Bnode *t, *parent;
	Bnodekey *Bk, *tk;
	Keyvalue *k;
	int i, ret, insertCheck;
	insertCheck = 0;

	k = kHead;
	if((Bk = init_Bnodekey(Bk, k->key, k->count, k->DocuVector)) == NULL) /* ������ Key ���� �ʱ�ȭ */
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
		if ((ret = in_Bnode(Bk->key, t)) >= 0) { /* ������ key�� ������ ��� */
			
			_insert_documnet(t->key[ret], Bk);
			return parent; 
		}
		if (t->n == MAXIMUM)    /* �ش� node�� key�� �ִ밪��ŭ ����� ��� ���� */
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
			if(strcmp(Bk->key, tk->key) < 0) /* ���ĺ� �������� key�� ������ ����node�� Ž�� */
			{
				break;
			}
		}
		t = t->ptr[i];
	}
	insert_key(parent, Bk, NULL, NULL);		/* ã�� ��忡 Ű ���� */
	
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
B-tree ����
**********/

Bnode *delete_btree(Bnode *base)
{
	_delete(base->ptr[0]);
	base->ptr[0] = NULL;
	return base;
}


/**********
���� test��
B-tree ���
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

