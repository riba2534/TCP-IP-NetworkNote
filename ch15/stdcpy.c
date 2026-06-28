#include <stdio.h>
#define BUF_SIZE 3

int main(int argc, char *argv[])
{
    FILE *fp1;
    FILE *fp2;
    char buf[BUF_SIZE];

    fp1 = fopen("news.txt", "r");
    fp2 = fopen("cpy.txt", "w");

    while (fgets(buf, BUF_SIZE, fp1) != NULL)
        fputs(buf, fp2);
    fclose(fp1);
    fclose(fp2);
    return 0;
}
