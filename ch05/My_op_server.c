#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 10240
void error_handling(char *message);

char res[10];
char *calc(char *s)
{
    int len = strlen(s), i;
    int n = 0;
    for (i = 0; i < len; i++)
        if (s[i] == ' ')
        {
            i++;
            break;
        }
        else
            n = n * 10 + (s[i] - '0');
    int *num = malloc(sizeof(int) * n);
    int tot = 0, x = 0;

    for (; i < len; i++)
    {
        if (s[i] == '+' || s[i] == '*' || s[i] == '-')
            break;
        if (s[i] == ' ')
        {
            num[tot++] = x;
            x = 0;
        }
        else
            x = x * 10 + (s[i] - '0');
    }
    int ans = 0;
    if (s[i] == '+')
    {
        for (int i = 0; i < tot; i++)
            ans += num[i];
    }
    else if (s[i] == '*')
    {
        ans = 1;
        for (int i = 0; i < tot; i++)
            ans *= num[i];
    }
    else if (s[i] == '-')
    {
        ans = num[0];
        for (int i = 1; i < tot; i++)
            ans -= num[i];
    }
    free(num);
    sprintf(res, "%d", ans);
    return res;
}
int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    int str_len;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
    if (clnt_sock == -1)
        error_handling("accept() error");
    str_len = read(clnt_sock, message, BUF_SIZE);
    write(clnt_sock, calc(message), str_len);
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}