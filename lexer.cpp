#include "lexer.h"
/*DEBUG: 浮点型常量b = .67e2L;十六进制常量i=0xb21;检查八进制;检查词法分析异常中断*/
char token_text[MAXLEN];
int cnt_lines=1;

/*向token_text末尾添加字符*/
int add_text(char* token,char c);
int word_analyse(FILE *fp);
/*c是字母*/
bool isLetter(char c){
    if( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return true;
    else
        return false;
}

/*c是数字*/
bool isNum(char c){
    if(c >= '0' && c <= '9')
        return true;
    else 
        return false;
}

/*c是字母|数字*/
bool isLorN(char c){
    if(isLetter(c) || isNum(c))
        return true;
    else 
        return false;
}

/*是16进制符*/
bool isXnum(char c){
    if((isNum(c))||(c>='a'&&c<='f')||(c>='A'&&c<='F')){
        return 1;
    } else{
        return 0;
    }
}

/*对源码进行词法分析*/
int gettoken(FILE *fp)
{
    char c = 0;//当前c初始值为0

    /*过滤开头的空白字符*/
    do{
        c=fgetc(fp);
        if(c=='\n'){
            cnt_lines++;
        }
    }while (c==' '||c== 9||c=='\n');
    /*识别关键字*/
/*以字母或下划线开始的单词(也可能是数组)*/
    if(isLetter(c) || c == '_')
    {
        do {
            add_text(token_text, c);
        } while ( isLorN(c = fgetc(fp)) );
        ungetc(c, fp);
        //当读到非数字/字母时停止，即为读完该单词，并退回多读的字符到文件缓冲区
        /*判断是否是某种关键字*/
        if(strcmp(token_text,"int")==0){
            return INT;
        }
        if(strcmp(token_text,"double")==0){
            return DOUBLE;
        }
        if(strcmp(token_text,"char")==0){
            return CHAR;
        }
        if(strcmp(token_text,"short")==0){
            return SHORT;
        }
        if(strcmp(token_text,"long")==0){
            return LONG;
        }
        if(strcmp(token_text,"float")==0){
            return FLOAT;
        }
        if(strcmp(token_text,"if")==0){
            return IF;
        }
        if(strcmp(token_text,"else")==0){
            return ELSE;
        }
        if(strcmp(token_text,"do")==0){
            return DO;
        }
        if(strcmp(token_text,"while")==0){
            return WHILE;
        }
        if(strcmp(token_text,"for")==0){
            return FOR;
        }
        if(strcmp(token_text,"struct")==0){
            return STRUCT;
        }
        if(strcmp(token_text,"break")==0){
            return BREAK;
        }
        if(strcmp(token_text,"switch")==0){
            return SWITCH;
        }
        if(strcmp(token_text,"case")==0){
            return CASE;
        }
        if(strcmp(token_text,"typedef")==0){
            return TYPEDEF;
        }
        if(strcmp(token_text,"return")==0){
            return RETURN;
        }
        if(strcmp(token_text,"continue")==0){
            return CONTINUE;
        }
        if(strcmp(token_text,"void")==0){
            return VOID;
        }
        /*若不是关键字，判断是否为数组*/
        //数组名为标识符+[],例如: example[10]
        if((c = fgetc(fp)) == '['){
            //识别数组
            add_text(token_text,c);
            c=fgetc(fp);
            while(c>='0'&&c<='9'){
                add_text(token_text,c);//直接将数组中的数字作为字符传入token_text
                //add_text(string_num,c); 作用存疑
                c=fgetc(fp);
            }
            if(c!=']'){
                return ERROR_TOKEN;
            }
            add_text(token_text,c);
            return ARRAY;
        }else{//若标识符后不跟中括号，该单词即为标识符
            ungetc(c, fp);
            return IDENT;
        }
    }
/*以数字起始的常量：整型与浮点*/
    if (isNum(c))
    {
        do
        {
            add_text(token_text, c);
        } while (isNum(c = fgetc(fp)));
        if((token_text[0] == '0') && ((c == 'x') || (c == 'X'))){
            do
            {
                add_text(token_text, c);
            } while (isXnum(c = fgetc(fp)));
            return INT_CONST; // 16进制整形常量
        }
        if (c != '.' && c != 'u' && c != 'l')
        {
            if (c != ' ' && c != ';' && c != ')' && c != '+' && c != '-' && c != '*' && c != '/')
            {
                return ERROR_TOKEN;
            }
            ungetc(c, fp);
            return INT_CONST;
        }
        else if (c == '.')
        {
            c = fgetc(fp);
            if ( (c == 'f') || (c == 'F') )
            {
                add_text(token_text, '.');
                add_text(token_text, c);
                return FLOAT_CONST;
            }else if(!isNum(c)){
                return ERROR_TOKEN;
            }
            //数字.非数字 为非法数字常量
            else
            {
                ungetc(c, fp);
                c = '.';
                add_text(token_text, c);
                c = fgetc(fp);
                do
                {
                    add_text(token_text, c);
                } while (isNum(c = fgetc(fp)));
                if ( (c == 'f') || (c == 'F') )
                {
                    add_text(token_text, c);
                    return FLOAT_CONST;
                }//e.g. 1.23f
                else if( (c == 'e') || (c == 'E') ){
                    do{
                        add_text(token_text, c);
                    } while (isNum(c = fgetc(fp)));
                    if( (c == 'l') || (c == 'L') ){
                        add_text(token_text, c);
                        return FLOAT_CONST;
                    } // e.g. 11.67e2L
                    else if (c != ' ' && c != ';' && c != ')' && c != '+' && c != '-' && c != '*' && c != '/' && c != '\t')
                    {
                        return ERROR_TOKEN;
                    }
                    ungetc(c, fp);
                    return FLOAT_CONST;
                }
                else if(c != ' ' && c != ';' && c != ')' && c != '+' && c != '-' && c != '*' && c != '/' && c != '\t')
                {
                    return ERROR_TOKEN;
                }else{
                    ungetc(c, fp);
                    return FLOAT_CONST;
                }//e.g. 1.23
                
            }
        }
        else if (c == 'u')
        {
            add_text(token_text, c);
            c = fgetc(fp);
            if (c == 'l')
            {
                add_text(token_text, c);
                c = fgetc(fp);
                if (c == 'l')
                {
                    add_text(token_text, c);
                    return INT_CONST;
                }
                else
                {
                    ungetc(c, fp);
                    return INT_CONST;
                }
            }
            else
            {
                ungetc(c, fp);
                return INT_CONST;
            }
        }
        else if (c == 'l')
        {
            add_text(token_text, c);
            return INT_CONST;
        }
        else
        {
            return ERROR_TOKEN;
        }
    }
/*以小数点开头，浮点常量*/
    if (c == '.')
    {
        do
        {
            add_text(token_text, c);
        } while (isNum(c = fgetc(fp)));
        if ( (c == 'f') || (c == 'F') )
        {
            add_text(token_text, c);
            return FLOAT_CONST;
        }//e.g. 1.23f
        else if( (c == 'e') || (c == 'E') ){
            do{
                add_text(token_text, c);
            } while (isNum(c = fgetc(fp)));
            if( (c == 'l') || (c == 'L') ){
                add_text(token_text, c);
                return FLOAT_CONST;
            } // e.g. 11.67e2L
            else if (c != ' ' && c != ';' && c != ')' && c != '+' && c != '-' && c != '*' && c != '/' && c != '\t')
            {
                return ERROR_TOKEN;
            }
            ungetc(c, fp);
            return FLOAT_CONST;
        }
        else if((c == 'l') || (c == 'L')){
            add_text(token_text, c);
            c = fgetc(fp);
            if (c != ' ' && c != ';' && c != ')' && c != '+' && c != '-' && c != '*' && c != '/' && c != '\t')
            {
                return ERROR_TOKEN;
            }
            ungetc(c, fp);
            return FLOAT_CONST;
        }
        else if(c != ' ' && c != ';' && c != ')' && c != '+' && c != '-' && c != '*' && c != '/' && c != '\t')
        {
            return ERROR_TOKEN;
        }else{
            ungetc(c, fp);
            return FLOAT_CONST;
        }//e.g. .23
    }
/*识别头文件引用和宏定义*/
    if (c == '#')
    {
        add_text(token_text, c);
        if (isLetter(c = fgetc(fp)))
        {
            do
            {
                add_text(token_text, c);
            } while (isLetter(c = fgetc(fp)));
            if (strcmp(token_text, "#include") == 0)
            {
                do
                {
                    add_text(token_text, c);
                } while ((c = fgetc(fp)) != '\n');/* 读取'#include'所在改行全部内容 */
                ungetc(c, fp);
                return INCLUDE;
            }
            else if (strcmp(token_text, "#define") == 0)
            {
                do
                {
                    add_text(token_text, c);
                } while ((c = fgetc(fp)) != '\n');
                return MACRO;
            }
            else
            {
                return ERROR_TOKEN;
            }
        }
        else
        {
            return ERROR_TOKEN;
        }
    }
/*识别字符串常量*/
    if (c == '"')
    {
        do
        {
            if (c != '\\')
                add_text(token_text, c);
            if (c == '\\')
            {
                c = fgetc(fp);
                add_text(token_text, c);
            }
        } while ((c = fgetc(fp)) != '"' && c != '\n');
        if (c == '"')
        {
            add_text(token_text, '"');
            return STRING_CONST;
        }
        else
        {
            return ERROR_TOKEN;
        }
    }
/*识别字符型常量*/
    if (c == '\'')
    {
        add_text(token_text, '\'');
        if ((c = fgetc(fp)) != '\\')
        {
            add_text(token_text, c);
            if ((c = fgetc(fp)) == '\'')
            {
                add_text(token_text, c);
                return CHAR_CONST;
            }
            else
            {
                return ERROR_TOKEN;
            }
        }
        else
        {
            /*转义字符*/
            add_text(token_text, '\\');
            c = fgetc(fp);
            if (c == 'n' || c == 't' || c == '\\' || c == '\'' || c == '\"')
            {
                /*普通转义字符*/
                add_text(token_text, c);
                if ((c = fgetc(fp)) == '\'')
                {
                    add_text(token_text, c);
                    return CHAR_CONST;
                }
                else
                {
                    return ERROR_TOKEN;
                }
            }
            else if (c == 'x')
            {
                /*十六进制数转义字符*/
                add_text(token_text, c);
                if (isXnum((c = fgetc(fp))))
                {
                    add_text(token_text, c);
                    if (isXnum((c = fgetc(fp))))
                    {
                        add_text(token_text, c);
                    }
                    else
                    {
                        ungetc(c, fp);
                    }
                    if ((c = fgetc(fp)) == '\'')
                    {
                        add_text(token_text, '\'');
                        return CHAR_CONST;
                    }
                    else
                    {
                        return ERROR_TOKEN;
                    }
                }
                else
                {
                    return ERROR_TOKEN;
                }
            }
            else if (c >= '0' && c <= '7')
            {
                /*八进制数转义字符*/
                add_text(token_text, c);
                if ((c = fgetc(fp)) >= '0' && c <= '7')
                {
                    add_text(token_text, c);
                    if ((c = fgetc(fp)) >= '0' && c <= '7')
                    {
                        add_text(token_text, c);
                        if ((c = fgetc(fp)) == '\'')
                        {
                            add_text(token_text, '\'');
                            return CHAR_CONST;
                        }
                        else
                        {
                            return ERROR_TOKEN;
                        }
                    }
                    else if (c == '\'')
                    {
                        add_text(token_text, '\'');
                        return CHAR_CONST;
                    }
                    else
                    {
                        return ERROR_TOKEN;
                    }
                }
                else
                {
                    if (c == '\'')
                    {
                        add_text(token_text, c);
                        return CHAR_CONST;
                    }
                    else
                    {
                        ungetc(c, fp);
                        return ERROR_TOKEN;
                    }
                }
            }
            else
            {
                return ERROR_TOKEN;
            }
        }
    }
/*识别除号与注释*/
    if (c == '/')
    {
        add_text(token_text, c);
        if ((c = fgetc(fp)) == '/')
        {
            do
            {
                add_text(token_text, c);
            } while ((c = fgetc(fp)) != '\n');
            ungetc(c, fp);
            return ANNO;
        }
        else if (c == '*')
        {
            while (1)
            {
                add_text(token_text, c);
                c = fgetc(fp);
                if (c == '*')
                {
                    add_text(token_text, c);
                    if ((c = fgetc(fp)) == '/')
                    {
                        add_text(token_text, c);
                        return ANNO;
                    }
                }
                if (c == '\n')
                {
                    add_text(token_text, c);
                    c = '\t';
                    add_text(token_text, c);
                    add_text(token_text, c);
                }
            }
        }
        else
        {
            ungetc(c, fp);
            return DIVIDE;
        }
    }
/*识别运算符和定界符*/
    switch (c){
        case ',':
            add_text(token_text,c);
            return COMMA;
        case ';':
            add_text(token_text,c);
            return SEMI;
        case '=':
            c=fgetc(fp);
            if(c=='='){
                add_text(token_text,c);
                add_text(token_text,c);
                return EQ;
            }
            ungetc(c,fp);
            add_text(token_text,'=');
            return ASSIGN;
        case '!':
            c=fgetc(fp);
            if(c=='='){
                add_text(token_text,'!');
                add_text(token_text,'=');
                return NEQ;
            } else{
                return ERROR_TOKEN;
            }
        case '+':
            c=fgetc(fp);
            if(c=='+'){
                add_text(token_text,c);
                add_text(token_text,c);
                return PLUSPLUS;
            }
            ungetc(c,fp);
            add_text(token_text,'+');
            return PLUS;
        case '-':
            c=fgetc(fp);
            if(c=='-'){
                add_text(token_text,c);
                add_text(token_text,c);
                return MINUSMINUS;
            }
            ungetc(c,fp);
            add_text(token_text,'-');
            return MINUS;
        case '(':
            add_text(token_text,c);
            return LP;
        case ')':
            add_text(token_text,c);
            return RP;
        case '{':
            add_text(token_text,c);
            return LB;
        case '}':
            add_text(token_text,c);
            return RB;
        case '[':
            add_text(token_text,c);
            return LM;
        case ']':
            add_text(token_text,c);
            return RM;
        case '*':
            add_text(token_text,c);
            return TIMES;
        case '>':
            add_text(token_text,c);
            if((c=fgetc(fp))=='='){
                add_text(token_text,c);
                return MOREEQUAL;
            } else{
                ungetc(c,fp);
                return MORE;
            }
        case '<':
            add_text(token_text,c);
            if((c=fgetc(fp))=='='){
                add_text(token_text,c);
                return LESSEQUAL;
            } else{
                ungetc(c,fp);
                return LESS;
            }
        case '%':
            add_text(token_text,c);
            return MOD;
        case '&':
            add_text(token_text,c);
            if((c=fgetc(fp)) == '&'){
                add_text(token_text,c);
                return AND;
            } else{
                ungetc(c,fp);
                return BIT_AND;
            }
        case '|':
            add_text(token_text,c);
            if((c=fgetc(fp)) == '|'){
                add_text(token_text,c);
                return OR;
            } else{
                ungetc(c,fp);
                return BIT_OR;
            }
    }

/* EOF */
    if(c == EOF)
        return EndOfFile;
/* fgetc所得c为未定义 */
    add_text(token_text, c);
    return UNDEF/* 未识别的token */;
}

/* 向token_text串尾添加字符c */
int add_text(char *token_text, char c)
{
    int i = 0;
    while (*(token_text + i) != '\0')
    {
        i++;
    }
    /*读取的单词长度不能超过MAXLEN*/
    if (i >= MAXLEN - 2)
    {
        return -1; //添加失败
    }
    *(token_text + i) = c;
    *(token_text + i + 1) = '\0';
    return 1;
}


int word_analyse(FILE *fp) {
    int kind;
    if(fp==NULL){
        printf("文件打开失败\n");
    } else{
        printf("文件打开成功\n");
    }
    printf("\n");
    printf("  单词类别");
    printf("\t单词值\n");
    do {
        kind=gettoken(fp);
        if(kind!=ERROR_TOKEN) {
            switch (kind) {
                case IDENT:
                    printf("   标识符");
                    break;
                case INT_CONST:
                    printf("  整型常量");
                    break;
                case FLOAT_CONST:
                    printf("浮点型常量");
                    break;
                case CHAR_CONST:
                    printf("  字符常量");
                    break;
                case STRING_CONST:
                    printf("字符串常量");
                    break;
                case KEYWORD:
                    printf("   关键字");
                    break;
                case INT:
                    printf("类型关键字");
                    break;
                case DOUBLE:
                    printf("类型关键字");
                    break;
                case FLOAT:
                    printf("类型关键字");
                    break;
                case CHAR:
                    printf("类型关键字");
                    break;
                case SHORT:
                    printf("类型关键字");
                    break;
                case LONG:
                    printf("类型关键字");
                    break;
                case IF:
                    printf("   关键字");
                    break;
                case ELSE:
                    printf("   关键字");
                    break;
                case DO:
                    printf("   关键字");
                    break;
                case WHILE:
                    printf("   关键字");
                    break;
                case FOR:
                    printf("   关键字");
                    break;
                case STRUCT:
                    printf("   关键字");
                    break;
                case BREAK:
                    printf("   关键字");
                    break;
                case SWITCH:
                    printf("   关键字");
                    break;
                case CASE:
                    printf("   关键字");
                    break;
                case TYPEDEF:
                    printf("   关键字");
                    break;
                case RETURN:
                    printf("   关键字");
                    break;
                case CONTINUE:
                    printf("   关键字");
                    break;
                case VOID:
                    printf("   关键字");
                    break;
                case EQ:
                    printf("   等于号");
                    break;
                case NEQ:
                    printf("  不等于号");
                    break;
                case ASSIGN:
                    printf("   赋值号");
                    break;
                case LP:
                    printf("   左括号");
                    break;
                case RP:
                    printf("   右括号");
                    break;
                case LB:
                    printf("  左大括号");
                    break;
                case RB:
                    printf("  右大括号");
                    break;
                case LM:
                    printf("  左中括号");
                    break;
                case RM:
                    printf("  右中括号");
                    break;
                case SEMI:
                    printf("     分号");
                    break;
                case COMMA:
                    printf("     逗号");
                    break;
                case TIMES:
                    printf("     乘号");
                    break;
                case DIVIDE:
                    printf("     除号");
                    break;
                case ANNO:
                    printf("     注释");
                    break;
                case PLUS:
                    printf("     加法");
                    break;
                case PLUSPLUS:
                    printf("  自增运算");
                    break;
                case MINUS:
                    printf("     减法");
                    break;
                case MOD:
                    printf("     取模");
                    break;
                case AND:
                    printf("   逻辑与");
                    break;
                case OR:
                    printf("   逻辑或");
                    break;
                case BIT_AND:
                    printf("   按位与");
                    break;
                case BIT_OR:
                    printf("   按位或");
                    break;
                case MINUSMINUS:
                    printf("  自减运算");
                    break;
                case MORE:
                    printf("   大于号");
                    break;
                case MOREEQUAL:
                    printf("大于等于号");
                    break;
                case LESS:
                    printf("   小于号");
                    break;
                case LESSEQUAL:
                    printf("小于等于号");
                    break;
                case INCLUDE:
                    printf("头文件引用");
                    break;
                case MACRO:
                    printf("   宏定义");
                    break;
                case ARRAY:
                    printf("     数组");
                    break;
                case UNDEF:
                    printf("        未定义");
                    break;
                case EndOfFile:
                    printf("    文档结束");
                    break;
            }
            printf("\t%s\n", token_text);
            memset(token_text, 0, sizeof(token_text));
        } else{
            printf("\t第%d行出现错误\n",cnt_lines);
            break;
        }
    } while (kind!=EndOfFile);

    return 0;
}