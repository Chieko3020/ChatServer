#include "main.h"

void help(int, string)
{
    cout << "show command list >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

// 获取系统时间（聊天信息需要添加时间信息）
string get_Current_Time()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}
// 主聊天页面程序
void Menu(int clientfd)
{
    help(0,"");

    char buffer[1024] = {0};
    while (is_Menu_Running)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; // 存储命令
        int idx = commandbuf.find(":");
        if (idx == -1)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        // 调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 调用命令处理方法
    }
}
// 显示当前登录成功用户的基本信息
void show_Current_User_Data()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << global_current_User.get_Id() << " name:" << global_current_User.get_Name() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!global_current_User_Friend_List.empty())
    {
        for (User &user : global_current_User_Friend_List)
        {
            cout << user.get_Id() << " " << user.get_Name() << " " << user.get_State() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!global_current_User_Group_List.empty())
    {
        for (Group &group : global_current_User_Group_List)
        {
            cout << group.get_Id() << " " << group.get_Name() << " " << group.get_Desc() << endl;
            for (GroupUser &user : group.get_Users())
            {
                cout << user.get_Id() << " " << user.get_Name() << " " << user.get_State()
                     << " " << user.get_Role() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}



// 处理注册的响应逻辑
void Sign_up_Response(json &responsejs)
{
    if ( responsejs["errno"].get<int>() != 0) // 注册失败
    {
        cerr << "name is already exist, register error!" << endl;
    }
    else // 注册成功
    {
        cout << "name register success, userid is " << responsejs["id"]
                << ", do not forget it!" << endl;
    }
}

// 处理登录的响应逻辑
void Login_Response(json &responsejs)
{
    if ( responsejs["errno"].get<int>() != 0) // 登录失败
    {
        cerr << responsejs["errmsg"] << endl;
        global_is_Login = false;
    }
    else // 登录成功
    {
        // 记录当前用户的id和name
        global_current_User.set_Id(responsejs["id"].get<int>());
        global_current_User.set_Name(responsejs["name"]);

        // 记录当前用户的好友列表信息
        if (responsejs.contains("friends"))
        {
            // 初始化
            global_current_User_Friend_List.clear();

            vector<string> vec = responsejs["friends"];
            for (string &str : vec)
            {
                json js = json::parse(str);
                User user;
                user.set_Id(js["id"].get<int>());
                user.set_Name(js["name"]);
                user.set_State(js["state"]);
                global_current_User_Friend_List.push_back(user);
            }
        }

        // 记录当前用户的群组列表信息
        if (responsejs.contains("groups"))
        {
            // 初始化
            global_current_User_Group_List.clear();

            vector<string> vec1 = responsejs["groups"];
            for (string &groupstr : vec1)
            {
                json grpjs = json::parse(groupstr);
                Group group;
                group.set_Id(grpjs["id"].get<int>());
                group.set_Name(grpjs["groupname"]);
                group.set_Desc(grpjs["groupdesc"]);

                vector<string> vec2 = grpjs["users"];
                for (string &userstr : vec2)
                {
                    GroupUser user;
                    json js = json::parse(userstr);
                    user.set_Id(js["id"].get<int>());
                    user.set_Name(js["name"]);
                    user.set_State(js["state"]);
                    user.set_Role(js["role"]);
                    group.get_Users().push_back(user);
                }

                global_current_User_Group_List.push_back(group);
            }
        }

        // 显示登录用户的基本信息
        show_Current_User_Data();

        // 显示当前用户的离线消息  个人聊天信息或者群组消息
        if (responsejs.contains("offlinemsg"))
        {
            vector<string> vec = responsejs["offlinemsg"];
            for (string &str : vec)
            {
                json js = json::parse(str);
                // time + [id] + name + " said: " + xxx
                if (ONE_CHAT_MSG == js["msgid"].get<int>())
                {
                    cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                            << " said: " << js["msg"].get<string>() << endl;
                }
                else
                {
                    cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                            << " said: " << js["msg"].get<string>() << endl;
                }
            }
        }

       global_is_Login = true;
    }
}

void add_friend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = global_current_User.get_Id();
    js["friendid"] = friendid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addfriend msg error -> " << buffer << endl;
    }
}

void chat(int clientfd, string str)
{
    int idx = str.find(":"); // friendid:message
    if (idx == -1)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = global_current_User.get_Id();
    js["name"] = global_current_User.get_Name();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = get_Current_Time();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send chat msg error -> " << buffer << endl;
    }
}

