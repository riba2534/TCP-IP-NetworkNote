# 《TCP/IP网络编程》学习笔记

:flags:此仓库是我的《TCP/IP网络编程》学习笔记及具体代码实现，代码部分请参考本仓库对应章节文件夹下的代码。

我的环境是：Ubuntu18.04 LTS

编译器版本：`g++ (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0` 和 `gcc (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0`

所以本笔记中只学习有关于 Linux 的部分。

## 第 1 章：理解网络编程和套接字

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到，直接点连接可能进不去。

### 1.1 理解网络编程和套接字

#### 1.1.1构建打电话套接字

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

套接字同样如此。就想给电话机分配电话号码一样，利用以下函数给创建好的套接字分配地址信息（IP地址和端口号）：

```c
#include <sys/socket.h>
int bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen);
//成功时返回0，失败时返回-1
```

调用 bind 函数给套接字分配地址之后，就基本完成了所有的准备工作。接下来是需要连接电话线并等待来电。

**调用 listen 函数（连接电话线）时进行的对话**：

> 问：已架设完电话机后是否只需链接电话线？
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
int accept(int sockfd,struct sockaddr *addr,socklen_t *addrlen);
//成功时返回文件描述符，失败时返回-1
```

网络编程中和接受连接请求的套接字创建过程可整理如下：

1. 第一步：调用 socket 函数创建套接字。
2. 第二步：调用 bind 函数分配IP地址和端口号。
3. 第三步：调用 listen 函数转换为可接受请求状态。
4. 第四步：调用 accept 函数受理套接字请求。

#### 1.1.2  编写`Hello World`套接字程序

**服务端**：

服务器端（server）是能够受理连接请求的程序。下面构建服务端以验证之前提到的函数调用过程，该服务器端收到连接请求后向请求者返回`Hello World!`答复。除各种函数的调用顺序外，我们还未涉及任何实际编程。因此，阅读代码时请重点关注套接字相关的函数调用过程，不必理解全过程。

服务器端代码请参见：[hello_server.c](ch01/hello_server.c)

**客户端**：

客户端程序只有`调用 socket 函数创建套接字` 和 `调用 connect 函数向服务端发送连接请求`这两个步骤，下面给出客户端，需要查看以下两方面的内容：

1. 调用 socket 函数 和 connect 函数
2. 与服务端共同运行以收发字符串数据

客户端代码请参见：[hello_client.c](ch01/hello_client.c)

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

运行的时候，首先再 9190 端口启动服务，然后 heserver 就会一直等待客户端进行响应，当客户端监听位于本地的 IP 为 127.0.0.1 的地址的9190端口时，客户端就会收到服务端的回应，输出`Hello World!`

### 1.2 基于 Linux 的文件操作

讨论套接字的过程中突然谈及文件也许有些奇怪。但是对于 Linux 而言，socket 操作与文件操作没有区别，因而有必要详细了解文件。在 Linux 世界里，socket 也被认为是文件的一种，因此在网络数据传输过程中自然可以使用 I/O 的相关函数。Windows 与 Linux 不同，是要区分 socket 和文件的。因此在 Windows 中需要调用特殊的数据传输相关函数。

#### 1.2.1 底层访问和文件描述符

分配给标准输入输出及标准错误的文件描述符。

| 文件描述符 |           对象            |
| :--------: | :-----------------------: |
|     0      | 标准输入：Standard Input  |
|     1      | 标准输出：Standard Output |
|     2      | 标准错误：Standard Error  |

文件和套接字一般经过创建过程才会被分配文件描述符。

文件描述符也被称为「文件句柄」，但是「句柄」主要是 Windows 中的术语。因此，在本书中如果设计 Windows 平台将使用「句柄」，如果是 Linux 将使用「描述符」。

#### 1.2.2 打开文件:

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

若调用此函数同时传递文件描述符参数，则关闭（终止）响应文件。另外需要注意的是，此函数不仅可以关闭文件，还可以关闭套接字。再次证明了「Linux 操作系统不区分文件与套接字」的特点。

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

在此函数的定义中，size_t 是通过 typedef 声明的 unsigned int 类型。对 ssize_t 来说，ssize_t 前面多加的 s 代表 signed ，即 ssize_t 是通过 typedef 声明的 signed int 类型。

创建新文件并保存数据：

代码见：[low_open.c](ch01/low_open.c)

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

代码见：[low_read.c](ch01/low_read.c)

编译运行：

```shell
gcc low_read.c -o lread
./lread
```

在上一步的 data.txt 文件与没有删的情况下，会输出：

```
file descriptor: 3
file data: Let's go!
```

关于文件描述符的 I/O 操作到此结束，要明白，这些内容同样适合于套接字。

#### 1.2.6 文件描述符与套接字

下面将同时创建文件和套接字，并用整数型态比较返回的文件描述符的值.

代码见：[fd_seri.c](ch01/fd_seri.c)

**编译运行**：

```shell
gcc fd_seri.c -o fds
./fds
```

**输出结果**:

```
file descriptor 1: 3
file descriptor 2: 15
file descriptor 3: 16
```

### 1.3 基于 Windows 平台的实现

暂略

### 1.4 基于 Windows 的套接字相关函数及示例

暂略

### 1.5 习题

> :heavy_exclamation_mark:以下部分的答案，仅代表我个人观点，可能不是正确答案

1. 套接字在网络编程中的作用是什么？为何称它为套接字？

   > 答：操作系统会提供「套接字」（socket）的部件，套接字是网络数据传输用的软件设备。因此，「网络编程」也叫「套接字编程」。「套接字」就是用来连接网络的工具。

2. 在服务器端创建套接字以后，会依次调用 listen 函数和 accept 函数。请比较二者作用。

   > 答：调用 listen 函数将套接字转换成可受连接状态（监听），调用 accept 函数受理连接请求。如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。

3. Linux 中，对套接字数据进行 I/O 时可以直接使用文件 I/O 相关函数；而在 Windows 中则不可以。原因为何？

   > 答：暂略。

4. 创建套接字后一般会给他分配地址，为什么？为了完成地址分配需要调用哪个函数？

   > 答：套接字被创建之后，只有为其分配了IP地址和端口号后，客户端才能够通过IP地址及端口号与服务器端建立连接，需要调用 bind 函数来完成地址分配。

5.  Linux 中的文件描述符与 Windows 的句柄实际上非常类似。请以套接字为对象说明它们的含义。

   > 答：暂略。

6. 底层 I/O 函数与 ANSI 标准定义的文件 I/O 函数有何区别？

   > 答：文件 I/O 又称为低级磁盘 I/O，遵循 POSIX 相关标准。任何兼容 POSIX 标准的操作系统上都支持文件I/O。标准 I/O 被称为高级磁盘 I/O，遵循 ANSI C 相关标准。只要开发环境中有标准 I/O 库，标准 I/O 就可以使用。（Linux 中使用的是 GLIBC，它是标准C库的超集。不仅包含 ANSI C 中定义的函数，还包括 POSIX 标准中定义的函数。因此，Linux 下既可以使用标准 I/O，也可以使用文件 I/O）。

7. 参考本书给出的示例`low_open.c`和`low_read.c`，分别利用底层文件 I/O 和 ANSI 标准 I/O 编写文件复制程序。可任意指定复制程序的使用方法。

   > 答：暂略。

## 第 2 章 套接字类型与协议设置

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到，直接点连接可能进不去。

本章仅需了解创建套接字时调用的 socket 函数。

### 2.1 套接字协议及数据传输特性

#### 2.1.1 创建套接字

```c
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
/*
成功时返回文件描述符，失败时返回-1
domain: 套接字中使用的协议族（Protocol Family）
type: 套接字数据传输的类型信息
protocol: 计算机间通信中使用的协议信息
*/
```

#### 2.1.2 协议族（Protocol Family）

通过 socket 函数的第一个参数传递套接字中使用的协议分类信息。此协议分类信息称为协议族，可分成如下几类：

> 头文件 `sys/socket.h` 中声明的协议族
>

| 名称      | 协议族               |
| --------- | -------------------- |
| PF_INET   | IPV4 互联网协议族    |
| PF_INET6  | IPV6 互联网协议族    |
| PF_LOCAL  | 本地通信 Unix 协议族 |
| PF_PACKET | 底层套接字的协议族   |
| PF_IPX    | IPX Novel 协议族     |

本书着重讲 PF_INET 对应的 IPV4 互联网协议族。其他协议并不常用，或并未普及。**另外，套接字中采用的最终的协议信息是通过 socket 函数的第三个参数传递的。在指定的协议族范围内通过第一个参数决定第三个参数。**

#### 2.1.3 套接字类型（Type）

套接字类型指的是套接字的数据传输方式，是通过 socket 函数的第二个参数进行传递，只有这样才能决定创建的套接字的数据传输方式。**已经通过第一个参数传递了协议族信息，为什么还要决定数据传输方式？问题就在于，决定了协议族并不能同时决定数据传输方式。换言之， socket 函数的第一个参数 PF_INET 协议族中也存在多种数据传输方式。**

#### 2.1.4 套接字类型1：面向连接的套接字（SOCK_STREAM）

如果 socket 函数的第二个参数传递`SOCK_STREAM`，将创建面向连接的套接字。

传输方式特征整理如下：

- 传输过程中数据不会消失
- 按序传输数据
- 传输的数据不存在数据边界（Boundary）

这种情形适用于之前说过的 write 和 read 函数

> 传输数据的计算机通过调用3次 write 函数传递了 100 字节的数据，但是接受数据的计算机仅仅通过调用 1 次 read 函数调用就接受了全部 100 个字节。

收发数据的套接字内部有缓冲（buffer），简言之就是字节数组。只要不超过数组容量，那么数据填满缓冲后过 1 次 read 函数的调用就可以读取全部，也有可能调用多次来完成读取。

**套接字缓冲已满是否意味着数据丢失？**

> 答：缓冲并不总是满的。如果读取速度比数据传入过来的速度慢，则缓冲可能被填满，但是这时也不会丢失数据，因为传输套接字此时会停止数据传输，所以面向连接的套接字不会发生数据丢失。

套接字联机必须一一对应。面向连接的套接字可总结为：

**可靠地、按序传递的、基于字节的面向连接的数据传输方式的套接字。**

#### 2.1.5 面向消息的套接字（SOCK_DGRAM）

如果 socket 函数的第二个参数传递`SOCK_DGRAM`，则将创建面向消息的套接字。面向消息的套接字可以比喻成高速移动的摩托车队。特点如下：

- 强调快速传输而非传输有序
- 传输的数据可能丢失也可能损毁
- 传输的数据有边界
- 限制每次传输数据的大小

面向消息的套接字比面向连接的套接字更具哟传输速度，但可能丢失。特点可总结为：

**不可靠的、不按序传递的、以数据的高速传输为目的套接字。**

#### 2.1.6 协议的最终选择

socket 函数的第三个参数决定最终采用的协议。前面已经通过前两个参数传递了协议族信息和套接字数据传输方式，这些信息还不够吗？为什么要传输第三个参数呢？

> 可以应对同一协议族中存在的多个数据传输方式相同的协议，所以数据传输方式相同，但是协议不同，需要用第三个参数指定具体的协议信息。

本书用的是 Ipv4 的协议族，和面向连接的数据传输，满足这两个条件的协议只有 TPPROTO_TCP ，因此可以如下调用 socket 函数创建套接字，这种套接字称为 TCP 套接字。

```c
int tcp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
```

SOCK_DGRAM 指的是面向消息的数据传输方式，满足上述条件的协议只有 TPPROTO_UDP 。这种套接字称为 UDP 套接字：

```c
int udp_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
```

#### 2.1.7 面向连接的套接字：TCP 套接字示例

需要对第一章的代码做出修改，修改好的代码如下：

- [tcp_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch02/tcp_client.c)
- [tcp_server.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch02/tcp_server.c)

编译：

```shell
gcc tcp_client.c -o hclient
gcc tcp_server.c -o hserver
```

运行：

```shell
./hserver 9190
./hclient 127.0.0.1 9190
```

结果：

```
Message from server : Hello World! 
Function read call count: 13
```

从运行结果可以看出服务端发送了13字节的数据，客户端调用13次 read 函数进行读取。

### 2.2 Windows 平台下的实现及验证

暂略

### 2.3 习题

1. 什么是协议？在收发数据中定义协议有何意义？

   > 答：协议是对话中使用的通信规则，简言之，协议就是为了完成数据交换而定好的约定。在收发数据中定义协议，能够让计算机之间进行正确无误的对话，以此来交换数据。

2. 面向连接的套接字 TCP 套接字传输特性有 3 点，请分别说明。

   > 答：①传输过程中数据不会消失②按序传输数据③传输的数据不存在数据边界（Boundary）

3. 下面那些是面向消息的套接字的特性？

   - **传输数据可能丢失**
   - 没有数据边界（Boundary）
   - **以快速传递为目标**
   - 不限制每次传输数据大小
   - **与面向连接的套接字不同，不存在连接概念**

4. 下列数据适合用哪类套接字进行传输？

   - 演唱会现场直播的多媒体数据（UDP）
   - 某人压缩过的文本文件（TCP）
   - 网上银行用户与银行之间的数据传递（TCP）

5. 何种类型的套接字不存在数据边界？这类套接字接收数据时应该注意什么？

   > 答：TCP 不存在数据边界。在接收数据时，需要保证在接收套接字的缓冲区填充满之时就从buffer里读取数据。也就是，在接收套接字内部，写入buffer的速度要小于读出buffer的速度。

## 第 3 章 地址族与数据序列

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

把套接字比喻成电话，那么目前只安装了电话机，本章讲解给电话机分配号码的方法，即给套接字分配 IP 地址和端口号。

### 3.1 分配给套接字的 IP 地址与端口号

IP 是 Internet Protocol（网络协议）的简写，是为手法网络数据而分配给计算机的值。端口号并非赋予计算机的值，而是为了区分程序中创建的套接字而分配给套接字的端口号。

#### 3.1.1 网络地址（Internet Address）

为使计算机连接到网络并收发数据，必须为其分配 IP 地址。IP 地址分为两类。

- IPV4（Internet Protocol version 4）4 字节地址族
- IPV6（Internet Protocol version 6）6 字节地址族

两者之间的主要差别是 IP 地址所用的字节数，目前通用的是 IPV4 , IPV6 的普及还需要时间。

IPV4 标准的 4 字节 IP 地址分为网络地址和主机（指计算机）地址，且分为 A、B、C、D、E 等类型。

![](https://i.loli.net/2019/01/13/5c3ab0eb17bbe.png)

数据传输过程：

![](https://i.loli.net/2019/01/13/5c3ab19174fa4.png)

某主机向 203.211.172.103 和 203.211.217.202 传递数据，其中 203.211.172 和 203.211.217 为该网络的网络地址，所以「向相应网络传输数据」实际上是向构成网络的路由器或者交换机传输数据，然后又路由器或者交换机根据数据中的主机地址向目标主机传递数据。

#### 3.1.2 网络地址分类与主机地址边界

只需通过IP地址的第一个字节即可判断网络地址占用的总字节数，因为我们根据IP地址的边界区分网络地址，如下所示：

- A 类地址的首字节范围为：0~127
- B 类地址的首字节范围为：128~191
- C 类地址的首字节范围为：192~223

还有如下这种表示方式：

- A 类地址的首位以 0 开始
- B 类地址的前2位以 10 开始
- C 类地址的前3位以 110 开始

因此套接字手法数据时，数据传到网络后即可轻松找到主机。

#### 3.1.3 用于区分套接字的端口号

IP地址用于区分计算机，只要有IP地址就能向目标主机传输数据，但是只有这些还不够，我们需要把信息传输给具体的应用程序。

所以计算机一般有 NIC（网络接口卡）数据传输设备。通过 NIC 接受的数据内有端口号，操作系统参考端口号把信息传给相应的应用程序。

端口号由 16 位构成，可分配的端口号范围是 0~65535 。但是 0~1023 是知名端口，一般分配给特定的应用程序，所以应当分配给此范围之外的值。

虽然端口号不能重复，但是 TCP 套接字和 UDP 套接字不会共用端接口号，所以允许重复。如果某 TCP 套接字使用了 9190 端口号，其他 TCP 套接字就无法使用该端口号，但是 UDP 套接字可以使用。

总之，数据传输目标地址同时包含IP地址和端口号，只有这样，数据才会被传输到最终的目的应用程序。

### 3.2 地址信息的表示

应用程序中使用的IP地址和端口号以结构体的形式给出了定义。本节围绕结构体讨论目标地址的表示方法。

#### 3.2.1 表示 IPV4 地址的结构体

结构体的定义如下

```c
struct sockaddr_in
{
    sa_family_t sin_family;  //地址族（Address Family）
    uint16_t sin_port;       //16 位 TCP/UDP 端口号
    struct in_addr sin_addr; //32位 IP 地址
    char sin_zero[8];        //不使用
};
```

该结构体中提到的另一个结构体 in_addr 定义如下，它用来存放 32 位IP地址

```c
struct in_addr
{
    in_addr_t s_addr; //32位IPV4地址
}
```

关于以上两个结构体的一些数据类型。

| 数据类型名称 |             数据类型说明             | 声明的头文件 |
| :----------: | :----------------------------------: | :----------: |
|   int 8_t    |           signed 8-bit int           | sys/types.h  |
|   uint8_t    |  unsigned 8-bit int (unsigned char)  | sys/types.h  |
|   int16_t    |          signed 16-bit int           | sys/types.h  |
|   uint16_t   | unsigned 16-bit int (unsigned short) | sys/types.h  |
|   int32_t    |          signed 32-bit int           | sys/types.h  |
|   uint32_t   | unsigned 32-bit int (unsigned long)  | sys/types.h  |
| sa_family_t  |       地址族（address family）       | sys/socket.h |
|  socklen_t   |       长度（length of struct）       | sys/socket.h |
|  in_addr_t   |       IP地址，声明为 uint_32_t       | netinet/in.h |
|  in_port_t   |       端口号，声明为 uint_16_t       | netinet/in.h |

为什么要额外定义这些数据类型呢？这是考虑扩展性的结果

#### 3.2.2 结构体 sockaddr_in 的成员分析

- 成员 sin_family

每种协议适用的地址族不同，比如，IPV4 使用 4 字节的地址族，IPV6 使用 16 字节的地址族。

> 地址族

| 地址族（Address Family） | 含义                               |
| ------------------------ | ---------------------------------- |
| AF_INET                  | IPV4用的地址族                     |
| AF_INET6                 | IPV6用的地址族                     |
| AF_LOCAL                 | 本地通信中采用的 Unix 协议的地址族 |

AF_LOACL 只是为了说明具有多种地址族而添加的。

- 成员 sin_port

  该成员保存 16 位端口号，重点在于，它以网络字节序保存。

- 成员 sin_addr

  该成员保存 32 为IP地址信息，且也以网络字节序保存

- 成员 sin_zero

  无特殊含义。只是为结构体 sockaddr_in 结构体变量地址值将以如下方式传递给 bind 函数。

  在之前的代码中

  ```c
  if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
      error_handling("bind() error");
  ```

  此处 bind 第二个参数期望得到的是 sockaddr 结构体变量的地址值，包括地址族、端口号、IP地址等。

  ```c
  struct sockaddr
  {
      sa_family_t sin_family; //地址族
      char sa_data[14];       //地址信息
  }
  ```

  此结构体 sa_data 保存的地址信息中需要包含IP地址和端口号，剩余部分应该填充 0 ，但是这样对于包含地址的信息非常麻烦，所以出现了 sockaddr_in 结构体，然后强制转换成 sockaddr 类型，则生成符合 bind 条件的参数。

### 3.3 网络字节序与地址变换

不同的 CPU 中，4 字节整数值1在内存空间保存方式是不同的。

有些 CPU 这样保存：

```
00000000 00000000 00000000 00000001
```

有些 CPU 这样保存：

```
00000001 00000000 00000000 00000000
```

两种一种是顺序保存，一种是倒序保存 。

#### 3.3.1 字节序（Order）与网络字节序

CPU 保存数据的方式有两种，这意味着 CPU 解析数据的方式也有 2 种：

- 大端序（Big Endian）：高位字节存放到低位地址
- 小端序（Little Endian）：高位字节存放到高位地址

![big.png](https://i.loli.net/2019/01/13/5c3ac9c1b2550.png)
![small.png](https://i.loli.net/2019/01/13/5c3ac9c1c3348.png)

两台字节序不同的计算机在数据传递的过程中可能出现的问题：

![zijiexu.png](https://i.loli.net/2019/01/13/5c3aca956c8e9.png)

因为这种原因，所以在通过网络传输数据时必须约定统一的方式，这种约定被称为网络字节序，非常简单，统一为大端序。即，先把数据数组转化成大端序格式再进行网络传输。

#### 3.3.2 字节序转换

帮助转换字节序的函数：

```c
unsigned short htons(unsigned short);
unsigned short ntohs(unsigned short);
unsigned long htonl(unsigned long);
unsigned long ntohl(unsigned long);
```

通过函数名称掌握其功能，只需要了解：

- htons 的 h 代表主机（host）字节序。
- htons 的 n 代表网络（network）字节序。
- s 代表 short
- l 代表 long

下面的代码是示例，说明以上函数调用过程：

[endian_conv.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch03/endian_conv.c)

```cpp
#include <stdio.h>
#include <arpa/inet.h>
int main(int argc, char *argv[])
{
    unsigned short host_port = 0x1234;
    unsigned short net_port;
    unsigned long host_addr = 0x12345678;
    unsigned long net_addr;

    net_port = htons(host_port); //转换为网络字节序
    net_addr = htonl(host_addr);

    printf("Host ordered port: %#x \n", host_port);
    printf("Network ordered port: %#x \n", net_port);
    printf("Host ordered address: %#lx \n", host_addr);
    printf("Network ordered address: %#lx \n", net_addr);

    return 0;
}
```

编译运行：

```shell
gcc endian_conv.c -o conv
./conv
```

结果：

```
Host ordered port: 0x1234
Network ordered port: 0x3412
Host ordered address: 0x12345678
Network ordered address: 0x78563412
```

这是在小端 CPU 的运行结果。大部分人会得到相同的结果，因为 Intel 和 AMD 的 CPU 都是小端序为标准。

### 3.4 网络地址的初始化与分配

#### 3.4.1 将字符串信息转换为网络字节序的整数型

sockaddr_in 中需要的是 32 位整数型，但是我们只熟悉点分十进制表示法，那么改如何把类似于 201.211.214.36 转换为 4 字节的整数类型数据呢 ?幸运的是，有一个函数可以帮助我们完成它。

```C
#include <arpa/inet.h>
in_addr_t inet_addr(const char *string);
```

具体示例：

[inet_addr.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch03/inet_addr.c)

```c
#include <stdio.h>
#include <arpa/inet.h>
int main(int argc, char *argv[])
{
    char *addr1 = "1.2.3.4";
    char *addr2 = "1.2.3.256";

    unsigned long conv_addr = inet_addr(addr1);
    if (conv_addr == INADDR_NONE)
        printf("Error occured! \n");
    else
        printf("Network ordered integer addr: %#lx \n", conv_addr);

    conv_addr = inet_addr(addr2);
    if (conv_addr == INADDR_NONE)
        printf("Error occured! \n");
    else
        printf("Network ordered integer addr: %#lx \n", conv_addr);
    return 0;
}
```

编译运行：

```shell
gcc inet_addr.c -o addr
./addr
```

输出：

```
Network ordered integer addr: 0x4030201
Error occured!
```

1个字节能表示的最大整数是255，所以代码中 addr2 是错误的IP地址。从运行结果看，inet_addr 不仅可以转换地址，还可以检测有效性。

inet_aton 函数与 inet_addr 函数在功能上完全相同，也是将字符串形式的IP地址转换成整数型的IP地址。只不过该函数用了 in_addr 结构体，且使用频率更高。

```c
#include <arpa/inet.h>
int inet_aton(const char *string, struct in_addr *addr);
/*
成功时返回 1 ，失败时返回 0
string: 含有需要转换的IP地址信息的字符串地址值
addr: 将保存转换结果的 in_addr 结构体变量的地址值
*/
```

函数调用示例：

[inet_aton.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch03/inet_aton.c)

```c
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
void error_handling(char *message);

