#pragma once
#include"Buffer.h"
#include"EventLoop.h"
#include"Channel.h"
#include"HttpRequest.h"
#include"HttpResponse.h"
#include<string>


class TcpConnection{
public:
    TcpConnection(int fd,EventLoop* evLoop);
    ~TcpConnection();

    // 回调函数
    static int readCallBack(void* arg);
    static int destoryCallback(void* arg);


private:
    EventLoop* m_evLoop;
    Buffer* m_readBuf;
    Buffer* m_sendBuf;
    Channel* m_channel;
    HttpRequest* m_request;
    HttpResponse* m_response;
};
