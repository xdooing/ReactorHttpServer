#include"TcpConnection.h"

TcpConnection::TcpConnection(int fd,EventLoop* evLoop){
    m_evLoop = evLoop;
    m_readBuf = new Buffer(10240);
    m_sendBuf = new Buffer(10240);
    m_request = new HttpRequest;
    m_response = new HttpResponse;
    m_channel = new Channel(fd,FDEvent::ReadEvent,readCallBack,nullptr,destoryCallback,this);

    // 将传入的fd封装到Channel，添加到队列中
    evLoop->addTask(m_channel,ElemType::ADD);
}

TcpConnection::~TcpConnection(){
    if(m_readBuf && m_readBuf->readableSize() == 0 && m_sendBuf && m_sendBuf->readableSize() == 0){
        delete m_readBuf;
        delete m_sendBuf;
        delete m_request;
        delete m_response;
        m_evLoop->freeChannel(m_channel);
    }
}

int TcpConnection::readCallBack(void* arg){
    TcpConnection* conn = (TcpConnection*)arg;
    int socket = conn->m_channel->get_fd();
    int count = conn->m_readBuf->recvData(socket);

    if(count > 0){
        // 接收到http请求，接下来需要解析与发送响应
        bool flag = conn->m_request->parseProcess(conn->m_readBuf,conn->m_response,conn->m_sendBuf,socket);
        if(!flag){
            // 解析失败
            string errMsg = "Http/1.1 400 Bad Request\r\n\r\n";
            conn->m_sendBuf->appendString(errMsg.c_str());
        }
    }
    conn->m_evLoop->addTask(conn->m_channel,ElemType::DELETE);
    return 0;
}

int TcpConnection::destoryCallback(void* arg){
    TcpConnection* conn = (TcpConnection*)arg;
    if(conn != nullptr){
        delete conn;
    }
    return 0;
}