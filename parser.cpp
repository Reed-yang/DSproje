#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <stack>
#include "parser.h"
#include "lexer.h"

using namespace std;

extern char token_text[MAXLEN];//存放自身单词值
//extern char string_num[20];
extern int cnt_lines;
int w,type;//全局变量，存放当前读入的单词种类编码
bool mistake = false /* 全局，出错为true */, in_recycle = false /* 在循环体内时为true */;
extern FILE* fp;
VNL head;//变量名链表根节点，head为空

/* 生成一棵语法树，根指针指向一个外部定义序列的结点 */
AST* program()
{
    printf("Enter program!\n");
    w = gettoken(fp);
    while(w == INCLUDE || w == ANNO){
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
    }
    AST *p = ExtDefList();
    if( p ){
        // 程序语法正确，返回的语法树根结点指针，可遍历显示
        AST* root = p;
        root->type = EXTDEFLIST;
        return root;
    }
    else{
        // 有语法错误
        mistake = true;
        return NULL;
    }
}

/* 递归定义，该子程序处理一系列的外部定义，每个外部定义序列的结点，其第一个子树对应一个外部定义，第二棵子树对应后续的外部定义 */
AST* ExtDefList()
{
    // printf("Enter ExtDefList!\n");
    if(mistake || w == EndOfFile)
        return NULL;
    AST *root = (AST*)malloc(sizeof(AST));
    root->data.data=NULL;
    root->type = EXTDEFLIST;
    root->l = ExtDef();
    memset(token_text, 0, sizeof(token_text));
    w = gettoken(fp);
    while(w == INCLUDE || w == ANNO)
        w = gettoken(fp);
    root->r = ExtDefList();
    return root;
}

/* 实现<外部定义>，并返回一个<外部定义>结点的指针 */
AST* ExtDef()
{
    if(mistake)
        return NULL;
    //调用前已经读入了一个token
    if( !(w >= INT && w<= VOID))/* 若w不是类型关键字 */
    {
        printf("第%d行出现错误\n",cnt_lines);
        printf("错误：外部定义出现错误\n");
        mistake = true;
        return NULL;
    }
    type = w;/* 保存类型说明符至type */
    memset(token_text, 0, sizeof(token_text));
    w = gettoken(fp);
    while (w == ANNO || w == INCLUDE) /* 跳过注释 */
        w = gettoken(fp);
    if(w != IDENT && w != ARRAY ){
        printf("第%d行出现错误\n",cnt_lines);
        printf("错误：外部定义出错\n");
        mistake = true;
        return NULL;
    }
    int ident_type = w;
    char token_text0[MAXLEN];
    strcpy(token_text0,token_text);//保存第一个变量名或者函数名到token_text0, 防止在读取下一token时当前text丢失
    AST *root_def;
    //root->data.data = NULL;
    w = gettoken(fp);
    while (w == ANNO || w == INCLUDE)
        w = gettoken(fp);
    strcpy(token_text,token_text0);/* 已读取下一token, 重新载入之前的text */
    if (w == LP)
        root_def = FuncDef();
    else if (ident_type == ARRAY)//数组定义，读完了整个数组读到了分号
        root_def = ArrayDef();
    else
        root_def = ExtVarDef();
    return root_def;
}

