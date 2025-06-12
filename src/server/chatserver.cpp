#include "chatserver.h"
#include "chatservice.h"
#include "json.hpp"
#include <string>

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

// 初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
    : tcp_server(loop, listenAddr, nameArg), event_loop(loop)
{
    // 注册连接回调 void On_Connection(const TcpConnectionPtr &) 一个参数
    tcp_server.setConnectionCallback(std::bind(&ChatServer::On_Connection, this, _1));

    // 注册消息回调 void On_Message(const TcpConnectionPtr &, Buffer *, Timestamp) 三个参数
    tcp_server.setMessageCallback(std::bind(&ChatServer::On_Message, this, _1, _2, _3));

    // 设置线程数量
    tcp_server.setThreadNum(4);
}

// 启动服务
void ChatServer::start()
{
    tcp_server.start();
}

// 上报连接创建关闭相关信息的回调函数
void ChatServer::On_Connection(const TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 处理客户端异常退出事件
        ChatService::instance()->client_Close_Exception(conn);
        // 释放sockfd资源
        conn->shutdown();
    }
}

// 上报读写时间相关信息的回调函数
void ChatServer::On_Message(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    // 读取缓冲区所有数据  转换为string
    string buf = buffer->retrieveAllAsString();
    // 数据的反序列化
    json js = json::parse(buf);

    // 通过js["msgid"]绑定一个回调函数
    // 只要解析出来msgid就可以回调对应的函数
    // 完全解耦网络模块的代码和业务模块的代码

    // js[key]:value value就是需要的msgid 转换为int给get_Handler
    // MsgHandler get_Handler(int msgid); 从unordered_map获取cb functional
    // 然后执行相应的cb函数

    auto msgHandler = ChatService::instance()->get_Handler(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, time);
}
