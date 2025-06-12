#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <functional>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>
#include "group.h"
#include "user.h"
#include "public.h"

using namespace std;
using json = nlohmann::json;

void help(int,string);
string get_Current_Time();
void Menu(int clientfd);
void show_Current_User_Data();
void Sign_up_Response(json &responsejs);
void Login_Response(json &responsejs);
void Create_Group_Response(json &responsejs);
void add_friend(int clientfd, string str);
void chat(int clientfd, string str);
void create_group(int clientfd, string str);
void add_group(int clientfd, string str);
void group_chat(int clientfd, string str);
void log_out(int clientfd, string);


// 记录当前系统登录的用户信息
User global_current_User;
// 记录当前登录用户的好友列表信息
vector<User> global_current_User_Friend_List;
// 记录当前登录用户的群组列表信息
vector<Group> global_current_User_Group_List;
// 控制主菜单页面程序
bool is_Menu_Running = false;
// 用于读写线程之间的通信
sem_t sem;
// 记录登录状态
atomic_bool global_is_Login{false};
// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = 
{
    {"help", "显示所有支持的命令 格式help"},
    {"chat", "一对一聊天 格式chat:friendid:message"},
    {"addfriend", "添加好友 格式addfriend:friendid"},
    {"creategroup", "创建群组 格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组 格式addgroup:groupid"},
    {"groupchat", "群聊 格式groupchat:groupid:message"},
    {"logout", "注销 格式logout"}
};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = 
{
    {"help", help},
    {"chat", chat},
    {"addfriend", add_friend},
    {"creategroup", create_group},
    {"addgroup", add_group},
    {"groupchat", group_chat},
    {"logout", log_out}
};