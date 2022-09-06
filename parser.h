#ifndef PARSER_H
    #define PARSER_H

#include "lexer.h"
typedef enum syntax_type{
    EXTDEFLIST = 1/* 外部序列定义 */,
    EXTVARDEF,/* 外部变量定义 */
    EXTVARTYPE,/* 外部变量类型3 */
    EXTVARLIST,//外部变量名序列结点
    EXTVAR,//外部变量名结点5
    FUNCDEF,//函数定义结点
    FUNCRETURNTYPE,//函数返回值类型结点
    FUNCNAME,//函数名结点
    FUNCFORMALPARALIST,//函数形式参数序列结点
    FUNCFORMALPARADEF,//函数形式参数结点10
    FUNCFORMALPARATYPE,//函数形参类型结点
    FUNCFORMALPARA,//函数形参名结点
    FUNCBODY,//函数体结点
    LOCALVARDEFLIST,//局部变量定义序列结点
    LOCALVARDEF,//局部变量定义结点15
    LOCALVARTYPE,//局部变量类型结点
    LOCALVARNAMELIST,//局部变量名序列
    LOCALVARNAME,//局部变量名
    STATELIST,//语句序列结点
    OPERAND,//操作数结点20
    OPERATOR,//运算符结点
    EXPRESSION,//表达式
    IFPART,//if语句部分
    ELSEPART,//else部分
    IFSTATEMENT,//if语句25
    IFELSESTATEMENT,//if-else语句
    WHILESTATEMENT,//while语句结点
    WHILEPART,//while条件语句结点
    WHILEBODY,//while语句体
    FORSTATEMENT,//for语句结点30
    FORPART,//for条件语句
    FORPART1,//for语句条件一
    FORPART2,//for语句条件二
    FORPART3,//for语句条件三
    FORBODY,//for语句体35
    RETURNSTATEMENT,//return语句
    BREAKSTATEMENT,//break语句
    DOWHILESTATEMENT,//do-while循环语句
    DOWHILEBODY,//do-while语句体
    DOWHILECONDITION,//do-while条件40
    CONTINUESTATEMENT,//continue语句
    FUNCCLAIM,//函数声明
    ARRAYDEF,//数组定义
    ARRAYTYPE,//数组类型
    ARRAYNAME,//数组名45
    ARRAYSIZE,//数组大小
    COMSTATE,// 复合语句结点
}syntax_type;

/* 二叉树法构造的抽象语法树，左子右弟 */
typedef struct AST{
    struct AST *l;
    struct AST *r;
    struct data{
        int type;/* 若有data,存入当前结点data的类型 */
        char *data;/* 若有data,存入当前结点data的数据 */
    }data;
    int type;/* 该子树的类型 */
} AST;

/* Variable Name List 变量名列表*/
typedef struct VNL{
    char variable[MAXLEN];
    struct VNL *next;
}VNL;

AST* program();
AST* ExtDefList();
AST* ExtDef();
AST* ExtVarDef();
AST* ArrayDef();
AST* FuncDef();
AST* FormParaList();
AST* ComStatement();
AST* StatementList();
AST* Expression(int end_sign);
AST* Statement();
AST* FormParaDef();
AST* LocalVarDefList();
void returntoken(FILE *fp);
char precede(int c1, int c2);
int add2VNL(char token_text[MAXLEN]);
int free_all(AST* root);
void syntax_analyse();
void PreorderTranverse(AST* root,int depth);
void showType(int type);







#endif