int main(int argc, char *argv[])
{
    char *addr = "127.232.124.79";
    struct sockaddr_in addr_inet;

    if (!inet_aton(addr, &addr_inet.sin_addr))
        error_handling("Conversion error");
    else
        printf("Network ordered integer addr: %#x \n", addr_inet.sin_addr.s_addr);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```

编译运行：

```c
gcc inet_aton.c -o aton
./aton
```

运行结果：

```
Network ordered integer addr: 0x4f7ce87f
```

可以看出，已经成功的把转换后的地址放进了 addr_inet.sin_addr.s_addr 中。

还有一个函数，与 inet_aton() 正好相反，它可以把网络字节序整数型IP地址转换成我们熟悉的字符串形式，函数原型如下：

```c
#include <arpa/inet.h>
char *inet_ntoa(struct in_addr adr);
```

该函数将通过参数传入的整数型IP地址转换为字符串格式并返回。但要小心，返回值为 char 指针，返回字符串地址意味着字符串已经保存在内存空间，但是该函数未向程序员要求分配内存，而是再内部申请了内存保存了字符串。也就是说调用了该函数候要立即把信息复制到其他内存空间。因此，若再次调用 inet_ntoa 函数，则有可能覆盖之前保存的字符串信息。总之，再次调用 inet_ntoa 函数前返回的字符串地址是有效的。若需要长期保存，则应该将字符串复制到其他内存空间。

示例：

[inet_ntoa.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch03/inet_ntoa.c)

```c
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in addr1, addr2;
    char *str_ptr;
    char str_arr[20];

    addr1.sin_addr.s_addr = htonl(0x1020304);
    addr2.sin_addr.s_addr = htonl(0x1010101);
    //把addr1中的结构体信息转换为字符串的IP地址形式
    str_ptr = inet_ntoa(addr1.sin_addr);
    strcpy(str_arr, str_ptr);
    printf("Dotted-Decimal notation1: %s \n", str_ptr);

    inet_ntoa(addr2.sin_addr);
    printf("Dotted-Decimal notation2: %s \n", str_ptr);
    printf("Dotted-Decimal notation3: %s \n", str_arr);
    return 0;
}
```

编译运行：

```shell
gcc inet_ntoa.c -o ntoa
./ntoa
```

输出:

```c
Dotted-Decimal notation1: 1.2.3.4
Dotted-Decimal notation2: 1.1.1.1
Dotted-Decimal notation3: 1.2.3.4
```

#### 3.4.2 网络地址初始化

结合前面的内容，介绍套接字创建过程中，常见的网络信息初始化方法：

```c
struct sockaddr_in addr;
char *serv_ip = "211.217,168.13";          //声明IP地址族
char *serv_port = "9190";                  //声明端口号字符串
memset(&addr, 0, sizeof(addr));            //结构体变量 addr 的所有成员初始化为0
addr.sin_family = AF_INET;                 //制定地址族
addr.sin_addr.s_addr = inet_addr(serv_ip); //基于字符串的IP地址初始化
addr.sin_port = htons(atoi(serv_port));    //基于字符串的IP地址端口号初始化
```

### 3.5 基于 Windows 的实现

略

### 3.6 习题

> 答案仅代表本人个人观点，不一定正确

1. **IP地址族 IPV4 与 IPV6 有什么区别？在何种背景下诞生了 IPV6?**

   答：主要差别是IP地址所用的字节数，目前通用的是IPV4，目前IPV4的资源已耗尽，所以诞生了IPV6，它具有更大的地址空间。

2. **通过 IPV4 网络 ID 、主机 ID 及路由器的关系说明公司局域网的计算机传输数据的过程**

   答：网络ID是为了区分网络而设置的一部分IP地址，假设向`www.baidu.com`公司传输数据，该公司内部构建了局域网。因为首先要向`baidu.com`传输数据，也就是说并非一开始就浏览所有四字节IP地址，首先找到网络地址，进而由`baidu.com`（构成网络的路由器）接收到数据后，传输到主机地址。比如向 203.211.712.103 传输数据，那就先找到 203.211.172 然后由这个网络的网关找主机号为 172 的机器传输数据。

3. **套接字地址分为IP地址和端口号，为什么需要IP地址和端口号？或者说，通过IP地址可以区分哪些对象？通过端口号可以区分哪些对象？**

   答：有了IP地址和端口号，才能把数据准确的传送到某个应用程序中。通过IP地址可以区分具体的主机，通过端口号可以区分主机上的应用程序。

4. **请说明IP地址的分类方法，并据此说出下面这些IP的分类。**

   - 214.121.212.102（C类）
   - 120.101.122.89（A类）
   - 129.78.102.211（B类）

   分类方法：A 类地址的首字节范围为：0~127、B 类地址的首字节范围为：128~191、C 类地址的首字节范围为：192~223

5. **计算机通过路由器和交换机连接到互联网，请说出路由器和交换机的作用。**

   答：路由器和交换机完成外网和本网主机之间的数据交换。

6. **什么是知名端口？其范围是多少？知名端口中具有代表性的 HTTP 和 FTP 的端口号各是多少？**

   答：知名端口是要把该端口分配给特定的应用程序，范围是 0~1023 ，HTTP 的端口号是 80 ，FTP 的端口号是20和21

7. **向套接字分配地址的 bind 函数原型如下：**

   ```c
   int bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen);
   ```

   **而调用时则用：**

   ```c
   bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)
   ```

   **此处 serv_addr 为 sockaddr_in 结构体变量。与函数原型不同，传入的是 sockaddr_in 结构体变量，请说明原因。**

   答：因为对于详细的地址信息使用 sockaddr 类型传递特别麻烦，进而有了 sockaddr_in 类型，其中基本与前面的类型保持一致，还有 sa_sata[4] 来保存地址信息，剩余全部填 0，所以强制转换后，不影响程序运行。

8. **请解释大端序，小端序、网络字节序，并说明为何需要网络字节序。**

   答：CPU 向内存保存数据有两种方式，大端序是高位字节存放低位地址，小端序是高位字节存放高位地址，网络字节序是为了方便传输的信息统一性，统一成了大端序。

9. **大端序计算机希望把 4 字节整数型 12 传递到小端序计算机。请说出数据传输过程中发生的字节序变换过程。**

   答：0x12->0x21

10. **怎样表示回送地址？其含义是什么？如果向会送地址处传输数据将会发生什么情况？**

    答：127.0.0.1 表示回送地址，指的是计算机自身的IP地址，无论什么程序，一旦使用回送地址发送数据，协议软件立即返回，不进行任何网络传输。

## 第 4 章 基于 TCP 的服务端/客户端（1）

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

### 4.1 理解 TCP 和 UDP

根据数据传输方式的不同，基于网络协议的套接字一般分为 TCP 套接字和 UDP 套接字。因为 TCP 套接字是面向连接的，因此又被称为基于流（stream）的套接字。

TCP 是 Transmission Control Protocol （传输控制协议）的简写，意为「对数据传输过程的控制」。因此，学习控制方法及范围有助于正确理解 TCP 套接字。

#### 4.1.1 TCP/IP 协议栈

![](https://i.loli.net/2019/01/14/5c3c21889db06.png)

TCP/IP 协议栈共分为 4 层，可以理解为数据收发分成了 4 个层次化过程，通过层次化的方式来解决问题

#### 4.1.2 链路层

链路层是物理链接领域标准化的结果，也是最基本的领域，专门定义LAN、WAN、MAN等网络标准。若两台主机通过网络进行数据交换，则需要物理连接，链路层就负责这些标准。

#### 4.1.3 IP 层

转备好物理连接候就要传输数据。为了再复杂网络中传输数据，首先要考虑路径的选择。向目标传输数据需要经过哪条路径？解决此问题的就是IP层，该层使用的协议就是IP。

IP 是面向消息的、不可靠的协议。每次传输数据时会帮我们选择路径，但并不一致。如果传输过程中发生错误，则选择其他路径，但是如果发生数据丢失或错误，则无法解决。换言之，IP协议无法应对数据错误。

#### 4.1.4 TCP/UDP 层

IP 层解决数据传输中的路径选择问题，秩序照此路径传输数据即可。TCP 和 UDP 层以 IP 层提供的路径信息为基础完成实际的数据传输，故该层又称为传输层。UDP 比 TCP 简单，现在我们只解释 TCP 。 TCP 可以保证数据的可靠传输，但是它发送数据时以 IP 层为基础（这也是协议栈层次化的原因）

IP 层只关注一个数据包（数据传输基本单位）的传输过程。因此，即使传输多个数据包，每个数据包也是由 IP 层实际传输的，也就是说传输顺序及传输本身是不可靠的。若只利用IP层传输数据，则可能导致后传输的数据包B比先传输的数据包A提早到达。另外，传输的数据包A、B、C中可能只收到A和C，甚至收到的C可能已经损毁 。反之，若添加 TCP 协议则按照如下对话方式进行数据交换。

> 主机A：正确接受第二个数据包
>
> 主机B：恩，知道了
>
> 主机A：正确收到第三个数据包
>
> 主机B：可我已经发送第四个数据包了啊！哦，您没收到吧，我给你重新发。

这就是 TCP 的作用。如果交换数据的过程中可以确认对方已经收到数据，并重传丢失的数据，那么即便IP层不保证数据传输，这类通信也是可靠的。

![](https://i.loli.net/2019/01/14/5c3c268b40be6.png)

#### 4.1.5 应用层

上述内容是套接字通信过程中自动处理的。选择数据传输路径、数据确认过程都被隐藏到套接字内部。向程序员提供的工具就是套接字，只需要利用套接字编出程序即可。编写软件的过程中，需要根据程序的特点来决定服务器和客户端之间的数据传输规则，这便是应用层协议。

### 4.2 实现基于 TCP 的服务器/客户端

#### 4.2.1 TCP 服务端的默认函数的调用程序

![](https://i.loli.net/2019/01/14/5c3c2782a7810.png)

调用 socket 函数创建套接字，声明并初始化地址信息的结构体变量，调用 bind 函数向套接字分配地址。

#### 4.2.2 进入等待连接请求状态

已经调用了 bind 函数给套接字分配地址，接下来就是要通过调用 listen 函数进入等待连接请求状态。只有调用了 listen 函数，客户端才能进入可发出连接请求的状态。换言之，这时客户端才能调用 connect 函数

```c
#include <sys/socket.h>
int listen(int sockfd, int backlog);
//成功时返回0，失败时返回-1
//sock: 希望进入等待连接请求状态的套接字文件描述符，传递的描述符套接字参数称为服务端套接字
//backlog: 连接请求等待队列的长度，若为5，则队列长度为5，表示最多使5个连接请求进入队列            
```
#### 4.2.3 受理客户端连接请求

调用 listen 函数后，则应该按序受理。受理请求意味着可接受数据的状态。进入这种状态所需的部件是**套接字**，但是此时使用的不是服务端套接字，此时需要另一个套接字，但是没必要亲自创建，下面的函数将自动创建套接字。

```c
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
/*
成功时返回文件描述符，失败时返回-1
sock: 服务端套接字的文件描述符
addr: 保存发起连接请求的客户端地址信息的变量地址值
addrlen: 的第二个参数addr结构体的长度，但是存放有长度的变量地址。
*/
```

sccept 函数受理连接请求队列中待处理的客户端连接请求。函数调用成功后，accept 内部将产生用于数据 I/O 的套接字，并返回其文件描述符。需要强调的是套接字是自动创建的，并自动与发起连接请求的客户端建立连接。

#### 4.2.4 回顾 Hello World 服务端

- 代码：[hello_server.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch04/hello_server.c)

重新整理一下代码的思路

1. 服务端实现过程中首先要创建套接字，此时的套接字并非是真正的服务端套接字
2. 为了完成套接字地址的分配，初始化结构体变量并调用 bind 函数。
3. 调用 listen 函数进入等待连接请求状态。连接请求状态队列的长度设置为5.此时的套接字才是服务端套接字。
4. 调用 accept 函数从队头取 1 个连接请求与客户端建立连接，并返回创建的套接字文件描述符。另外，调用 accept 函数时若等待队列为空，则 accept 函数不会返回，直到队列中出现新的客户端连接。
5. 调用 write 函数向客户端传送数据，调用 close 关闭连接

#### 4.2.5 TCP 客户端的默认函数调用顺序

![](https://i.loli.net/2019/01/14/5c3c31d77e86c.png)

与服务端相比，区别就在于「请求连接」，他是创建客户端套接字后向服务端发起的连接请求。服务端调用 listen 函数后创建连接请求等待队列，之后客户端即可请求连接。

```c
#include <sys/socket.h>
int connect(int sock, struct sockaddr *servaddr, socklen_t addrlen);
/*
成功时返回0，失败返回-1
sock:客户端套接字文件描述符
servaddr: 保存目标服务器端地址信息的变量地址值
addrlen: 以字节为单位传递给第二个结构体参数 servaddr 的变量地址长度
*/
```

客户端调用 connect 函数候，发生以下函数之一才会返回（完成函数调用）:

- 服务端接受连接请求
- 发生断网等一场状况而中断连接请求

注意：**接受连接**不代表服务端调用 accept 函数，其实只是服务器端把连接请求信息记录到等待队列。因此 connect 函数返回后并不应该立即进行数据交换。

#### 4.2.6 回顾 Hello World 客户端

- 代码：[hello_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch04/hello_client.c)

重新理解这个程序：

1. 创建准备连接服务器的套接字，此时创建的是 TCP 套接字
2. 结构体变量 serv_addr 中初始化IP和端口信息。初始化值为目标服务器端套接字的IP和端口信息。
3. 调用 connect 函数向服务端发起连接请求
4. 完成连接后，接收服务端传输的数据
5. 接收数据后调用 close 函数关闭套接字，结束与服务器端的连接。

#### 4.2.7 基于 TCP 的服务端/客户端函数调用关系

![](https://i.loli.net/2019/01/14/5c3c35a773b8c.png)

关系如上图所示。

### 4.3 实现迭代服务端/客户端

编写一个回声（echo）服务器/客户端。顾名思义，服务端将客户端传输的字符串数据原封不动的传回客户端，就像回声一样。在此之前，需要解释一下迭代服务器端。

#### 4.3.1 实现迭代服务器端

在 Hello World 的例子中，等待队列的作用没有太大意义。如果想继续处理好后面的客户端请求应该怎样扩展代码？最简单的方式就是插入循环反复调用 accept 函数，如图:

![](https://i.loli.net/2019/01/15/5c3d3c8a283ad.png)

可以看出，调用 accept 函数后，紧接着调用 I/O 相关的 read write 函数，然后调用 close 函数。这并非针对服务器套接字，而是针对 accept 函数调用时创建的套接字。

#### 4.3.2 迭代回声服务器端/客户端

程序运行的基本方式：

- 服务器端在同一时刻只与一个客户端相连，并提供回声服务。
- 服务器端依次向 5 个客户端提供服务并退出。
- 客户端接受用户输入的字符串并发送到服务器端。
- 服务器端将接受的字符串数据传回客户端，即「回声」
- 服务器端与客户端之间的字符串回声一直执行到客户端输入 Q 为止。

以下是服务端与客户端的代码：

- [echo_server.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch04/echo_server.c)
- [echo_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch04/echo_client.c)

编译:

```shell
gcc echo_client.c -o eclient
gcc echo_server.c -o eserver
```

分别运行:

```shell
./eserver 9190
./eclient 127.0.0.1 9190
```

过程和结果：

在一个服务端开启后，用另一个终端窗口开启客户端，然后程序会让你输入字符串，然后客户端输入什么字符串，客户端就会返回什么字符串，按 q 退出。这时服务端的运行并没有结束，服务端一共要处理 5 个客户端的连接，所以另外开多个终端窗口同时开启客户端，服务器按照顺序进行处理。

server:
![server.png](https://i.loli.net/2019/01/15/5c3d523d0a675.png)

client:
![client.png](https://i.loli.net/2019/01/15/5c3d523d336e7.png)

#### 4.3.3 回声客户端存在的问题

以上代码有一个假设「每次调用 read、write函数时都会以字符串为单位执行实际 I/O 操作」

但是「第二章」中说过「TCP 不存在数据边界」，上述客户端是基于 TCP 的，因此多次调用 write 函数传递的字符串有可能一次性传递到服务端。此时客户端有可能从服务端收到多个字符串，这不是我们想要的结果。还需要考虑服务器的如下情况：

「字符串太长，需要分 2 个包发送！」

服务端希望通过调用 1 次 write 函数传输数据，但是如果数据太大，操作系统就有可能把数据分成多个数据包发送到客户端。另外，在此过程中，客户端可能在尚未收到全部数据包时就调用 read 函数。

以上的问题都是源自 TCP 的传输特性，解决方法在第 5 章。

### 4.4 基于 Windows 的实现

暂略

### 4.5 习题

> 答案仅代表本人个人观点，不一定是正确答案。

1. **请你说明 TCP/IP 的 4 层协议栈，并说明 TCP 和 UDP 套接字经过的层级结构差异。**

   答：TCP/IP 的四层协议分为：应用层、TCP/UDP 层、IP层、链路层。差异是一个经过 TCP 层，一个经过 UDP 层。

2. **请说出 TCP/IP 协议栈中链路层和IP层的作用，并给出二者关系**

   答：链路层是物理链接领域标准化的结果，专门定义网络标准。若两台主机通过网络进行数据交换，则首先要做到的就是进行物理链接。IP层：为了在复杂的网络中传输数据，首先需要考虑路径的选择。关系：链路层负责进行一系列物理连接，而IP层负责选择正确可行的物理路径。

3. **为何需要把 TCP/IP 协议栈分成 4 层（或7层）？开放式回答。**

   答：ARPANET 的研制经验表明，对于复杂的计算机网络协议，其结构应该是层次式的。分册的好处：①隔层之间是独立的②灵活性好③结构上可以分隔开④易于实现和维护⑤能促进标准化工作。

4. **客户端调用 connect 函数向服务器端发送请求。服务器端调用哪个函数后，客户端可以调用 connect 函数？**

   答：服务端调用 listen 函数后，客户端可以调用 connect 函数。因为，服务端调用 listen 函数后，服务端套接字才有能力接受请求连接的信号。

5. **什么时候创建连接请求等待队列？它有何种作用？与 accept 有什么关系？**

   答：服务端调用 listen 函数后，accept函数正在处理客户端请求时， 更多的客户端发来了请求连接的数据，此时，就需要创建连接请求等待队列。以便于在accept函数处理完手头的请求之后，按照正确的顺序处理后面正在排队的其他请求。与accept函数的关系：accept函数受理连接请求等待队列中待处理的客户端连接请求。

6. **客户端中为何不需要调用 bind 函数分配地址？如果不调用 bind 函数，那何时、如何向套接字分配IP地址和端口号？**

   答：在调用 connect 函数时分配了地址，客户端IP地址和端口在调用 connect 函数时自动分配，无需调用标记的 bind 函数进行分配。

## 第 5 章 基于 TCP 的服务端/客户端（2）

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

上一章仅仅是从编程角度学习实现方法，并未详细讨论 TCP 的工作原理。因此，本章将想次讲解 TCP 中必要的理论知识，还将给出第 4 章客户端问题的解决方案。

### 5.1 回声客户端的完美实现

#### 5.1.1 回声服务器没有问题，只有回声客户端有问题？

问题不在服务器端，而在客户端，只看代码可能不好理解，因为 I/O 中使用了相同的函数。先回顾一下服务器端的 I/O 相关代码：

```c
while ((str_len = read(clnt_sock, message, BUF_SIZE)) != 0)
    write(clnt_sock, message, str_len);
