## 第 13 章 多种 I/O 函数

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

### 13.1 send & recv 函数

#### 13.1.1 Linux 中的 send & recv

首先看 sned 函数定义：

```c
#include <sys/socket.h>
ssize_t send(int sockfd, const void *buf, size_t nbytes, int flags);
/*
成功时返回发送的字节数，失败时返回 -1
sockfd: 表示与数据传输对象的连接的套接字和文件描述符
buf: 保存待传输数据的缓冲地址值
nbytes: 待传输字节数
flags: 传输数据时指定的可选项信息
*/
```

下面是 recv 函数的定义：

```c
#include <sys/socket.h>
ssize_t recv(int sockfd, void *buf, size_t nbytes, int flags);
/*
成功时返回接收的字节数（收到 EOF 返回 0），失败时返回 -1
sockfd: 表示数据接受对象的连接的套接字文件描述符
buf: 保存接受数据的缓冲地址值
nbytes: 可接收的最大字节数
flags: 接收数据时指定的可选项参数
*/
```

send 和 recv 函数的最后一个参数是收发数据的可选项，该选项可以用位或（bit OR）运算符（| 运算符）同时传递多个信息。

send & recv 函数的可选项意义：

| 可选项（Option） | 含义                                                         | send | recv |
| ---------------- | ------------------------------------------------------------ | ---- | ---- |
| MSG_OOB          | 用于传输带外数据（Out-of-band data）                         | O    | O    |
| MSG_PEEK         | 验证输入缓冲中是否存在接受的数据                             | X    | O    |
| MSG_DONTROUTE    | 数据传输过程中不参照本地路由（Routing）表，在本地（Local）网络中寻找目的地 | O    | X    |
| MSG_DONTWAIT     | 调用 I/O 函数时不阻塞，用于使用非阻塞（Non-blocking）I/O     | O    | O    |
| MSG_WAITALL      | 防止函数返回，直到接收到全部请求的字节数                     | X    | O    |

#### 13.1.2 MSG_OOB：发送紧急消息

MSG_OOB 可选项用于创建特殊发送方法和通道以发送紧急消息。下面为 MSG_OOB 的示例代码：

- [oob_recv.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch13/oob_recv.c)
- [oob_send.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch13/oob_send.c)

编译运行：

```shell
gcc oob_send.c -o send
gcc oob_recv.c -o recv
```

运行结果：

