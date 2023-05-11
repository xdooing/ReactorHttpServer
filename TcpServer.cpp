#include<arpa/inet.h>
#include<string.h>
#include"TcpServer.h"
#include"Channel.h"
#include"TcpConnection.h"
#include<iostream>


 TcpServer::TcpServer(unsigned short port,int threadNum){
    m_threadNum = threadNum;
    m_mainLoop = new EventLoop;
    m_threadPool = new ThreadPool(m_mainLoop,threadNum);
    m_port = port;
    setListen();
 }

TcpServer::~TcpServer(){

}

int TcpServer::acceptConnection(void* arg){
    TcpServer* server = static_cast<TcpServer*>(arg);
    int cfd = accept(server->m_lfd,NULL,NULL);
    // 将任务交给反应堆处理
    EventLoop* evLoop = server->m_threadPool->getEvLoop();
    // 拿到evLoop和cfd之后要交给TcpConnection结构体处理
    new TcpConnection(cfd,evLoop);
    return 0;
}

void TcpServer::setListen(){
    // 创建监听套接字
    m_lfd = socket(AF_INET,SOCK_STREAM,0);
    if(m_lfd == -1){
        perror("socket error");
        exit(-1);
    }
    // 设置端口复用
    int opt = 1;
    int ret = setsockopt(m_lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof opt);
    if(ret == -1){
        perror("setsockopt error");
        exit(-1);
    }
    // 绑定
    struct sockaddr_in addr;
    memset(&addr,0,sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(m_lfd,(struct sockaddr*)&addr,sizeof addr);
    if(ret == -1){
        perror("bind error");
        exit(-1);
    }
    // 设置监听
    ret = listen(m_lfd,128);
    if(ret == -1){
        perror("listen error");
        exit(-1);
    }
}

void TcpServer::run(){
    // 启动线程池
    m_threadPool->run();

    // 监听Channel实例
    auto obj = bind(&TcpServer::acceptConnection,this,placeholders::_1);
    Channel* channel = new Channel(m_lfd,FDEvent::ReadEvent,obj,nullptr,nullptr,this);
    m_mainLoop->addTask(channel,ElemType::ADD);
    // 启动主反应堆
    m_mainLoop->run();
}