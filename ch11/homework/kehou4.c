#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int fds1[2], fds2[2];
    //const char* 以"\0"作为结束符
    char str1[] = "Do you like cooffee?";
    char str2[] = "I like coffee";
    char str3[] = "I like long legs";
    char * str_arr[] = {str1, str2, str3};
    char buf[BUF_SIZE];
    pid_t pid;
    int i;

    pipe(fds1), pipe(fds2);
    pid = fork();

    if(pid == 0) 
    {
        for(i = 0; i < 3; ++i)
        {
            //strlen所作的是一个计数器的工作，它从内存的某个位置（可以是字符串开头，中间某个位置，甚至是某个不确定的内存区域）开始扫描，
            //直到碰到第一个字符串结束符'\0'为止，然后返回计数器值(长度不包含'\0')
            write(fds1[1], str_arr[i], strlen(str_arr[i]) + 1);//这里长度必须加上1，将字符串结束符加进去，否则会发生消息错乱
            read(fds2[0], buf, BUF_SIZE);
            printf("子进程收到的消息：%s\n", buf);
        }
    }
    else
    {
        for(i = 0; i < 3; ++i)
        {
            read(fds1[0], buf, BUF_SIZE);
            printf("父进程收到的消息：%s\n", buf);
            write(fds2[1], str_arr[i], strlen(str_arr[i]) + 1);
        }
    }
    return 0;
}