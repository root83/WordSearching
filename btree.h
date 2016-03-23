void init_Btree();
Bnodekey *init_Bnodekey(Bnodekey* node, char* key, int count, Docidx* docVector);
int in_Bnode(char *key, Bnode *t);
Bnodekey* serch_btree(char *key, Bnode *base);
void insert_key(Bnode *t, Bnodekey *key, Bnode *left, Bnode *right);
Bnode* split(Bnodekey *key, Bnode *pivot, Bnode *base);
Bnode* insert_btree(Keyvalue *kHead, Bnode *base);
void _list(Bnode *t);
void Btree_list(Bnode *base);
void _delete(Bnode *t);
Bnode *delete_btree(Bnode *base);

