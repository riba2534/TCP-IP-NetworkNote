## 第 24 章 制作 HTTP 服务器端

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

### 24.1 HTTP 概要

本章将编写 HTTP（HyperText Transfer Protocol，超文本传输协议）服务器端，即 Web 服务器端。

#### 24.1.1 理解 Web 服务器端

web服务器端就是要基于 HTTP 协议，将网页对应文件传输给客户端的服务器端。

#### 24.1.2 HTTP

无状态的 Stateless 协议

![](https://i.loli.net/2019/02/07/5c5bc6973a4d0.png)

从上图可以看出，服务器端相应客户端请求后立即断开连接。换言之，服务器端不会维持客户端状态。即使同一客户端再次发送请求，服务器端也无法辨认出是原先那个，而会以相同方式处理新请求。因此，HTTP 又称「无状态的 Stateless 协议」

#### 24.1.3 请求消息（Request Message）的结构

下面是客户端向服务端发起请求消息的结构：

![](https://i.loli.net/2019/02/07/5c5bcbb75202f.png)

从图中可以看出，请求消息可以分为请求头、消息头、消息体 3 个部分。其中，请求行含有请求方式（请求目的）信息。典型的请求方式有 GET 和 POST ，GET 主要用于请求数据，POST 主要用于传输数据。为了降低复杂度，我们实现只能响应 GET 请求的 Web 服务器端，下面解释图中的请求行信息。其中「GET/index.html HTTP/1.1」 具有如下含义：

> 请求（GET）index.html 文件，通常以 1.1 版本的 HTTP 协议进行通信。

请求行只能通过  1 行（line）发送，因此，服务器端很容易从 HTTP 请求中提取第一行，并分别分析请求行中的信息。

请求行下面的消息头中包含发送请求的浏览器信息、用户认证信息等关于 HTTP 消息的附加信息。最后的消息体中装有客户端向服务端传输的数据，为了装入数据，需要以 POST 方式发送请求。但是我们的目标是实现 GET 方式的服务器端，所以可以忽略这部分内容。另外，消息体和消息头与之间以空行隔开，因此不会发生边界问题

#### 24.1.4 响应消息（Response Message）的结构

下面是 Web 服务器端向客户端传递的响应信息的结构。从图中可以看出，该响应消息由状态行、头信息、消息体等 3 个部分组成。状态行中有关于请求的状态信息，这是与请求消息相比最为显著地区别。

![](https://i.loli.net/2019/02/07/5c5bf9ad1b5f9.png)

第一个字符串状态行中含有关于客户端请求的处理结果。例如，客户端请求 index.html 文件时，表示 index.html 文件是否存在、服务端是否发生问题而无法响应等不同情况的信息写入状态行。图中的「HTTP/1.1 200 OK」具有如下含义：

- 200 OK : 成功处理了请求!
- 404 Not Found : 请求的文件不存在!
- 400 Bad Request : 请求方式错误，请检查！

消息头中含有传输的数据类型和长度等信息。图中的消息头含有如下信息：

> 服务端名为 SimpleWebServer ，传输的数据类型为 text/html。数据长度不超过 2048 个字节。

最后插入一个空行后，通过消息体发送客户端请求的文件数据。以上就是实现 Web 服务端过程中必要的 HTTP 协议。

### 24.2 实现简单的 Web 服务器端

#### 24.2.1 实现基于 Windows 的多线程 Web 服务器端

暂略

#### 24.2.2 实现基于 Linux 的多线程 Web 服务器端

下面是代码：

- [webserv_linux.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch24/webserv_linux.c)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define SMALL_BUF 100

void *request_handler(void *arg);
void send_data(FILE *fp, char *ct, char *file_name);
char *content_type(char *file);
void send_error(FILE *fp);
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_size;
    char buf[BUF_SIZE];
    pthread_t t_id;
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 20) == -1)
        error_handling("listen() error");

    while (1)
    {
        clnt_adr_size = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_size);
        printf("Connection Request : %s:%d\n",
               inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));
        pthread_create(&t_id, NULL, request_handler, &clnt_sock);
        pthread_detach(t_id);
    }
    close(serv_sock);
    return 0;
}

