Bnodekey *init_Bnodekey(char* key, int count, Docidx* docVector);
int in_Bnode(char *key, Bnode *t);
Bnodekey *serch_btree(char *key, Bnode *base);
void insert_key(Bnode *t, Bnodekey *key, Bnode *left, Bnode *right);
Bnode *split(Bnodekey *key, Bnode *pivot, Bnode *base) ;
void _insert_documnet(Bnodekey *key, Bnodekey *Bk);
Bnode *insert_btree(Keyvalue *kHead, Bnode *base);
void _delete(Bnode *t);
Bnode *delete_btree(Bnode *base);
void _list(Bnode *t);
void list_btree(Bnode *base);
void _write_list(Bnode *t, char *fileName);
void write_btree(Bnode *base, char *fileName);


