#pragma once
#include<string>
#include<map>
#include"Buffer.h"
#include<functional>
using namespace std;

//定义状态码枚举
enum class StatusCode {
	Unknow,
	OK = 200,
	MovedPermanently = 301,
	MovedTemporarily = 302,
	BadRequest = 400,
	NotFound = 404
};

class HttpResponse{
public:
    HttpResponse();
    ~HttpResponse();

    // 添加响应头
    void addHeader(const string key,const string value);
    // 组织http响应
    void prepareMag(Buffer* sendBuf,int socketfd);
    // 发送静态资源 -- 文件/目录 
    function<void(const string,Buffer*,int)> sendFunc;

    // set方法
    inline void setfileName(string name){m_fileName = name;}
    inline void setstatusCode(StatusCode code){m_statusCode = code;}

private:
    // 状态行：协议版本 状态码 状态描述
    StatusCode m_statusCode;
    string m_fileName;
    // 响应头键值对
    map<string,string> m_headers;
    // 状态码和状态描述映射关系
    const map<int,string> m_info = {
        {200,"OK"},
        {301,"MovedPermanently"},
        {302,"MovedTemporarily"},
        {400,"BadRequest"},
        {404,"NotFound"}
    };
};