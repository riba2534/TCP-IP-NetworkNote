#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in adr;
    pthread_t send_thread, recv_thread;
    void * thread_return;//线程返回值
    if(argc != 4) {
        printf("Usage: %s <IP> <port> <name> \n", argv[0]);
        exit(1);
    }

    sprintf(name, "[%s]", argv[3]);//把名字写入name数组
    sock = socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = inet_addr(argv[1]);
    adr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*) &adr, sizeof(adr)) == -1)
        error_handling("connect() error");
    /*线程创建*/
    pthread_create(&send_thread, NULL, send_msg, (void *)&sock);
    pthread_create(&recv_thread, NULL, recv_msg, (void *)&sock);
    /*进程阻塞，进入发送消息的线程*/
    pthread_join(send_thread, &thread_return);
    /*进程阻塞，进入接收消息的线程*/
    pthread_join(recv_thread, &thread_return);
    close(sock);
    return 0;
}

void * send_msg(void * arg)
{
    int sock = *((int *)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    
    while(1)
    {   
        fputs("Input message(q to quit):\n", stdout);
        fgets(msg, BUF_SIZE, stdin);
        if(!(strcmp(msg, "q\n")) || !strcmp(msg, "Q\n"))
        {
            close(sock);
            exit(0);
        }
        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));//strlen()函数碰到'\0'会结束
    }
    return NULL;
}

void * recv_msg(void * arg)
{
    int sock = *((int *)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    int str_len;
    
    while(1)
    {
        str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);
        if(str_len == -1)
            return (void *)-1;
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}