```

接着是客户端代码:

```c
write(sock, message, strlen(message));
str_len = read(sock, message, BUF_SIZE - 1);
```

二者都在村换调用 read 和 write 函数。实际上之前的回声客户端将 100% 接受字节传输的数据，只不过接受数据时的单位有些问题。扩展客户端代码回顾范围，下面是，客户端的代码:

```c
while (1)
{
    fputs("Input message(Q to quit): ", stdout);
    fgets(message, BUF_SIZE, stdin);

    if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        break;

    write(sock, message, strlen(message));
    str_len = read(sock, message, BUF_SIZE - 1);
    message[str_len] = 0;
    printf("Message from server: %s", message);
}
```

现在应该理解了问题，回声客户端传输的是字符串，而且是通过调用 write 函数一次性发送的。之后还调用一次 read 函数，期待着接受自己传输的字符串，这就是问题所在。

#### 5.1.2 回声客户端问题的解决办法

这个问题其实很容易解决，因为可以提前接受数据的大小。若之前传输了20字节长的字符串，则再接收时循环调用 read 函数读取 20 个字节即可。既然有了解决办法，那么代码如下：

- [echo_client2.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch05/echo_client2.c)

这样修改为了接收所有传输数据而循环调用 read 函数。测试及运行结果可参考第四章。

#### 5.1.3 如果问题不在于回声客户端：定义应用层协议

回声客户端可以提前知道接收数据的长度，这在大多数情况下是不可能的。那么此时无法预知接收数据长度时应该如何手法数据？这是需要的是**应用层协议**的定义。在收发过程中定好规则（协议）以表示数据边界，或者提前告知需要发送的数据的大小。服务端/客户端实现过程中逐步定义的规则集合就是应用层协议。

现在写一个小程序来体验应用层协议的定义过程。要求：

1. 服务器从客户端获得多个数组和运算符信息。
2. 服务器接收到数字候对齐进行加减乘运算，然后把结果传回客户端。

例：

1. 向服务器传递3,5,9的同事请求加法运算，服务器返回3+5+9的结果
2. 请求做乘法运算，客户端会收到`3*5*9`的结果
3. 如果向服务器传递4,3,2的同时要求做减法，则返回4-3-2的运算结果。

请自己实现一个程序来实现功能。

我自己的实现：

- [My_op_server.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch05/My_op_server.c)
- [My_op_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch05/My_op_client.c)

编译：

```shell
gcc My_op_client.c -o myclient
gcc My_op_server.c -o myserver
```

结果：

![](https://i.loli.net/2019/01/15/5c3d966b81c03.png)

其实主要是对程序的一点点小改动，只需要再客户端固定好发送的格式，服务端按照固定格式解析，然后返回结果即可。

书上的实现：

- [op_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch05/op_client.c)
- [op_server.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch05/op_server.c)

阅读代码要注意一下，`int*`与`char`之间的转换。TCP 中不存在数据边界。

编译：

```shell
gcc op_client.c -o opclient
gcc op_server.c -o opserver
```

运行:

```shell
./opserver 9190
./opclient 127.0.0.1 9190
```

结果：

![](https://i.loli.net/2019/01/16/5c3ea297c7649.png)

### 5.2 TCP 原理

#### 5.2.1 TCP 套接字中的 I/O 缓冲

TCP 套接字的数据收发无边界。服务器即使调用 1 次 write 函数传输 40 字节的数据，客户端也有可能通过 4 次 read 函数调用每次读取 10 字节。但此处也有一些一问，服务器一次性传输了 40 字节，而客户端竟然可以缓慢的分批接受。客户端接受 10 字节后，剩下的 30 字节在何处等候呢？

实际上，write 函数调用后并非立即传输数据， read 函数调用后也并非马上接收数据。如图所示，write 函数滴啊用瞬间，数据将移至输出缓冲；read 函数调用瞬间，从输入缓冲读取数据。

![](https://i.loli.net/2019/01/16/5c3ea41cd93c6.png)

I/O 缓冲特性可以整理如下：

- I/O 缓冲在每个 TCP 套接字中单独存在
- I/O 缓冲在创建套接字时自动生成
- 即使关闭套接字也会继续传递输出缓冲中遗留的数据
- 关闭套接字将丢失输入缓冲中的数据

假设发生以下情况，会发生什么事呢？

> 客户端输入缓冲为 50 字节，而服务器端传输了 100 字节。

因为 TCP 不会发生超过输入缓冲大小的数据传输。也就是说，根本不会发生这类问题，因为 TCP 会控制数据流。TCP 中有滑动窗口（Sliding Window）协议，用对话方式如下：

> - A：你好，最多可以向我传递 50 字节
> - B：好的
> - A：我腾出了 20 字节的空间，最多可以接受 70 字节
> - B：好的

数据收发也是如此，因此 TCP 中不会因为缓冲溢出而丢失数据。

**write 函数在数据传输完成时返回。**

#### 5.2.2 TCP 内部工作原理 1：与对方套接字的连接

TCP 套接字从创建到消失所经过的过程分为如下三步：

- 与对方套接字建立连接
- 与对方套接字进行数据交换
- 断开与对方套接字的连接

首先讲解与对方套接字建立连接的过程。连接过程中，套接字的对话如下：

- 套接字A：你好，套接字 B。我这里有数据给你，建立连接吧
- 套接字B：好的，我这边已就绪
- 套接字A：谢谢你受理我的请求

TCP 在实际通信中也会经过三次对话过程，因此，该过程又被称为 **Three-way handshaking（三次握手）**。接下来给出连接过程中实际交换的信息方式：

![](https://i.loli.net/2019/01/16/5c3ecdec9fc04.png)

套接字是全双工方式工作的。也就是说，它可以双向传递数据。因此，收发数据前要做一些准备。首先请求连接的主机 A 要给主机 B 传递以下信息：

> [SYN] SEQ : 1000 , ACK:-

该消息中的 SEQ 为 1000 ，ACK 为空，而 SEQ 为1000 的含义如下：

> 现在传递的数据包的序号为 1000，如果接收无误，请通知我向您传递 1001 号数据包。

这是首次请求连接时使用的消息，又称为 SYN。SYN 是 Synchronization 的简写，表示收发数据前传输的同步消息。接下来主机 B 向 A 传递以下信息：

> [SYN+ACK] SEQ: 2000, ACK: 1001

此时 SEQ 为 2000，ACK 为 1001，而 SEQ 为 2000 的含义如下：

> 现传递的数据包号为 2000 ，如果接受无误，请通知我向您传递 2001 号数据包。

而 ACK 1001 的含义如下：

> 刚才传输的 SEQ 为 1000 的数据包接受无误，现在请传递 SEQ 为 1001 的数据包。

对于主机 A 首次传输的数据包的确认消息（ACK 1001）和为主机 B 传输数据做准备的同步消息（SEQ 2000）捆绑发送。因此，此种类消息又称为 SYN+ACK。

收发数据前向数据包分配序号，并向对方通报此序号，这都是为了防止数据丢失做的准备。通过项数据包分配序号并确认，可以在数据包丢失时马上查看并重传丢失的数据包。因此 TCP 可以保证可靠的数据传输。

通过这三个过程，这样主机 A 和主机 B 就确认了彼此已经准备就绪。

#### 5.2.3 TCP 内部工作原理 2：与对方主机的数据交换

通过第一步三次握手过程完成了数据交换准备，下面就开始正式收发数据，其默认方式如图所示：

![](https://i.loli.net/2019/01/16/5c3ed1a97ce2b.png)

图上给出了主机 A 分成 2 个数据包向主机 B 传输 200 字节的过程。首先，主机 A 通过 1 个数据包发送 100 个字节的数据，数据包的 SEQ 为 1200 。主机 B 为了确认这一点，向主机 A 发送 ACK 1301 消息。

此时的 ACK 号为 1301 而不是 1201，原因在于 ACK 号的增量为传输的数据字节数。假设每次 ACK 号不加传输的字节数，这样虽然可以确认数据包的传输，但无法明确 100 个字节全都正确传递还是丢失了一部分，比如只传递了 80 字节。因此按照如下公式传递 ACK 信息：

> ACK 号 = SEQ 号 + 传递的字节数 + 1

与三次握手协议相同，最后 + 1 是为了告知对方下次要传递的 SEQ 号。下面分析传输过程中数据包丢失的情况：

![](https://i.loli.net/2019/01/16/5c3ed371187a6.png)'

上图表示了通过 SEQ 1301 数据包向主机 B 传递 100 字节数据。但中间发生了错误，主机 B 未收到，经过一段时间后，主机 A 仍然未收到对于 SEQ 1301 的 ACK 的确认，因此试着重传该数据包。为了完成该数据包的重传，TCP 套接字启动计时器以等待 ACK 应答。若相应计时器发生超时（Time-out!）则重传。

#### 5.2.4 TCP 内部工作原理 3：断开套接字的连接

TCP 套接字的结束过程也非常优雅。如果对方还有数据需要传输时直接断掉该连接会出问题，所以断开连接时需要双方协商，断开连接时双方的对话如下：

> - 套接字A：我希望断开连接
> - 套接字B：哦，是吗？请稍后。
> - 套接字A：我也准备就绪，可以断开连接。
> - 套接字B：好的，谢谢合作。

先由套接字 A 向套接字 B 传递断开连接的信息，套接字 B 发出确认收到的消息，然后向套接字 A 传递可以断开连接的消息，套接字 A 同样发出确认消息。

![](https://i.loli.net/2019/01/16/5c3ed7503c18c.png)

图中数据包内的 FIN 表示断开连接。也就是说，双方各发送 1 次 FIN 消息后断开连接。此过过程经历 4 个阶段，因此又称四次握手（Four-way handshaking）。SEQ 和 ACK 的含义与之前讲解的内容一致，省略。图中，主机 A 传递了两次 ACK 5001，也许这里会有困惑。其实，第二次 FIN 数据包中的 ACK 5001 只是因为接收了 ACK 消息后未接收到的数据重传的。

### 5.3 基于 Windows 的实现

暂略

### 5.4 习题

> 答案仅代表本人个人观点，可能不是正确答案。

1. **请说明 TCP 套接字连接设置的三次握手过程。尤其是 3 次数据交换过程每次收发的数据内容。**

   答：三次握手主要分为：①与对方套接字建立连接②与对方套接字进行数据交换③断开与对方套接字的连接。每次收发的数据内容主要有：①由主机1给主机2发送初始的SEQ，首次连接请求是关键字是SYN，表示收发数据前同步传输的消息。②主机2收到报文以后，给主机 1 传递信息，用一个新的SEQ表示自己的序号，然后ACK代表已经接受到主机1的消息，希望接受下一个消息③主机1收到主机2的确认以后，还需要给主机2给出确认，此时再发送一次SEQ和ACK。

2. **TCP 是可靠的数据传输协议，但在通过网络通信的过程中可能丢失数据。请通过 ACK 和 SEQ 说明 TCP 通过和何种机制保证丢失数据的可靠传输。**

   答：通过超时重传机制来保证，如果报文发出去的特定时间内，发送消息的主机没有收到另一个主机的回复，那么就继续发送这条消息，直到收到回复为止。

3. **TCP 套接字中调用 write 和 read 函数时数据如何移动？结合 I/O 缓冲进行说明。**

   答：TCP 套接字调用 write 函数时，数据将移至输出缓冲，在适当的时候，传到对方输入缓冲。这时对方将调用 read 函数从输入缓冲中读取数据。

4. **对方主机的输入缓冲剩余 50 字节空间时，若本主机通过 write 函数请求传输 70 字节，请问 TCP 如何处理这种情况？**

   答：TCP 中有滑动窗口控制协议，所以传输的时候会保证传输的字节数小于等于自己能接受的字节数。

## 第 6 章 基于 UDP 的服务端/客户端

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

TCP 是内容较多的一个协议，而本章中的 UDP 内容较少，但是也很重要。

### 6.1 理解 UDP

#### 6.1.1 UDP 套接字的特点

通过寄信来说明 UDP 的工作原理，这是讲解 UDP 时使用的传统示例，它与 UDP 的特点完全相同。寄信前应现在信封上填好寄信人和收信人的地址，之后贴上邮票放进邮筒即可。当然，信件的特点使我们无法确认信件是否被收到。邮寄过程中也可能发生信件丢失的情况。也就是说，信件是一种不可靠的传输方式，UDP 也是一种不可靠的数据传输方式。

因为 UDP 没有 TCP 那么复杂，所以编程难度比较小，性能也比 TCP 高。在更重视性能的情况下可以选择 UDP 的传输方式。

TCP 与 UDP 的区别很大一部分来源于流控制。也就是说 TCP 的生命在于流控制。

#### 6.1.2 UDP 的工作原理

如图所示：

![](https://i.loli.net/2019/01/17/5c3fd29c70bf2.png)

从图中可以看出，IP 的作用就是让离开主机 B 的 UDP 数据包准确传递到主机 A 。但是把 UDP 数据包最终交给主机 A 的某一 UDP 套接字的过程是由 UDP 完成的。UDP 的最重要的作用就是根据端口号将传到主机的数据包交付给最终的 UDP 套接字。

#### 6.1.3 UDP 的高效使用

UDP 也具有一定的可靠性。对于通过网络实时传递的视频或者音频时情况有所不同。对于多媒体数据而言，丢失一部分数据也没有太大问题，这只是会暂时引起画面抖动，或者出现细微的杂音。但是要提供实时服务，速度就成为了一个很重要的因素。因此流控制就显得有一点多余，这时就要考虑使用 UDP 。TCP 比 UDP 慢的原因主要有以下两点：

- 收发数据前后进行的连接设置及清楚过程。
- 收发过程中为保证可靠性而添加的流控制。

如果收发的数据量小但是需要频繁连接时，UDP 比 TCP 更高效。

### 6.2 实现基于 UDP 的服务端/客户端

#### 6.2.1 UDP 中的服务端和客户端没有连接

UDP 中的服务端和客户端不像 TCP 那样在连接状态下交换数据，因此与 TCP 不同，无需经过连接过程。也就是说，不必调用 TCP 连接过程中调用的 listen 和 accept 函数。UDP 中只有创建套接字和数据交换的过程。

#### 6.2.2 UDP 服务器和客户端均只需一个套接字

TCP 中，套接字之间应该是一对一的关系。若要向 10 个客户端提供服务，除了守门的服务器套接字之外，还需要 10 个服务器套接字。但在 UDP 中，不管事服务器端还是客户端都只需要 1 个套接字。只需要一个 UDP 套接字就可以向任意主机传输数据，如图所示：

![](https://i.loli.net/2019/01/17/5c3fd703f3c40.png)

图中展示了 1 个 UDP 套接字与 2 个不同主机交换数据的过程。也就是说，只需 1 个 UDP 套接字就能和多台主机进行通信。

#### 6.2.3 基于 UDP 的数据 I/O 函数

创建好 TCP 套接字以后，传输数据时无需加上地址信息。因为 TCP 套接字将保持与对方套接字的连接。换言之，TCP 套接字知道目标地址信息。但 UDP 套接字不会保持连接状态（UDP 套接字只有简单的邮筒功能），因此每次传输数据时都需要添加目标的地址信息。这相当于寄信前在信件中填写地址。接下来是 UDP 的相关函数：

```c
#include <sys/socket.h>
ssize_t sendto(int sock, void *buff, size_t nbytes, int flags,
               struct sockaddr *to, socklen_t addrlen);
