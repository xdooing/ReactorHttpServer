#include"HttpRequest.h"
#include<string.h>
#include<assert.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<dirent.h>
#include<fcntl.h>
#include<unistd.h>



HttpRequest::HttpRequest(){
    m_method = m_url = m_version = string();
    m_reqHeaders.clear();
    m_status = ProcessStatus::ParseReqLine;
}

HttpRequest::~HttpRequest(){

}

// 将请求头键值对加到map中
void HttpRequest::addHeader(const string key,const string value){
    if(key.empty() || value.empty()){
        return;
    }
    m_reqHeaders.insert(make_pair(key,value));
}

// 根据key得到value  -- 避免key插入数据
string HttpRequest::getValue(const string key){
    auto it = m_reqHeaders.find(key);
    if(it == m_reqHeaders.end()){
        return string();
    }
    return m_reqHeaders[key];
}

// 切割请求行
char* HttpRequest::splitReqLine(const char* begin,const char* end,const char* sub,function<void(string)>callback){
    char* temp = const_cast<char*>(end);
    if(sub != nullptr){
        temp = (char*)memmem(begin,end - begin,sub,strlen(sub));
        assert(temp != nullptr);
    }
    int length = temp - begin;
    callback(string(begin,length));
    return temp + 1;
}

// 解析请求行
bool HttpRequest::parseReqLine(Buffer* readBuf){
    char* end = readBuf->findCRLF();
    char* begin = readBuf->beginPos();
    // 请求行长度
    int length = end - begin;
    if(length > 0){
        auto methodFunc = bind(&HttpRequest::setMethod,this,placeholders::_1);
        begin = splitReqLine(begin,end," ",methodFunc);

        auto urlFunc = bind(&HttpRequest::setUrl,this,placeholders::_1);
        begin = splitReqLine(begin,end," ",urlFunc);

        auto versionFunc = bind(&HttpRequest::setVersion,this,placeholders::_1);
        splitReqLine(begin,end,NULL,versionFunc);

        // 准备解析请求头  "\r\n"
        readBuf->readPosIncrease(length + 2);
        setStatus(ProcessStatus::ParseReqHeader);
        return true;
    }
    return false;
}

// 解析请求头 -- 这个函数解析其中一行
bool HttpRequest::parseReqHeader(Buffer* readBuf){
    char* end = readBuf->findCRLF();
    if(end != nullptr){
        char* begin = readBuf->beginPos();
        int length = end - begin;
        char* mid = (char*)memmem(begin,length,": ",2);
        if(mid != nullptr){
            int keyLen = mid - begin;
            int valueLen = end - mid - 2;
            if(keyLen > 0 && valueLen > 0){
                string key(begin,keyLen);
                string value(mid+2,valueLen);
                // 添加到map中
                addHeader(key,value);
            }
            readBuf->readPosIncrease(length + 2);
        }
        else{
            readBuf->readPosIncrease(2);
            // setStatus(ProcessStatus::ParseReqBody); 会导致程序运行不下去
            setStatus(ProcessStatus::ParseReqDone);
        }
        return true;
    }
    return false;
}

// 解析流程控制函数
bool HttpRequest::parseProcess(Buffer* readBuf,HttpResponse* response,Buffer* sendBuf,int socketfd){
    bool flag = true;
    while(m_status != ProcessStatus::ParseReqDone){
        switch (m_status)
        {
        case ProcessStatus::ParseReqLine:
            flag = parseReqLine(readBuf);
            break;
        case ProcessStatus::ParseReqHeader:
            flag = parseReqHeader(readBuf);
            break;
        case ProcessStatus::ParseReqBody:
            break;
        default:
            break;
        }
        if(!flag){
            return flag;
        }

        // 若解析完成，则封装回复数据并发送
        if(m_status == ProcessStatus::ParseReqDone){
            // 准备回复数据
            processHttpReq(response);
            // 发送数据
            response->prepareMag(sendBuf,socketfd);
        }
    }
    m_status = ProcessStatus::ParseReqLine; // 复原状态
    return flag;
}

