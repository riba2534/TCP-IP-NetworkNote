## 第 6 章 基于 UDP 的服务端/客户端

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

TCP 是内容较多的一个协议，而本章中的 UDP 内容较少，但是也很重要。

### 6.1 理解 UDP

#### 6.1.1 UDP 套接字的特点

通过寄信来说明 UDP 的工作原理，这是讲解 UDP 时使用的传统示例，它与 UDP 的特点完全相同。寄信前应先在信封上填好寄信人和收信人的地址，之后贴上邮票放进邮筒即可。当然，信件的特点使我们无法确认信件是否被收到。邮寄过程中也可能发生信件丢失的情况。也就是说，信件是一种不可靠的传输方式，UDP 也是一种不可靠的数据传输方式。

因为 UDP 没有 TCP 那么复杂，所以编程难度比较小，性能也比 TCP 高。在更重视性能的情况下可以选择 UDP 的传输方式。

TCP 与 UDP 的区别很大一部分来源于流控制。也就是说 TCP 的生命在于流控制。

#### 6.1.2 UDP 的工作原理

如图所示：

![](https://i.loli.net/2019/01/17/5c3fd29c70bf2.png)

从图中可以看出，IP 的作用就是让离开主机 B 的 UDP 数据包准确传递到主机 A 。但是把 UDP 数据包最终交给主机 A 的某一 UDP 套接字的过程是由 UDP 完成的。UDP 的最重要的作用就是根据端口号将传到主机的数据包交付给最终的 UDP 套接字。

#### 6.1.3 UDP 的高效使用

UDP 也具有一定的可靠性。对于通过网络实时传递的视频或者音频时情况有所不同。对于多媒体数据而言，丢失一部分数据也没有太大问题，这只是会暂时引起画面抖动，或者出现细微的杂音。但是要提供实时服务，速度就成为了一个很重要的因素。因此流控制就显得有一点多余，这时就要考虑使用 UDP 。TCP 比 UDP 慢的原因主要有以下两点：

- 收发数据前后进行的连接设置及清除过程。
- 收发过程中为保证可靠性而添加的流控制。

如果收发的数据量小但是需要频繁连接时，UDP 比 TCP 更高效。

### 6.2 实现基于 UDP 的服务端/客户端

#### 6.2.1 UDP 中的服务端和客户端没有连接

UDP 中的服务端和客户端不像 TCP 那样在连接状态下交换数据，因此与 TCP 不同，无需经过连接过程。也就是说，不必调用 TCP 连接过程中调用的 listen 和 accept 函数。UDP 中只有创建套接字和数据交换的过程。

#### 6.2.2 UDP 服务器和客户端均只需一个套接字

TCP 中，套接字之间应该是一对一的关系。若要向 10 个客户端提供服务，除了守门的服务器套接字之外，还需要 10 个服务器套接字。但在 UDP 中，不管是服务器端还是客户端都只需要 1 个套接字。只需要一个 UDP 套接字就可以向任意主机传输数据，如图所示：

![](https://i.loli.net/2019/01/17/5c3fd703f3c40.png)

图中展示了 1 个 UDP 套接字与 2 个不同主机交换数据的过程。也就是说，只需 1 个 UDP 套接字就能和多台主机进行通信。

#### 6.2.3 基于 UDP 的数据 I/O 函数

创建好 TCP 套接字以后，传输数据时无需加上地址信息。因为 TCP 套接字将保持与对方套接字的连接。换言之，TCP 套接字知道目标地址信息。但 UDP 套接字不会保持连接状态（UDP 套接字只有简单的邮筒功能），因此每次传输数据时都需要添加目标的地址信息。这相当于寄信前在信件中填写地址。接下来是 UDP 的相关函数：

```c
#include <sys/socket.h>
ssize_t sendto(int sock, void *buff, size_t nbytes, int flags,
               struct sockaddr *to, socklen_t addrlen);
/*
成功时返回发送的字节数，失败时返回 -1
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
成功时返回接收的字节数，失败时返回 -1
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

仔细观察 UDP 客户端可以发现，UDP 客户端缺少了把IP和端口分配给套接字的过程。TCP 客户端调用 connect 函数自动完成此过程，而 UDP 中连能承担相同功能的函数调用语句都没有。究竟在什么时候分配IP和端口号呢？

UDP 程序中，调用 sendto 函数传输数据前应该完成对套接字的地址分配工作，因此调用 bind 函数。当然，bind 函数在 TCP 程序中出现过，但 bind 函数不区分 TCP 和 UDP，也就是说，在 UDP 程序中同样可以调用。另外，如果调用 sendto 函数尚未分配地址信息，则在首次调用 sendto 函数时给相应套接字自动分配 IP 和端口。而且此时分配的地址一直保留到程序结束为止，因此也可以用来和其他 UDP 套接字进行数据交换。当然，IP 用主机IP，端口号用未选用的任意端口号。

综上所述，调用 sendto 函数时自动分配IP和端口号，因此，UDP 客户端中通常无需额外的地址分配过程。所以之前的示例中省略了该过程。这也是普遍的实现方式。

### 6.3 UDP 的数据传输特性和调用 connect 函数

#### 6.3.1 存在数据边界的 UDP 套接字

前面说得 TCP 数据传输中不存在数据边界，这表示「数据传输过程中调用 I/O 函数的次数不具有任何意义」

相反，UDP 是具有数据边界的下一，传输中调用 I/O 函数的次数非常重要。因此，输入函数的调用次数和输出函数的调用次数应该完全一致，这样才能保证接收全部已经发送的数据。例如，调用 3 次输出函数发送的数据必须通过调用 3 次输入函数才能接收完。通过一个例子来进行验证：

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
