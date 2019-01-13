# 《TCP/IP网络编程》学习笔记

:flags:此仓库是我的《TCP/IP网络编程》学习笔记及具体代码实现，代码部分请参考本仓库对应章节文件夹下的代码。

我的环境是：Ubuntu18.04 LTS

编译器版本：`g++ (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0` 和 `gcc (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0`

所以本笔记中只学习有关于 Linux 的部分。

## 第一章：理解网络编程和套接字

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

## 第二章 套接字类型与协议设置

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

## 第三章 地址族与数据序列

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

还有一个函数，与 inet_aton() 正好相反。

## License

本仓库遵循 CC BY-NC-SA 4.0（署名 - 非商业性使用） 协议，转载请注明出处。

[![CC BY-NC-SA 4.0](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)](http://creativecommons.org/licenses/by-nc-sa/4.0/)

