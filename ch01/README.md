## 第一章：理解网络编程和套接字

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到，直接点链接可能进不去。

### 1.1 理解网络编程和套接字

#### 1.1.1 构建打电话套接字

以电话机打电话的方式来理解套接字。

**调用 socket 函数（安装电话机）时进行的对话**：

> 问：接电话需要准备什么？
>
> 答：当然是电话机。

有了电话机才能安装电话，于是就要准备一个电话机，下面函数相当于电话机的套接字。

```c
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
//成功时返回文件描述符，失败时返回-1
```

**调用 bind 函数（分配电话号码）时进行的对话**：

> 问：请问我的电话号码是多少
>
> 答：我的电话号码是123-1234

套接字同样如此。就像给电话机分配电话号码一样，利用以下函数给创建好的套接字分配地址信息（IP地址和端口号）：

```c
#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
//成功时返回0，失败时返回-1
```

调用 bind 函数给套接字分配地址之后，就基本完成了所有的准备工作。接下来是需要连接电话线并等待来电。

**调用 listen 函数（连接电话线）时进行的对话**：

> 问：已架设完电话机后是否只需连接电话线？
>
> 答：对，只需要连接就能接听电话。

一连接电话线，电话机就可以转换为可接听状态，这时其他人可以拨打电话请求连接到该机。同样，需要把套接字转化成可接受连接状态。

```c
#include <sys/socket.h>
int listen(int sockfd, int backlog);
//成功时返回0，失败时返回-1
```

连接好电话线以后，如果有人拨打电话就响铃，拿起话筒才能接听电话。

**调用 accept 函数（拿起话筒）时进行的对话**：

> 问：电话铃响了，我该怎么办？
>
> 答：接听啊。

```c
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
//成功时返回文件描述符，失败时返回-1
```

网络编程中接受连接请求的套接字创建过程可整理如下：

1. 第一步：调用 socket 函数创建套接字。
2. 第二步：调用 bind 函数分配IP地址和端口号。
3. 第三步：调用 listen 函数转换为可接受请求状态。
4. 第四步：调用 accept 函数受理套接字请求。

#### 1.1.2  编写`Hello World`套接字程序

**服务端**：

服务器端（server）是能够受理连接请求的程序。下面构建服务端以验证之前提到的函数调用过程，该服务器端收到连接请求后向请求者返回`Hello World!`答复。除各种函数的调用顺序外，我们还未涉及任何实际编程。因此，阅读代码时请重点关注套接字相关的函数调用过程，不必理解全过程。

服务器端代码请参见：[hello_server.c](hello_server.c)

**客户端**：

客户端程序只有`调用 socket 函数创建套接字` 和 `调用 connect 函数向服务端发送连接请求`这两个步骤，下面给出客户端，需要查看以下两方面的内容：

1. 调用 socket 函数 和 connect 函数
2. 与服务端共同运行以收发字符串数据

客户端代码请参见：[hello_client.c](hello_client.c)

**编译**：

分别对客户端和服务端程序进行编译：

```shell
gcc hello_server.c -o hserver
gcc hello_client.c -o hclient
```

**运行**：

```shell
./hserver 9190
./hclient 127.0.0.1 9190
```

运行的时候，首先在 9190 端口启动服务，然后 hserver 就会一直等待客户端进行连接，当客户端连接到本地地址 127.0.0.1 的 9190 端口时，客户端就会收到服务端的回应，输出`Hello World!`

### 1.2 基于 Linux 的文件操作

讨论套接字的过程中突然谈及文件也许有些奇怪。但是对于 Linux 而言，socket 操作与文件操作没有区别，因而有必要详细了解文件。在 Linux 世界里，socket 也被认为是文件的一种，因此在网络数据传输过程中自然可以使用 I/O 的相关函数。Windows 与 Linux 不同，是要区分套接字和文件的。因此在 Windows 中需要调用特殊的数据传输相关函数。

#### 1.2.1 底层访问和文件描述符

分配给标准输入输出及标准错误的文件描述符。

| 文件描述符 |           对象            |
| :--------: | :-----------------------: |
|     0      | 标准输入：Standard Input  |
|     1      | 标准输出：Standard Output |
|     2      | 标准错误：Standard Error  |

文件和套接字一般经过创建过程才会被分配文件描述符。

文件描述符也被称为「文件句柄」，但是「句柄」主要是 Windows 中的术语。因此，在本书中如果涉及 Windows 平台将使用「句柄」，如果是 Linux 将使用「描述符」。

