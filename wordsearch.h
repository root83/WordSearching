List* listInfo;
int keyCount;
Bnode *head;

void init();
int _ascii_check(char *input);
void _get_keylist(char *inputString, int docNum, List *base);
void get_words(char *inputFile);
int input_command(char *input, List *andKey, List *orKey);
Doclist *_sort_list(Doclist *base);
void search_prompt(Bnode *base);
void main(int argc, char *argv[]);
 
