## 第 14 章 多播与广播

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

### 14.1 多播

多播（Multicast）方式的数据传输是基于 UDP 完成的。因此 ，与 UDP 服务器端/客户端的实现方式非常接近。区别在于，UDP 数据传输以单一目标进行，而多播数据同时传递到加入（注册）特定组的大量主机。换言之，采用多播方式时，可以同时向多个主机传递数据。

#### 14.1.1 多播的数据传输方式以及流量方面的优点

多播的数据传输特点可整理如下：

- 多播服务器端针对特定多播组，只发送 1 次数据。
- 即使只发送 1 次数据，但该组内的所有客户端都会接收数据
- 多播组数可以在 IP 地址范围内任意增加

多播组是 D 类IP地址（224.0.0.0~239.255.255.255），「加入多播组」可以理解为通过程序完成如下声明：

> 在 D 类IP地址中，我希望接收发往目标 239.234.218.234 的多播数据

多播是基于 UDP 完成的，也就是说，多播数据包的格式与 UDP 数据包相同。只是与一般的 UDP 数据包不同。向网络传递 1 个多播数据包时，路由器将复制该数据包并传递到多个主机。像这样，多播需要借助路由器完成。如图所示：

![](https://i.loli.net/2019/01/27/5c4d310daa6be.png)

若通过 TCP 或 UDP 向 1000 个主机发送文件，则共需要传递 1000 次。但是此时如果用多播网络传输文件，则只需要发送一次。这时由 1000 台主机构成的网络中的路由器负责复制文件并传递到主机。就因为这种特性，多播主要用于「多媒体数据实时传输」。

另外，理论上可以完成多播通信，但是不少路由器并不支持多播，或即便支持也因网络拥堵问题故意阻断多播。因此，为了在不支持多播的路由器中完成多播通信，也会使用隧道（Tunneling）技术。

#### 14.1.2 路由（Routing）和 TTL（Time to Live,生存时间），以及加入组的办法

为了传递多播数据包，必须设置 TTL 。TTL 是 Time to Live的简写，是决定「数据包传递距离」的主要因素。TTL 用整数表示，并且每经过一个路由器就减一。TTL 变为 0 时，该数据包就无法再被传递，只能销毁。因此，TTL 的值设置过大将影响网络流量。当然，设置过小，也无法传递到目标。

![](https://i.loli.net/2019/01/27/5c4d3960001eb.png)

接下来是 TTL 的设置方法。TTL 是可以通过第九章的套接字可选项完成的。与设置 TTL 相关的协议层为 IPPROTO_IP ，选项名为 IP_MULTICAST_TTL。因此，可以用如下代码把 TTL 设置为 64

```c
int send_sock;
int time_live = 64;
...
send_sock=socket(PF_INET,SOCK_DGRAM,0);
setsockopt(send_sock,IPPROTO_IP,IP_MULTICAST_TTL,(void*)&time_live,sizeof(time_live);
...
```

加入多播组也通过设置套接字可选项来完成。加入多播组相关的协议层为 IPPROTO_IP，选项名为 IP_ADD_MEMBERSHIP 。可通过如下代码加入多播组：

```c
int recv_sock;
struct ip_mreq join_adr;
...
recv_sock=socket(PF_INET,SOCK_DGRAM,0);
...
join_adr.imr_multiaddr.s_addr="多播组地址信息";
join_adr.imr_interface.s_addr="加入多播组的主机地址信息";
setsockopt(recv_sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(void*)&join_adr,sizeof(join_adr);
...
```

下面是 ip_mreq 结构体的定义：

```c
struct ip_mreq
{
    struct in_addr imr_multiaddr; //写入加入组的IP地址
    struct in_addr imr_interface; //加入该组的套接字所属主机的IP地址
};
```

#### 14.1.3 实现多播 Sender 和 Receiver

多播中用「发送者」（以下称为 Sender） 和「接收者」（以下称为 Receiver）替代服务器端和客户端。顾名思义，此处的 Sender 是多播数据的发送主体，Receiver 是需要多播组加入过程的数据接收主体。下面是示例，示例的运行场景如下：

- Sender : 向 AAA 组广播（Broadcasting）文件中保存的新闻信息
- Receiver : 接收传递到 AAA 组的新闻信息。

下面是两个代码：

- [news_sender.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch14/news_sender.c)
- [news_receiver.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch14/news_receiver.c)

编译运行：

```
gcc news_sender.c -o sender
gcc news_receiver.c -o receiver
./sender 224.1.1.2 9190
./receiver 224.1.1.2 9190
```

结果：

![](https://i.loli.net/2019/01/28/5c4e85a9aabcc.png)

通过结果可以看出，使用 sender 多播信息，通过 receiver 接收广播，如果延迟运行 receiver 将无法接受之前发送的信息。

### 14.2 广播

广播（Broadcast）在「一次性向多个主机发送数据」这一点上与多播类似，但传输数据的范围有区别。多播即使在跨越不同网络的情况下，只要加入多播组就能接受数据。相反，广播只能向同一网络中的主机传输数据。

#### 14.2.1 广播的理解和实现方法

广播是向同一网络中的所有主机传输数据的方法。与多播相同，广播也是通过 UDP 来完成的。根据传输数据时使用的IP地址形式，广播分为以下两种：

- 直接广播（Directed Broadcast）
- 本地广播（Local Broadcast）

二者在实现上的差别主要在于IP地址。直接广播的IP地址中除了网络地址外，其余主机地址全部设置成 1。例如，希望向网络地址 192.12.34 中的所有主机传输数据时，可以向 192.12.34.255 传输。换言之，可以采取直接广播的方式向特定区域内所有主机传输数据。

反之，本地广播中使用的IP地址限定为 255.255.255.255 。例如，192.32.24 网络中的主机向 255.255.255.255 传输数据时，数据将传输到 192.32.24 网络中所有主机。

**数据通信中使用的IP地址是与 UDP 示例的唯一区别。默认生成的套接字会阻止广播，因此，只需通过如下代码更改默认设置。**

```c
int send_sock;
int bcast;
...
send_sock=socket(PF_INET,SOCK_DGRAM,0);
...
setsockopt(send_sock,SOL_SOCKET,SO_BROADCAST,(void*)&bcast,sizeof(bcast));
...
```

### 14.2.2 实现广播数据的 Sender 和 Receiver

下面是广播数据的 Sender 和 Receiver的代码：

- [news_sender_brd.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch14/news_sender_brd.c)
- [news_receiver_brd.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch14/news_receiver_brd.c)

编译运行：

```
gcc news_receiver_brd.c -o receiver
gcc news_sender_brd.c -o sender
./sender 255.255.255.255 9190
./receiver 9190
```

结果：

![](https://i.loli.net/2019/01/28/5c4e9113368dd.png)

### 14.3 基于 Windows 的实现

暂略

### 14.4 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **TTL 的含义是什么？请从路由器的角度说明较大的 TTL 值与较小的 TTL 值之间的区别及问题。**

   答：TTL 是决定「数据包传递距离」的主要因素。TTL 每经过一个路由器就减一。TTL 变为 0 时，数据包就无法再被传递，只能销毁。因此，TTL设置过大会影响网络流量。当然，设置过小无法传递到目标。

2. **多播与广播的异同点是什么？请从数据通信的角度进行说明**。

   答：在「一次性向多个主机发送数据」这一点上与多播类似，但传输的数据范围有区别。多播即使在跨越不同网络的情况下，只要加入多播组就能接受数据。相反，广播只能向同一网络中的主机传输数据。

3. **下面关于多播的说法描述错误的是**？

   答：以下内容加粗的为描述正确

   1. 多播是用来加入多播组的所有主机传输数据的协议
   2. 主机连接到同一网络才能加入到多播组，也就是说，多播组无法跨越多个网络
   3. **能够加入多播组的主机数并无限制，但只能有 1个主机（Sender）向该组发送数据**
   4. **多播时使用的套接字是 UDP 套接字，因为多播是基于 UDP 进行数据通信的。**

4. **多播也对网络流量有利，请比较 TCP 交换方式解释其原因**

   答：TCP 是必须建立一对一的连接，如果要向1000个主机发送文件，就得传递1000次。但是此时用多播方式传输数据，就只需要发送一次。

5. **多播方式的数据通信需要 MBone 虚拟网络。换言之，MBone 是用于多播的网络，但它是虚拟网络。请解释此处的「虚拟网络」**

   答：可以理解为「通过网络中的特殊协议工作的软件概念上的网络」。也就是说， MBone 并非可以触及的物理网络。他是以物理网络为基础，通过软件方法实现的多播通信必备虚拟网络。
