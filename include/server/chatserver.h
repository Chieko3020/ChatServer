#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>

using namespace muduo;
using namespace muduo::net;

// 聊天服务器的主类
class ChatServer
{
public:
//muduo 的线程模型为「one loop per thread + threadPool」模型
//一个线程对应一个事件循环（EventLoop），
//也对应着一个 Reactor 模型。
//EventLoop 负责 IO 和定时器事件的分派。
//其中有 mainReactor 和 subReactor。
//mainReactor通过Acceptor接收新连接，然后将新连接派发到subReactor上进行连接的维护。
//这样mainReactor可以只专注于监听新连接的到来，而从维护旧连接的业务中得到解放。
//同时多个Reactor可以并行运行在多核 CPU 中，增加服务效率。
//Reactor 内部实现是 epoll/poll + 非阻塞I/O，epoll 采用 LT 模式。

//这里也就是webserver中熟知的 主线程负责监听连接 工作线程负责处理业务

    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg);
    // 启动服务
    void start();
    //

private:
//muduo 使用 TcpConnection 封装一次 TCP 连接
//而 TcpServer 用于编写网络服务器，接收客户端发起连接
//muduo 认为 TCP 网络编程只处理以下事件

/*
    连接的建立：包括服务端接收新连接和客户端成功发起连接
    连接的断开：包括主动断开（close、shutdown）和被动断开（read(2) 返回 0）
    消息到达：文件描述符可读
    消息发送完毕：有时候会用到
*/

//针对不同的事件，TcpServer 保存着不同事件发生时要调用的「回调函数」。
//要通过向 TcpServer 注册不同的回调函数来完成业务逻辑

    // 上报连接创建关闭相关信息的回调函数
    void On_Connection(const TcpConnectionPtr &);

    // 上报读写时间相关信息的回调函数
    void On_Message(const TcpConnectionPtr &, Buffer *, Timestamp);
    
    TcpServer tcp_server; // 组合的muduo库，实现服务器功能的类对象
    EventLoop* event_loop;  // 指向事件循环对象的指针
};

#endif