/*
成功时返回传输的字节数，失败是返回 -1
sock: 用于传输数据的 UDP 套接字
buff: 保存待传输数据的缓冲地址值
nbytes: 待传输的数据长度，以字节为单位
flags: 可选项参数，若没有则传递 0
to: 存有目标地址的 sockaddr 结构体变量的地址值
addrlen: 传递给参数 to 的地址值结构体变量长度
*/
```

上述函数与之前的 TCP 输出函数最大的区别在于，此函数需要向它传递目标地址信息。接下来介绍接收 UDP 数据的函数。UDP 数据的发送并不固定，因此该函数定义为可接受发送端信息的形式，也就是将同时返回 UDP 数据包中的发送端信息。

```c
#include <sys/socket.h>
ssize_t recvfrom(int sock, void *buff, size_t nbytes, int flags,
                 struct sockaddr *from, socklen_t *addrlen);
/*
成功时返回传输的字节数，失败是返回 -1
sock: 用于传输数据的 UDP 套接字
buff: 保存待传输数据的缓冲地址值
nbytes: 待传输的数据长度，以字节为单位
flags: 可选项参数，若没有则传递 0
from: 存有发送端地址信息的 sockaddr 结构体变量的地址值
addrlen: 保存参数 from 的结构体变量长度的变量地址值。
*/
```

编写 UDP 程序的最核心的部分就在于上述两个函数，这也说明二者在 UDP 数据传输中的地位。

#### 6.2.4 基于 UDP 的回声服务器端/客户端

下面是实现的基于 UDP 的回声服务器的服务器端和客户端：

代码：

- [uecho_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch06/uecho_client.c)
- [uecho_server.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch06/uecho_server.c)

编译运行：

```shell
gcc uecho_client.c -o uclient
gcc uecho_server.c -o userver
./server 9190
./uclient 127.0.0.1 9190
```

结果：

![](https://i.loli.net/2019/01/17/5c3feb85baa83.png)

TCP 客户端套接字在调用 connect 函数时自动分配IP地址和端口号，既然如此，UDP 客户端何时分配IP地址和端口号？

#### 6.2.5 UDP 客户端套接字的地址分配

仔细观察 UDP 客户端可以发现，UDP 客户端缺少了把IP和端口分配给套接字的过程。TCP 客户端调用 connect 函数自动完成此过程，而 UDP 中连接能承担相同功能的函数调用语句都没有。究竟在什么时候分配IP和端口号呢？

UDP 程序中，调用 sendto 函数传输数据前应该完成对套接字的地址分配工作，因此调用 bind 函数。当然，bind 函数在 TCP 程序中出现过，但 bind 函数不区分 TCP 和 UDP，也就是说，在 UDP 程序中同样可以调用。另外，如果调用 sendto 函数尚未分配地址信息，则在首次调用 sendto 函数时给相应套接字自动分配 IP 和端口。而且此时分配的地址一直保留到程序结束为止，因此也可以用来和其他 UDP 套接字进行数据交换。当然，IP 用主机IP，端口号用未选用的任意端口号。

综上所述，调用 sendto 函数时自动分配IP和端口号，因此，UDP 客户端中通常无需额外的地址分配过程。所以之前的示例中省略了该过程。这也是普遍的实现方式。

### 6.3 UDP 的数据传输特性和调用 connect 函数

#### 6.3.1 存在数据边界的 UDP 套接字

前面说得 TCP 数据传输中不存在数据边界，这表示「数据传输过程中调用 I/O 函数的次数不具有任何意义」

相反，UDP 是具有数据边界的下一，传输中调用 I/O 函数的次数非常重要。因此，输入函数的调用次数和输出函数的调用次数完全一致，这样才能保证接收全部已经发送的数据。例如，调用 3 次输出函数发送的数据必须通过调用 3 次输入函数才能接收完。通过一个例子来进行验证：

- [bound_host1.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch06/bound_host1.c)
- [bound_host2.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch06/bound_host2.c)

编译运行：

```shell
gcc bound_host1.c -o host1
gcc bound_host2.c -o host2
./host1 9190
./host2 127.0.0.1 9190
```

运行结果：

![](https://i.loli.net/2019/01/17/5c3ff966a8d34.png)

host1 是服务端，host2 是客户端，host2 一次性把数据发给服务端后，结束程序。但是因为服务端每隔五秒才接收一次，所以服务端每隔五秒接收一次消息。

**从运行结果也可以证明 UDP 通信过程中 I/O 的调用次数必须保持一致**

#### 6.3.2 已连接（connect）UDP 套接字与未连接（unconnected）UDP 套接字

TCP 套接字中需注册待传传输数据的目标IP和端口号，而在 UDP 中无需注册。因此通过 sendto 函数传输数据的过程大概可以分为以下 3 个阶段：

- 第 1 阶段：向 UDP 套接字注册目标 IP 和端口号
- 第 2 阶段：传输数据
- 第 3 阶段：删除 UDP 套接字中注册的目标地址信息。

每次调用 sendto 函数时重复上述过程。每次都变更目标地址，因此可以重复利用同一 UDP 套接字向不同目标传递数据。这种未注册目标地址信息的套接字称为未连接套接字，反之，注册了目标地址的套接字称为连接 connected 套接字。显然，UDP 套接字默认属于未连接套接字。当一台主机向另一台主机传输很多信息时，上述的三个阶段中，第一个阶段和第三个阶段占整个通信过程中近三分之一的时间，缩短这部分的时间将会大大提高整体性能。

#### 6.3.3 创建已连接 UDP 套接字

创建已连接 UDP 套接字过程格外简单，只需针对 UDP 套接字调用 connect 函数。

```c
sock = socket(PF_INET, SOCK_DGRAM, 0);
memset(&adr, 0, sizeof(adr));
adr.sin_family = AF_INET;
adr.sin_addr.s_addr = inet_addr(argv[1]);
adr.sin_port = htons(atoi(argv[2]));
connect(sock, (struct sockaddr *)&adr, sizeof(adr));
```

上述代码看似与 TCP 套接字创建过程一致，但 socket 函数的第二个参数分明是 SOCK_DGRAM 。也就是说，创建的的确是 UDP 套接字。当然针对 UDP 调用 connect 函数并不是意味着要与对方 UDP 套接字连接，这只是向 UDP 套接字注册目标IP和端口信息。

之后就与 TCP 套接字一致，每次调用 sendto 函数时只需传递信息数据。因为已经指定了收发对象，所以不仅可以使用 sendto、recvfrom 函数，还可以使用 write、read 函数进行通信。

下面的例子把之前的 [uecho_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch06/uecho_client.c) 程序改成了基于已连接 UDP 的套接字的程序，因此可以结合 [uecho_server.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch06/uecho_server.c) 程序运行。代码如下：

- [uecho_con_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch06/uecho_con_client.c)

编译运行过程与上面一样，故省略。

上面的代码中用 write、read 函数代替了 sendto、recvfrom 函数。

### 6.4 基于 Windows 的实现

暂略

### 6.5 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **UDP 为什么比 TCP 快？为什么 TCP 传输可靠而 TCP 传输不可靠？**

   答：为了提供可靠的数据传输服务，TCP 在不可靠的IP层进行流控制，而 UDP 缺少这种流控制。所以 UDP 是不可靠的连接。

2. **下面不属于 UDP 特点的是？**

   下面加粗的代表此句话正确

   1. **UDP 不同于 TCP ，不存在连接概念，所以不像 TCP 那样只能进行一对一的数据传输。**
   2. 利用 UDP 传输数据时，如果有 2 个目标，则需要 2 个套接字。
   3. UDP 套接字中无法使用已分配给 TCP 的同一端口号
   4. **UDP 套接字和 TCP 套接字可以共存。若需要，可以同时在同一主机进行 TCP 和 UDP 数据传输。**
   5. 针对 UDP 函数也可以调用 connect 函数，此时 UDP 套接字跟 TCP 套接字相同，也需要经过 3 次握手阶段。

3. **UDP 数据报向对方主机的 UDP 套接字传递过程中，IP 和 UDP 分别负责哪些部分？**

   答：IP的作用就是让离开主机的 UDP 数据包准确传递到另一个主机。但把 UDP 包最终交给主机的某一 UDP 套接字的过程则是由 UDP 完成的。UDP 的最重要的作用就是根据端口号将传到主机的数据包交付给最终的 UDP 套接字。

4. **UDP 一般比 TCP 快，但根据交换数据的特点，其差异可大可小。请你说明何种情况下 UDP 的性能优于 TCP？**

   答：如果收发数据量小但需要频繁连接时，UDP 比 TCP 更高效。

5. **客户端 TCP 套接字调用 connect 函数时自动分配IP和端口号。UDP 中不调用 bind 函数，那何时分配IP和端口号？**

   答：在首次调用 sendto 函数时自动给相应的套接字分配IP和端口号。而且此时分配的地址一直保留到程序结束为止。

6. **TCP 客户端必须调用 connect 函数，而 UDP 可以选择性调用。请问，在 UDP 中调用 connect 函数有哪些好处？** 

   答：要与同一个主机进行长时间通信时，将 UDP 套接字变成已连接套接字会提高效率。因为三个阶段中，第一个阶段和第三个阶段占用了一大部分时间，调用 connect 函数可以节省这些时间。

## 第 7 章 优雅的断开套接字的连接

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

本章讨论如何优雅的断开套接字的连接，之前用的方法不够优雅是因为，我们是调用 close 函数或 closesocket 函数单方面断开连接的。

### 7.1 基于 TCP 的半关闭

TCP 的断开连接过程比建立连接更重要，因为连接过程中一般不会出现大问题，但是断开过程可能发生预想不到的情况。因此应该准确掌控。所以要**掌握半关闭（Half-close）**，才能明确断开过程。

#### 7.1.1 单方面断开连接带来的问题

Linux 和 Windows 的 closesocket 函数意味着完全断开连接。完全断开不仅指无法传输数据，而且也不能接收数据。因此在某些情况下，通信一方单方面的断开套接字连接，显得不太优雅。如图所示：

![](https://i.loli.net/2019/01/18/5c412a8baa2d8.png)

图中描述的是 2 台主机正在进行双向通信，主机 A 发送完最后的数据后，调用 close 函数断开了最后的连接，之后主机 A 无法再接受主机 B 传输的数据。实际上，是完全无法调用与接受数据相关的函数。最终，由主机 B 传输的、主机 A 必须要接受的数据也销毁了。

为了解决这类问题，「只关闭一部分数据交换中使用的流」的方法应运而生。断开一部分连接是指，可以传输数据但是无法接收，或可以接受数据但无法传输。顾名思义就是只关闭流的一半。

#### 7.1.2 套接字和流（Stream）

两台主机通过套接字建立连接后进入可交换数据的状态，又称「流形成的状态」。也就是把建立套接字后可交换数据的状态看作一种流。

此处的流可以比作水流。水朝着一个方向流动，同样，在套接字的流中，数据也止呕能向一个方向流动。因此，为了进行双向通信，需要如图所示的两个流：

![](https://i.loli.net/2019/01/18/5c412c3ba25dd.png)

一旦两台主机之间建立了套接字连接，每个主机就会拥有单独的输入流和输出流。当然，其中一个主机的输入流与另一个主机的输出流相连，而输出流则与另一个主机的输入流相连。另外，本章讨论的「优雅的断开连接方式」只断开其中 1 个流，而非同时断开两个流。Linux 和 Windows 的 closesocket 函数将同时断开这两个流，因此与「优雅」二字还有一段距离。

#### 7.1.3 针对优雅断开的 shutdown 函数

shutdown 用来关闭其中一个流：

```c
#include <sys/socket.h>
int shutdown(int sock, int howto);
/*
成功时返回 0 ，失败时返回 -1
sock: 需要断开套接字文件描述符
howto: 传递断开方式信息
*/
```

调用上述函数时，第二个参数决定断开连接的方式，其值如下所示：

- `SHUT_RD` : 断开输入流
- `SHUT_WR` : 断开输出流
- `SHUT_RDWR` : 同时断开 I/O 流

若向 shutdown 的第二个参数传递`SHUT_RD`，则断开输入流，套接字无法接收数据。即使输入缓冲收到数据也回抹去，而且无法调用相关函数。如果向  shutdown 的第二个参数传递`SHUT_WR`，则中断输出流，也就无法传输数据。若如果输出缓冲中还有未传输的数据，则将传递给目标主机。最后，若传递关键字`SHUT_RDWR`，则同时中断 I/O 流。这相当于分 2 次调用 shutdown ，其中一次以`SHUT_RD`为参数，另一次以`SHUT_WR`为参数。

#### 7.1.4 为何要半关闭

考虑以下情况：

> 一旦客户端连接到服务器，服务器将约定的文件传输给客户端，客户端收到后发送字符串「Thank you」给服务器端。

此处「Thank you」的传递是多余的，这只是用来模拟客户端断开连接前还有数据要传输的情况。此时程序的还嫌难度并不小，因为传输文件的服务器端只需连续传输文件数据即可，而客户端无法知道需要接收数据到何时。客户端也没办法无休止的调用输入函数，因为这有可能导致程序**阻塞**。

> 是否可以让服务器和客户端约定一个代表文件尾的字符？

这种方式也有问题，因为这意味这文件中不能有与约定字符相同的内容。为了解决该问题，服务端应最后向客户端传递 EOF 表示文件传输结束。客户端通过函数返回值接受 EOF ，这样可以避免与文件内容冲突。那么问题来了，服务端如何传递 EOF ？

> 断开输出流时向主机传输 EOF。

当然，调用 close 函数的同时关闭 I/O 流，这样也会向对方发送 EOF 。但此时无法再接受对方传输的数据。换言之，若调用 close 函数关闭流，就无法接受客户端最后发送的字符串「Thank you」。这时需要调用 shutdown 函数，只关闭服务器的输出流。这样既可以发送 EOF ，同时又保留了输入流。下面实现收发文件的服务器端/客户端。

#### 7.1.5 基于半关闭的文件传输程序

上述文件传输服务器端和客户端的数据流可以整理如图：

![](https://i.loli.net/2019/01/18/5c41326280ab5.png)

下面的代码为编程简便，省略了大量错误处理代码。

- [file_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch07/file_client.c)
- [file_server.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch07/file_server.c)

编译运行：

```shell
gcc file_client.c -o fclient
gcc file_server.c -o fserver
./fserver 9190
./fclient 127.0.0.1 9190
```

结果：

![](https://i.loli.net/2019/01/18/5c4140bc8db2f.png)

客户端接受完成后，服务器会接收到来自客户端的感谢信息。

### 7.2 基于 Windows 的实现

暂略

### 7.3 习题

> 以下答案仅代表本人个人观点，可能不是正确答案

1. **解释 TCP 中「流」的概念。UDP 中能否形成流？请说明原因。**

   答：两台主机中通过套接字建立连接后进入可交换数据的状态，又称「流形成的状态」。也就是把建立套接字后可交换数据的状态看做一种流。UDP 没有建立连接的过程，所以不能形成流。

2. **Linux 中的 close 函数或 Windows 中的 closesocket 函数属于单方面断开连接的方法，有可能带来一些问题。什么是单方面断开连接？什么情形下会出现问题？**

   答：单方面断开连接就是两台主机正在通信，其中一台主机关闭了所有连接，那么一台主机向另一台主机传输的数据可能会没有接收到而损毁。传输文件的服务器只需连续传输文件数据即可，而客户端不知道需要接收数据到何时。客户端也没有办法无休止的调用输入函数。现在需要一个 EOF 代表数据已经传输完毕，那么这时就需要半关闭，服务端把自己的输出流关了，这时客户端就知数据已经传输完毕，因为服务端的输入流还没关，客户端可以给服务器汇报，接收完毕。

3. **什么是半关闭？针对输出流执行半关闭的主机处于何种状态？半关闭会导致对方主机接收什么消息？**

   答：半关闭就是把输入流或者输出流关了。针对输出流执行半关闭的主机处于可以接收数据而不能发送数据。半关闭会导致对方主机接收一个 EOF 文件结束符。对方就知道你的数据已经传输完毕。

## 第 8 章 域名及网络地址

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

### 8.1 域名系统

DNS 是对IP地址和域名进行相互转换的系统，其核心是 DNS 服务器

#### 8.1.1 什么是域名

域名就是我们常常在地址栏里面输入的地址，将比较难记忆的IP地址变成人类容易理解的信息。

#### 8.1.2 DNS 服务器

相当于一个字典，可以查询出某一个域名对应的IP地址

![](https://i.loli.net/2019/01/18/5c41854859ae3.png)

如图所示，显示了 DNS 服务器的查询路径。

### 8.2 IP地址和域名之间的转换

#### 8.2.1 程序中有必要使用域名吗？

一句话，需要，因为IP地址可能经常改变，而且也不容易记忆，通过域名可以随时更改解析，达到更换IP的目的

#### 8.2.2 利用域名获取IP地址

使用以下函数可以通过传递字符串格式的域名获取IP地址

```c
#include <netdb.h>
struct hostent *gethostbyname(const char *hostname);
/*
成功时返回 hostent 结构体地址，失败时返回 NULL 指针
*/
```

这个函数使用方便，只要传递字符串，就可以返回域名对应的IP地址。只是返回时，地址信息装入 hostent 结构体。此结构体的定义如下：

```c
struct hostent
{
    char *h_name;       /* Official name of host.  */
    char **h_aliases;   /* Alias list.  */
    int h_addrtype;     /* Host address type.  */
    int h_length;       /* Length of address.  */
    char **h_addr_list; /* List of addresses from name server.  */
};
```

从上述结构体可以看出，不止返回IP信息，同事还带着其他信息一起返回。域名转换成IP时只需要关注 h_addr_list 。下面简要说明上述结构体的成员：

- h_name：该变量中存有官方域名（Official domain name）。官方域名代表某一主页，但实际上，一些著名公司的域名并没有用官方域名注册。
- h_aliases：可以通过多个域名访问同一主页。同一IP可以绑定多个域名，因此，除官方域名外还可以指定其他域名。这些信息可以通过 h_aliases 获得。
- h_addrtype：gethostbyname 函数不仅支持 IPV4 还支持 IPV6 。因此可以通过此变量获取保存在 h_addr_list 的IP地址族信息。若是 IPV4 ，则此变量中存有 AF_INET。
- h_length：保存IP地址长度。若是 IPV4 地址，因为是 4 个字节，则保存4；IPV6 时，因为是 16 个字节，故保存 16
- h_addr_list：这个是最重要的的成员。通过此变量以整数形式保存域名相对应的IP地址。另外，用户比较多的网站有可能分配多个IP地址给同一个域名，利用多个服务器做负载均衡，。此时可以通过此变量获取IP地址信息。

调用 gethostbyname 函数后，返回的结构体变量如图所示：

![](https://i.loli.net/2019/01/18/5c41898ae45e8.png)

下面的代码通过一个例子来演示 gethostbyname 的应用，并说明 hostent 结构体变量特性。

- [gethostbyname.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch08/gethostbyname.c)

编译运行：

```shell
gcc gethostbyname.c -o hostname
./hostname www.baidu.com
```

结果：

![](https://i.loli.net/2019/01/18/5c418faf20495.png)

如图所示，显示出了对百度的域名解析

可以看出，百度有一个域名解析是 CNAME 解析的，指向了`shifen.com`，关于百度具体的解析过程。

> 这一部分牵扯到了很多关于DNS解析的过程，还有 Linux 下关于域名解析的一些命令，我找了一部分资料，可以点下面的链接查看比较详细的：
>
> - [关于百度DNS的解析过程](http://zhan.renren.com/starshen?gid=3602888498023142484&checked=true)
> - [DNS解析的过程是什么，求详细的？](https://www.zhihu.com/question/23042131/answer/66571369)
> - [Linux DNS 查询剖析](https://zhuanlan.zhihu.com/p/45535596)
> - [Linux DNS查询命令](http://www.live-in.org/archives/1938.html)
> - [Linux中DNS服务器地址查询命令nslookup使用教程](https://blog.csdn.net/shangdi1988/article/details/65713077)
> - [DNS 原理入门](http://www.ruanyifeng.com/blog/2016/06/dns.html)
>

仔细阅读这一段代码：

```c
inet_ntoa(*(struct in_addr *)host->h_addr_list[i])
```

若只看 hostent 的定义，结构体成员 h_addr_list 指向字符串指针数组（由多个字符串地址构成的数组）。但是字符串指针数组保存的元素实际指向的是 in_addr 结构体变量中地址值而非字符串，也就是说`(struct in_addr *)host->h_addr_list[i]`其实是一个指针，然后用`*`符号取具体的值。如图所示：

![](https://i.loli.net/2019/01/18/5c419658a73b8.png)

#### 8.2.3 利用IP地址获取域名

请看下面的函数定义：

```c
#include <netdb.h>
struct hostent *gethostbyaddr(const char *addr, socklen_t len, int family);
/*
成功时返回 hostent 结构体变量地址值，失败时返回 NULL 指针
addr: 含有IP地址信息的 in_addr 结构体指针。为了同时传递 IPV4 地址之外的全部信息，该变量的类型声明为 char 指针
len: 向第一个参数传递的地址信息的字节数，IPV4时为 4 ，IPV6 时为16.
family: 传递地址族信息，ipv4 是 AF_INET ，IPV6是 AF_INET6
*/
```

下面的代码演示使用方法：

- [gethostbyaddr.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch08/gethostbyaddr.c)

编译运行：

```shell
gcc gethostbyaddr.c -o hostaddr
./hostaddr 8.8.8.8
```

结果：

![](https://i.loli.net/2019/01/18/5c41a019085d4.png)

从图上可以看出，`8.8.8.8`这个IP地址是谷歌的。

### 8.3 基于 Windows 的实现

暂略

### 8.4 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **下列关于DNS的说法错误的是？**

   答：字体加粗的表示正确答案。

   1. **因为DNS从存在，故可以使用域名代替IP**
   2. DNS服务器实际上是路由器，因为路由器根据域名决定数据的路径
   3. **所有域名信息并非集中与 1 台 DNS 服务器，但可以获取某一 DNS 服务器中未注册的所有地址**
   4. DNS 服务器根据操作系统进行区分，Windows 下的 DNS 服务器和 Linux 下的 DNS 服务器是不同的。

2. **阅读如下对话，并说明东秀的方案是否可行。（因为对话的字太多，用图代替）**

   ![](https://i.loli.net/2019/01/18/5c41a22f35390.png)

   答：答案就是可行，DNS 服务器是分布式的，一台坏了可以找其他的。

3. **再浏览器地址输入 www.orentec.co.kr ，并整理出主页显示过程。假设浏览器访问默认 DNS 服务器中并没有关于 www.orentec.co.kr 的地址信息.**

   答：可以参考一下知乎回答，[在浏览器地址栏输入一个URL后回车，背后会进行哪些技术步骤？](https://www.zhihu.com/question/34873227/answer/518086565),我用我自己的理解，简单说一下，首先会去向上一级的 DNS 服务器去查询，通过这种方式逐级向上传递信息，一直到达根服务器时，它知道应该向哪个 DNS 服务器发起询问。向下传递解析请求，得到IP地址候原路返回，最后会将解析的IP地址传递到发起请求的主机。

## 第 9 章 套接字的多种可选项

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

### 9.1 套接字可选项和 I/O 缓冲大小

我们进行套接字编程时往往只关注数据通信，而忽略了套接字具有的不同特性。但是，理解这些特性并根据实际需要进行更改也很重要

#### 9.1.1 套接字多种可选项

我们之前写得程序都是创建好套接字之后直接使用的，此时通过默认的套接字特性进行数据通信，这里列出了一些套接字可选项。

| 协议层 | 选项名 | 读取 | 设置 |
| :----: | :----: |:--: | :--: |
| SOL_SOCKET  |     SO_SNDBUF     |  O   |  O   |
| SOL_SOCKET  |     SO_RCVBUF     |  O   |  O   |
| SOL_SOCKET  |   SO_REUSEADDR    |  O   |  O   |
| SOL_SOCKET  |   SO_KEEPALIVE    |  O   |  O   |
| SOL_SOCKET  |   SO_BROADCAST    |  O   |  O   |
| SOL_SOCKET  |   SO_DONTROUTE    |  O   |  O   |
| SOL_SOCKET  |   SO_OOBINLINE    |  O   |  O   |
| SOL_SOCKET  |     SO_ERROR      |  O   |  X   |
| SOL_SOCKET  |      SO_TYPE      |  O   |  X   |
| IPPROTO_IP  |      IP_TOS       |  O   |  O   |
| IPPROTO_IP  |      IP_TTL       |  O   |  O   |
| IPPROTO_IP  | IP_MULTICAST_TTL  |  O   |  O   |
| IPPROTO_IP  | IP_MULTICAST_LOOP |  O   |  O   |
| IPPROTO_IP  |  IP_MULTICAST_IF  |  O   |  O   |
| IPPROTO_TCP |   TCP_KEEPALIVE   |  O   |  O   |
| IPPROTO_TCP |    TCP_NODELAY    |  O   |  O   |
| IPPROTO_TCP |    TCP_MAXSEG     |  O   |  O   |

从表中可以看出，套接字可选项是分层的。

- IPPROTO_IP 可选项是IP协议相关事项

- IPPROTO_TCP 层可选项是 TCP 协议的相关事项

- SOL_SOCKET 层是套接字的通用可选项。

#### 9.1.2 `getsockopt` & `setsockopt`

可选项的读取和设置通过以下两个函数来完成

```c
#include <sys/socket.h>

int getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);
/*
成功时返回 0 ，失败时返回 -1
sock: 用于查看选项套接字文件描述符
level: 要查看的可选项协议层
optname: 要查看的可选项名
optval: 保存查看结果的缓冲地址值
optlen: 向第四个参数传递的缓冲大小。调用函数候，该变量中保存通过第四个参数返回的可选项信息的字节数。
*/
```

上述函数可以用来读取套接字可选项，下面的函数可以更改可选项：d

```c
#include <sys/socket.h>

int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);
/*
成功时返回 0 ，失败时返回 -1
sock: 用于更改选项套接字文件描述符
level: 要更改的可选项协议层
optname: 要更改的可选项名
optval: 保存更改结果的缓冲地址值
optlen: 向第四个参数传递的缓冲大小。调用函数候，该变量中保存通过第四个参数返回的可选项信息的字节数。
*/
```

下面的代码可以看出 getsockopt 的使用方法。下面示例用协议层为 SOL_SOCKET 、名为 SO_TYPE 的可选项查看套接字类型（TCP 和 UDP ）。

- [sock_type.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch09/sock_type.c)

编译运行：

```shell
gcc sock_type.c -o sock_type
./sock_type
```

结果：

```
SOCK_STREAM: 1
SOCK_DGRAM: 2
Socket type one: 1
Socket type two: 2
```

首先创建了一个 TCP 套接字和一个 UDP 套接字。然后通过调用 getsockopt 函数来获得当前套接字的状态。

验证套接类型的 SO_TYPE 是只读可选项，因为**套接字类型只能在创建时决定，以后不能再更改**。

#### 9.1.3 `SO_SNDBUF` & `SO_RCVBUF`

创建套接字的同时会生成 I/O 缓冲。关于 I/O 缓冲，可以去看第五章。

SO_RCVBUF 是输入缓冲大小相关可选项，SO_SNDBUF 是输出缓冲大小相关可选项。用这 2 个可选项既可以读取当前 I/O 大小，也可以进行更改。通过下列示例读取创建套接字时默认的 I/O 缓冲大小。

- [get_buf.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch09/get_buf.c)

编译运行：

```shell
gcc get_buf.c -o getbuf
./getbuf
```

运行结果：

```
Input buffer size: 87380
Output buffer size: 16384
```

可以看出本机的输入缓冲和输出缓冲大小。

下面的代码演示了，通过程序设置 I/O 缓冲区的大小

- [set_buf.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch09/set_buf.c)

编译运行：

```shell
gcc get_buf.c -o setbuf
./setbuf
```

结果:

```
Input buffer size: 6144
Output buffer size: 6144
```

输出结果和我们预想的不是很相同，缓冲大小的设置需谨慎处理，因此不会完全按照我们的要求进行。

### 9.2 `SO_REUSEADDR`

#### 9.2.1 发生地址分配错误（Binding Error）

在学习 SO_REUSEADDR 可选项之前，应该好好理解 Time-wait 状态。看以下代码的示例：

- [reuseadr_eserver.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch09/reuseadr_eserver.c)

这是一个回声服务器的服务端代码，可以配合第四章的 [echo_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch04/echo_client.c) 使用，在这个代码中，客户端通知服务器终止程序。在客户端控制台输入 Q 可以结束程序，向服务器发送 FIN 消息并经过四次握手过程。当然，输入 CTRL+C 也会向服务器传递 FIN 信息。强制终止程序时，由操作系统关闭文件套接字，此过程相当于调用 close 函数，也会向服务器发送 FIN 消息。

这样看不到是什么特殊现象，考虑以下情况：

> 服务器端和客户端都已经建立连接的状态下，向服务器控制台输入 CTRL+C ，强制关闭服务端

如果用这种方式终止程序，如果用同一端口号再次运行服务端，就会输出「bind() error」消息，并且无法再次运行。但是在这种情况下，再过大约 3 分钟就可以重新运行服务端。

#### 9.2.2 `Time-wait` 状态

观察以下过程：

![](https://i.loli.net/2019/01/19/5c42db182cade.png)

假设图中主机 A 是服务器，因为是主机 A 向 B 发送 FIN 消息，故可想象成服务器端在控制台中输入 CTRL+C 。但是问题是，套接字经过四次握手后并没有立即消除，而是要经过一段时间的 Time-wait 状态。当然，只有先断开连接的（先发送 FIN 消息的）主机才经过 Time-wait 状态。因此，若服务器端先断开连接，则无法立即重新运行。套接字处在 Time-wait 过程时，相应端口是正在使用的状态。因此，就像之前验证过的，bind 函数调用过程中会发生错误。

**实际上，不论是服务端还是客户端，都要经过一段时间的 Time-wait 过程。先断开连接的套接字必然会经过 Time-wait 过程，但是由于客户端套接字的端口是任意制定的，所以无需过多关注 Time-wait 状态。**

那到底为什么会有 Time-wait 状态呢，在图中假设，主机 A 向主机 B 传输 ACK 消息（SEQ 5001 , ACK 7502 ）后立刻消除套接字。但是最后这条 ACK 消息在传递过程中丢失，没有传递主机 B ，这时主机 B 就会试图重传。但是此时主机 A 已经是完全终止状态，因为主机 B 永远无法收到从主机 A 最后传来的 ACK 消息。基于这些问题的考虑，所以要设计 Time-wait 状态。

#### 9.2.3 地址再分配

Time-wait 状态看似重要，但是不一定讨人喜欢。如果系统发生故障紧急停止，这时需要尽快重启服务起以提供服务，但因处于 Time-wait 状态而必须等待几分钟。因此，Time-wait 并非只有优点，这些情况下容易引发大问题。下图中展示了四次握手时不得不延长 Time-wait 过程的情况。

![](https://i.loli.net/2019/01/19/5c42dec2ba42b.png)

从图上可以看出，在主机 A 四次握手的过程中，如果最后的数据丢失，则主机 B 会认为主机 A 未能收到自己发送的 FIN 信息，因此重传。这时，收到的 FIN 消息的主机 A 将重启  Time-wait 计时器。因此，如果网络状况不理想， Time-wait 将持续。

解决方案就是在套接字的可选项中更改 SO_REUSEADDR 的状态。适当调整该参数，可将 Time-wait 状态下的套接字端口号重新分配给新的套接字。SO_REUSEADDR 的默认值为 0.这就意味着无法分配 Time-wait 状态下的套接字端口号。因此需要将这个值改成 1 。具体作法已在示例 [reuseadr_eserver.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch09/reuseadr_eserver.c) 给出，只需要把注释掉的东西接解除注释即可。

```c
optlen = sizeof(option);
option = TRUE;
setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);
```

此时，已经解决了上述问题。

### 9.3 `TCP_NODELAY`

#### 9.3.1 `Nagle` 算法

为了防止因数据包过多而发生网络过载，`Nagle` 算法诞生了。它应用于 TCP 层。它是否使用会导致如图所示的差异：

![](https://i.loli.net/2019/01/19/5c42e12abc5b8.png)

图中展示了通过 `Nagle` 算法发送字符串 `Nagle` 和未使用 `Nagle` 算法的差别。可以得到一个结论。

**只有接收到前一数据的 ACK 消息， `Nagle` 算法才发送下一数据。**

TCP 套接字默认使用 `Nagle` 算法交换数据，因此最大限度的进行缓冲，直到收到 ACK 。左图也就是说一共传递 4 个数据包以传输一个字符串。从右图可以看出，发送数据包一共使用了 10 个数据包。由此可知，不使用 `Nagle` 算法将对网络流量产生负面影响。即使只传输一个字节的数据，其头信息都可能是几十个字节。因此，为了提高网络传输效率，必须使用 `Nagle` 算法。

 `Nagle` 算法并不是什么情况下都适用，网络流量未受太大影响时，不使用 `Nagle` 算法要比使用它时传输速度快。最典型的就是「传输大文数据」。将文件数据传入输出缓冲不会花太多时间，因此，不使用 `Nagle` 算法，也会在装满输出缓冲时传输数据包。这不仅不会增加数据包的数量，反而在无需等待 ACK 的前提下连续传输，因此可以大大提高传输速度。

所以，未准确判断数据性质时不应禁用 `Nagle` 算法。

#### 9.3.2 禁用 `Nagle` 算法

禁用 `Nagle` 算法应该使用：

```c
int opt_val = 1;
setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt_val, sizeof(opt_val));
```

通过 TCP_NODELAY 的值来查看`Nagle` 算法的设置状态。

```c
opt_len = sizeof(opt_val);
getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt_val, opt_len);
```

如果正在使用`Nagle` 算法，那么 opt_val 值为 0，如果禁用则为 1.

关于这个算法，可以参考这个回答：[TCP连接中启用和禁用TCP_NODELAY有什么影响？](https://www.zhihu.com/question/42308970/answer/246334766)


### 9.4 基于 Windows 的实现

暂略

### 9.5 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **下列关于 Time-wait 状态的说法错误的是？**

   答：以下字体加粗的代表正确。

   1. Time-wait 状态只在服务器的套接字中发生
   2. **断开连接的四次握手过程中，先传输 FIN 消息的套接字将进入 Time-wait 状态。**
   3. Time-wait 状态与断开连接的过程无关，而与请求连接过程中 SYN 消息的传输顺序有关
   4. Time-wait 状态通常并非必要，应尽可能通过更改套接字可选项来防止其发生

2. **TCP_NODELAY 可选项与 Nagle 算法有关，可通过它禁用 Nagle 算法。请问何时应考虑禁用 Nagle 算法？结合收发数据的特性给出说明。**

   答：当网络流量未受太大影响时，不使用 Nagle 算法要比使用它时传输速度快，比如说在传输大文件时。

## 第 10 章 多进程服务端

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

### 10.1 进程概念及应用

#### 10.1.1 并发服务端的实现方法

通过改进服务端，使其同时向所有发起请求的客户端提供服务，以提高平均满意度。而且，网络程序中数据通信时间比 CPU 运算时间占比更大，因此，向多个客户端提供服务是一种有效的利用 CPU 的方式。接下来讨论同时向多个客户端提供服务的并发服务器端。下面列出的是具有代表性的并发服务端的实现模型和方法：

- 多进程服务器：通过创建多个进程提供服务
- 多路复用服务器：通过捆绑并统一管理 I/O 对象提供服务
- 多线程服务器：通过生成与客户端等量的线程提供服务

先是第一种方法：多进程服务器

#### 10.1.2 理解进程

进程的定义如下：

> 占用内存空间的正在运行的程序

假如你下载了一个游戏到电脑上，此时的游戏不是进程，而是程序。只有当游戏被加载到主内存并进入运行状态，这是才可称为进程。

#### 10.1.3 进程 ID 

在说进程创建方法之前，先要简要说明进程 ID。无论进程是如何创建的，所有的进程都会被操作系统分配一个 ID。此 ID 被称为「进程ID」，其值为大于 2 的证书。1 要分配给操作系统启动后的（用于协助操作系统）首个进程，因此用户无法得到 ID 值为 1 。接下来观察在 Linux 中运行的进程。

```shell
ps au
```

通过上面的命令可查看当前运行的所有进程。需要注意的是，该命令同时列出了 PID（进程ID）。参数 a 和 u列出了所有进程的详细信息。

![](https://i.loli.net/2019/01/20/5c43d7c1f2a8b.png)

#### 10.1.4 通过调用 fork 函数创建进程

创建进程的方式很多，此处只介绍用于创建多进程服务端的 fork 函数。

```c
#include <unistd.h>
pid_t fork(void);
// 成功时返回进程ID,失败时返回 -1
```

fork 函数将创建调用的进程副本。也就是说，并非根据完全不同的程序创建进程，而是复制正在运行的、调用 fork 函数的进程。另外，两个进程都执行 fork 函数调用后的语句（准确的说是在 fork 函数返回后）。但因为是通过同一个进程、复制相同的内存空间，之后的程序流要根据 fork 函数的返回值加以区分。即利用 fork 函数的如下特点区分程序执行流程。

- 父进程：fork 函数返回子进程 ID
- 子进程：fork 函数返回 0

此处，「父进程」（Parent Process）指原进程，即调用 fork 函数的主体，而「子进程」（Child Process）是通过父进程调用 fork 函数复制出的进程。接下来是调用 fork 函数后的程序运行流程。如图所示：

![](https://i.loli.net/2019/01/20/5c43da5412b90.png)

从图中可以看出，父进程调用 fork 函数的同时复制出子进程，并分别得到 fork 函数的返回值。但复制前，父进程将全局变量 gval 增加到 11,将局部变量 lval 的值增加到 25，因此在这种状态下完成进程复制。复制完成后根据 fork 函数的返回类型区分父子进程。父进程的 lval 的值增加 1 ，但这不会影响子进程的 lval 值。同样子进程将 gval 的值增加 1 也不会影响到父进程的 gval 。因为 fork 函数调用后分成了完全不同的进程，只是二者共享同一段代码而已。接下来给出一个例子：

- [fork.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch10/fork.c)

```c
#include <stdio.h>
#include <unistd.h>
int gval = 10;
int main(int argc, char *argv[])
{
    pid_t pid;
    int lval = 20;
    gval++, lval += 5;
    pid = fork();
    if (pid == 0)
        gval += 2, lval += 2;
    else
        gval -= 2, lval -= 2;
    if (pid == 0)
        printf("Child Proc: [%d,%d] \n", gval, lval);
    else
        printf("Parent Proc: [%d,%d] \n", gval, lval);
    return 0;
}
```

编译运行：

```shell
gcc fork.c -o fork
./fork
```

运行结果：

![](https://i.loli.net/2019/01/20/5c43e054e7f6f.png)

可以看出，当执行了 fork 函数之后，此后就相当于有了两个程序在执行代码，对于父进程来说，fork 函数返回的是子进程的ID，对于子进程来说，fork 函数返回 0。所以这两个变量，父进程进行了 +2 操作 ，而子进程进行了 -2 操作，所以结果是这样。

### 10.2 进程和僵尸进程

文件操作中，关闭文件和打开文件同等重要。同样，进程销毁和进程创建也同等重要。如果未认真对待进程销毁，他们将变成僵尸进程。

#### 10.2.1 僵尸（Zombie）进程

进程的工作完成后（执行完 main 函数中的程序后）应被销毁，但有时这些进程将变成僵尸进程，占用系统中的重要资源。这种状态下的进程称作「僵尸进程」，这也是给系统带来负担的原因之一。

> 僵尸进程是当子进程比父进程先结束，而父进程又没有回收子进程，释放子进程占用的资源，此时子进程将成为一个僵尸进程。如果父进程先退出 ，子进程被init接管，子进程退出后init会回收其占用的相关资源

**维基百科**：

> 在类UNIX系统中，僵尸进程是指完成执行（通过exit系统调用，或运行时发生致命错误或收到终止信号所致）但在操作系统的进程表中仍然有一个表项（进程控制块PCB），处于"终止状态"的进程。这发生于子进程需要保留表项以允许其父进程读取子进程的exit status：一旦退出态通过wait系统调用读取，僵尸进程条目就从进程表中删除，称之为"回收（reaped）"。正常情况下，进程直接被其父进程wait并由系统回收。进程长时间保持僵尸状态一般是错误的并导致资源泄漏。
>
> 英文术语zombie process源自丧尸 — 不死之人，隐喻子进程已死但仍然没有被收割。与正常进程不同，kill命令对僵尸进程无效。孤儿进程不同于僵尸进程，其父进程已经死掉，但孤儿进程仍能正常执行，但并不会变为僵尸进程，因为被init（进程ID号为1）收养并wait其退出。
>
> 子进程死后，系统会发送SIGCHLD 信号给父进程，父进程对其默认处理是忽略。如果想响应这个消息，父进程通常在SIGCHLD 信号事件处理程序中，使用wait系统调用来响应子进程的终止。
>
> 僵尸进程被收割后，其进程号(PID)与在进程表中的表项都可以被系统重用。但如果父进程没有调用wait，僵尸进程将保留进程表中的表项，导致了资源泄漏。某些情况下这反倒是期望的：父进程创建了另外一个子进程，并希望具有不同的进程号。如果父进程通过设置事件处理函数为SIG_IGN显式忽略SIGCHLD信号，而不是隐式默认忽略该信号，或者具有SA_NOCLDWAIT标志，所有子进程的退出状态信息将被抛弃并且直接被系统回收。
>
> UNIX命令ps列出的进程的状态（"STAT"）栏标示为 "Z"则为僵尸进程。[1]
>
> 收割僵尸进程的方法是通过kill命令手工向其父进程发送SIGCHLD信号。如果其父进程仍然拒绝收割僵尸进程，则终止父进程，使得init进程收养僵尸进程。init进程周期执行wait系统调用收割其收养的所有僵尸进程。

#### 10.2.2 产生僵尸进程的原因

为了防止僵尸进程产生，先解释产生僵尸进程的原因。利用如下两个示例展示调用 fork 函数产生子进程的终止方式。

- 传递参数并调用 exit() 函数
- main 函数中执行 return 语句并返回值

**向 exit 函数传递的参数值和 main 函数的 return 语句返回的值都回传递给操作系统。而操作系统不会销毁子进程，直到把这些值传递给产生该子进程的父进程。处在这种状态下的进程就是僵尸进程。**也就是说将子进程变成僵尸进程的正是操作系统。既然如此，僵尸进程何时被销毁呢？

> 应该向创建子进程册父进程传递子进程的 exit 参数值或 return 语句的返回值。

如何向父进程传递这些值呢？操作系统不会主动把这些值传递给父进程。只有父进程主动发起请求（函数调用）的时候，操作系统才会传递该值。换言之，如果父进程未主动要求获得子进程结束状态值，操作系统将一直保存，并让子进程长时间处于僵尸进程状态。也就是说，父母要负责收回自己生的孩子。接下来的示例是创建僵尸进程：

- [zombie.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch10/zombie.c)

```c
#include <stdio.h>
#include <unistd.h>
int main(int argc, char *argv[])
{
    pid_t pid = fork();
    if (pid == 0)
    {
        puts("Hi, I am a child Process");
    }
    else
    {
        printf("Child Process ID: %d \n", pid);
        sleep(30);
    }
    if (pid == 0)
        puts("End child proess");
    else
        puts("End parent process");
    return 0;
}
```

编译运行：

```shell
gcc zombie.c -o zombie
./zombie
```

结果：

![](https://i.loli.net/2019/01/20/5c443890f1781.png)

因为暂停了 30 秒，所以在这个时间内可以验证一下子进程是否为僵尸进程。

![](https://i.loli.net/2019/01/20/5c4439a751b11.png)

通过 `ps au` 命令可以看出，子进程仍然存在，并没有被销毁，僵尸进程在这里显示为 `Z+`.30秒后，红框里面的两个进程会同时被销毁。

> 利用 `./zombie &`可以使程序在后台运行，不用打开新的命令行窗口。

#### 10.2.3 销毁僵尸进程 1：利用 wait 函数

如前所述，为了销毁子进程，父进程应该主动请求获取子进程的返回值。下面是发起请求的具体方法。有两种，下面的函数是其中一种。

```c
#include <sys/wait.h>
pid_t wait(int *statloc);
/*
成功时返回终止的子进程 ID ,失败时返回 -1
*/
```

调用此函数时如果已有子进程终止，那么子进程终止时传递的返回值（exit 函数的参数返回值，main 函数的 return 返回值）将保存到该函数的参数所指的内存空间。但函数参数指向的单元中还包含其他信息，因此需要用下列宏进行分离：

- WIFEXITED 子进程正常终止时返回「真」
- WEXITSTATUS 返回子进程时的返回值

也就是说，向 wait 函数传递变量 status 的地址时，调用 wait 函数后应编写如下代码：

```c
if (WIFEXITED(status))
{
    puts("Normal termination");
    printf("Child pass num: %d", WEXITSTATUS(status));
}
```

根据以上内容，有如下示例：

- [wait.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch10/wait.c)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int status;
    pid_t pid = fork(); //这里的子进程将在第13行通过 return 语句终止

    if (pid == 0)
    {
        return 3;
    }
    else
    {
        printf("Child PID: %d \n", pid);
        pid = fork(); //这里的子进程将在 21 行通过 exit() 函数终止
        if (pid == 0)
        {
            exit(7);
        }
        else
        {
            printf("Child PID: %d \n", pid);
            wait(&status);         //之间终止的子进程相关信息将被保存到 status 中，同时相关子进程被完全销毁
            if (WIFEXITED(status)) //通过 WIFEXITED 来验证子进程是否正常终止。如果正常终止，则调用 WEXITSTATUS 宏输出子进程返回值
                printf("Child send one: %d \n", WEXITSTATUS(status));

            wait(&status); //因为之前创建了两个进程，所以再次调用 wait 函数和宏
            if (WIFEXITED(status))
                printf("Child send two: %d \n", WEXITSTATUS(status));
            sleep(30);
        }
    }
    return 0;
}
```