/* 外部变量定义 */
AST* ExtVarDef()
{
    if(mistake)
        return NULL;
    if(type == VOID){
        //外部变量类型不能是void
        printf("第%d行出现错误\n",cnt_lines);
        printf("错误：外部变量类型不能是void\n");
        mistake = true;
        return NULL;
    }
    if(add2VNL(token_text)){
        /* add2VNL报错 */
        mistake = true;
        return NULL;
    }
    AST *def=(AST*)malloc(sizeof(AST));/* 生成外部变量定义结点 *//* 生成外部变量类型结点 */
    def->l=NULL;
    def->r=NULL;
    def->data.data=NULL;
    def->type=EXTVARDEF;
    AST *var_type=(AST*)malloc(sizeof(AST));
    var_type->r=NULL;
    var_type->l=NULL;
    var_type->type = EXTVARTYPE;
    var_type->data.type = type;
    if(type==INT){
        var_type->data.data=(char*)malloc(MAXLEN*sizeof(char));
        strcpy(var_type->data.data, "int");
    }else if(type==DOUBLE){
        var_type->data.data=(char*)malloc(MAXLEN*sizeof(char));
        strcpy(var_type->data.data, "double");
    }else if(type==CHAR){
        var_type->data.data=(char*)malloc(MAXLEN*sizeof(char));
        strcpy(var_type->data.data, "char");
    }else if(type==FLOAT){
        var_type->data.data=(char*)malloc(MAXLEN*sizeof(char));
        strcpy(var_type->data.data, "float");
    }else if(type==LONG){
        var_type->data.data=(char*)malloc(MAXLEN*sizeof(char));
        strcpy(var_type->data.data, "long");
    }else if(type==SHORT){
        var_type->data.data=(char*)malloc(MAXLEN*sizeof(char));
        strcpy(var_type->data.data, "short");
    }
    def->l = var_type;/* 根据已读入的外部变量的类型，生成外部变量类型结点/var_type，作为def的第一个孩子 */
    /* 生成外部变量名序列结点 */
    AST *var_list = (AST *)malloc(sizeof(AST));
    var_list->l = NULL;
    var_list->r = NULL;
    var_list->data.data = NULL;
    var_list->type = EXTVARLIST;
    def->r = var_list;/* 生成外部变量序列结点/var_list，var_list作为def的第二个孩子，每个外部变量序列结点会对应一个变量名 */
    /* 生成外部变量名结点 */
    AST *var_name = (AST *)malloc(sizeof(AST));
    var_name->l = NULL;
    var_name->r = NULL;
    var_name->data.data = NULL;
    var_name->type = EXTVAR;
    var_list->l = var_name;
    char *token_text1 = (char*)malloc(sizeof(token_text)); /* 存当前变量名  */
    strcpy(token_text1, token_text); 
    var_name->data.data = token_text1;
    //w = gettoken(fp);
    while(1)/* 每个外部变量序列结点的第一个孩子对应一个变量 */
    {
        if(w != COMMA && w != SEMI){
            free_all(def);
            printf("第%d行出现错误\n",cnt_lines);//%
            printf("错误：外部变量定义处出错\n");
            mistake = true;
            return NULL;
        }
        if(w == SEMI){
            //??要不要多读一个词w = gettoken(fp);
            // 进入下一个定义序列前先清空分号及之前的token_text
            memset(token_text, 0, sizeof(token_text));
            return def;
        }
        /* w为COMMA的情形，继续读取变量 */
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        if(w != IDENT){
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：外部变量定义处出错\n");
            mistake = true;
            return NULL;
        }
        if(add2VNL(token_text)){
            mistake = true;
            return NULL;
        }
        /* 递归读入所有变量至SEMMI */
        AST *sub_list = (AST *)malloc(sizeof(AST));
        sub_list->l = NULL;
        sub_list->r = NULL;
        sub_list->data.data = NULL;
        sub_list->type = EXTVARLIST;
        var_list->r = sub_list;
        var_list = sub_list;
        AST *var_name = (AST *)malloc(sizeof(AST));
        var_name->l = NULL;
        var_name->r = NULL;
        var_name->data.data = NULL;
        var_name->type = EXTVAR;
        var_list->l = var_name;
        char *token_text1 = (char*)malloc(sizeof(token_text));/* 存当前变量名 */
        strcpy(token_text1, token_text);
        var_name->data.data = token_text1;
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
    }
}

/* 函数定义 *///若函数返回值为空,报错并return NULL
AST* FuncDef()
{
    if(mistake)
        return NULL;
    // 生成函数定义节点
    AST *func_def = (AST*)malloc(sizeof(AST));
    func_def->data.data = NULL;
    func_def->l = NULL;
    func_def->r = NULL;
    func_def->type = FUNCDEF;
    // 生成返回值类型节点
    AST *func_returntype = (AST*)malloc(sizeof(AST));
    func_def->l = func_returntype;
    func_returntype->l = NULL;
    func_returntype->r = NULL;//返回值类型节点无子树
    func_returntype->type = FUNCRETURNTYPE;
    func_returntype->data.data = (char*)malloc(MAXLEN*sizeof(char));
    if(type==INT){
        strcpy(func_returntype->data.data, "int");
    }if(type==DOUBLE){
        strcpy(func_returntype->data.data, "double");
    }if(type==CHAR){
        strcpy(func_returntype->data.data, "char");
    }if(type==FLOAT){
        strcpy(func_returntype->data.data, "float");
    }if(type==LONG){
        strcpy(func_returntype->data.data, "long");
    }if(type==SHORT){
        strcpy(func_returntype->data.data, "short");
    }if(type==VOID){
        printf("第%d行出现错误\n",cnt_lines);
        printf("错误：函数缺少返回值\n");
        return NULL;
    }
    // 函数名节点
    AST* func_name = (AST*)malloc(sizeof(AST));
    func_name->l = NULL;
    func_name->r = NULL;
    func_name->type = FUNCNAME;
    func_name->data.data = (char*)malloc(MAXLEN*sizeof(char));
    // 此前应已经先读取了函数名
    strcpy(func_name->data.data, token_text);
    add2VNL(func_name->data.data);

    func_def->r = func_name;
    func_name->l = FormParaList();//func_name左子树是形参序列
    // 判断函数定义的类型：函数原型/函数体
    w = gettoken(fp);
    while (w==ANNO||w==INCLUDE)
        w = gettoken(fp);
    if(w == SEMI){
        // 函数原型声明
        func_name->r = NULL;
        func_def->type = FUNCCLAIM;
    }else if(w == LB){
        // 函数体声明
        func_name->r = ComStatement();
        func_name->type = FUNCBODY;
    }else{
        mistake = true;
        printf("第%d行出现错误\n",cnt_lines);
        printf("错误：函数定义处出错\n");
        return NULL;
    }
    return func_def;
}

/* 类型说明符 */

/* 变量序列 */

