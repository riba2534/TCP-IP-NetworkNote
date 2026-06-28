## 第 8 章 域名及网络地址

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

### 8.1 域名系统

DNS 是对IP地址和域名进行相互转换的系统，其核心是 DNS 服务器

#### 8.1.1 什么是域名

域名就是我们常常在地址栏里面输入的地址，将比较难记忆的IP地址变成人类容易理解的信息。

#### 8.1.2 DNS 服务器

相当于一个字典，可以查询出某一个域名对应的IP地址

![](images/5c41854859ae3.png)

如图所示，显示了 DNS 服务器的查询路径。

### 8.2 IP地址和域名之间的转换

#### 8.2.1 程序中有必要使用域名吗？

一句话，需要，因为IP地址可能经常改变，而且也不容易记忆，通过域名可以随时更改解析，达到更换IP的目的。

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

从上述结构体可以看出，不止返回IP信息，同时还带着其他信息一起返回。域名转换成IP时只需要关注 h_addr_list。下面简要说明上述结构体的成员：

- h_name：该变量中存有官方域名（Official domain name）。官方域名代表某一主页，但实际上，一些著名公司的域名并没有用官方域名注册。
- h_aliases：可以通过多个域名访问同一主页。同一IP可以绑定多个域名，因此，除官方域名外还可以指定其他域名。这些信息可以通过 h_aliases 获得。
- h_addrtype：该成员用于获取保存在 h_addr_list 中的地址族信息。若是 IPv4，则此变量中存有 AF_INET；若是 IPv6，则存有 AF_INET6。需要注意，gethostbyname 函数本身只返回 IPv4（AF_INET）地址，若需解析 IPv6 地址应改用 getaddrinfo 函数。
- h_length：保存IP地址长度。若是 IPv4 地址，因为是 4 个字节，则保存 4；IPv6 时，因为是 16 个字节，故保存 16。
- h_addr_list：这个是最重要的成员。通过此变量以整数形式保存域名相对应的IP地址。另外，用户比较多的网站有可能分配多个IP地址给同一个域名，利用多个服务器做负载均衡。此时可以通过此变量获取IP地址信息。

调用 gethostbyname 函数后，返回的结构体变量如图所示：

![](images/5c41898ae45e8.png)

下面的代码通过一个例子来演示 gethostbyname 的应用，并说明 hostent 结构体变量特性。

- [gethostbyname.c](gethostbyname.c)

编译运行：

```shell
gcc gethostbyname.c -o hostname
./hostname www.baidu.com
```

结果：

![](images/5c418faf20495.png)

如图所示，显示出了对百度的域名解析

可以看出，百度的一个域名解析使用了 CNAME 记录，指向了 `shifen.com`。关于百度具体的解析过程，可参考下方资料。

> 这一部分牵扯到了很多关于DNS解析的过程，还有 Linux 下关于域名解析的一些命令，我找了一部分资料，可以点下面的链接查看比较详细的：
>
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

![](images/5c419658a73b8.png)

#### 8.2.3 利用IP地址获取域名

请看下面的函数定义：

```c
#include <netdb.h>
struct hostent *gethostbyaddr(const char *addr, socklen_t len, int family);
/*
成功时返回 hostent 结构体变量地址值，失败时返回 NULL 指针
addr: 含有IP地址信息的 in_addr 结构体指针。为了同时传递 IPv4 地址之外的全部信息，该变量的类型声明为 char 指针。
len: 向第一个参数传递的地址信息的字节数，IPv4 时为 4，IPv6 时为 16。
family: 传递地址族信息，IPv4 是 AF_INET，IPv6 是 AF_INET6。
*/
```

下面的代码演示使用方法：

- [gethostbyaddr.c](gethostbyaddr.c)

编译运行：

```shell
gcc gethostbyaddr.c -o hostaddr
./hostaddr 8.8.8.8
```

结果：

![](images/5c41a019085d4.png)

从图上可以看出，`8.8.8.8`这个IP地址是谷歌的。

### 8.3 基于 Windows 的实现

暂略

### 8.4 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **下列关于 DNS 的说法错误的是？**

   答：说法错误的是选项 2 和 4。

   1. 因为 DNS 的存在，故可以使用域名代替 IP。（正确）
   2. DNS 服务器实际上是路由器，因为路由器根据域名决定数据的路径。（**错误**：DNS 服务器不是路由器，路由器是根据 IP 地址而非域名来决定数据转发路径的）
   3. 所有域名信息并非集中于 1 台 DNS 服务器，但可以获取某一 DNS 服务器中未注册的所有地址。（正确：通过 DNS 的层次化结构，可以逐级查询获取未在本地 DNS 服务器中注册的地址）
   4. DNS 服务器根据操作系统进行区分，Windows 下的 DNS 服务器和 Linux 下的 DNS 服务器是不同的。（**错误**：DNS 协议是标准化的，与操作系统无关）

2. **阅读如下对话，并说明东秀的方案是否可行。（因为对话的字太多，用图代替）**

   ![](images/5c41a22f35390.png)

   答：东秀的方案是可行的。DNS 服务器采用分布式层次结构，具有冗余性和容错性。当一台 DNS 服务器故障时，可以自动切换到其他可用的 DNS 服务器进行查询，不会导致整个域名解析系统瘫痪。此外，DNS 解析结果通常会在本地缓存一段时间，即使 DNS 服务器暂时不可用，已缓存的解析记录仍然可以正常使用。

3. **在浏览器地址输入 www.orentec.co.kr，并整理出主页显示过程。假设浏览器访问默认 DNS 服务器中并没有关于 www.orentec.co.kr 的地址信息。**

   答：完整的域名解析过程如下：

   1. **本地缓存查询**：浏览器首先检查本地缓存和操作系统的 hosts 文件，如果找到对应 IP 地址则直接使用。
   2. **向本地 DNS 服务器发起查询**：如果没有找到缓存，浏览器向系统配置的本地 DNS 服务器发起递归查询请求。
   3. **本地 DNS 服务器迭代查询**：
      - 本地 DNS 服务器首先检查自己的缓存，若没有则向根域名服务器（Root Server，"."）发起查询。
      - 根域名服务器返回负责 .kr 域的顶级域名服务器（TLD Server）地址。
      - 本地 DNS 服务器向 .kr 的顶级域名服务器发起查询。
      - 顶级域名服务器返回负责 orentec.co.kr 的权威域名服务器地址。
      - 本地 DNS 服务器向权威域名服务器发起查询。
      - 权威域名服务器返回 www.orentec.co.kr 的 IP 地址。
   4. **返回结果**：本地 DNS 服务器将解析到的 IP 地址返回给浏览器，同时在本地缓存该结果。
   5. **建立连接**：浏览器使用获得的 IP 地址与目标服务器建立 TCP 连接，发送 HTTP 请求，获取并渲染网页内容。

   可以参考知乎回答：[在浏览器地址栏输入一个 URL 后回车，背后会进行哪些技术步骤？](https://www.zhihu.com/question/34873227/answer/518086565)