编译运行：

```shell
gcc wait.c -o wait
./wait
```

结果：

![](https://i.loli.net/2019/01/20/5c4441951df43.png)

此时，系统中并没有上述 PID 对应的进程，这是因为调用了 wait 函数，完全销毁了该子进程。另外两个子进程返回时返回的 3 和 7 传递到了父进程。

这就是通过 wait 函数消灭僵尸进程的方法，调用 wait 函数时，如果没有已经终止的子进程，那么程序将阻塞（Blocking）直到有子进程终止，因此要谨慎调用该函数。

#### 10.2.4 销毁僵尸进程 2：使用 waitpid 函数

wait 函数会引起程序阻塞，还可以考虑调用 waitpid 函数。这是防止僵尸进程的第二种方法，也是防止阻塞的方法。

```c
#include <sys/wait.h>
pid_t waitpid(pid_t pid, int *statloc, int options);
/*
成功时返回终止的子进程ID 或 0 ，失败时返回 -1
pid: 等待终止的目标子进程的ID,若传 -1，则与 wait 函数相同，可以等待任意子进程终止
statloc: 与 wait 函数的 statloc 参数具有相同含义
options: 传递头文件 sys/wait.h 声明的常量 WNOHANG ,即使没有终止的子进程也不会进入阻塞状态，而是返回 0 退出函数。
*/
```

以下是 waitpid 的使用示例：

- [waitpid.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch10/waitpid.c)

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char *argv[])
{
    int status;
    pid_t pid = fork();
    if (pid == 0)
    {
        sleep(15); //用 sleep 推迟子进程的执行
        return 24;
    }
    else
    {
        //调用waitpid 传递参数 WNOHANG ，这样之前有没有终止的子进程则返回0
        while (!waitpid(-1, &status, WNOHANG))
        {
            sleep(3);
            puts("sleep 3 sec.");
        }
        if (WIFEXITED(status))
            printf("Child send %d \n", WEXITSTATUS(status));
    }
    return 0;
}
```

编译运行：

```shell
gcc waitpid.c -o waitpid
./waitpid
```

结果:

![](https://i.loli.net/2019/01/20/5c444785a16ae.png)

可以看出来，在 while 循环中正好执行了 5 次。这也证明了 waitpid 函数并没有阻塞

### 10.3 信号处理

我们已经知道了进程的创建及销毁的办法，但是还有一个问题没有解决。

> 子进程究竟何时终止？调用 waitpid 函数后要无休止的等待吗？

#### 10.3.1 向操作系统求助

子进程终止的识别主题是操作系统，因此，若操作系统能把如下信息告诉正忙于工作的父进程，将有助于构建更高效的程序

为了实现上述的功能，引入信号处理机制（Signal Handing）。此处「信号」是在特定事件发生时由操作系统向进程发送的消息。另外，为了响应该消息，执行与消息相关的自定义操作的过程被称为「处理」或「信号处理」。

#### 10.3.2 信号与 signal 函数

下面进程和操作系统的对话可以帮助理解信号处理。

> 进程：操作系统，如果我之前创建的子进程终止，就帮我调用 zombie_handler 函数。
>
> 操作系统：好的，如果你的子进程终止，我舅帮你调用 zombie_handler 函数，你先把要函数要执行的语句写好。

上述的对话，相当于「注册信号」的过程。即进程发现自己的子进程结束时，请求操作系统调用的特定函数。该请求可以通过如下函数调用完成：

```c
#include <signal.h>
void (*signal(int signo, void (*func)(int)))(int);
/*
为了在产生信号时调用，返回之前注册的函数指针
函数名: signal
参数：int signo,void(*func)(int)
返回类型：参数类型为int型，返回 void 型函数指针
*/
```

调用上述函数时，第一个参数为特殊情况信息，第二个参数为特殊情况下将要调用的函数的地址值（指针）。发生第一个参数代表的情况时，调用第二个参数所指的函数。下面给出可以在 signal 函数中注册的部分特殊情况和对应的函数。

- SIGALRM：已到通过调用 alarm 函数注册时间
- SIGINT：输入 ctrl+c
- SIGCHLD：子进程终止

接下来编写调用 signal 函数的语句完成如下请求：

> 「子进程终止则调用 mychild 函数」

此时 mychild 函数的参数应为 int ，返回值类型应为 void 。只有这样才能称为 signal 函数的第二个参数。另外，常数 SIGCHLD 定义了子进程终止的情况，应成为 signal 函数的第一个参数。也就是说，signal 函数调用语句如下：

```c
signal(SIGCHLD , mychild);
```

接下来编写 signal 函数的调用语句，分别完成如下两个请求：

1. 已到通过 alarm 函数注册时间，请调用 timeout 函数
2. 输入 ctrl+c 时调用 keycontrol 函数

代表这 2 种情况的常数分别为 SIGALRM 和 SIGINT ，因此按如下方式调用 signal 函数。

```c
signal(SIGALRM , timeout);
signal(SIGINT , keycontrol);
```

以上就是信号注册过程。注册好信号之后，发生注册信号时（注册的情况发生时），操作系统将调用该信号对应的函数。先介绍 alarm 函数。

```c
#include <unistd.h>
unsigned int alarm(unsigned int seconds);
// 返回0或以秒为单位的距 SIGALRM 信号发生所剩时间
```

如果调用该函数的同时向它传递一个正整型参数，相应时间后（以秒为单位）将产生 SIGALRM 信号。若向该函数传递为 0 ，则之前对 SIGALRM 信号的预约将取消。如果通过改函数预约信号后未指定该信号对应的处理函数，则（通过调用 signal 函数）终止进程，不做任何处理。

- [signal.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch10/signal.c)

```c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
void timeout(int sig) //信号处理器
{
    if (sig == SIGALRM)
        puts("Time out!");
    alarm(2); //为了每隔 2 秒重复产生 SIGALRM 信号，在信号处理器中调用 alarm 函数
}
void keycontrol(int sig) //信号处理器
{
    if (sig == SIGINT)
        puts("CTRL+C pressed");
}
int main(int argc, char *argv[])
{
    int i;
    signal(SIGALRM, timeout); //注册信号及相应处理器
    signal(SIGINT, keycontrol);
    alarm(2); //预约 2 秒候发生 SIGALRM 信号

    for (i = 0; i < 3; i++)
    {
        puts("wait...");
        sleep(100);
    }
    return 0;
}
```

编译运行：

```shell
gcc signal.c -o signal
./signal
```

结果：

![](https://i.loli.net/2019/01/20/5c446c877acb7.png)

上述结果是没有任何输入的运行结果。当输入 ctrl+c 时:

![](https://i.loli.net/2019/01/20/5c446ce0b1143.png)

就可以看到 `CTRL+C pressed` 的字符串。

> 发生信号时将唤醒由于调用 sleep 函数而进入阻塞状态的进程。

调用函数的主题的确是操作系统，但是进程处于睡眠状态时无法调用函数，因此，产生信号时，为了调用信号处理器，将唤醒由于调用 sleep 函数而进入阻塞状态的进程。而且，进程一旦被唤醒，就不会再进入睡眠状态。即使还未到 sleep 中规定的时间也是如此。所以上述示例运行不到 10 秒后就会结束，连续输入 CTRL+C 可能连一秒都不到。

**简言之，就是本来系统要睡眠100秒，但是到了 alarm(2) 规定的两秒之后，就会唤醒睡眠的进程，进程被唤醒了就不会再进入睡眠状态了，所以就不用等待100秒。如果把 timeout() 函数中的 alarm(2) 注释掉，就会先输出`wait...`，然后再输出`Time out!` (这时已经跳过了第一次的 sleep(100) 秒),然后就真的会睡眠100秒，因为没有再发出 alarm(2)  的信号。**

#### 10.3.3 利用 sigaction 函数进行信号处理

前面所学的内容可以防止僵尸进程，还有一个函数，叫做 sigaction 函数，他类似于 signal 函数，而且可以完全代替后者，也更稳定。之所以稳定，是因为：

> signal 函数在 Unix 系列的不同操作系统可能存在区别，但 sigaction 函数完全相同

实际上现在很少用 signal 函数编写程序，他只是为了保持对旧程序的兼容，下面介绍 sigaction 函数，只讲解可以替换 signal 函数的功能。

```c
#include <signal.h>

