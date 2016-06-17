#define MINIMUM  5
#define MAXIMUM MINIMUM*2+1

typedef struct _Bnode /* B-tree 노드 */
{
	int n;
	struct _Bnode *ptr[MAXIMUM+1];
	struct Bnodekey *key[MAXIMUM];
} Bnode;

typedef struct _Bnodekey /* B-tree Node Key값 */
{
	char *key;
	int count;
	struct Docidx *DocuVector;
} Bnodekey;

typedef struct _Keyvalue /* 색인작업 Key List */
{
	char *key;
	int count;
	struct Docidx *DocuVector;
	struct _Keyvalue *next;
} Keyvalue;

typedef struct _Docidx /* record Index 리스트 */
{
	int	documentNum;
	int	termFreq;
	struct _Docidx *next;
} Docidx;

typedef Keyvalue *kptr; /* Key List head 정보 */

typedef struct _List
{
	kptr head;
} List;

typedef Docidx *dptr; /* record List head 정보 */

typedef struct _Doclist
{
	dptr head;
} Doclist;

