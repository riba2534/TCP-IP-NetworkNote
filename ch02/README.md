## 第二章 套接字类型与协议设置

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到，直接点链接可能进不去。

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
| PF_INET   | IPv4 互联网协议族    |
| PF_INET6  | IPv6 互联网协议族    |
| PF_LOCAL  | 本地通信 Unix 协议族 |
| PF_PACKET | 底层套接字的协议族   |
| PF_IPX    | IPX Novell 协议族    |

本书着重讲 PF_INET 对应的 IPv4 互联网协议族。其他协议并不常用，或并未普及。**另外，套接字中采用的最终的协议信息是通过 socket 函数的第三个参数传递的。在指定的协议族(第一个参数)和套接字类型(第二个参数)范围内,即可确定第三个参数(具体协议)。**

#### 2.1.3 套接字类型（Type）

套接字类型指的是套接字的数据传输方式，是通过 socket 函数的第二个参数进行传递，只有这样才能决定创建的套接字的数据传输方式。**已经通过第一个参数传递了协议族信息，为什么还要决定数据传输方式？问题就在于，决定了协议族并不能同时决定数据传输方式。换言之， socket 函数的第一个参数 PF_INET 协议族中也存在多种数据传输方式。**

#### 2.1.4 套接字类型1：面向连接的套接字（SOCK_STREAM）

如果 socket 函数的第二个参数传递`SOCK_STREAM`，将创建面向连接的套接字。

传输方式特征整理如下：

- 传输过程中数据不会消失
- 按序传输数据
- 传输的数据不存在数据边界（Boundary）

这种情形适用于之前说过的 write 和 read 函数

> 传输数据的计算机通过调用3次 write 函数传递了 100 字节的数据，但是接收数据的计算机仅仅通过调用 1 次 read 函数调用就接收了全部 100 个字节。

收发数据的套接字内部有缓冲（buffer），简言之就是字节数组。只要不超过数组容量，那么数据填满缓冲后通过 1 次 read 函数的调用就可以读取全部，也有可能调用多次来完成读取。

**套接字缓冲已满是否意味着数据丢失？**

> 答：缓冲并不总是满的。如果读取速度比数据传入过来的速度慢，则缓冲可能被填满，但是这时也不会丢失数据，因为传输套接字此时会停止数据传输，所以面向连接的套接字不会发生数据丢失。

套接字联机必须一一对应。面向连接的套接字可总结为：

**可靠的、按序传递的、基于字节的面向连接的数据传输方式的套接字。**

#### 2.1.5 面向消息的套接字（SOCK_DGRAM）

如果 socket 函数的第二个参数传递`SOCK_DGRAM`，则将创建面向消息的套接字。面向消息的套接字可以比喻成高速移动的摩托车队。特点如下：

- 强调快速传输而非传输有序
- 传输的数据可能丢失也可能损毁
- 传输的数据有边界
- 限制每次传输数据的大小

面向消息的套接字比面向连接的套接字传输速度更快，但数据可能丢失。特点可总结为：

**不可靠的、不按序传递的、以数据的高速传输为目的的套接字。**

#### 2.1.6 协议的最终选择

socket 函数的第三个参数决定最终采用的协议。前面已经通过前两个参数传递了协议族信息和套接字数据传输方式，这些信息还不够吗？为什么要传递第三个参数呢？

> 可以应对同一协议族中存在的多个数据传输方式相同的协议，所以数据传输方式相同，但是协议不同，需要用第三个参数指定具体的协议信息。

本书用的是 IPv4 的协议族，和面向连接的数据传输，满足这两个条件的协议只有 IPPROTO_TCP ，因此可以如下调用 socket 函数创建套接字，这种套接字称为 TCP 套接字。

```c
int tcp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
```

SOCK_DGRAM 指的是面向消息的数据传输方式，满足上述条件的协议只有 IPPROTO_UDP 。这种套接字称为 UDP 套接字：

```c
int udp_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
```

#### 2.1.7 面向连接的套接字：TCP 套接字示例