int sigaction(int signo, const struct sigaction *act, struct sigaction *oldact);
/*
成功时返回 0 ，失败时返回 -1
act: 对于第一个参数的信号处理函数（信号处理器）信息。
oldact: 通过此参数获取之前注册的信号处理函数指针，若不需要则传递 0
*/
```

声明并初始化 sigaction 结构体变量以调用上述函数，该结构体定义如下：

```c
struct sigaction
{
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
};
```

此结构体的成员 sa_handler 保存信号处理的函数指针值（地址值）。sa_mask 和 sa_flags 的所有位初始化 0 即可。这 2 个成员用于指定信号相关的选项和特性，而我们的目的主要是防止产生僵尸进程，故省略。

下面的示例是关于 sigaction 函数的使用方法。

- [sigaction.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch10/sigaction.c)

```c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void timeout(int sig)
{
    if (sig == SIGALRM)
        puts("Time out!");
    alarm(2);
}

int main(int argc, char *argv[])
{
    int i;
    struct sigaction act;
    act.sa_handler = timeout;    //保存函数指针
    sigemptyset(&act.sa_mask);   //将 sa_mask 函数的所有位初始化成0
    act.sa_flags = 0;            //sa_flags 同样初始化成 0
    sigaction(SIGALRM, &act, 0); //注册 SIGALRM 信号的处理器。

    alarm(2); //2 秒后发生 SIGALRM 信号

    for (int i = 0; i < 3; i++)
    {
        puts("wait...");
        sleep(100);
    }
    return 0;
}