#### 1.2.2 打开文件

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int open(const char *path, int flag);
/*
成功时返回文件描述符，失败时返回-1
path : 文件名的字符串地址
flag : 文件打开模式信息
*/
```

文件打开模式如下表：

| 打开模式 |            含义            |
| :------: | :------------------------: |
| O_CREAT  |       必要时创建文件       |
| O_TRUNC  |      删除全部现有数据      |
| O_APPEND | 维持现有数据，保存到其后面 |
| O_RDONLY |          只读打开          |
| O_WRONLY |          只写打开          |
|  O_RDWR  |          读写打开          |

#### 1.2.3 关闭文件：

```c
#include <unistd.h>
int close(int fd);
/*
成功时返回 0 ，失败时返回 -1
fd : 需要关闭的文件或套接字的文件描述符
*/
```

若调用此函数同时传递文件描述符参数，则关闭（终止）相应文件。另外需要注意的是，此函数不仅可以关闭文件，还可以关闭套接字。再次证明了「Linux 操作系统不区分文件与套接字」的特点。

#### 1.2.4 将数据写入文件：

```c
#include <unistd.h>
ssize_t write(int fd, const void *buf, size_t nbytes);
/*
成功时返回写入的字节数 ，失败时返回 -1
fd : 显示数据传输对象的文件描述符
buf : 保存要传输数据的缓冲值地址
nbytes : 要传输数据的字节数
*/
```

在此函数的定义中，size_t 是通过 typedef 声明的无符号整型（通常为 `unsigned long`）。对 ssize_t 来说，ssize_t 前面多加的 s 代表 signed（有符号），即 ssize_t 是通过 typedef 声明的有符号整型（通常为 `signed long`）。

创建新文件并保存数据：

代码见：[low_open.c](low_open.c)

编译运行：

```shell
gcc low_open.c -o lopen
./lopen
```

然后会生成一个`data.txt`的文件，里面有`Let's go!`

#### 1.2.5 读取文件中的数据：

与之前的`write()`函数相对应，`read()`用来输入（接收）数据。

```c
#include <unistd.h>
ssize_t read(int fd, void *buf, size_t nbytes);
/*
成功时返回接收的字节数（但遇到文件结尾则返回 0），失败时返回 -1
fd : 显示数据接收对象的文件描述符
buf : 要保存接收的数据的缓冲地址值。
nbytes : 要接收数据的最大字节数
*/
```

下面示例通过 read() 函数读取 data.txt 中保存的数据。

代码见：[low_read.c](low_read.c)

编译运行：

```shell
gcc low_read.c -o lread
./lread
```

在上一步的 data.txt 文件如果没有删的情况下，会输出：

```
file descriptor: 3
file data: Let's go!
```

关于文件描述符的 I/O 操作到此结束，要明白，这些内容同样适合于套接字。

#### 1.2.6 文件描述符与套接字

下面将同时创建文件和套接字，并用整数类型比较返回的文件描述符的值。

代码见：[fd_seri.c](fd_seri.c)

**编译运行**：

```shell
gcc fd_seri.c -o fds
./fds
```

**输出结果**:

```
file descriptor 1: 3
file descriptor 2: 4
file descriptor 3: 5
```

### 1.3 基于 Windows 平台的实现

暂略

### 1.4 基于 Windows 的套接字相关函数及示例

暂略

### 1.5 习题

> :heavy_exclamation_mark:以下部分的答案，仅代表我个人观点，可能不是正确答案

1. 套接字在网络编程中的作用是什么？为何称它为套接字？

   > 答：操作系统会提供「套接字」（socket）的部件，套接字是网络数据传输用的软件设备，因此「网络编程」也叫「套接字编程」。「套接字」是 socket 的中文译名，socket 原意为「插座、插口」，在网络编程中引申为数据通信的连接端点，即用来连接网络的工具。

2. 在服务器端创建套接字以后，会依次调用 listen 函数和 accept 函数。请比较二者作用。

   > 答：调用 listen 函数将套接字转换成可接受连接状态（监听），调用 accept 函数受理连接请求。如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。

3. Linux 中，对套接字数据进行 I/O 时可以直接使用文件 I/O 相关函数；而在 Windows 中则不可以。原因为何？

   > 答：在 Linux 中，套接字（socket）被视为文件的一种，遵循「一切皆文件」的设计哲学。因此可以使用标准的文件 I/O 函数（如 `read`、`write`、`close`）对套接字进行操作。而在 Windows 中，套接字和文件是区分开的，套接字操作需要使用专门的 Winsock 函数（如 `send`、`recv`、`closesocket`），通常不使用文件 I/O 函数（如 `ReadFile`、`WriteFile`）直接操作套接字。

4. 创建套接字后一般会给它分配地址，为什么？为了完成地址分配需要调用哪个函数？

   > 答：套接字被创建之后，只有为其分配了IP地址和端口号后，客户端才能够通过IP地址及端口号与服务器端建立连接，需要调用 bind 函数来完成地址分配。