void create_group(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = global_current_User.get_Id();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send creategroup msg error -> " << buffer << endl;
    }
}


// 处理群组创建响应
void Create_Group_Response(json &responsejs)
{
    if (responsejs["errno"].get<int>() != 0)
    {
        cerr << "创建群组失败: " << responsejs["errmsg"] << endl;
    }
    else
    {
        cout << "创建群组成功！群组信息：" << endl;
        cout << "群组ID: " << responsejs["groupid"] << endl;
        cout << "群组名称: " << responsejs["groupname"] << endl;
        cout << "群组描述: " << responsejs["groupdesc"] << endl;
    }
}

void add_group(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = global_current_User.get_Id();
    js["groupid"] = groupid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addgroup msg error -> " << buffer << endl;
    }
}

void group_chat(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = global_current_User.get_Id();
    js["name"] = global_current_User.get_Name();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = get_Current_Time();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send groupchat msg error -> " << buffer << endl;
    }
}

void log_out(int clientfd, string)
{
    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = global_current_User.get_Id();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send loginout msg error -> " << buffer << endl;
    }
    else
    {
        is_Menu_Running = false;
    }   
}




// 聊天客户端程序实现 main线程用作发送线程 子线程用作接收线程
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6666" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建client端的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }


    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    if (connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)) == -1)
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    // 初始化读写线程通信用的信号量
    sem_init(&sem, 0, 0);

    // 连接服务器成功 启动接收子线程
    std::thread([=](int clientfd)
    {
        while(true)
        {
            char buffer[1024] = {0};
            int len = recv(clientfd, buffer, 1024, 0);
            if (len == -1 || len == 0)
            {
                close(clientfd);
                exit(-1);
            }

            // 接收ChatServer转发的数据，反序列化生成json数据对象
            json js = json::parse(buffer);
            int msgtype = js["msgid"].get<int>();
            if (ONE_CHAT_MSG == msgtype)
            {
                cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                    << " said: " << js["msg"].get<string>() << endl;
                continue;
            }

            if (GROUP_CHAT_MSG == msgtype)
            {
                cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                    << " said: " << js["msg"].get<string>() << endl;
                continue;
            }

            if (LOGIN_MSG_ACK == msgtype)
            {
                Login_Response(js); // 处理登录响应的业务逻辑
                sem_post(&sem);    // 通知主线程，登录结果处理完成
                continue;
            }

            if (SIGN_UP_MSG_ACK == msgtype)
            {
                Sign_up_Response(js);
                sem_post(&sem);    // 通知主线程，注册结果处理完成
                continue;
            }

            if (CREATE_GROUP_MSG_ACK == msgtype)
            {
                Create_Group_Response(js);
                continue;
            }
        }
    },clientfd).detach(); 
                              

    // main线程用于接收用户输入，负责发送数据
    while(true)
    {
        // 显示首页面菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车


        switch (choice)
        {
            case 1: // login业务
            {
                int id = 0;
                char pwd[50] = {0};
                cout << "user id:";
                cin >> id;
                cin.get(); // 读掉缓冲区残留的回车
                cout << "user password:";
                cin.getline(pwd, 50);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                string request = js.dump();

                global_is_Login = false;

                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1)
                {
                    cerr << "send login msg error:" << request << endl;
                }

                sem_wait(&sem); // 等待子线程处理登录响应消息
                    
                if (global_is_Login) 
                {
                    // 进入聊天主菜单页面
                    is_Menu_Running = true;
                    Menu(clientfd);
                }
            }
            break;
            case 2: // register业务
            {
                char name[50] = {0};
                char pwd[50] = {0};
                cout << "user name:";
                cin.getline(name, 50);
                cout << "user password:";
                cin.getline(pwd, 50);

                json js;
                js["msgid"] = SIGN_UP_MSG;
                js["name"] = name;
                js["password"] = pwd;
                string request = js.dump();

                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1)
                {
                    cerr << "send reg msg error:" << request << endl;
                }
                
                sem_wait(&sem); // 等待子线程处理注册消息
            }
            break;
            case 3: // quit业务
                close(clientfd);
                sem_destroy(&sem);
                exit(0);
            default:
                cerr << "invalid input!" << endl;
                break;
        }
    }

    return 0;
}