/* 形式参数序列 */
AST* FormParaList()
{
    // 进入函数前，已经读取了左括号(
    if(mistake)
        return NULL;
    memset(token_text, 0, sizeof(token_text));
    w = gettoken(fp);
    while (w == ANNO || w == INCLUDE)
        w = gettoken(fp);
    if (w == RP)
    {
        return NULL;
    }
    if (w == COMMA)
    {// 递归调用时若是COMMA可以继续读取下一参数
        w = gettoken(fp);
        while (w == ANNO || w == INCLUDE)
            w = gettoken(fp);
    }
    AST* form_para_list = (AST*)malloc(sizeof(AST));
    form_para_list->data.data = NULL;
    form_para_list->type = FUNCFORMALPARALIST;
    form_para_list->l = FormParaDef();
    form_para_list->r = FormParaList();
    return form_para_list;
}

/* 形参定义，在FormParaList中调用 */
AST* FormParaDef()
{
    if(mistake)
        return NULL;
    //调用前已经读入了一个token
    if( !(w >= INT && w<= VOID))/* 若w不是类型关键字 */
    {
        printf("第%d行出现错误\n",cnt_lines);
        printf("错误：形参定义出错\n");
        mistake = true;
        return NULL;
    }
    type = w;/* 保存类型说明符至type */
    memset(token_text, 0, sizeof(token_text));
    w = gettoken(fp);
    while (w == ANNO || w == INCLUDE) /* 跳过注释 */
        w = gettoken(fp);
    if(w != IDENT && w != ARRAY ){
        printf("第%d行出现错误\n",cnt_lines);
        printf("错误：形参定义出错\n");
        mistake = true;
        return NULL;
    }
    // 生成函数形参定义节点
    AST* para_def = (AST*)malloc(sizeof(AST));
    para_def->data.data = NULL;
    para_def->type = FUNCFORMALPARADEF;
    // 生成函数形参类型节点
    AST* para_type = (AST*)malloc(sizeof(AST));
    para_def->l = para_type;
    para_type->l = NULL;
    para_type->r = NULL;
    para_type->type = FUNCFORMALPARATYPE;
    para_type->data.type = type;
    para_type->data.data = (char*)malloc(MAXLEN*sizeof(char));
    if(type==INT){
        strcpy(para_type->data.data, "int");
    }if(type==DOUBLE){
        strcpy(para_type->data.data, "double");
    }if(type==CHAR){
        strcpy(para_type->data.data, "char");
    }if(type==FLOAT){
        strcpy(para_type->data.data, "float");
    }if(type==LONG){
        strcpy(para_type->data.data, "long");
    }if(type==SHORT){
        strcpy(para_type->data.data, "short");
    }if(type==VOID){
        mistake = true;
        printf("第%d行出现错误\n",cnt_lines);
        printf("错误：形参定义不能为VOID\n");
        return NULL;
    }
    if(add2VNL(token_text))
    {
        mistake = true;
        return NULL;
    }
    // 生成形参名节点
    AST *para_name = (AST*)malloc(sizeof(AST));
    para_name->l = NULL;
    para_name->r = NULL;
    para_def->r = para_name;
    para_name->type = FUNCFORMALPARA;
    para_name->data.data = (char*)malloc(MAXLEN*sizeof(char));
    strcpy(para_name->data.data, token_text);

    return para_def;

}

/* 复合语句 *///复合语句处理两个大括号内的一系列语句{...State...}
AST* ComStatement()
{
    if(mistake)
        return NULL;
    AST* com_statement = (AST*)malloc(sizeof(AST));
    com_statement->type = COMSTATE;
    com_statement->data.data = NULL;
    com_statement->l =NULL;
    com_statement->r =NULL;
    memset(token_text, 0, sizeof(token_text));
    w = gettoken(fp);
    while(w==ANNO||w==INCLUDE)
        w = gettoken(fp);
    if(w >= INT && w <= DOUBLE){
        com_statement->l = LocalVarDefList();
    }else{
        // 无局部变量
        com_statement->l = NULL;
    }
    com_statement->r = StatementList();
    if(w == RB /* || w == EndOfFile */)
        return com_statement;
    else{
        mistake = true;
        printf("错误：复合语句语法出错，没有“}”\n");
        return NULL;
    }
}

