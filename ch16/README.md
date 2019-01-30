## 第 16 章 关于 I/O 流分离的其他内容

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

### 16.1 分离 I/O 流

「分离 I/O 流」是一种常用表达。有 I/O 工具可区分二者，无论采用哪种方法，都可以认为是分离了 I/O 流。

#### 16.1.1 2次  I/O 流分离

之前有两种分离方法：

- 第一种是第 10 章的「TCP I/O 过程」分离。通过调用 fork 函数复制出一个文件描述符，以区分输入和输出中使用的文件描述符。虽然文件描述符本身不会根据输入和输出进行区分，但我们分开了 2 个文件描述符的用途，因此，这也属于「流」的分离。
- 第二种分离是在第 15 章。通过 2 次 fdopen 函数的调用，创建读模式 FILE 指针（FILE 结构体指针）和写模式 FILE 指针。换言之，我们分离了输入工具和输出工具，因此也可视为「流」的分离。下面是分离的理由。

#### 16.1.2 分离「流」的好处

首先是第 10 章「流」的分离目的：

- 通过分开输入过程（代码）和输出过程降低实现难度
- 与输入无关的输出操作可以提高速度

下面是第 15 章「流」分离的目的：

- 为了将 FILE 指针按读模式和写模式加以区分
- 可以通过区分读写模式降低实现难度
- 通过区分 I/O 缓冲提高缓冲性能

#### 16.1.3 「流」分离带来的 EOF 问题

第 7 章介绍过 EOF 的传递方法和半关闭的必要性。有一个语句：

```c
shutdown(sock,SHUT_WR);
```

当时说过调用 shutdown 函数的基于半关闭的 EOF 传递方法。第十章的 [echo_mpclient.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch10/echo_mpclient.c) 添加了半关闭的相关代码。但是还没有讲采用 fdopen 函数怎么半关闭。那么是否是通过 fclose 函数关闭流呢？我们先试试

下面是服务端和客户端码：

- [sep_clnt.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch16/sep_clnt.c)
- [sep_serv.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch16/sep_serv.c)

编译运行：

```shell
gcc sep_clnt.c -o clnt
gcc sep_serv.c -o serv
./serv 9190
./clnt 127.0.0.1 9190
```

结果：

