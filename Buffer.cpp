#include"Buffer.h"
#include<stdlib.h>
#include<string.h>
#include<sys/uio.h>
#include<sys/socket.h>
#include<unistd.h>

Buffer::Buffer(int size):m_capaticy(size){
    // 后面需要扩容，因此使用malloc
    m_data = (char*)malloc(size);
    memset(m_data,0,size);
}
Buffer::~Buffer(){
    if(m_data != nullptr){
        free(m_data);
    }
}

void Buffer::extendSize(int size){
    if(writeableSize() >= size){
        return;
    }
    else if(writeableSize() + m_readPos >= size){
        // 合并内存
        int readable = readableSize();
        memcpy(m_data, m_data + m_readPos, readable);
        m_readPos = 0;
        m_writePos = readable;
    }
    else{
        // 这种情况下才应该扩容
        void* temp = realloc(m_data,m_capaticy + size);
        if(temp == nullptr){
            return;
        }
        // memset(m_data + m_capaticy,0,size);
        memset((char*)temp + m_capaticy,0,size);
        m_data = (char*)temp;
        m_capaticy += size;
    }
}

// 不确定传入的字符串是不是以"\0"
int Buffer::appendString(const char* data,int size){
    if(data == nullptr || size <= 0){
        return -1;
    }
    // 尝试扩容--内部有判断
    extendSize(size);
    memcpy(m_data + m_writePos,data,size);
    m_writePos += size;
    return 0;
}

// 传入的字符串以"\0"结束
int Buffer::appendString(const char* data){
    int size = strlen(data);
    int ret = appendString(data,size);
    return ret;
}

char* Buffer::findCRLF(){
    char* ptr = (char*)memmem(m_data + m_readPos,readableSize(),"\r\n",2);
    return ptr;
}

int Buffer::recvData(int socketfd){
    // 指定多个缓冲区
    struct iovec vec[2];
    int writeable = writeableSize();
    vec[0].iov_base = m_data + m_writePos;
    vec[0].iov_len = writeable;

    char* tmpbuf = (char*)malloc(40960); 

    vec[1].iov_base = m_data + m_writePos;
    vec[1].iov_len = 40960;
    int result = readv(socketfd,vec,2);
    if(result == -1){
        return -1;
    }
    else if(result <= writeable){
        m_writePos += result;
    }
    else{
        m_writePos = m_capaticy;
        // appendString((char*)vec[1].iov_base,result - m_writePos);
        appendString(tmpbuf,result - writeable);
    }
    free(tmpbuf);
    return result;
}


int Buffer::sendData(int socketfd){
    int readable = readableSize();
    if(readable > 0){
        int count = send(socketfd,m_data + m_readPos,readable,MSG_NOSIGNAL);
        // int count = send(socketfd,m_data + m_readPos,readable,0);
        if(count > 0){
            m_readPos += count;
            usleep(10);
        }
        return count;
    }
    return 0;
}