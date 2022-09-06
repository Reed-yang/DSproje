#include "lexer.h"
#include "parser.h"

void openfile(FILE *(*fp), char filename[30]); //打开一个源码文件

char filename[30];
FILE *fp;

int main()
{
    int ch = 1;
    while (ch)
    {
        system("clear");
        printf("高级语言源程序格式处理菜单\n");
        printf("1. 词法分析\n");
        printf("2. 语法分析\n");
        printf("3. 格式化源文件\n");
        printf("4. 重新选择文件\n");
        printf("0. 退出程序\n");
        printf("\n");
        printf("请进行选择：");
        scanf("%d", &ch);
        system("clear");
        switch (ch)
        {
        case 1:
        {
            openfile(&fp, filename);
            printf("已经进入词法分析！\n");
            word_analyse(fp);
            getchar();getchar();
            break;
        }
        case 2:
        {
            openfile(&fp, filename);
            printf("已经进入语法分析！\n");
            syntax_analyse();
            getchar();getchar();
            break;
        }
        case 4:
        {
            openfile(&fp, filename);
            break;
        }
        default:
            return 0;
        }
    }
}

void openfile(FILE *(*fp), char filename[30])
{
    system("clear");
    printf("输入文件名：");
    scanf("%s", filename);
    if (!((*fp) = fopen(filename, "r")))
    {
        printf("文件打开失败！\n");
        printf("请重新输入有效文件名...");
        getchar();getchar();
        openfile(fp, filename);
    } //打开文件并验证是否成功
}


