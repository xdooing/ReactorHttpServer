#if 1

#pragma once
#include<string>
#include<map>
#include<functional>
#include"Buffer.h"
#include"HttpResponse.h"
using namespace std;

enum class ProcessStatus : char{
    ParseReqLine,
    ParseReqHeader,
    ParseReqBody,
    ParseReqDone
};

class HttpRequest{
public:
    HttpRequest();
    ~HttpRequest();
    
    // 将请求头键值对加到map中
    void addHeader(const string key,const string value);
    // 根据key得到value
    string getValue(const string key);
    // 解析请求行
    bool parseReqLine(Buffer* readBuf);
    // 解析请求头
    bool parseReqHeader(Buffer* readBuf);
    // 解析流程控制函数
    bool parseProcess(Buffer* readBuf,HttpResponse* response,Buffer* sendBuf,int socketfd);
    // 准备回复数据,封装到response对象中
    bool processHttpReq(HttpResponse* response);

    // 解码字符串
	string decodeMsg(string from);
    // 将字符转换为整形数
    int hexToDec(char c);
    // 获取文件类型
	const string getFileType(const string name);

    // 发送文件/目录函数 function<void(const string,Buffer*,int)> sendFunc;
    static void sendDir(string dirName,Buffer* sendBuf,int socketfd);
    static void sendFile(string fileName,Buffer* sendBuf,int socketfd);

    // function回调函数
    inline void setMethod(string method){m_method = method;}
    inline void setUrl(string url){m_url = url;}
    inline void setVersion(string version){m_version = version;}

    //状态处理函数
    inline ProcessStatus getStatus(){return m_status;}
    inline void setStatus(ProcessStatus status){m_status = status;}

private:
    // 切割请求行
    char* splitReqLine(const char* begin,const char* end,const char* sub,function<void(string)>callback);


private:
    string m_method;
    string m_url;
    string m_version;
    map<string,string> m_reqHeaders;
    ProcessStatus m_status;
};

#endif