需要对第一章的代码做出修改，修改好的代码如下：

- [tcp_client.c](tcp_client.c)
- [tcp_server.c](tcp_server.c)

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

在 Windows 平台下使用套接字需要进行一些初始化工作，这与 Linux 平台有所不同。

#### 2.2.1 Winsock 初始化

Windows 下的套接字称为 Winsock，使用前必须调用 `WSAStartup` 函数进行初始化：

```c
#include <winsock2.h>
int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
/*
成功时返回0，失败时返回非0错误代码
wVersionRequested: 程序员请求的Winsock版本，高字节指副版本，低字节指主版本
lpWSAData: 指向WSADATA结构的指针，用于接收Windows Sockets实现的详细信息
*/
```

使用 Winsock 完成后，应调用 `WSACleanup` 函数释放资源：

```c
int WSACleanup(void);
/*
成功时返回0，失败时返回SOCKET_ERROR
*/
```

#### 2.2.2 Windows 与 Linux 套接字编程的主要区别

| 特性 | Linux | Windows |
| ---- | ----- | ------- |
| 头文件 | `sys/socket.h` 等 | `winsock2.h`, `ws2tcpip.h` |
| 初始化 | 无需初始化 | 必须调用 `WSAStartup` |
| 套接字类型 | 文件描述符（int） | `SOCKET` 类型（实际是 `UINT_PTR`） |
| 错误检查 | 返回-1表示失败 | 返回 `INVALID_SOCKET` 表示失败 |
| 关闭套接字 | `close(fd)` | `closesocket(socket)` |
| 获取错误码 | 访问全局变量 `errno` | 调用 `WSAGetLastError()` |
| I/O 函数 | `read`, `write` 可用于套接字 | `recv`, `send` 必须用于套接字 |

#### 2.2.3 Windows 平台代码示例

Windows 平台下的 TCP 客户端和服务端代码结构类似，主要区别在于初始化和清理过程。以下是 Windows 版本的基本结构：

```c
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")  // 链接 Winsock 库

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    SOCKADDR_IN servAddr, clntAddr;
    int szClntAddr;

    // 1. 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup() error");
        return 1;
    }

    // 2. 创建套接字
    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == INVALID_SOCKET) {
        printf("socket() error");
        WSACleanup();
        return 1;
    }

    // ... 其余代码与 Linux 版本类似 ...

    // 9. 关闭套接字
    closesocket(hServSock);

    // 10. 清理 Winsock
    WSACleanup();

    return 0;
}
```

编译命令（使用 Visual Studio 的命令行工具）：

```cmd
cl tcp_client_win.c /link ws2_32.lib
cl tcp_server_win.c /link ws2_32.lib
```

或者使用 MinGW：

```cmd
gcc tcp_client_win.c -o hclient -lws2_32
gcc tcp_server_win.c -o hserver -lws2_32
```

### 2.3 习题

1. 什么是协议？在收发数据中定义协议有何意义？

   > 答：协议是对话中使用的通信规则，简言之，协议就是为了完成数据交换而定好的约定。在收发数据中定义协议，能够让计算机之间进行对话，以此来实现信息交换和资源共享。

2. 面向连接的套接字 TCP 套接字传输特性有 3 点，请分别说明。

   > 答：①传输过程中数据不会消失②按序传输数据③传输的数据不存在数据边界（Boundary）

3. 下面哪些是面向消息的套接字的特性？

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

   > 答：面向连接的 TCP 套接字不存在数据边界。接收数据时需要注意：
   > 1. **数据可能分多次到达**：发送方调用一次 write 发送 100 字节，接收方可能需要多次 read 才能读完，或者一次 read 就能读完多次 write 的数据。
   > 2. **需要定义应用层协议**：由于没有边界，必须在应用层定义数据边界（如固定长度、分隔符、长度前缀等方式），否则无法正确解析数据。
   > 3. **缓冲区管理**：虽然 TCP 内部有流量控制机制保证不丢数据，但应用层仍应及时读取数据，避免接收缓冲区占用过多内存。
