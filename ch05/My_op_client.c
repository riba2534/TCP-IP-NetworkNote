#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 10240
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;
    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("connect() error");
    else
        printf("连接成功!\n");
    int n, i;
    char temp[20];
    puts("请输入你要计算的数字个数:");
    scanf("%d", &n);
    sprintf(temp, "%d", n);
    strcat(temp, " ");
    strcat(message, temp);
    for (i = 0; i < n; i++)
    {
        printf("请输入第 %d 个数字:", i + 1);
        scanf("%s", temp);
        strcat(temp, " ");
        strcat(message, temp);
    }
    puts("请输入你要进行的运算符(+,-,*):");
    scanf("%s", temp);
    strcat(message, temp);
    write(sock, message, strlen(message));
    str_len = read(sock, message, BUF_SIZE - 1);
    message[str_len] = 0;
    printf("运算的结果是: %s\n", message);
    return 0;
}
void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}