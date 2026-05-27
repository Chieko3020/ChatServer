#include "chatservice.h"
#include "public.h"
#include "user.h"
#include <muduo/base/Logging.h>
#include <string>
#include <vector>
using namespace muduo;
// 单例模式 static对象保证线程安全并且只会被初始化一次 懒汉
// 负责解耦业务模块和网络模块 内部保存了各个业务模块的回调函数

ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    // std::bind() 函数对象 类指针 函数参数占位（根据函数参数个数决定）
    msgHandler_Map.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    msgHandler_Map.insert({SIGN_UP_MSG, std::bind(&ChatService::sign_up, this, _1, _2, _3)});
    msgHandler_Map.insert({ONE_CHAT_MSG, std::bind(&ChatService::one_Chat, this, _1, _2, _3)});
    msgHandler_Map.insert({ADD_FRIEND_MSG, std::bind(&ChatService::add_Friend, this, _1, _2, _3)});
    msgHandler_Map.insert({CREATE_GROUP_MSG, std::bind(&ChatService::create_Group, this, _1, _2, _3)});
    msgHandler_Map.insert({ADD_GROUP_MSG, std::bind(&ChatService::add_Group, this, _1, _2, _3)});
    msgHandler_Map.insert({GROUP_CHAT_MSG, std::bind(&ChatService::group_Chat, this, _1, _2, _3)});
    msgHandler_Map.insert({LOGOUT_MSG, std::bind(&ChatService::log_out, this, _1, _2, _3)});

    if (redis.connect())
    {
        redis.init_notify_handler(std::bind(&ChatService::redis_subscribe_message, this, _1, _2));
    }
}

// 获取消息对应的回调函数
// ChatService 将各个业务模块的代码放入了std::function
// 使用unordered_map<msgid, std::function(也就是函数MsgHandler)
// 通过getHandler(msgid)得到对应的
// 业务处理函数 MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>
// 再执行MsgHandler
MsgHandler ChatService::get_Handler(int msgid)
{
    // 记录错误日志 msgid没有对应的事件处理回调
    auto it = msgHandler_Map.find(msgid);
    if (it == msgHandler_Map.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid: " << msgid << " can not find handler";
        };
    }
    else
    {
        return msgHandler_Map[msgid];
    }
}

// 处理登录业务
// 从json对象中获取用户ID和密码 并在数据库中查询获取用户信息
// 登录成功后，需要在用户表userConn_Map中记录新登录的用户
// 考虑到多线程对此表进行操作需要使用锁
// 还需要查询该用户离线时是否有离线消息，
// 如果有离线消息 取出放到vector<string>容器 再转为json返回
// 还需要显示这个登录用户好友列表 查询MYSQL表 取出放到vector<string>容器 再转为json返回
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "login service.";
    int id = js["id"].get<int>();
    std::string password = js["password"];

    User user = userModel.query(id);
    if (user.get_Id() == id && user.get_Password() == password)
    {
        if (user.get_State() == "online")
        {
            // 该用户已经登录，不能重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息到unordered_map
            // 需要考虑线程安全问题
            {
                lock_guard<mutex> locker(mtx);
                userConn_Map.insert({id, conn});
            }

            // 登录成功，更新用户状态信息 state offline => online
            user.set_State("online");
            userModel.update_State(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.get_Id();
            response["name"] = user.get_Name();

            // 查询该用户是否有离线消息
            std::vector<std::string> vec = offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取该用户的离线消息后，将该用户离线消息删除掉
                offlineMsgModel.remove(id);
            }
            else
            {
                LOG_INFO << "无离线消息";
            }

            std::vector<User> userVec = friendModel.query(id);
            if (!userVec.empty())
            {
                std::vector<std::string> vec;
                for (auto &user : userVec)
                {
                    json js;
                    js["id"] = user.get_Id();
                    js["name"] = user.get_Name();
                    js["state"] = user.get_State();
                    vec.push_back(js.dump());
                }
                response["friends"] = vec;
            }

            conn->send(response.dump());
        }
    }
}

// 处理登出业务
void ChatService::log_out(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    
    {
        lock_guard<mutex> locker(mtx);
        auto it = userConn_Map.find(userid);
        if (it != userConn_Map.end())
        {
            userConn_Map.erase(it);
        }
    }

    // 用户注销
    redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    userModel.update_State(user);
}