![](https://i.loli.net/2019/01/26/5c4bda167ae08.png)

![](https://i.loli.net/2019/01/26/5c4bdb4d99823.png)

从运行结果可以看出，send 是客户端，recv 是服务端，客户端给服务端发送消息，服务端接收完消息之后显示出来。可以从图中看出，每次运行的效果，并不是一样的。

代码中关于:

```c
fcntl(recv_sock, F_SETOWN, getpid());
```

的意思是：

> 文件描述符 recv_sock 指向的套接字引发的 SIGURG 信号处理进程变为 getpid 函数返回值用作 ID 进程.

上述描述中的「处理 SIGURG 信号」指的是「调用 SIGURG 信号处理函数」。但是之前讲过，多个进程可以拥有 1 个套接字的文件描述符。例如，通过调用 fork 函数创建子进程并同时复制文件描述符。此时如果发生 SIGURG 信号，应该调用哪个进程的信号处理函数呢？可以肯定的是，不会调用所有进程的信号处理函数。因此，处理 SIGURG 信号时必须指定处理信号所用的进程，而 getpid 返回的是调用此函数的进程 ID 。上述调用语句指当前为处理 SIGURG 信号的主体。

输出结果，可能出乎意料：

> 通过 MSG_OOB 可选项传递数据时只返回 1 个字节，而且也不快

的确，通过 MSG_OOB 并不会加快传输速度，而通过信号处理函数 urg_handler 也只能读取一个字节。剩余数据只能通过未设置 MSG_OOB 可选项的普通输入函数读取。因为 TCP 不存在真正意义上的「外带数据」。实际上，MSG_OOB 中的 OOB 指的是 Out-of-band ，而「外带数据」的含义是：

> 通过完全不同的通信路径传输的数据

即真正意义上的 Out-of-band 需要通过单独的通信路径高速传输数据，但是 TCP 不另外提供，只利用 TCP 的紧急模式（Urgent mode）进行传输。

#### 13.1.3 紧急模式工作原理

MSG_OOB 的真正意义在于督促数据接收对象尽快处理数据。这是紧急模式的全部内容，而 TCP 「保持传输顺序」的传输特性依然成立。TCP 的紧急消息无法保证及时到达，但是可以要求急救。下面是 MSG_OOB 可选项状态下的数据传输过程，如图：

![](https://i.loli.net/2019/01/26/5c4be222845cc.png)

上面是:

```c
send(sock, "890", strlen("890"), MSG_OOB);
```

图上是调用这个函数的缓冲状态。如果缓冲最左端的位置视作偏移量 0 。字符 0 保存于偏移量 2 的位置。另外，字符 0 右侧偏移量为 3 的位置存有紧急指针（Urgent Pointer）。紧急指针指向紧急消息的下一个位置（偏移量加一），同时向对方主机传递以下信息：

> 紧急指针指向的偏移量为 3 之前的部分就是紧急消息。

也就是说，实际上只用了一个字节表示紧急消息。这一点可以通过图中用于传输数据的 TCP 数据包（段）的结构看得更清楚，如图：

![](https://i.loli.net/2019/01/26/5c4beeae46b4e.png)

TCP 数据包实际包含更多信息。TCP 头部包含如下两种信息：

- URG=1：载有紧急消息的数据包
- URG指针：紧急指针位于偏移量为 3 的位置。

指定 MSG_OOB 选项的数据包本身就是紧急数据包，并通过紧急指针表示紧急消息所在的位置。

紧急消息的意义在于督促消息处理，而非紧急传输形式受限的信息。

#### 13.1.4 检查输入缓冲

同时设置 MSG_PEEK 选项和 MSG_DONTWAIT 选项，以验证输入缓冲是否存在接收的数据。设置 MSG_PEEK 选项并调用 recv 函数时，即使读取了输入缓冲的数据也不会删除。因此，该选项通常与 MSG_DONTWAIT 合作，用于以非阻塞方式验证待读数据存在与否。下面的示例是二者的含义：

- [peek_recv.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch13/peek_recv.c)
- [peek_send.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch13/peek_send.c)

编译运行：

```
gcc peek_recv.c -o recv
gcc peek_send.c -o send
./recv 9190
./send 127.0.0.1 9190
```

结果：

![](https://i.loli.net/2019/01/26/5c4c0d1dc83af.png)

可以通过结果验证，仅发送了一次的数据被读取了 2 次，因为第一次调用 recv 函数时设置了 MSG_PEEK 可选项。

### 13.2 readv & writev 函数

#### 13.2.1 使用 readv & writev 函数

readv & writev 函数的功能可概括如下：

> 对数据进行整合传输及发送的函数

也就是说，通过 writev 函数可以将分散保存在多个缓冲中的数据一并发送，通过 readv 函数可以由多个缓冲分别接收。因此，适用这 2 个函数可以减少 I/O 函数的调用次数。下面先介绍 writev 函数。

```c
#include <sys/uio.h>
ssize_t writev(int filedes, const struct iovec *iov, int iovcnt);
/*
成功时返回发送的字节数，失败时返回 -1
filedes: 表示数据传输对象的套接字文件描述符。但该函数并不仅限于套接字，因此，可以像 read 一样向向其传递文件或标准输出描述符.
iov: iovec 结构体数组的地址值，结构体 iovec 中包含待发送数据的位置和大小信息
iovcnt: 向第二个参数传递数组长度
*/
```

上述第二个参数中出现的数组 iovec 结构体的声明如下：

```c
struct iovec
{
    void *iov_base; //缓冲地址
    size_t iov_len; //缓冲大小
};
```

下图是该函数的使用方法：

![](https://i.loli.net/2019/01/26/5c4c61b07d207.png)

writev 的第一个参数，是文件描述符，因此向控制台输出数据，ptr 是存有待发送数据信息的 iovec 数组指针。第三个参数为 2，因此，从 ptr 指向的地址开始，共浏览 2 个 iovec 结构体变量，发送这些指针指向的缓冲数据。

下面是 writev 函数的使用方法：

- [writev.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch13/writev.c)

```c
#include <stdio.h>
#include <sys/uio.h>
int main(int argc, char *argv[])
{
    struct iovec vec[2];
    char buf1[] = "ABCDEFG";
    char buf2[] = "1234567";
    int str_len;

    vec[0].iov_base = buf1;
    vec[0].iov_len = 3;
    vec[1].iov_base = buf2;
    vec[1].iov_len = 4;

    str_len = writev(1, vec, 2);
    puts("");
    printf("Write bytes: %d \n", str_len);
    return 0;
}
```

编译运行：

```shell
gcc writev.c -o writev
./writevi
```

结果：

```
ABC1234
Write bytes: 7
```

下面介绍 readv 函数，功能和 writev 函数正好相反.函数为：

```c
#include <sys/uio.h>
ssize_t readv(int filedes, const struct iovc *iov, int iovcnt);
/*
成功时返回接收的字节数，失败时返回 -1
filedes: 表示数据传输对象的套接字文件描述符。但该函数并不仅限于套接字，因此，可以像 write 一样向向其传递文件或标准输出描述符.
iov: iovec 结构体数组的地址值，结构体 iovec 中包含待数据保存的位置和大小信息
iovcnt: 第二个参数中数组的长度
*/
```

下面是示例代码：

- [readv.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch13/readv.c)

```c
#include <stdio.h>
#include <sys/uio.h>
#define BUF_SIZE 100

int main(int argc, char *argv[])
{
    struct iovec vec[2];
    char buf1[BUF_SIZE] = {
        0,
    };
    char buf2[BUF_SIZE] = {
        0,
    };
    int str_len;

    vec[0].iov_base = buf1;
    vec[0].iov_len = 5;
    vec[1].iov_base = buf2;
    vec[1].iov_len = BUF_SIZE;

    str_len = readv(0, vec, 2);
    printf("Read bytes: %d \n", str_len);
    printf("First message: %s \n", buf1);
    printf("Second message: %s \n", buf2);
    return 0;
}

```

编译运行：

```shell
gcc readv.c -o rv
./rv
```

运行结果：

![](https://i.loli.net/2019/01/26/5c4c718555398.png)

从图上可以看出，首先截取了长度为 5 的数据输出，然后再输出剩下的。

#### 13.2.2 合理使用 readv & writev 函数

实际上，能使用该函数的所有情况都适用。例如，需要传输的数据分别位于不同缓冲（数组）时，需要多次调用 write 函数。此时可通过 1 次 writev 函数调用替代操作，当然会提高效率。同样，需要将输入缓冲中的数据读入不同位置时，可以不必多次调用 read 函数，而是利用 1 次 readv 函数就能大大提高效率。

其意义在于减少数据包个数。假设为了提高效率在服务器端明确禁用了 Nagle 算法。其实 writev 函数在不采用 Nagle 算法时更有价值，如图：

![](https://i.loli.net/2019/01/26/5c4c731323e19.png)

### 13.3 基于 Windows 的实现

暂略

### 13.4 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。
>

1. **下列关于 MSG_OOB 可选项的说法错误的是**？

   答：以下加粗的字体代表说法正确。

   1. MSG_OOB 指传输 Out-of-band 数据，是通过其他路径高速传输数据
   2. MSG_OOB 指通过其他路径高速传输数据，因此 TCP 中设置该选项的数据先到达对方主机
   3. **设置 MSG_OOB 是数据先到达对方主机后，以普通数据的形式和顺序读取。也就是说，只是提高了传输速度，接收方无法识别这一点**。
   4. **MSG_OOB 无法脱离 TCP 的默认数据传输方式，即使脱离了 MSG_OOB ，也会保持原有的传输顺序。该选项只用于要求接收方紧急处理**。

2. **利用 readv & writev 函数收发数据有何优点？分别从函数调用次数和 I/O 缓冲的角度给出说明**。

   答：需要传输的数据分别位于不同缓冲（数组）时，需要多次调用 write 函数。此时可通过 1 次 writev 函数调用替代操作，当然会提高效率。同样，需要将输入缓冲中的数据读入不同位置时，可以不必多次调用 read 函数，而是利用 1 次 readv 函数就能大大提高效率。

3. **通过 recv 函数验证输入缓冲中是否存在数据时（确认后立即返回时），如何设置 recv 函数最后一个参数中的可选项？分别说明各可选项的含义**。

   答：使用 MSG_PEEK 来验证输入缓冲中是否存在待接收的数据。各个可选项的意义参见上面对应章节的表格。
