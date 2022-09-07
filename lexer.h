
#ifndef _LEXER_H
    #define _LEXER_H
//防止重复包含该头文件

#include<cstdio>
#include<stdlib.h>
#include<malloc.h>
#include<string.h>

#define MAXLEN 100
#define ToBeContinue printf("ToBeContinue here!\n");

//Hash表的数据元素
enum token_kind
{
    UNDEF = -1,/* 未识别的token */
    ERROR_TOKEN=1,
    IDENT,//标识符
    INT_CONST,//整形常量
    FLOAT_CONST,
    CHAR_CONST,
    STRING_CONST,
    KEYWORD,
    INT,//begin_类型关键字
    FLOAT,
    CHAR,
    LONG,
    SHORT,
    DOUBLE,
    VOID,//end_类型关键字
    IF,
    ELSE,
    DO,
    WHILE,
    FOR,
    STRUCT,
    BREAK,
    SWITCH,
    CASE,
    TYPEDEF,
    RETURN,
    CONTINUE,
    LB,//左大括号
    RB,//右大括号
    LM,//左中括号
    RM,//右中括号
    SEMI,//分号31
    COMMA,//逗号
    /*EQ到MINUSMINUS为运算符，必须连在一起*/
    EQ,//'=='
    NEQ,//‘!=’
    ASSIGN,//'='35
    LP,//左括号
    RP,//右括号
    TIMES,//乘法
    DIVIDE,//除法
    PLUS,//加法40
    PLUSPLUS,//自增运算
    POUND,//井号42
    MORE,//大于号
    LESS,//小于号
    MOREEQUAL,//大于等于
    LESSEQUAL,//小于等于
    MINUS,//减法
    MOD, // 取模运算
    AND, // 逻辑 与 &&
    OR, // 逻辑 或 ||
    BIT_AND, // 按位 与 &
    BIT_OR, // 按位 或 |
    MINUSMINUS,//自减运算
    ANNO,//注释
    INCLUDE,//头文件引用
    MACRO,//宏定义
    ARRAY,//数组
    EndOfFile,
};
//定义枚举类型，使不同类型的单词得到唯一的标识码

extern int cnt_lines;				//行数计数器，记录当前已分析到源代码的第几行
extern char token_text[MAXLEN];		//存放单词值的字符串
int word_analyse(FILE *);

/*
const char* KeyWords[]={
        "struct","break","else",
        "switch","case","typedef",
        "return","continue",
        "for","void","do","if","while"
};//关键字查找表
*/

int gettoken(FILE *fp);

#endif