## 第 11 章 进程间通信

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

进程间通信，意味着两个不同的进程之间可以交换数据。

### 11.1 进程间通信的基本概念

#### 11.1.1 通过管道实现进程间通信

下图是基于管道（PIPE）的进程间通信的模型：

![](images/kFlk0s.png)

可以看出，为了完成进程间通信，需要创建进程。管道并非属于进程的资源，而是和套接字一样，属于操作系统（也就不是 fork 函数的复制对象）。所以，两个进程通过操作系统提供的内存空间进行通信。下面是创建管道的函数。

```c
#include <unistd.h>
int pipe(int filedes[2]);
/*
成功时返回 0 ，失败时返回 -1
filedes[0]: 通过管道接收数据时使用的文件描述符，即管道出口
filedes[1]: 通过管道传输数据时使用的文件描述符，即管道入口
*/
```

父进程调用函数时将创建管道，同时获取对应于出入口的文件描述符，此时父进程可以读写同一管道。但父进程的目的是与子进程进行数据交换，因此需要通过 fork 让子进程继承这两个文件描述符，父子进程各自使用其中一端进行通信（实际编程中还应关闭各自不需要的一端）。下面的例子是关于该函数的使用方法：

- [pipe1.c](pipe1.c)

```c
#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int fds[2];
    char str[] = "Who are you?";
    char buf[BUF_SIZE];
    pid_t pid;
    // 调用  pipe 函数创建管道，fds 数组中保存用于 I/O 的文件描述符
    pipe(fds);
    pid = fork(); //子进程将同时拥有创建管道获取的2个文件描述符，复制的并非管道，而是文件描述符
    if (pid == 0)
    {
        write(fds[1], str, sizeof(str));
    }
    else
    {
        read(fds[0], buf, BUF_SIZE);
        puts(buf);
    }
    return 0;
}
```

编译运行：

```shell
gcc pipe1.c -o pipe1
./pipe1
```

结果：

```
Who are you?
```

可以从程序中看出，首先创建了一个管道，子进程通过 fds[1] 把数据写入管道，父进程从 fds[0] 再把数据读出来。可以从下图看出：

![](images/kF8A7d.png)

#### 11.1.2 通过管道进行进程间双向通信

下图可以看出双向通信模型：

![](images/kF84De.png)

下面是双向通信的示例：

- [pipe2.c](pipe2.c)

```c
#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int fds[2];
    char str1[] = "Who are you?";
    char str2[] = "Thank you for your message";
    char buf[BUF_SIZE];
    pid_t pid;

    pipe(fds);
    pid = fork();
    if (pid == 0)
    {
        write(fds[1], str1, sizeof(str1));
        sleep(2);
        read(fds[0], buf, BUF_SIZE);
        printf("Child proc output: %s \n", buf);
    }
    else
    {
        read(fds[0], buf, BUF_SIZE);
        printf("Parent proc output: %s \n", buf);
        write(fds[1], str2, sizeof(str2));
        sleep(3);
    }
    return 0;
}
```

编译运行：

```shell
gcc pipe2.c -o pipe2
./pipe2
```

结果：

```
Parent proc output: Who are you?
Child proc output: Thank you for your message
```

运行结果是正确的，但是如果注释掉代码中子进程里的 `sleep(2);`（第18行），就会出现问题，导致一直等待下去。因为数据进入管道后变成了无主数据。也就是通过 read 函数先读取数据的进程将得到数据，即使该进程将数据传到了管道。此时子进程可能在父进程读取前就把自己写入的数据读回，导致父进程的 read 调用无限期阻塞，等待永远不会到来的数据。

当一个管道不满足需求时，就需要创建两个管道，各自负责不同的数据流动，过程如下图所示：

![](images/kFJW0e.png)

下面采用上述模型改进 `pipe2.c` 。

- [pipe3.c](pipe3.c)

```c
#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int fds1[2], fds2[2];
    char str1[] = "Who are you?";
    char str2[] = "Thank you for your message";
    char buf[BUF_SIZE];
    pid_t pid;

    pipe(fds1), pipe(fds2);
    pid = fork();
    if (pid == 0)
    {
        write(fds1[1], str1, sizeof(str1));
        read(fds2[0], buf, BUF_SIZE);
        printf("Child proc output: %s \n", buf);
    }
    else
    {
        read(fds1[0], buf, BUF_SIZE);
        printf("Parent proc output: %s \n", buf);
        write(fds2[1], str2, sizeof(str2));
    }
    return 0;
}
```

上面通过创建两个管道实现了功能，此时，不需要额外再使用 sleep 函数。运行结果和上面一样。

### 11.2 运用进程间通信

#### 11.2.1 保存消息的回声服务器

下面对第 10 章的 [echo_mpserv.c](../ch10/echo_mpserv.c) 进行改进，添加一个功能：

> 将回声客户端传输的字符串按序保存到文件中

实现该任务将创建一个新进程，从向客户端提供服务的进程读取字符串信息，下面是代码：

- [echo_storeserv.c](echo_storeserv.c)

编译运行：

```shell
gcc echo_storeserv.c -o serv
./serv 9190
```

此服务端配合第 10 章的客户端 [echo_mpclient.c](../ch10/echo_mpclient.c) 使用，运行结果如下图：

![](images/kFUCct.png)

![](images/kFUAHS.png)

从图上可以看出，服务端已经生成了文件，把客户端的消息保存了下来，只保存了 10 条消息（代码中固定循环读取 10 次）。

### 11.3 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **什么是进程间通信？分别从概念和内存的角度进行说明。**

   答：进程间通信（Inter-Process Communication，IPC）是指两个不同的进程间可以交换数据的机制。从概念上讲，它允许独立运行的进程之间传递信息、同步操作。从内存角度来说，由于每个进程都有自己独立的内存地址空间，进程间无法直接访问彼此的内存，因此需要操作系统提供特殊的共享内存区域或通信机制（如管道、共享内存、消息队列等），通过这个内核提供的缓冲区来进行数据交换。

2. **进程间通信需要特殊的 IPC 机制，这是由于操作系统提供的。进程间通信时为何需要操作系统的帮助？**

   答：进程间通信需要操作系统的帮助是因为：每个进程都有独立的内存地址空间，进程之间无法直接访问对方的内存。IPC 机制（如管道）属于操作系统内核管理的资源，而非单个进程的资源。通过 fork 函数复制的是文件描述符，而非管道本身。因此，两个进程必须通过操作系统提供的内核缓冲区来实现数据交换。

3. **「管道」是典型的 IPC 技法。关于管道，请回答以下问题：**

   1. **管道是进程间交换数据的路径。如何创建此路径？由谁创建？**

      答：使用 pipe 函数进行创建，由操作系统创建。父进程调用该函数时将创建管道。

   2. **为了完成进程间通信。2 个进程要同时连接管道。那2 个进程如何连接到同一管道？**

      答：数组中有两个文件描述符，父子进程调用相关函数时，通过 fork 函数，把 1 个文件描述符传递给子进程。

   3. **管道允许 2 个进程间的双向通信。双向通信中需要注意哪些内容？**

      答：向管道传输数据时，先调用 read 函数的进程会把数据取走。换言之，数据进入管道后会变成无主数据，任何连接到该管道的进程都可以读取。因此，在使用单个管道进行双向通信时，一个进程可能会读取到自己写入的数据，导致另一个进程无限等待。为了避免这个问题，通常需要创建两个管道，各自负责不同方向的数据流动。
