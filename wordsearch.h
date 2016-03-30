void init();
Docidx *init_doc(int docNum, int termFreq);
Keyvalue *init_key(char *keyword, Docidx *doc);
int _ascii_check(char *input);
void _get_keylist(char *inputString, int docNum, List *listInfo);
void get_words(char *inputFile, Bnode *base);
int _input_check(char *input);
int input_command(char *input, List *andKey, List *orKey);
Doclist *_sort_list(Doclist *base);
Docidx *_search_document(Doclist *listDoc, Docidx *inputDoc);
int _insert_document(Doclist *listDoc, Docidx *inputDoc);
int _delete_document(Doclist *listDoc, Docidx *inputDoc);
void _or_search(List* inputList, Doclist* resultList, Bnode *base);
int _and_search(List* inputList, Doclist* resultList, Bnode *base);
void search_prompt(Bnode *base);
void main(int argc, char *argv[]);

