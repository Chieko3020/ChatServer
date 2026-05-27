#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "json.hpp"
#include "usermodel.h"
#include "offlinemsgmodel.h"
#include "friendmodel.h"
#include "groupmodel.h"
#include "redis.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

// 聊天服务器业务类 单例模式
// 负责解耦业务模块和网络模块 内部保存了各个业务模块的回调函数
class ChatService
{
public:

    // 获取单例对象的接口函数
    static ChatService *instance();
    // 获取消息对应的回调函数
    MsgHandler get_Handler(int msgid);

    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注册业务
    void sign_up(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理登出业务
    void log_out(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 一对一聊天业务
    void one_Chat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 添加好友业务
    void add_Friend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 创建群组业务
    void create_Group(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加入群组业务
    void add_Group(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群组聊天业务
    void group_Chat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理客户端异常退出
    void client_Close_Exception(const TcpConnectionPtr &conn);
    // 服务端异常终止之后的操作
    void server_Close_Exception();
    //redis订阅消息触发的回调函数
    void redis_subscribe_message(int id, string message);

private:
    ChatService();
    //保证单例 禁用拷贝构造和赋值运算符
    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) = delete;

    // 存储消息id和其对应的业务处理方法
    unordered_map<int, MsgHandler> msgHandler_Map;

    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> userConn_Map;

    // 数据操作类对象
    UserModel userModel;
    OfflineMsgModel offlineMsgModel;
    FriendModel friendModel;
    GroupModel groupModel;
    std::mutex mtx;
    
    //redis操作对象
    Redis redis;
};

#endif