5.  Linux 中的文件描述符与 Windows 的句柄实际上非常类似。请以套接字为对象说明它们的含义。

   > 答：文件描述符（File Descriptor）是 Linux 内核为了高效管理已被打开的文件所创建的索引，用于标识打开的文件、套接字等 I/O 资源，是一个非负整数。句柄（Handle）是 Windows 中用于标识资源（如文件、套接字、窗口等）的抽象引用，本质上是一个指针或索引。两者的相似之处在于：它们都是操作系统用来标识和访问资源的标识符；区别在于：Linux 的文件描述符是整数值，且遵循「一切皆文件」原则，套接字可以使用文件 I/O 函数操作；而 Windows 的句柄是 opaque 类型（不透明的指针类型），且需要针对不同资源类型使用专门的 API 函数。

6. 底层 I/O 函数与 ANSI 标准定义的文件 I/O 函数有何区别？

   > 答：文件 I/O 又称为低级磁盘 I/O，遵循 POSIX 相关标准。任何兼容 POSIX 标准的操作系统上都支持文件I/O。标准 I/O 被称为高级磁盘 I/O，遵循 ANSI C 相关标准。只要开发环境中有标准 I/O 库，标准 I/O 就可以使用。（Linux 中使用的是 GLIBC，它是标准C库的超集。不仅包含 ANSI C 中定义的函数，还包括 POSIX 标准中定义的函数。因此，Linux 下既可以使用标准 I/O，也可以使用文件 I/O）。

7. 参考本书给出的示例`low_open.c`和`low_read.c`，分别利用底层文件 I/O 和 ANSI 标准 I/O 编写文件复制程序。可任意指定复制程序的使用方法。

   > 答：
   >
   > **使用底层文件 I/O（Linux 系统调用）的文件复制程序：**
   >
   > ```c
   > #include <stdio.h>
   > #include <stdlib.h>
   > #include <unistd.h>
   > #include <fcntl.h>
   > #include <sys/stat.h>
   >
   > #define BUF_SIZE 4096
   >
   > int main(int argc, char *argv[]) {
   >     if (argc != 3) {
   >         printf("Usage: %s <src_file> <dest_file>\n", argv[0]);
   >         exit(1);
   >     }
   >
   >     int src_fd = open(argv[1], O_RDONLY);
   >     if (src_fd == -1) {
   >         perror("open source file failed");
   >         exit(1);
   >     }
   >
   >     int dest_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
   >     if (dest_fd == -1) {
   >         perror("open dest file failed");
   >         close(src_fd);
   >         exit(1);
   >     }
   >
   >     char buf[BUF_SIZE];
   >     ssize_t read_cnt;
   >
   >     while ((read_cnt = read(src_fd, buf, BUF_SIZE)) > 0) {
   >         write(dest_fd, buf, read_cnt);
   >     }
   >
   >     close(src_fd);
   >     close(dest_fd);
   >
   >     return 0;
   > }
   > ```
   >
   > **使用 ANSI 标准 I/O（标准 C 库）的文件复制程序：**
   >
   > ```c
   > #include <stdio.h>
   > #include <stdlib.h>
   >
   > #define BUF_SIZE 4096
   >
   > int main(int argc, char *argv[]) {
   >     if (argc != 3) {
   >         printf("Usage: %s <src_file> <dest_file>\n", argv[0]);
   >         exit(1);
   >     }
   >
   >     FILE *src_fp = fopen(argv[1], "rb");
   >     if (src_fp == NULL) {
   >         perror("fopen source file failed");
   >         exit(1);
   >     }
   >
   >     FILE *dest_fp = fopen(argv[2], "wb");
   >     if (dest_fp == NULL) {
   >         perror("fopen dest file failed");
   >         fclose(src_fp);
   >         exit(1);
   >     }
   >
   >     char buf[BUF_SIZE];
   >     size_t read_cnt;
   >
   >     while ((read_cnt = fread(buf, 1, BUF_SIZE, src_fp)) > 0) {
   >         fwrite(buf, 1, read_cnt, dest_fp);
   >     }
   >
   >     fclose(src_fp);
   >     fclose(dest_fp);
   >
   >     return 0;
   > }
   > ```
   >
   > **编译运行示例：**
   >
   > ```shell
   > # 底层 I/O 版本
   > gcc file_copy_low.c -o fcopy_low
   > ./fcopy_low source.txt dest.txt
   >
   > # 标准 I/O 版本
   > gcc file_copy_stdio.c -o fcopy_stdio
   > ./fcopy_stdio source.txt dest.txt
   > ```
