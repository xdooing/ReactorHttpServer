#include"HttpResponse.h"

HttpResponse::HttpResponse(){
    m_statusCode = StatusCode::Unknow;
    m_fileName = string();
    m_headers.clear();
    sendFunc = nullptr;
}

HttpResponse::~HttpResponse(){

}

// 添加响应头
void HttpResponse::addHeader(const string key,const string value){
    if(key.empty() || value.empty()){
        return;
    }
    m_headers.insert(make_pair(key,value));
}

// 组织http响应
void HttpResponse::prepareMag(Buffer* sendBuf,int socketfd){
    // 状态行： HTTP/1.1 200 Ok
    char temp[1024] = {0};
    int code = (int)m_statusCode;
    sprintf(temp,"HTTP/1.1 %d %s\r\n",code,m_info.at(code).c_str());
    sendBuf->appendString(temp);

    // 响应头
    for(auto it = m_headers.begin();it != m_headers.end();++it){
        sprintf(temp,"%s: %s\r\n",it->first.c_str(),it->second.c_str());
        sendBuf->appendString(temp);
    }

    // 空行
    sendBuf->appendString("\r\n");

    // 数据组织好之后就立即发送出去
    sendBuf->sendData(socketfd);

    // 发送要回复的静态资源
    sendFunc(m_fileName,sendBuf,socketfd);
}