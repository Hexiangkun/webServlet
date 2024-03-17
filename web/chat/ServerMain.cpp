//
// Created by 37496 on 2024/3/16.
//
#include "server/ChatServer.h"
#include "service/ChatService.h"
#include <iostream>
#include <signal.h>

// 注册一个信号捕捉。捕捉SIGINT 2号信号 Ctrl + C
void myhandler(int num)
{
    // 要把登录在线的用户全部置为offline
    ChatService::getInstance()->serverCloseException();
    // 退出整个进程
    exit(-1);
}
bool addSignal(int signum)
{
    // 创建sigaction结构体，并赋于相应的属性
    struct sigaction act = {0};
    act.sa_flags = 0; // 使用默认的函数指针执行动作函数
    act.sa_handler = myhandler;
    sigemptyset(&act.sa_mask);
    // sigaddset(&act.sa_mask, signum);
    // 调用sigaction函数
    return sigaction(signum, &act, NULL);
}


int main(int argc, char const *argv[])
{
    // 注册一个信号捕捉。捕捉SIGINT 2号信号 Ctrl + C
    if(argc < 2)
    {
        std::cerr << "start chatServer LIKE: ./server 6000" << std::endl;
        exit(-1);
    }
    addSignal(SIGINT);
    Tiny_muduo::net::EventLoop lp;
    int port = atoi(argv[1]);
    Tiny_muduo::net::InetAddress addr(static_cast<uint16_t>(port));
    ChatServer server(&lp, addr, "chatserver");
    server.setThreadNum(2);
    server.start();
    lp.loop();
    return 0;
}
