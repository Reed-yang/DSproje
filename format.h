#ifndef _FORMAT
    #define _FORMAT

typedef struct token{
    char *str;
    struct token *next;
}token;

void format();
token* readline();

#endif 
