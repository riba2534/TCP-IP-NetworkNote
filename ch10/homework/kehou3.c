#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);

    pid = fork();
    if(pid == 0)
    {
        printf("Child sockfd: %d \n", sockfd);
    }
    else
    {
        printf("Parent sockfd: %d \n", sockfd);
    }
    return 0;
}

/*
结果：
Parent sockfd: 3 
Child sockfd: 3 
*/