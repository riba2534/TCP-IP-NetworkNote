## 第 18 章 多线程服务器端的实现

本章代码，在[TCP-IP-NetworkNote](https://github.com/riba2534/TCP-IP-NetworkNote)中可以找到。

### 18.1 理解线程的概念

#### 18.1.1 引入线程背景

第 10 章介绍了多进程服务端的实现方法。多进程模型与 select 和 epoll 相比的确有自身的优点，但同时也有问题。如前所述，创建（复制）进程的工作本身会给操作系统带来相当沉重的负担。而且，每个进程都具有独立的内存空间，所以进程间通信的实现难度也会随之提高。换言之，多进程的缺点可概括为：

- 创建进程的过程会带来一定的开销
- 为了完成进程间数据交换，需要特殊的 IPC 技术。

但是更大的缺点是下面的：

- 每秒少则 10 次，多则千次的「上下文切换」是创建进程的最大开销

只有一个 CPU 的系统是将时间分成多个微小的块后分配给了多个进程。为了分时使用 CPU ，需要「上下文切换」的过程。「上下文切换」是指运行程序前需要将相应进程信息读入内存，如果运行进程 A 后紧接着需要运行进程 B ，就应该将进程 A 相关信息移出内存，并读入进程 B 相关信息。这就是上下文切换。但是此时进程 A 的数据将被移动到硬盘，所以上下文切换要很长时间，即使通过优化加快速度，也会存在一定的局限。

为了保持多进程的优点，同时在一定程度上克服其缺点，人们引入的线程（Thread）的概念。这是为了将进程的各种劣势降至最低程度（不是直接消除）而设立的一种「轻量级进程」。线程比进程具有如下优点：

- 线程的创建和上下文切换比进程的创建和上下文切换更快
- 线程间交换数据无需特殊技术

#### 18.1.2 线程和进程的差异

线程是为了解决：为了得到多条代码执行流而复制整个内存区域的负担太重。

每个进程的内存空间都由保存全局变量的「数据区」、向 malloc 等函数动态分配提供空间的堆（Heap）、函数运行时间使用的栈（Stack）构成。每个进程都有独立的这种空间，多个进程的内存结构如图所示：

![](https://i.loli.net/2019/02/02/5c55aa57db3c7.png)

但如果以获得多个代码执行流为目的，则不应该像上图那样完全分离内存结构，而只需分离栈区域。通过这种方式可以获得如下优势：

- 上下文切换时不需要切换数据区和堆
- 可以利用数据区和堆交换数据

实际上这就是线程。线程为了保持多条代码执行流而隔开了栈区域，因此具有如下图所示的内存结构：

![](https://i.loli.net/2019/02/02/5c55ab455e399.png)

如图所示，多个线程共享数据区和堆。为了保持这种结构，线程将在进程内创建并运行。也就是说，进程和线程可以定义为如下形式：

- 进程：在操作系统构成单独执行流的单位
- 线程：在进程构成单独执行流的单位

如果说进程在操作系统内部生成多个执行流，那么线程就在同一进程内部创建多条执行流。因此，操作系统、进程、线程之间的关系可以表示为下图：

![](https://i.loli.net/2019/02/02/5c55ac20aa776.png)

### 18.2 线程创建及运行

可移植操作系统接口（英语：Portable Operating System Interface，缩写为POSIX）是IEEE为要在各种UNIX操作系统上运行软件，而定义API的一系列互相关联的标准的总称，其正式称呼为IEEE Std 1003，而国际标准名称为ISO/IEC 9945。此标准源于一个大约开始于1985年的项目。POSIX这个名称是由理查德·斯托曼（RMS）应IEEE的要求而提议的一个易于记忆的名称。它基本上是Portable Operating System Interface（可移植操作系统接口）的缩写，而X则表明其对Unix API的传承。

Linux基本上逐步实现了POSIX兼容，但并没有参加正式的POSIX认证。

微软的Windows NT声称部分实现了POSIX标准。

当前的POSIX主要分为四个部分：Base Definitions、System Interfaces、Shell and Utilities和Rationale。

#### 18.2.1 线程的创建和执行流程

线程具有单独的执行流，因此需要单独定义线程的 main 函数，还需要请求操作系统在单独的执行流中执行该函数，完成函数功能的函数如下：

```c
#include <pthread.h>

int pthread_create(pthread_t *restrict thread,
                   const pthread_attr_t *restrict attr,
                   void *(*start_routine)(void *),
                   void *restrict arg);
/*
成功时返回 0 ，失败时返回 -1
thread : 保存新创建线程 ID 的变量地址值。线程与进程相同，也需要用于区分不同线程的 ID
attr : 用于传递线程属性的参数，传递 NULL 时，创建默认属性的线程
start_routine : 相当于线程 main 函数的、在单独执行流中执行的函数地址值（函数指针）
arg : 通过第三个参数传递的调用函数时包含传递参数信息的变量地址值
*/
```

下面通过简单示例了解该函数功能：

- [thread1.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/thread1.c)

```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
void *thread_main(void *arg);

int main(int argc, char *argv[])
{
    pthread_t t_id;
    int thread_param = 5;
    // 请求创建一个线程，从 thread_main 调用开始，在单独的执行流中运行。同时传递参数
    if (pthread_create(&t_id, NULL, thread_main, (void *)&thread_param) != 0)
    {
        puts("pthread_create() error");
        return -1;
    }
    sleep(10); //延迟进程终止时间
    puts("end of main");
    return 0;
}
void *thread_main(void *arg) //传入的参数是 pthread_create 的第四个
{
    int i;
    int cnt = *((int *)arg);
    for (int i = 0; i < cnt; i++)
    {
        sleep(1);
        puts("running thread");
    }
    return NULL;
}
```

编译运行：

```shell
gcc thread1.c -o tr1 -lpthread # 线程相关代码编译时需要添加 -lpthread 选项声明需要连接到线程库
./tr1
```

运行结果：

![](https://i.loli.net/2019/02/02/5c55b5eb4daf6.png)

上述程序的执行如图所示：

![](https://i.loli.net/2019/02/02/5c55b6943255b.png)

可以看出，程序在主进程没有结束时，生成的线程每隔一秒输出一次 `running thread` ，但是如果主进程没有等待十秒，而是直接结束，这样也会强制结束线程，不论线程有没有运行完毕。

那是否意味着主进程必须每次都 sleep 来等待线程执行完毕？并不需要，可以通过以下函数解决。

```c
#include <pthread.h>
int pthread_join(pthread_t thread, void **status);
/*
成功时返回 0 ，失败时返回 -1
thread : 该参数值 ID 的线程终止后才会从该函数返回
status : 保存线程的 main 函数返回值的指针的变量地址值
*/
```

作用就是调用该函数的进程（或线程）将进入等待状态，知道第一个参数为 ID 的线程终止为止。而且可以得到线程的 main 函数的返回值。下面是该函数的用法代码：

- [thread2.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/thread2.c)

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
void *thread_main(void *arg);

int main(int argc, char *argv[])
{
    pthread_t t_id;
    int thread_param = 5;
    void *thr_ret;
    // 请求创建一个线程，从 thread_main 调用开始，在单独的执行流中运行。同时传递参数
    if (pthread_create(&t_id, NULL, thread_main, (void *)&thread_param) != 0)
    {
        puts("pthread_create() error");
        return -1;
    }
    //main函数将等待 ID 保存在 t_id 变量中的线程终止
    if (pthread_join(t_id, &thr_ret) != 0)
    {
        puts("pthread_join() error");
        return -1;
    }
    printf("Thread return message : %s \n", (char *)thr_ret);
    free(thr_ret);
    return 0;
}
void *thread_main(void *arg) //传入的参数是 pthread_create 的第四个
{
    int i;
    int cnt = *((int *)arg);
    char *msg = (char *)malloc(sizeof(char) * 50);
    strcpy(msg, "Hello,I'am thread~ \n");
    for (int i = 0; i < cnt; i++)
    {
        sleep(1);
        puts("running thread");
    }
    return (void *)msg; //返回值是 thread_main 函数中内部动态分配的内存空间地址值
}
```

编译运行：

```shell
gcc thread2.c -o tr2 -lpthread 
./tr2
```

运行结果：

![](https://i.loli.net/2019/02/02/5c55bd6032f1e.png)

可以看出，线程输出了5次字符串，并且把返回值给了主进程

下面是该函数的执行流程图：

![](https://i.loli.net/2019/02/02/5c55bdd3bb3c8.png)

#### 18.2.2 可在临界区内调用的函数

在同步的程序设计中，临界区块（Critical section）指的是一个访问共享资源（例如：共享设备或是共享存储器）的程序片段，而这些共享资源有无法同时被多个线程访问的特性。

当有线程进入临界区块时，其他线程或是进程必须等待（例如：bounded waiting 等待法），有一些同步的机制必须在临界区块的进入点与离开点实现，以确保这些共享资源是被异或的使用，例如：semaphore。

只能被单一线程访问的设备，例如：打印机。

一个最简单的实现方法就是当线程（Thread）进入临界区块时，禁止改变处理器；在uni-processor系统上，可以用“禁止中断（CLI）”来完成，避免发生系统调用（System Call）导致的上下文交换（Context switching）；当离开临界区块时，处理器恢复原先的状态。

根据临界区是否引起问题，函数可以分为以下 2 类：

- 线程安全函数（Thread-safe function）
- 非线程安全函数（Thread-unsafe function）

线程安全函数被多个线程同时调用也不会发生问题。反之，非线程安全函数被同时调用时会引发问题。但这并非有关于临界区的讨论，线程安全的函数中同样可能存在临界区。只是在线程安全的函数中，同时被多个线程调用时可通过一些措施避免问题。

幸运的是，大多数标准函数都是线程安全函数。操作系统在定义非线程安全函数的同时，提供了具有相同功能的线程安全的函数。比如，第 8 章的：

```c
struct hostent *gethostbyname(const char *hostname);
```

同时，也提供了同一功能的安全函数：

```c
struct hostent *gethostbyname_r(const char *name,
                                struct hostent *result,
                                char *buffer,
                                int intbuflen,
                                int *h_errnop);
```

线程安全函数结尾通常是 `_r` 。但是使用线程安全函数会给程序员带来额外的负担，可以通过以下方法自动将 gethostbyname 函数调用改为 gethostbyname_r 函数调用。

> 声明头文件前定义 `_REENTRANT` 宏。

无需特意更改源代码加，可以在编译的时候指定编译参数定义宏。

```shell
gcc -D_REENTRANT mythread.c -o mthread -lpthread
```

#### 18.2.3 工作（Worker）线程模型

下面的示例是计算从 1 到 10 的和，但并不是通过 main 函数进行运算，而是创建两个线程，其中一个线程计算 1 到 5 的和，另一个线程计算 6 到 10 的和，main 函数只负责输出运算结果。这种方式的线程模型称为「工作线程」。显示该程序的执行流程图：

![](https://i.loli.net/2019/02/03/5c55c330e8b5b.png)

下面是代码：

- [thread3.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/thread3.c)

```c
#include <stdio.h>
#include <pthread.h>
void *thread_summation(void *arg);
int sum = 0;

int main(int argc, char *argv[])
{
    pthread_t id_t1, id_t2;
    int range1[] = {1, 5};
    int range2[] = {6, 10};

    pthread_create(&id_t1, NULL, thread_summation, (void *)range1);
    pthread_create(&id_t2, NULL, thread_summation, (void *)range2);

    pthread_join(id_t1, NULL);
    pthread_join(id_t2, NULL);
    printf("result: %d \n", sum);
    return 0;
}
void *thread_summation(void *arg)
{
    int start = ((int *)arg)[0];
    int end = ((int *)arg)[1];
    while (start <= end)
    {
        sum += start;
        start++;
    }
    return NULL;
}
```

编译运行：

```shell
gcc thread3.c -D_REENTRANT -o tr3 -lpthread
./tr3
```

结果：

![](https://i.loli.net/2019/02/03/5c55c53d70494.png)

可以看出计算结果正确，两个线程都用了全局变量 sum ,证明了 2 个线程共享保存全局变量的数据区。

但是本例子本身存在问题。存在临界区相关问题，可以从下面的代码看出，下面的代码和上面的代码相似，只是增加了发生临界区错误的可能性，即使在高配置系统环境下也容易产生的错误：

- [thread4.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/thread4.c)

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define NUM_THREAD 100

void *thread_inc(void *arg);
void *thread_des(void *arg);
long long num = 0;

int main(int argc, char *argv[])
{
    pthread_t thread_id[NUM_THREAD];
    int i;

    printf("sizeof long long: %d \n", sizeof(long long));
    for (i = 0; i < NUM_THREAD; i++)
    {
        if (i % 2)
            pthread_create(&(thread_id[i]), NULL, thread_inc, NULL);
        else
            pthread_create(&(thread_id[i]), NULL, thread_des, NULL);
    }

    for (i = 0; i < NUM_THREAD; i++)
        pthread_join(thread_id[i], NULL);

    printf("result: %lld \n", num);
    return 0;
}

void *thread_inc(void *arg)
{
    int i;
    for (i = 0; i < 50000000; i++)
        num += 1;
    return NULL;
}
void *thread_des(void *arg)
{
    int i;
    for (i = 0; i < 50000000; i++)
        num -= 1;
    return NULL;
}
```

编译运行：

```shell
gcc thread4.c -D_REENTRANT -o tr4 -lpthread
./tr4
```

结果：

![](https://i.loli.net/2019/02/03/5c55c884e7c11.png)

从图上可以看出，每次运行的结果竟然不一样。理论上来说，上面代码的最后结果应该是 0 。原因暂时不得而知，但是可以肯定的是，这对于线程的应用是个大问题。

### 18.3 线程存在的问题和临界区

下面分析 [thread4.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/thread4.c) 中产生问题的原因，并给出解决方案。

#### 18.3.1 多个线程访问同一变量是问题

 [thread4.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/thread4.c) 的问题如下：

> 2 个线程正在同时访问全局变量 num

任何内存空间，只要被同时访问，都有可能发生问题。

因此，线程访问变量 num 时应该阻止其他线程访问，直到线程 1 运算完成。这就是同步（Synchronization）

#### 18.3.2 临界区位置

那么在刚才代码中的临界区位置是：

> 函数内同时运行多个线程时引发问题的多条语句构成的代码块

全局变量 num 不能视为临界区，因为他不是引起问题的语句，只是一个内存区域的声明。下面是刚才代码的两个 main 函数

```c
void *thread_inc(void *arg)
{
    int i;
    for (i = 0; i < 50000000; i++)
        num += 1;//临界区
    return NULL;
}
void *thread_des(void *arg)
{
    int i;
    for (i = 0; i < 50000000; i++)
        num -= 1;//临界区
    return NULL;
}
```

由上述代码可知，临界区并非 num 本身，而是访问 num 的两条语句，这两条语句可能由多个线程同时运行，也是引起这个问题的直接原因。产生问题的原因可以分为以下三种情况：

- 2 个线程同时执行 thread_inc 函数
- 2 个线程同时执行 thread_des 函数
- 2 个线程分别执行 thread_inc 和 thread_des 函数

比如发生以下情况：

> 线程 1 执行 thread_inc 的 num+=1 语句的同时，线程 2  执行 thread_des 函数的 num-=1 语句

也就是说，两条不同的语句由不同的线程执行时，也有可能构成临界区。前提是这 2 条语句访问同一内存空间。

### 18.4 线程同步

前面讨论了线程中存在的问题，下面就是解决方法，线程同步。

#### 18.4.1 同步的两面性

线程同步用于解决线程访问顺序引发的问题。需要同步的情况可以从如下两方面考虑。

- 同时访问同一内存空间时发生的情况
- 需要指定访问同一内存空间的线程顺序的情况

情况一之前已经解释过，下面讨论情况二。这是「控制线程执行的顺序」的相关内容。假设有 A B 两个线程，线程 A 负责向指定的内存空间内写入数据，线程 B 负责取走该数据。所以这是有顺序的，不按照顺序就可能发生问题。所以这种也需要进行同步。

#### 18.4.2 互斥量

互斥锁（英语：英语：Mutual exclusion，缩写 Mutex）是一种用于多线程编程中，防止两条线程同时对同一公共资源（比如全域变量）进行读写的机制。该目的通过将代码切片成一个一个的临界区域（critical section）达成。临界区域指的是一块对公共资源进行访问的代码，并非一种机制或是算法。一个程序、进程、线程可以拥有多个临界区域，但是并不一定会应用互斥锁。

通俗的说就互斥量就是一把优秀的锁，当临界区被占据的时候就上锁，等占用完毕然后再放开。

下面是互斥量的创建及销毁函数。

```c
#include <pthread.h>
int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
/*
成功时返回 0，失败时返回其他值
mutex : 创建互斥量时传递保存互斥量的变量地址值，销毁时传递需要销毁的互斥量地址
attr : 传递即将创建的互斥量属性，没有特别需要指定的属性时传递 NULL
*/
```

从上述函数声明中可以看出，为了创建相当于锁系统的互斥量，需要声明如下 pthread_mutex_t 型变量：

```c
pthread_mutex_t mutex
```

该变量的地址值传递给 pthread_mutex_init 函数，用来保存操作系统创建的互斥量（锁系统）。调用 pthread_mutex_destroy 函数时同样需要该信息。如果不需要配置特殊的互斥量属性，则向第二个参数传递 NULL 时，可以利用 PTHREAD_MUTEX_INITIALIZER 进行如下声明：

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
```

推荐尽可能的使用 pthread_mutex_init 函数进行初始化，因为通过宏进行初始化时很难发现发生的错误。

下面是利用互斥量锁住或释放临界区时使用的函数。

```c
#include <pthread.h>
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
/*
成功时返回 0 ，失败时返回其他值
*/
```

函数本身含有 lock unlock 等词汇，很容易理解其含义。进入临界区前调用的函数就是 pthread_mutex_lock 。调用该函数时，发现有其他线程已经进入临界区，则 pthread_mutex_lock 函数不会返回，直到里面的线程调用 pthread_mutex_unlock 函数退出临界区位置。也就是说，其他线程让出临界区之前，当前线程一直处于阻塞状态。接下来整理一下代码的编写方式：

```c
pthread_mutex_lock(&mutex);
//临界区开始
//...
//临界区结束
pthread_mutex_unlock(&mutex);
```

简言之，就是利用 lock 和 unlock 函数围住临界区的两端。此时互斥量相当于一把锁，阻止多个线程同时访问，还有一点要注意，线程退出临界区时，如果忘了调用 pthread_mutex_unlock 函数，那么其他为了进入临界区而调用 pthread_mutex_lock 的函数无法摆脱阻塞状态。这种情况称为「死锁」。需要格外注意，下面是利用互斥量解决示例 [thread4.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/thread4.c) 中遇到的问题代码：

- [mutex.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/mutex.c)

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define NUM_THREAD 100
void *thread_inc(void *arg);
void *thread_des(void *arg);

long long num = 0;
pthread_mutex_t mutex; //保存互斥量读取值的变量

int main(int argc, char *argv[])
{
    pthread_t thread_id[NUM_THREAD];
    int i;

    pthread_mutex_init(&mutex, NULL); //创建互斥量

    for (i = 0; i < NUM_THREAD; i++)
    {
        if (i % 2)
            pthread_create(&(thread_id[i]), NULL, thread_inc, NULL);
        else
            pthread_create(&(thread_id[i]), NULL, thread_des, NULL);
    }

    for (i = 0; i < NUM_THREAD; i++)
        pthread_join(thread_id[i], NULL);

    printf("result: %lld \n", num);
    pthread_mutex_destroy(&mutex); //销毁互斥量
    return 0;
}

void *thread_inc(void *arg)
{
    int i;
    pthread_mutex_lock(&mutex); //上锁
    for (i = 0; i < 50000000; i++)
        num += 1;
    pthread_mutex_unlock(&mutex); //解锁
    return NULL;
}
void *thread_des(void *arg)
{
    int i;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < 50000000; i++)
        num -= 1;
    pthread_mutex_unlock(&mutex);
    return NULL;
}
```

编译运行：

```shell
gcc mutex.c -D_REENTRANT -o mutex -lpthread
./mutex
```

运行结果：

![](https://i.loli.net/2019/02/03/5c567e4aafbb8.png)

从运行结果可以看出，通过互斥量机制得出了正确的运行结果。

在代码中：

```c
void *thread_inc(void *arg)
{
    int i;
    pthread_mutex_lock(&mutex); //上锁
    for (i = 0; i < 50000000; i++)
        num += 1;
    pthread_mutex_unlock(&mutex); //解锁
    return NULL;
}
```

以上代码的临界区划分范围较大，但这是考虑如下优点所做的决定:

> 最大限度减少互斥量 lock unlock 函数的调用次数

#### 18.4.3 信号量

信号量（英语：Semaphore）又称为信号标，是一个同步对象，用于保持在0至指定最大值之间的一个计数值。当线程完成一次对该semaphore对象的等待（wait）时，该计数值减一；当线程完成一次对semaphore对象的释放（release）时，计数值加一。当计数值为0，则线程等待该semaphore对象不再能成功直至该semaphore对象变成signaled状态。semaphore对象的计数值大于0，为signaled状态；计数值等于0，为nonsignaled状态.

semaphore对象适用于控制一个仅支持有限个用户的共享资源，是一种不需要使用忙碌等待（busy waiting）的方法。

信号量的概念是由荷兰计算机科学家艾兹赫尔·戴克斯特拉（Edsger W. Dijkstra）发明的，广泛的应用于不同的操作系统中。在系统中，给予每一个进程一个信号量，代表每个进程当前的状态，未得到控制权的进程会在特定地方被强迫停下来，等待可以继续进行的信号到来。如果信号量是一个任意的整数，通常被称为计数信号量（Counting semaphore），或一般信号量（general semaphore）；如果信号量只有二进制的0或1，称为二进制信号量（binary semaphore）。在linux系统中，二进制信号量（binary semaphore）又称互斥锁（Mutex）。

下面介绍信号量，在互斥量的基础上，很容易理解信号量。此处只涉及利用「二进制信号量」（只用 0 和 1）完成「控制线程顺序」为中心的同步方法。下面是信号量的创建及销毁方法：

```c
#include <semaphore.h>
int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
/*
成功时返回 0 ，失败时返回其他值
sem : 创建信号量时保存信号量的变量地址值，销毁时传递需要销毁的信号量变量地址值
pshared : 传递其他值时，创建可由多个继承共享的信号量；传递 0 时，创建只允许 1 个进程内部使用的信号量。需要完成同一进程的线程同步，故为0
value : 指定创建信号量的初始值
*/
```

上述的 shared 参数超出了我们的关注范围，故默认向其传递为 0 。下面是信号量中相当于互斥量 lock unlock 的函数。

```c
#include <semaphore.h>
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
/*
成功时返回 0 ，失败时返回其他值
sem : 传递保存信号量读取值的变量地址值，传递给 sem_post 的信号量增1，传递给 sem_wait 时信号量减一
*/
```

调用 sem_init 函数时，操作系统将创建信号量对象，此对象中记录这「信号量值」（Semaphore Value）整数。该值在调用 sem_post 函数时增加 1 ，调用 wait_wait 函数时减一。但信号量的值不能小于 0 ，因此，在信号量为 0 的情况下调用 sem_wait 函数时，调用的线程将进入阻塞状态（因为函数未返回）。当然，此时如果有其他线程调用 sem_post 函数，信号量的值将变为 1 ，而原本阻塞的线程可以将该信号重新减为 0 并跳出阻塞状态。实际上就是通过这种特性完成临界区的同步操作，可以通过如下形式同步临界区（假设信号量的初始值为 1）

```c
sem_wait(&sem);//信号量变为0...
// 临界区的开始
//...
//临界区的结束
sem_post(&sem);//信号量变为1...
```

上述代码结构中，调用 sem_wait 函数进入临界区的线程在调用 sem_post 函数前不允许其他线程进入临界区。信号量的值在 0 和  1 之间跳转，因此，具有这种特性的机制称为「二进制信号量」。接下来的代码是信号量机制的代码。下面代码并非是同时访问的同步，而是关于控制访问顺序的同步，该场景为：

> 线程  A 从用户输入得到值后存入全局变量 num ，此时线程 B 将取走该值并累加。该过程一共进行 5 次，完成后输出总和并退出程序。

下面是代码：

- [semaphore.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/semaphore.c)

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

void *read(void *arg);
void *accu(void *arg);
static sem_t sem_one;
static sem_t sem_two;
static int num;

int main(int argc, char const *argv[])
{
    pthread_t id_t1, id_t2;
    sem_init(&sem_one, 0, 0);
    sem_init(&sem_two, 0, 1);

    pthread_create(&id_t1, NULL, read, NULL);
    pthread_create(&id_t2, NULL, accu, NULL);

    pthread_join(id_t1, NULL);
    pthread_join(id_t2, NULL);

    sem_destroy(&sem_one);
    sem_destroy(&sem_two);
    return 0;
}

void *read(void *arg)
{
    int i;
    for (i = 0; i < 5; i++)
    {
        fputs("Input num: ", stdout);

        sem_wait(&sem_two);
        scanf("%d", &num);
        sem_post(&sem_one);
    }
    return NULL;
}
void *accu(void *arg)
{
    int sum = 0, i;
    for (i = 0; i < 5; i++)
    {
        sem_wait(&sem_one);
        sum += num;
        sem_post(&sem_two);
    }
    printf("Result: %d \n", sum);
    return NULL;
}
```

编译运行：

```shell
gcc semaphore.c -D_REENTRANT -o sema -lpthread
./sema
```

结果：

![](https://i.loli.net/2019/02/03/5c568c2717d1e.png)

从上述代码可以看出，设置了两个信号量 one 的初始值为 0 ，two 的初始值为 1，然后在调用函数的时候，「读」的前提是 two 可以减一，如果不能减一就会阻塞在这里，一直等到「计算」操作完毕后，给 two 加一，然后就可以继续执行下一句输入。对于「计算」函数，也一样。

### 18.5 线程的销毁和多线程并发服务器端的实现

先介绍线程的销毁，然后再介绍多线程服务端

#### 18.5.1 销毁线程的 3 种方法

Linux 的线程并不是在首次调用的线程 main 函数返回时自动销毁，所以利用如下方法之一加以明确。否则由线程创建的内存空间将一直存在。

- 调用 pthread_join 函数
- 调用 pthread_detach 函数

之前调用过 pthread_join 函数。调用该函数时，不仅会等待线程终止，还会引导线程销毁。但该函数的问题是，线程终止前，调用该函数的线程将进入阻塞状态。因此，通过如下函数调用引导线程销毁。

```c
#include <pthread.h>
int pthread_detach(pthread_t th);
/*
成功时返回 0 ，失败时返回其他值
thread : 终止的同时需要销毁的线程 ID
*/
```

调用上述函数不会引起线程终止或进入阻塞状态，可以通过该函数引导销毁线程创建的内存空间。调用该函数后不能再针对相应线程调用 pthread_join 函数。

#### 18.5.2 多线程并发服务器端的实现

下面是多个客户端之间可以交换信息的简单聊天程序。

- [chat_server.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/chat_server.c)
- [chat_clnt.c](https://github.com/riba2534/TCP-IP-NetworkNote/blob/master/ch18/chat_clnt.c)

上面的服务端示例中，需要掌握临界区的构成，访问全局变量 clnt_cnt 和数组 clnt_socks 的代码将构成临界区，添加和删除客户端时，变量 clnt_cnt 和数组 clnt_socks 将同时发生变化。因此下列情形会导致数据不一致，从而引发错误：

- 线程 A 从数组 clnt_socks 中删除套接字信息，同时线程 B 读取 clnt_cnt 变量
- 线程 A 读取变量 clnt_cnt ，同时线程 B 将套接字信息添加到 clnt_socks 数组

编译运行：

```shell
gcc chat_server.c -D_REENTRANT -o cserv -lpthread
gcc chat_clnt.c -D_REENTRANT -o cclnt -lpthread
./cserv 9191
./cclnt 127.0.0.1 9191 张三
./cclnt 127.0.0.1 9191 李四
```

结果：

![](https://i.loli.net/2019/02/03/5c569b70634ff.png)

### 18.6 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **单 CPU 系统中如何同时执行多个进程？请解释该过程中发生的上下文切换**。

   答：系统将 CPU 时间分成多个微小的时间块后分配给了多个进程。为了分时使用 CPU ，需要「上下文切换」过程。运行程序前需要将相应进程信息读入内存，如果运行进程 A 后需要紧接着运行进程 B ，就应该将进程 A 相关信息移出内存，并读入进程 B 的信息。这就是上下文切换

2. **为何线程的上下文切换速度相对更快？线程间数据交换为何不需要类似 IPC 特别技术**。

   答：线程上下文切换过程不需要切换数据区和堆。可以利用数据区和堆交换数据。

3. **请从执行流角度说明进程和线程的区别**。

   答：进程：在操作系统构成单独执行流的单位。线程：在进程内部构成单独执行流的单位。线程为了保持多条代码执行流而隔开了栈区域。

4. **下面关于临界区的说法错误的是**？

   答：下面加粗的选项为说法正确。（全错）

   1. 临界区是多个线程同时访问时发生问题的区域
   2. 线程安全的函数中不存在临界区，即便多个线程同时调用也不会发生问题
   3. 1 个临界区只能由 1 个代码块，而非多个代码块构成。换言之，线程 A 执行的代码块 A 和线程 B 执行的代码块 B 之间绝对不会构成临界区。
   4. 临界区由访问全局变量的代码构成。其他变量中不会发生问题。

5. **下列关于线程同步的说法错误的是**？

   答：下面加粗的选项为说法正确。

   1. 线程同步就是限制访问临界区
   2. **线程同步也具有控制线程执行顺序的含义**
   3. **互斥量和信号量是典型的同步技术**
   4. 线程同步是代替进程 IPC 的技术。

6. **请说明完全销毁 Linux 线程的 2 种办法**

   答：①调用 pthread_join 函数②调用 pthread_detach 函数。第一个会阻塞调用的线程，而第二个不阻塞。都可以引导线程销毁。
