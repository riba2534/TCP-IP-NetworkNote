#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int cfd1, cfd2;
    char str1[] = "Hi~ \n";
    char str2[] = "It's nice day~ \n";

    cfd1 = dup(1);        //复制文件描述符 1
    cfd2 = dup2(cfd1, 7); //再次复制文件描述符,定为数值 7

    printf("fd1=%d , fd2=%d \n", cfd1, cfd2);
    write(cfd1, str1, sizeof(str1));
    write(cfd2, str2, sizeof(str2));

    close(cfd1);
    close(cfd2); //终止复制的文件描述符，但是仍有一个文件描述符
    write(1, str1, sizeof(str1));
    close(1);
    write(1, str2, sizeof(str2)); //无法完成输出
    return 0;
}
