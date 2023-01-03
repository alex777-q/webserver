# Unix/Linux 上的五种IO模型
## 阻塞 blocking
- 调用者调用了某个函数，等待这个函数返回，期间什么也做不了，不停的去检查这个函数有没有返回，必须等这个函数返回才能进行下一步动作

## 非阻塞 non-blokcing(NIO)
- 非阻塞等待，每隔一段时间就去检测IO事件是否就绪，没有就绪就可以做其他的事，非阻塞IO执行系统调用总是立即返回，不管事件是否已经发生，若事件没有发生，则返回-1，此时可以根据errno区分这两种情况，对于accept，recv，send，事件未发生时，errno通常被设置成EAGAIN。

## IO复用(IO multiplexing)
- Linux 用select/poll/epoll/ 函数实现IO 复用模型，这些函数也会使进程阻塞，但是和阻塞IO所不同的是这些函数可以同时阻塞多个IO操作。而且可以同时对多个读操作、写操作的IO函数进行检测，直到有数据可读或可写时，才真正调用IO操作函数。

## 信号驱动(signal-driven)
- Linux 用套接口进行信号驱动IO，安装一个信号处理函数，进程继续运行并不阻塞，当IO事件就绪，进程收到SIGIO信号，然后处理IO事件。
- 内核在第一个阶段是异步，在第二个阶段是同步；与非阻塞IO的区别在于它提供了消息通知机制，不需要用户进程不断的轮询检查，减少了系统API的调用次数，提高了效率。 

## 异步(asynchronous)
- Linux中，可以调用aio_read()函数告诉内核描述字缓冲区指针和缓冲区的大小、文件偏移及通知的方式，然后立即返回，当内核数据拷贝到缓冲区后，再通知应用程序。
```c
/* Asynchronous IO control block */
struct aiocb{
    int aio_fildes;  /* file desriptor */
    int aio_lio_opcode;  /* operation to be performed */
    int aio_reqrio;  /* request priority offset */
    volatile void *aio_buf;  /* location of buffer */
    size_t aio_nbytes;  /* length of transfer */
    struct sigevent aio_sigevent;  /* signal number and value */
} 
```