/* 局部变量定义序列 */// 此函数退出时读取了下一个单词
AST* LocalVarDefList()
{
    if(mistake)
        return NULL;
    // 生成局部变量定义序列结点
    AST* local_vardeflist = (AST*)malloc(sizeof(AST));
    local_vardeflist->data.data = NULL;
    local_vardeflist->type = LOCALVARDEFLIST;
    local_vardeflist->l = NULL;
    local_vardeflist->r = NULL;
    // 生成局部变量定义结点
    AST* local_vardef = (AST*)malloc(sizeof(AST));
    local_vardeflist->l = local_vardef;
    local_vardef->data.data = NULL;
    local_vardef->type = LOCALVARDEF;
    local_vardef->l = NULL;
    local_vardef->r = NULL;
    // 生成局部变量类型结点
    AST* local_vartype = (AST*)malloc(sizeof(AST));
    local_vardef->l = local_vartype;
    local_vartype->data.data = (char*)malloc(MAXLEN*sizeof(char));
    local_vartype->type = LOCALVARTYPE;
    local_vartype->l = NULL;
    local_vartype->r = NULL;
    // w = gettoken(fp);
    // while(w==ANNO||w==INCLUDE)
    //     w = gettoken(fp);
    // 此前已经读入了一个变量名
    strcpy(local_vartype->data.data, token_text);
    // 生成变量名序列结点
    AST* local_varnamelist = (AST*)malloc(sizeof(AST));
    local_vardef->r = local_varnamelist;
    local_varnamelist->type = LOCALVARNAMELIST;
    local_varnamelist->l = NULL;
    local_varnamelist->r = NULL;
    local_varnamelist->data.data = NULL;
    // 生成局部变量名结点
    AST* local_varname = (AST*)malloc(sizeof(AST));
    local_varnamelist->l = local_varname;
    local_varname->type = LOCALVARNAME;
    local_varname->data.data = (char*)malloc(MAXLEN*sizeof(char));
    local_varname->l = NULL;
    local_varname->r = NULL;
    memset(token_text, 0, sizeof(token_text));
    w = gettoken(fp);
    while(w==ANNO||w==INCLUDE)
        w = gettoken(fp);
    strcpy(local_varname->data.data, token_text);
    if(add2VNL(token_text)){
        mistake = true;
        return NULL;
    }
    while(1)
    {
        // 递归生成局部变量定义序列
        w = gettoken(fp);
        while(w==ANNO||w==INCLUDE)
            w = gettoken(fp);
        if(w == SEMI){
            // 遇分号，局部变量定义结束
            local_varnamelist->r = NULL;
            memset(token_text, 0, sizeof(token_text));
            w = gettoken(fp);
            while(w==ANNO||w==INCLUDE)
                w = gettoken(fp);
            break;
        }else if(w == COMMA)
        {
            // 新建变量名序列结点
            AST* p = (AST*)malloc(sizeof(AST));
            p->data.data = NULL;
            local_varnamelist->r = p;
            local_varnamelist = p;
            local_varnamelist->type = LOCALVARNAMELIST;
            // 新建变量名结点
            local_varnamelist->l = (AST*)malloc(sizeof(AST));
            local_varnamelist->l->data.data = (char*)malloc(MAXLEN*sizeof(char));
            local_varnamelist->l->l = NULL;
            local_varnamelist->l->r = NULL;
            local_varnamelist->l->type = LOCALVARNAME;
            memset(token_text, 0, sizeof(token_text));
            w = gettoken(fp);
            while(w==ANNO||w==INCLUDE)
                w = gettoken(fp);
            if(add2VNL(token_text)){
                mistake = true;
                return NULL;
            }
            strcpy(local_varnamelist->l->data.data, token_text);
        }else{
            mistake = true;
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：局部变量定义出错\n");
            return NULL;
        }
    }
        // 局部变量定义可能有多行
    if(w >= INT && w <= DOUBLE)
        local_vardeflist->r = LocalVarDefList();
    else
        local_vardeflist->r = NULL;
    return local_vardeflist;
        // 此函数退出时读取了下一个单词
}

/* 语句序列 */
AST* StatementList()
{
    if(mistake)
        return NULL;
    AST *state_list = NULL;
    AST *state = Statement();
    if(state == NULL){
        // 语句序列结束
        return NULL;
    }else{
        state_list = (AST*)malloc(sizeof(AST));
        state_list->data.data = NULL;
        state_list->type = STATELIST;
        state_list->l = state;
        state_list->r = NULL;
        memset(token_text , 0, sizeof(token_text));
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        if(/* w == EndOfFile || */ w == RB){
            // 应该在函数的大括号结束时return state_list
            ToBeContinue
            //
            return state_list;
        }else if(w != RB)
        {
            // 未至右大括号
            state_list->r = StatementList();
            return state_list;
        }
        
    }
}