// 准备回复数据
bool HttpRequest::processHttpReq(HttpResponse* response){
    // 此项目只支持get方法
    if(strcasecmp(m_method.c_str(),"GET") != 0){
        return false;
    }
    m_url = decodeMsg(m_url);
    const char* file = NULL;
    if(strcmp(m_url.c_str(),"/") == 0){
        file = "./";
    }
    else{
        file = m_url.c_str() + 1;  // 去掉path中的 / ，变相对路径
    }

    // 获取文件属性
    struct stat st;
    int ret = stat(file,&st);
    if(ret == -1){  // 404
        // 状态行
        response->setfileName("404.html");
        response->setstatusCode(StatusCode::NotFound);
        // 响应头
        response->addHeader("Content-type",getFileType(".html"));
        response->sendFunc = sendFile;
        return false;
    }
    response->setfileName(file);
    response->setstatusCode(StatusCode::OK);

    // 判断文件类型
    if(S_ISDIR(st.st_mode)){
        // 目录文件
        response->addHeader("Content-type",getFileType(".html"));
        response->sendFunc = sendDir;
    }
    else{
        // 文件
        response->addHeader("Content-type",getFileType(file));
        response->addHeader("Content-length",to_string(st.st_size));
        response->sendFunc = sendFile;
    }
    return true;
}

const string HttpRequest::getFileType(const string name)
{
	//a.jpg  a.mp4  a.html
	//从右往左找'.'字符，如果不存在返回NULL，从右往左找的原因是，文件名中保不齐就有'.'
	const char* dot = strchr(name.data(), '.');
	if (dot == NULL)
		return "text/plain;charset=utf-8";//纯文本
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";
}

void HttpRequest::sendDir(const string dirName,Buffer* sendBuf,int socketfd){
    // 将读出来的目录发送给html网页
    char buf[4096] = { 0 };
	sprintf(buf, "<html><head><meta charset=\"UTF-8\"><title>%s</title></head><body><table>", dirName.data());
    struct dirent** namelist;
    int num = scandir(dirName.c_str(),&namelist,NULL,alphasort);
    for(int i = 0; i < num; i++){
        // 取出文件名
        char* name = namelist[i]->d_name;
        // 判断文件类型
        struct stat st;
        char subPath[1024] = {0};
        sprintf(subPath,"%s/%s",dirName.c_str(),name);
        stat(subPath,&st);
        if(S_ISDIR(st.st_mode)){
            // 将文件名字和大小通过行和列的方式拼在一起
            // a标签：<a href="">name</a>  用于跳转,我们要跳转到哪个目录的话，要在后面加一个/   \"%s/\"转义""
            sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
				name, name, st.st_size);
        }
        else{
            // 非目录操作
            sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
				name, name, st.st_size);
        }
        sendBuf->appendString(buf);
        sendBuf->sendData(socketfd);
        memset(buf,0,sizeof buf);
        free(namelist[i]); // 释放函数内部分配的内存
    }
    //将页面的结束标签拼接一下发送给浏览器
	sprintf(buf, "</table></body></html>");
	sendBuf->appendString(buf);
    sendBuf->sendData(socketfd);
    free(namelist);
}

void HttpRequest::sendFile(const string fileName,Buffer* sendBuf,int socketfd){
    // 打开文件，读一部分发一部分 TCP可靠连接
    int fd = open(fileName.c_str(),O_RDONLY);
    assert(fd > 0);
    while(true){
        char buf[1024] = {0};
        int len = read(fd,buf,sizeof buf);
        if(len > 0){
            sendBuf->appendString(buf,len);  // 这里不保证文件中是不是有'\0'
            sendBuf->sendData(socketfd);  
        }
        else if(len == 0){
            break;
        }
        else{
            perror("read error!");
        }
    }
    close(fd);
}


// 解码字符串
string HttpRequest::decodeMsg(string msg){
    string str = string();
	const char* from = msg.data();
	for (; *from != '\0'; ++from)
	{
		// isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
		// Linux%E5%86%85%E6%A0%B8.jpg
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
		{
			// 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
			// B2 == 178
			// 将3个字符, 变成了一个字符, 这个字符就是原始数据
			str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));
			// 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
			from += 2;
		}
		else
		{
			// 字符拷贝, 赋值
			str.append(1, *from);
		}
	}
	str.append(1, '\0');
	return str;
}

// 将字符转换为整形数
int HttpRequest::hexToDec(char c){
    if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0;
}