void *request_handler(void *arg)
{
    int clnt_sock = *((int *)arg);
    char req_line[SMALL_BUF];
    FILE *clnt_read;
    FILE *clnt_write;

    char method[10];
    char ct[15];
    char file_name[30];

    clnt_read = fdopen(clnt_sock, "r");
    clnt_write = fdopen(dup(clnt_sock), "w");
    fgets(req_line, SMALL_BUF, clnt_read);
    if (strstr(req_line, "HTTP/") == NULL)
    {
        send_error(clnt_write);
        fclose(clnt_read);
        fclose(clnt_write);
        return;
    }
    strcpy(method, strtok(req_line, " /"));
    strcpy(file_name, strtok(NULL, " /"));
    strcpy(ct, content_type(file_name));
    if (strcmp(method, "GET") != 0)
    {
        send_error(clnt_write);
        fclose(clnt_read);
        fclose(clnt_write);
        return;
    }
    fclose(clnt_read);
    send_data(clnt_write, ct, file_name);
}
void send_data(FILE *fp, char *ct, char *file_name)
{
    char protocol[] = "HTTP/1.0 200 OK\r\n";
    char server[] = "Server:Linux Web Server \r\n";
    char cnt_len[] = "Content-length:2048\r\n";
    char cnt_type[SMALL_BUF];
    char buf[BUF_SIZE];
    FILE *send_file;

    sprintf(cnt_type, "Content-type:%s\r\n\r\n", ct);
    send_file = fopen(file_name, "r");
    if (send_file == NULL)
    {
        send_error(fp);
        return;
    }

    //传输头信息
    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_len, fp);
    fputs(cnt_type, fp);

    //传输请求数据
    while (fgets(buf, BUF_SIZE, send_file) != NULL)
    {
        fputs(buf, fp);
        fflush(fp);
    }
    fflush(fp);
    fclose(fp);
}
char *content_type(char *file)
{
    char extension[SMALL_BUF];
    char file_name[SMALL_BUF];
    strcpy(file_name, file);
    strtok(file_name, ".");
    strcpy(extension, strtok(NULL, "."));

    if (!strcmp(extension, "html") || !strcmp(extension, "htm"))
        return "text/html";
    else
        return "text/plain";
}
void send_error(FILE *fp)
{
    char protocol[] = "HTTP/1.0 400 Bad Request\r\n";
    char server[] = "Server:Linux Web Server \r\n";
    char cnt_len[] = "Content-length:2048\r\n";
    char cnt_type[] = "Content-type:text/html\r\n\r\n";
    char content[] = "<html><head><title>NETWORK</title></head>"
                     "<body><font size=+5><br>发生错误！ 查看请求文件名和请求方式!"
                     "</font></body></html>";
    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_len, fp);
    fputs(cnt_type, fp);
    fflush(fp);
}
void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```

编译运行：

```shell
gcc webserv_linux.c -D_REENTRANT -o web_serv -lpthread
./web_serv 9190
```

结果：

![](https://i.loli.net/2019/02/07/5c5c107deba11.png)

![](https://i.loli.net/2019/02/07/5c5c19cbb3718.png)

经过测试，这个简单的 HTTP 服务器可以正常的显示出页面。

### 24.3 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **下列关于 Web 服务器端和 Web 浏览器端的说法错误的是**？

   答：以下加粗选项代表正确。

   1. **Web 浏览器并不是通过自身创建的套接字连接服务端的客户端**
   2. Web 服务器端通过 TCP 套接字提供服务，因为它将保持较长的客户端连接并交换数据
   3. 超文本与普通文本的最大区别是其具有可跳转的特性
   4. Web 浏览器可视为向浏览器提供请求文件的文件传输服务器端
   5. 除 Web 浏览器外，其他客户端都无法访问 Web 服务器端。

2. **下列关于 HTTP 协议的描述错误的是**？

   答：以下加粗选项代表正确。

   1. HTTP 协议是无状态的 Stateless 协议，不仅可以通过 TCP 实现，还可以通过 UDP 来实现
   2. **HTTP 协议是无状态的 Stateless 协议，因为其在 1 次请求和响应过程完成后立即断开连接。因此，如果同一服务器端和客户端需要  3 次请求及响应，则意味着需要经过 3 次套接字的创建过程**。
   3. **服务端向客户端传递的状态码中含有请求处理结果的信息**。
   4. **HTTP 协议是基于因特网的协议，因此，为了同时向大量客户端提供服务，HTTP 协议被设计为 Stateless 协议**。
