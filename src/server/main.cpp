#include "chatserver.h"
#include "chatservice.h"
#include <muduo/base/Logging.h>
#include <iostream>
#include <signal.h>
using namespace std;

// 捕获SIGINT的处理函数
void resetHandler(int)
{
    LOG_INFO << "capture the SIGINT, will reset state.\n";
    ChatService::instance()->server_Close_Exception();
    exit(0);
}

int main(int argc, char **argv)
{
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr("127.0.0.1", atoi(argv[1]));
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}