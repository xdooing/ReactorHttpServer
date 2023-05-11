#pragma once
#include"EventLoop.h"
#include"ThreadPool.h"



class TcpServer{
public:
    TcpServer(unsigned short port,int threadNum);
    ~TcpServer();
    // 初始化监听
    void setListen();
    // 启动反应堆
    void run(); 
    int acceptConnection(void* arg);
private:
    int m_threadNum;
    EventLoop* m_mainLoop;
    ThreadPool* m_threadPool;
    int m_lfd;
    unsigned short m_port;
};