/* 语句 */
AST* Statement()
{
    if(mistake)
        return NULL;
    AST* state = (AST*)malloc(sizeof(AST));
    state->l = NULL;
    state->r = NULL;
    state->data.data = NULL;
    switch (w)
    {
    case IF:{
        w = gettoken(fp);
        while (w == INCLUDE || w == ANNO)
            w = gettoken(fp);
        if(w != LP){
            mistake = true;
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：if语句语法出错\n");
            return NULL;
        }
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while (w == INCLUDE || w == ANNO)
            w = gettoken(fp);
        AST *if_part = (AST*)malloc(sizeof(AST));
        if_part->data.data = NULL;
        if_part->type = IFPART;
        if_part->l = Expression(RP);
        if(if_part->l == NULL){
            mistake = true;
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：if语句条件部分出错\n");
            return NULL;
        }
        w = gettoken(fp);
        while (w == INCLUDE || w == ANNO)
            w = gettoken(fp);
        if(w == LB){
            // if语句大括号内可以有多条语句{...state...}
            memset(token_text, 0, sizeof(token_text));
            w = gettoken(fp);
            while(w == ANNO || w == INCLUDE)
                w=gettoken(fp);
            if_part->r = StatementList();
        }else if(w >= IDENT && w <= KEYWORD){
            // if后无语句序列，只跟一条语句
            if_part->r = Statement();
        }else{
            mistake = true;
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：if语句体出错\n");
            return NULL;
        }
        state->l = if_part;
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        if(w == ELSE)
        {
            state->type = IFELSESTATEMENT;
            AST* else_part = (AST* )malloc(sizeof(AST));
            else_part->l = NULL;
            else_part->data.data = NULL;
            else_part->type = ELSEPART;
            state->r = else_part;
            memset(token_text, 0, sizeof(token_text));
            w = gettoken(fp);
            while (w == ANNO || w == INCLUDE)
                w = gettoken(fp);
            if( w == LB){
                memset(token_text, 0, sizeof(token_text));
                w = gettoken(fp);
                while (w == ANNO || w == INCLUDE)
                    w = gettoken(fp);
                else_part->r = StatementList();
            }else if(w >= IDENT && w <= KEYWORD){
                // 无大括号时，只跟一条语句
                else_part->r = Statement();
            }else if(w == IF){
                else_part->l = Statement();
            }else{
                printf("第%d行出现错误\n",cnt_lines);
                printf("错误：else子句出错\n");
                mistake = true;
                return NULL;
            }
        }else{
            state->type = IFSTATEMENT;
            // 此处为了判断是否跟else多读了一个单词,若不是else应把该单词退回
            returntoken(fp);
        }
        return state;
    }
    case WHILE:{
        in_recycle = true;
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        if(w != LP)
        {
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：while语句出错\n");
            mistake=1;
            return NULL;
        }
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        AST* while_part = (AST* )malloc(sizeof(AST));
        while_part->data.data = NULL;
        while_part->type = WHILEPART;
        while_part->r = NULL;
        while_part->l = Expression(RP); // 处理条件表达式
        if(while_part->l == NULL){
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：while语句条件部分出错\n");
            mistake = true;
            return NULL;
        }
        AST* while_body = (AST*)malloc(sizeof(AST));
        while_body->data.data = NULL;
        while_body->type = WHILEBODY;
        while_body->l = NULL;
        while_body->r = NULL;
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        if(w == LB){
            memset(token_text, 0, sizeof(token_text));
            w = gettoken(fp);
            while(w == ANNO || w == INCLUDE)
                w = gettoken(fp);
            while_body->r = StatementList();
        }else if( w >= IDENT && w <= KEYWORD){
            while_body->r = Statement();
        }else{
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：while语句出错\n");
            mistake = true;
            return NULL;
        }
        state->type = WHILESTATEMENT;
        state->l = while_part;
        state->r = while_body;
        in_recycle = false;
        return state;
    }
    case FOR:{
        in_recycle = true;
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        if(w != LP){
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：for语句出错\n");
            mistake = true;
            return NULL;
        }
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        AST *for_part = (AST *)malloc(sizeof(AST)); // 条件句
        for_part->data.data = NULL;
        for_part->type = FORPART;
        AST *for_1 = (AST *)malloc(sizeof(AST)); // 第一部分
        AST *for_2 = (AST *)malloc(sizeof(AST)); // 第二部分
        AST *for_3 = (AST *)malloc(sizeof(AST)); // 第三部分
        for_part -> l = for_1;
        for_1->type = FORPART1;
        for_1->data.data = NULL;
        for_1->l = Expression(SEMI);
        if(for_1->l == NULL) {
            for_1->data.data = (char*)malloc(MAXLEN*sizeof(char));
            strcpy(for_1->data.data, "无");
        }
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        for_1 -> r = for_2;
        for_2->type = FORPART2;
        for_2->data.data = NULL;
        for_2->l = Expression(SEMI);
        if(for_2->l == NULL) {
            for_2->data.data = (char*)malloc(MAXLEN*sizeof(char));
            strcpy(for_2->data.data, "无");
        }
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        for_2 -> r = for_3;
        for_3->type = FORPART3;
        for_3->data.data = NULL;
        for_3->l = Expression(RP); // 第三部分以')'结束
        if(for_3->l == NULL) {
            for_3->data.data = (char*)malloc(MAXLEN*sizeof(char));
            strcpy(for_3->data.data, "无");
        }
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while(w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        AST* for_body = (AST*)malloc(sizeof(AST));
        for_body->type = FORBODY;
        for_body->r = NULL;
        for_body->l = NULL;
        for_body->data.data = NULL;
        if(w == LB){
            memset(token_text, 0, sizeof(token_text));
            w = gettoken(fp);
            while(w == ANNO || w == INCLUDE)
                w = gettoken(fp);
            for_body->r = StatementList();
        }else if(w >= IDENT && w <= KEYWORD){
            for_body->r = Statement();
        }else{
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：for循环体出错\n");
            mistake = true;
            return NULL;
        }
        state->type = FORSTATEMENT;
        state->l = for_part;
        state->r = for_body;
        in_recycle = false;
        return state;
    }
    case RETURN:{
        state->type=RETURNSTATEMENT;
        state->l = NULL;
        state->r = NULL;
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while (w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        state->r = Expression(SEMI);
        return state;
    }
    case BREAK:{
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while (w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        if (w != SEMI)
        {
            printf("第%d行出现错误\n", cnt_lines);
            printf("错误：break语句缺少分号\n");
            mistake = true;
            return NULL;
        }
        if (in_recycle == false)
        {
            printf("第%d行出现错误\n", cnt_lines);
            printf("错误：非循环语句中出现了break语句\n");
            mistake = true;
            return NULL;
        }
        state->type = BREAKSTATEMENT;
        return state;
    }
    case CONTINUE:{
        memset(token_text, 0, sizeof(token_text));
        w = gettoken(fp);
        while (w == ANNO || w == INCLUDE)
            w = gettoken(fp);
        if (w != SEMI)
        {
            printf("第%d行出现错误\n", cnt_lines);
            printf("错误：continue语句缺少分号\n");
            mistake = true;
            return NULL;
        }
        if (in_recycle == false)
        {
            printf("第%d行出现错误\n", cnt_lines);
            printf("错误：非循环语句中出现了continue语句\n");
            mistake = true;
            return NULL;
        }
        state->type = CONTINUESTATEMENT;
        return state;
    }
    case INT_CONST:
    case FLOAT_CONST:
    case CHAR_CONST:
    case IDENT:
    case ARRAY:
        return Expression(SEMI);
    }
    return NULL;
}

/* 表达式 *///调用该函数时，已经读入了第一个单词
AST* Expression(int end_sign)
{
    if(mistake)
        return NULL;
    // 针对for循环可能会出现语句为空的情况
    int error = 0;
    stack<AST*> op; // operator 运算符栈
    // op初始化，将起止符#入站
    AST *p = (AST*)malloc(sizeof(AST));
    p->data.data = NULL;
    p->data.type = POUND;
    p->type = OPERATOR;
    op.push(p);
    stack<AST*> opn; // 操作数栈
    // 当运算符栈栈顶不是起止符号，并没有错误时
    while(((w != end_sign) || op.top()->data.type != POUND) && !error)
    {
        if(op.top()->data.type==RP){//去括号
            if(op.size()<3){
                error++;
                break;
            }
            op.pop();
            op.pop();
        }
        if(w >= IDENT && w <= STRING_CONST) // w是标识符或常数等操作数时
        {
            p = (AST*)malloc(sizeof(AST));
            p->data.data = (char*)malloc(MAXLEN*sizeof(char));
            strcpy(p->data.data, token_text);
            p->type = OPERAND;
            opn.push(p);
            memset(token_text, 0, sizeof(token_text));
            w = gettoken(fp);
            while(w == ANNO || w == INCLUDE)
                w = gettoken(fp);
        }else if(w == end_sign)
        {
            AST* p=(AST*)malloc(sizeof(AST)), *t2, *t1, *t;//定义起止符号结点
            p->data.data=NULL;
            p->type=OPERATOR;
            p->data.type=POUND;
            while(op.top()->data.type!=POUND){
                t2 = opn.top();
                if (!t2&&(op.top()->data.type)!=PLUSPLUS&&(op.top()->data.type)!=MINUSMINUS) {
                    error++;
                    break;
                }
                if(t2!=NULL)
                    opn.pop();
                if (opn.size()==0) {
                    t1 = NULL;
                }else{
                    t1 = opn.top();
                }
                if (!t1&&(op.top()->data.type)!=PLUSPLUS&&(op.top()->data.type)!=MINUSMINUS) {
                    error++;
                    break;
                }
                if(t1!=NULL)
                    opn.pop();
                t = op.top();
                if (!t) {
                    error++;
                    break;
                }
                op.pop();
                t->l = t1;
                t->r = t2;
                opn.push(t);

            }
            if(opn.size()!=1){
                error++;
            }
        }else if(w >= EQ && w <= MINUSMINUS) // w是运算符
        {
            switch (precede(op.top()->data.type, w)) // 比较当前运算符与栈顶运算符的优先级
            {
            case '<':{
                p = (AST*)malloc(sizeof(AST));
                p->data.data = (char*)malloc(MAXLEN*sizeof(char));
                strcpy(p->data.data, token_text);
                p->type = OPERATOR;
                p->data.type = w;
                op.push(p);
                memset(token_text, 0, sizeof(token_text));
                w = gettoken(fp);
                while (w == ANNO || w == INCLUDE)
                    w = gettoken(fp);
                break;
            }
            case '=':{ // 去括号
                AST* t = op.top();
                if(!t){
                    error++;
                    op.pop();
                }
                w = gettoken(fp);
                while (w == ANNO || w == INCLUDE)
                    w = gettoken(fp);
                break;
            }
            case '>':{
                AST* t1, *t2, *t;
                t2 = opn.top();
                if (!t2 && (op.top()->data.type) != PLUSPLUS && (op.top()->data.type) != MINUSMINUS)
                {
                    error++;
                    break;
                }
                if (t2 != NULL)
                    opn.pop();
                if (opn.size() == 0)
                {
                    t1 = NULL;
                }
                else
                {
                    t1 = opn.top();
                }
                if (!t1 && (op.top()->data.type) != PLUSPLUS && (op.top()->data.type) != MINUSMINUS)
                {
                    error++;
                    break;
                }
                if (t1 != NULL)
                    opn.pop();
                t = op.top();
                if (!t)
                {
                    error++;
                    break;
                }
                op.pop();
                t->l = t1;
                t->r = t2;
                opn.push(t);

                p = (AST *)malloc(sizeof(AST));
                p->data.data = (char*)malloc(MAXLEN*sizeof(char));
                strcpy(p->data.data, token_text);
                p->type = OPERATOR;
                p->data.type = w;
                op.push(p);
                memset(token_text, 0, sizeof(token_text));
                w = gettoken(fp);
                while (w == ANNO || w == INCLUDE)
                    w = gettoken(fp);
                break;
            }
            case '\0':{
                printf("第%d行出现警告\n",cnt_lines);
                printf("警告：出现未知运算符\n");
                mistake = true;
                exit(0);
            }
            default:
                if(w == end_sign)
                    w = POUND;
                else
                    error++;
                break;
            }
        }
        else
            error++;
    }
    // 操作数栈只有一个结点指针
    if((opn.size() == 1) && (op.top()->data.type == POUND) && error == 0)
    {
        AST *t = opn.top();
        opn.pop();
        AST *root = (AST*)malloc(sizeof(AST));
        root->data.data = NULL;
        root->l = NULL;
        root->r = NULL;
        root->type = EXPRESSION;
        root->l = t;
        return root;
    }else{
        // 表达式分析有错
        mistake = true;
        printf("第%d行出现错误\n",cnt_lines);
        printf("错误：表达式出现错误\n");
        return NULL;
    }
}

/* 实参序列 */

/* 数组定义 */
AST* ArrayDef()
{
    return NULL;
}

/* 退回多读取的单词 */
void returntoken(FILE *fp)
{
    int digit = strlen(token_text);
    int i;
    for (i = 0; i < digit; i++)
    {
        ungetc(token_text[digit - 1 - i], fp);
    }
}

/* 比较优先级函数 */
char precede(int c1, int c2)
{
    if(mistake)
        return '\0' ;
    if(c1==PLUS||c1==MINUS){
        switch (c2){
            case PLUS:
            case MINUS:
            case RP:
            case POUND:
            case MORE:
            case LESS:
            case MOREEQUAL:
            case LESSEQUAL:
            case EQ:
            case NEQ:
            case ASSIGN:
                return '>';
            case TIMES:
            case DIVIDE:
            case LP:
            case PLUSPLUS:
            case MINUSMINUS:
                return '<';
            default:
                return '\0';
                break;

        }
    } else if(c1==TIMES||c1==DIVIDE){
        switch (c2){
            case PLUS:
            case MINUS:
            case RP:
            case POUND:
            case TIMES:
            case DIVIDE:
            case MORE:
            case LESS:
            case MOREEQUAL:
            case LESSEQUAL:
            case EQ:
            case NEQ:
                return '>';
            case LP:
            case PLUSPLUS:
            case MINUSMINUS:
                return '<';
            default:
                return '\0';
        }
    } else if (c1==LP){
        switch (c2){
            case PLUS:
            case MINUS:
            case TIMES:
            case DIVIDE:
            case LP:
            case PLUSPLUS:
            case MINUSMINUS:
                return '<';
            case RP:
                return '=';
            case MORE:
            case LESS:
            case MOREEQUAL:
            case LESSEQUAL:
            case EQ:
            case NEQ:
            case POUND:
                return '>';
            default:
                return '\0';
        }
    } else if(c1==RP){
        switch (c2){
            case PLUS:
            case MINUS:
            case TIMES:
            case DIVIDE:
            case LP:
            case MORE:
            case LESS:
            case MOREEQUAL:
            case LESSEQUAL:
            case EQ:
            case NEQ:
            case PLUSPLUS:
            case MINUSMINUS:
            case POUND:
                return '>';
            default:
                return '\0';
        }
    } else if(c1==ASSIGN){
        switch (c2){
            case PLUS:
            case MINUS:
            case TIMES:
            case DIVIDE:
            case LP:
            case MORE:
            case LESS:
            case MOREEQUAL:
            case LESSEQUAL:
            case EQ:
            case NEQ:
            case PLUSPLUS:
            case MINUSMINUS:
                return '<';
            case POUND:
                return '>';
            default:
                return '\0';
        }
    }else if(c1==MORE||c1==LESS||c1==MOREEQUAL||c1==LESSEQUAL){
        switch (c2){
            case PLUS:
            case MINUS:
            case TIMES:
            case DIVIDE:
            case LP:
            case PLUSPLUS:
            case MINUSMINUS:
                return '<';
            case RP:
            case MORE:
            case LESS:
            case MOREEQUAL:
            case LESSEQUAL:
            case EQ:
            case NEQ:
            case POUND:
                return '>';
            default:
                return '\0';
        }
    }else if(c1==EQ||c1==NEQ){
        switch (c2){
            case PLUS:
            case MINUS:
            case TIMES:
            case DIVIDE:
            case LP:
            case MORE:
            case LESS:
            case MOREEQUAL:
            case LESSEQUAL:
            case PLUSPLUS:
            case MINUSMINUS:
                return '<';
            case RP:
            case EQ:
            case NEQ:
            case POUND:
                return '>';
            default:
                return '\0';
        }
    }else if(c1==POUND){
        switch (c2){
            case PLUS:
            case MINUS:
            case TIMES:
            case DIVIDE:
            case LP:
            case MORE:
            case LESS:
            case MOREEQUAL:
            case LESSEQUAL:
            case RP:
            case EQ:
            case NEQ:
            case ASSIGN:
            case PLUSPLUS:
            case MINUSMINUS:
                return '<';
            case POUND:
                return '=';
            default:
                return '\0';
        }
    }else if(c1==PLUSPLUS||c1==MINUSMINUS){
        switch (c2){
            case PLUS:
            case MINUS:
            case TIMES:
            case DIVIDE:
            case LP:
            case MORE:
            case LESS:
            case MOREEQUAL:
            case LESSEQUAL:
            case EQ:
            case NEQ:
            case ASSIGN:
            case POUND:
                return '>';
            case RP:
                return '<';
            default:
                return '\0';
        }
    }
}

/* 将变量名添加至VNL中, 返回0为添加成功, 返回1出错 */
int add2VNL(char token_text[MAXLEN])
{
    if(mistake)
        return 1;
    VNL* p = &(head);
    // 遍历VNL,检查有无重名变量，并得VNL长度
    while (p->next)
    {
        p = p->next;
        if(strcmp(token_text, p->variable) == 0){
            mistake = true;
            printf("第%d行出现错误\n",cnt_lines);
            printf("错误：变量重复定义\n");
            return 1;
        }
    }
    p->next = new VNL;
    p->next->next = NULL;
    strcpy(p->next->variable, token_text);
    return 0;
}

/* 释放以root为祖先的所有结点和root自己 */
int free_all(AST* root)
{
    if(root){
        free_all(root->l);
        free_all(root->r);
        if(root->data.data){
            free(root->data.data);
        }
        free(root);
    }
    return 0;
}

/* 在main中调用以生成AST */
void syntax_analyse(){
    AST* root=program();
    if(root==NULL||mistake==1){
        printf("程序语法错误\n");
        return;
    } else{
        printf("代码正确\n\n");
        /*呈现语法树*/
        /*递归先序遍历*/
        PreorderTranverse(root,0);
    }
}

/* 语法树先序遍历，通过depth控制缩进 */
void PreorderTranverse(AST* root,int depth)
{
    if(root == nullptr) return;
    else{
        int i;//控制缩进
        for(i=0;i<depth;i++){
            printf("\t");
        }
        showType(root->type);
        if(root->data.data!=NULL){
            for(i=0;i<depth;i++){
                printf("\t");
            }
            printf("%s\n",root->data.data);
        }
        PreorderTranverse(root->l,depth+1);
        PreorderTranverse(root->r,depth+1);
    }
}

/* 打印type */
void showType(int type)
{
    switch (type){
        case 1:
            printf("外部定义序列\n");
            break;
        case 2:
            printf("外部变量定义\n");
            break;
        case 3:
            printf("外部变量类型\n");
            break;
        case 4:
            printf("外部变量名序列\n");
            break;
        case 5:
            printf("外部变量名\n");
            break;
        case 6:
            printf("函数定义\n");
            break;
        case 7:
            printf("函数返回值类型\n");
            break;
        case 8:
            printf("函数名\n");
            break;
        case 9:
            printf("函数形参序列\n");
            break;
        case 10:
            printf("函数形参定义\n");
            break;
        case 11:
            printf("函数形参类型\n");
            break;
        case 12:
            printf("函数形参名\n");
            break;
        case 13:
            printf("函数体\n");
            break;
        case 14:
            printf("局部变量定义序列\n");
            break;
        case 15:
            printf("局部变量定义\n");
            break;
        case 16:
            printf("局部变量类型\n");
            break;
        case 17:
            printf("局部变量名序列\n");
            break;
        case 18:
            printf("局部变量名\n");
            break;
        case 19:
            printf("语句序列\n");
            break;
        case 20:
            printf("操作数\n");
            break;
        case 21:
            printf("运算符\n");
            break;
        case 22:
            printf("表达式\n");
            break;
        case 23:
            printf("if条件语句\n");
            break;
        case 24:
            printf("else语句体\n");
            break;
        case 25:
            printf("if语句\n");
            break;
        case 26:
            printf("if-else语句\n");
            break;
        case 27:
            printf("while语句\n");
            break;
        case 28:
            printf("while条件语句\n");
            break;
        case 29:
            printf("while语句体\n");
            break;
        case 30:
            printf("for语句\n");
            break;
        case 31:
            printf("for条件语句\n");
            break;
        case 32:
            printf("for语句条件一\n");
            break;
        case 33:
            printf("for语句条件二\n");
            break;
        case 34:
            printf("for语句条件三\n");
            break;
        case 35:
            printf("for循环体\n");
            break;
        case 36:
            printf("return语句\n");
            break;
        case 37:
            printf("break语句\n");
            break;
        case 38:
            printf("do-while语句\n");
            break;
        case 39:
            printf("do-while语句体\n");
            break;
        case 40:
            printf("do-while循环条件\n");
            break;
        case 41:
            printf("continue语句\n");
            break;
        case 42:
            printf("函数声明\n");
            break;
        case 43:
            printf("数组定义\n");
            break;
        case 44:
            printf("数组类型\n");
            break;
        case 45:
            printf("数组名\n");
            break;
        case 46:
            printf("数组大小\n");
            break;
        case 47:
            printf("复合语句\n");
            break;
    }
}



