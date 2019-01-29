#include <stdio.h>
#include <fcntl.h>

int main()
{
    FILE *fp;
    int fd = open("data.dat", O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1)
    {
        fputs("file open error");
        return -1;
    }

    printf("First file descriptor : %d \n", fd);
    fp = fdopen(fd, "w"); //转成 file 指针
    fputs("TCP/IP SOCKET PROGRAMMING \n", fp);
    printf("Second file descriptor: %d \n", fileno(fp)); //转回文件描述符
    fclose(fp);
    return 0;
}