// 处理注册业务
// 从json对象中获取用户 ID 和用户密码
// 并用此信息初始化一个User对象 其初始状态设置为offline(默认参数是offline)
// 之后调用 model 层代码与数据库交互(model层调用MYSQL封装类中的API) 插入这个用户的信息
// 若成功则发出response信息，
// 需要将信息序列化 conn->send(response.dump());
void ChatService::sign_up(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "sign_up service.";
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.set_Name(name);
    user.set_Password(pwd);
    bool state = userModel.insert(user);
    if (state)
    {
        // 注册成功 成功返回0，失败返回1
        json response;
        response["msgid"] = SIGN_UP_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.get_Id();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = SIGN_UP_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

// 处理客户端异常退出
// 如果客户端异常退出 要从服务端记录用户连接的userConn_Map表中找到用户
// 如果它断连了就删除 并设置其状态为 offline。
void ChatService::client_Close_Exception(const TcpConnectionPtr &conn)
{
    User user;
    // 互斥锁保护
    {
        lock_guard<mutex> locker(mtx);
        for (auto it = userConn_Map.begin(); it != userConn_Map.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的链接信息
                user.set_Id(it->first);
                userConn_Map.erase(it);
                break;
            }
        }
    }

    // 用户注销
    redis.unsubscribe(user.get_Id());

    // 更新用户的状态信息
    if (user.get_Id() != -1)
    {
        user.set_State("offline");
        userModel.update_State(user);
    }
}

void ChatService::server_Close_Exception()
{
    // 将所有online状态的用户，设置成offline
    userModel.reset_State();
}


// 一对一聊天业务
// 对方处于登录状态：直接向该用户发送信息
// 对方处于离线状态：需存储离线消息
void ChatService::one_Chat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 需要接收信息的用户ID
    int reciver = js["toid"].get<int>();
    
    {
        lock_guard<mutex> locker(mtx);
        auto it = userConn_Map.find(reciver);
        // 确认是在线状态
        if (it != userConn_Map.end())
        {
            // TcpConnection::send() 直接发送消息
            it->second->send(js.dump());
            return;
        }
    }

    // 对方不在线则存储离线消息
    offlineMsgModel.insert(reciver, js.dump());
}

// redis订阅消息触发的回调函数
void ChatService::redis_subscribe_message(int id, string message)
{
    //用户在线
    lock_guard<mutex> locker(mtx);
    auto it = userConn_Map.find(id);
    if (it != userConn_Map.end())
    {
        it->second->send(message);
        return;
    }

    //转储离线
    offlineMsgModel.insert(id, message);
}

// 添加朋友业务
void ChatService::add_Friend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userId = js["id"].get<int>();
    int friendId = js["friendid"].get<int>();

    // 存储好友信息
    friendModel.insert(userId, friendId);
}

// 创建群组业务
// 先创建一个空成员群组 再把creator加入群组成员表
void ChatService::create_Group(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userId = js["id"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];

    // 存储新创建的群组消息
    Group group(-1, name, desc);
    if (groupModel.create_Group(group))
    {
        // 存储群组创建人信息
        groupModel.add_Group(userId, group.get_Id(), "creator");
        
        // 发送成功响应
        json response;
        response["msgid"] = CREATE_GROUP_MSG_ACK;
        response["errno"] = 0;
        response["groupid"] = group.get_Id();
        response["groupname"] = name;
        response["groupdesc"] = desc;
        conn->send(response.dump());
    }
    else
    {
        // 发送失败响应
        json response;
        response["msgid"] = CREATE_GROUP_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "创建群组失败";
        conn->send(response.dump());
    }
}

// 加入群组业务
void ChatService::add_Group(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userId = js["id"].get<int>();
    int groupId = js["groupid"].get<int>();
    groupModel.add_Group(userId, groupId, "normal");
}

// 群组聊天业务
void ChatService::group_Chat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userId = js["id"].get<int>();
    int groupId = js["groupid"].get<int>();
    std::vector<int> userIdVec = groupModel.query_GroupUsers(userId, groupId);

    lock_guard<mutex> lock(mtx);
    for (int id : userIdVec)
    {
        auto it = userConn_Map.find(id);
        if (it != userConn_Map.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线
            User user = userModel.query(id);
            if (user.get_State() == "online")
            {
                // 向群组成员publish信息
                redis.publish(id, js.dump());
            }
            else
            {
                //转储离线消息
                offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