```

编译运行：

```shell
gcc sigaction.c -o sigaction
./sigaction
```

结果：

```
wait...
Time out!
wait...
Time out!
wait...
Time out!
```

可以发现，结果和之前用 signal 函数的结果没有什么区别。以上就是信号处理的相关理论。

#### 10.3.4 利用信号处理技术消灭僵尸进程

下面利用子进程终止时产生 SIGCHLD 信号这一点，来用信号处理来消灭僵尸进程。看以下代码：

- [remove_zomebie.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch10/remove_zomebie.c)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void read_childproc(int sig)
{
    int status;
    pid_t id = waitpid(-1, &status, WNOHANG);
    if (WIFEXITED(status))
    {
        printf("Removed proc id: %d \n", id);             //子进程的 pid
        printf("Child send: %d \n", WEXITSTATUS(status)); //子进程的返回值
    }
}

int main(int argc, char *argv[])
{
    pid_t pid;
    struct sigaction act;
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);

    pid = fork();
    if (pid == 0) //子进程执行阶段
    {
        puts("Hi I'm child process");
        sleep(10);
        return 12;
    }
    else //父进程执行阶段
    {
        printf("Child proc id: %d\n", pid);
        pid = fork();
        if (pid == 0)
        {
            puts("Hi! I'm child process");
            sleep(10);
            exit(24);
        }
        else
        {
            int i;
            printf("Child proc id: %d \n", pid);
            for (i = 0; i < 5; i++)
            {
                puts("wait");
                sleep(5);
            }
        }
    }
    return 0;
}
```

编译运行：

```shell
gcc remove_zomebie.c -o zombie
./zombie
```

结果：

```
Child proc id: 11211
Hi I'm child process
Child proc id: 11212 
wait
Hi! I'm child process

wait

wait
Removed proc id: 11211 
Child send: 12 
wait
Removed proc id: 11212 
Child send: 24 
wait
```

请自习观察结果，结果中的每一个空行代表间隔了5 秒，程序是先创建了两个子进程，然后子进程 10  秒之后会返回值，第一个 wait 由于子进程在执行，所以直接被唤醒，然后这两个子进程正在睡 10 秒，所以 5 秒之后第二个 wait 开始执行，又过了 5 秒，两个子进程同时被唤醒。所以剩下的 wait 也被唤醒。

所以在本程序的过程中，当子进程终止时候，会向系统发送一个信号，然后调用我们提前写好的处理函数，在处理函数中使用 waitpid 来处理僵尸进程，获取子进程返回值。

### 10.4 基于多任务的并发服务器

#### 10.4.1 基于进程的并发服务器模型

之前的回声服务器每次只能同事向 1 个客户端提供服务。因此，需要扩展回声服务器，使其可以同时向多个客户端提供服务。下图是基于多进程的回声服务器的模型。

![](https://i.loli.net/2019/01/21/5c453664cde26.png)

从图中可以看出，每当有客户端请求时（连接请求），回声服务器都创建子进程以提供服务。如果请求的客户端有 5 个，则将创建 5 个子进程来提供服务，为了完成这些任务，需要经过如下过程：

- 第一阶段：回声服务器端（父进程）通过调用 accept 函数受理连接请求
- 第二阶段：此时获取的套接字文件描述符创建并传递给子进程
- 第三阶段：进程利用传递来的文件描述符提供服务

#### 10.4.2 实现并发服务器

下面是基于多进程实现的并发的回声服务器的服务端，可以结合第四章的 [echo_client.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch04/echo_client.c) 回声客户端来运行。

- [echo_mpserv.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch10/echo_mpserv.c)

编译运行：

```shell
gcc echo_mpserv.c -o eserver
./eserver
```

结果：





## License

本仓库遵循 CC BY-NC-SA 4.0（署名 - 非商业性使用） 协议，转载请注明出处。

[![CC BY-NC-SA 4.0](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)](http://creativecommons.org/licenses/by-nc-sa/4.0/)