![](https://i.loli.net/2019/01/30/5c512086a75d9.png)

从运行结果可以看出，服务端最终没有收到客户端发送的信息。那么这是什么原因呢？

原因是：服务端代码的 `fclose(writefp);` 这一句，完全关闭了套接字而不是半关闭。这才是这一章需要解决的问题。

### 16.2 文件描述符的的复制和半关闭

#### 16.2.1 终止「流」时无法半关闭原因

下面的图描述的是服务端代码中的两个FILE 指针、文件描述符和套接字中的关系。

![](https://i.loli.net/2019/01/30/5c5121da89955.png)

从图中可以看到，两个指针都是基于同一文件描述符创建的。因此，针对于任何一个 FILE 指针调用 fclose 函数都会关闭文件描述符，如图所示：

![](https://i.loli.net/2019/01/30/5c51224051802.png)

从图中看到，销毁套接字时再也无法进行数据交换。那如何进入可以进入但是无法输出的半关闭状态呢？如下图所示：

![](https://i.loli.net/2019/01/30/5c5122a45c5f1.png)

只需要创建 FILE 指针前先复制文件描述符即可。复制后另外创建一个文件描述符，然后利用各自的文件描述符生成读模式的 FILE 指针和写模式的 FILE 指针。这就为半关闭创造好了环境，因为套接字和文件描述符具有如下关系：

> 销毁所有文件描述符候才能销毁套接字

也就是说，针对写模式 FILE 指针调用 fclose 函数时，只能销毁与该 FILE 指针相关的文件描述符，无法销毁套接字，如下图：

![](https://i.loli.net/2019/01/30/5c5123ad7df31.png)

那么调用 fclose 函数候还剩下 1 个文件描述符，因此没有销毁套接字。那此时的状态是否为半关闭状态？不是！只是准备好了进入半关闭状态，而不是已经进入了半关闭状态。仔细观察，还剩下一个文件描述符。而该文件描述符可以同时进行 I/O 。因此，不但没有发送 EOF ，而且仍然可以利用文件描述符进行输出。

#### 16.2.2 复制文件描述符

与调用 fork 函数不同，调用 fork 函数将复制整个进程，此处讨论的是同一进程内完成对完成描述符的复制。如图：

![](https://i.loli.net/2019/01/30/5c512579c45b6.png)

复制完成后，两个文件描述符都可以访问文件，但是编号不同。

#### 16.2.3 dup 和 dup2

下面给出两个函数原型：

```c
#include <unistd.h>
int dup(int fildes);
int dup2(int fildes, int fildes2);
/*
成功时返回复制的文件描述符，失败时返回 -1
fildes : 需要复制的文件描述符
fildes2 : 明确指定的文件描述符的整数值。
*/
```

dup2 函数明确指定复制的文件描述符的整数值。向其传递大于 0 且小于进程能生成的最大文件描述符值时，该值将成为复制出的文件描述符值。下面是代码示例：

- [dup.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch16/dup.c)

```c
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int cfd1, cfd2;
    char str1[] = "Hi~ \n";
    char str2[] = "It's nice day~ \n";

    cfd1 = dup(1);        //复制文件描述符 1
    cfd2 = dup2(cfd1, 7); //再次复制文件描述符,定为数值 7

    printf("fd1=%d , fd2=%d \n", cfd1, cfd2);
    write(cfd1, str1, sizeof(str1));
    write(cfd2, str2, sizeof(str2));

    close(cfd1);
    close(cfd2); //终止复制的文件描述符，但是仍有一个文件描述符
    write(1, str1, sizeof(str1));
    close(1);
    write(1, str2, sizeof(str2)); //无法完成输出
    return 0;
}

```

编译运行：

```
gcc dup.c -o dup
./dup
```

结果：

![](https://i.loli.net/2019/01/30/5c5135574d89a.png)

#### 16.2.4 复制文件描述符后「流」的分离

下面更改 [sep_clnt.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch16/sep_clnt.c) 和 [sep_serv.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch16/sep_serv.c) 可以使得让它正常工作，正常工作是指通过服务器的半关闭状态接收客户端最后发送的字符串。

下面是代码：

- [sep_serv2.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch16/sep_serv2.c)

这个代码可以与 [sep_clnt.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch16/sep_clnt.c) 配合起来食用，编译过程和上面一样，运行结果为：

![](https://i.loli.net/2019/01/30/5c513d54a27e0.png)

### 16.3 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **下列关于 FILE 结构体指针和文件描述符的说法错误的是**？

   答：以下加粗内容代表说法正确。

   1. 与 FILE 结构体指针相同，文件描述符也分输入描述符和输出描述符
   2. 复制文件描述符时将生成相同值的描述符，可以通过这 2 个描述符进行 I/O
   3. **可以利用创建套接字时返回的文件描述符进行 I/O ，也可以不通过文件描述符，直接通过 FILE 结构体指针完成**
   4. **可以从文件描述符生成 FILE 结构体指针，而且可以利用这种 FILE 结构体指针进行套接字 I/O**
   5. 若文件描述符为读模式，则基于该描述符生成的 FILE 结构体指针同样是读模式；若文件描述符为写模式，则基于该描述符生成的 FILE 结构体指针同样是写模式

2. **EOF 的发送相关描述中错误的是**？

   答：以下加粗内容代表说法正确。

   1. 终止文件描述符时发送 EOF
   2. **即使未完成终止文件描述符，关闭输出流时也会发送 EOF**
   3. 如果复制文件描述符，则包括复制的文件描述符在内，所有文件描述符都终止时才会发送 EOF
   4. **即使复制文件描述符，也可以通过调用 shutdown 函数进入半关闭状态并发送 EOF**
