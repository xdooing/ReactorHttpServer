#if 1

#pragma once

class Buffer{
public:
    Buffer(int size);
    ~Buffer();

    // 扩容
    void extendSize(int size);
    // 得到剩余可读可写容量
    inline int readableSize(){return m_writePos - m_readPos;}
    inline int writeableSize(){return m_capaticy - m_writePos;}
    // 写内存(重载函数，避免二进制数据中包含'\0')
    int appendString(const char* data,int size);
    int appendString(const char* data);
    // 返回\r\n的位置，便于后面分析
    char* findCRLF();

    // 接收数据
    int recvData(int socketfd);
    // 发送数据
    int sendData(int socketfd);

    // 得到读数据的起始位置
    inline char* beginPos(){return m_data + m_readPos;}
    // 更新readPos,跳过已读，跳过"\r\n"
    inline int readPosIncrease(int count){m_readPos += count;return m_readPos;}


private:   
    char* m_data;
    int m_capaticy;
    int m_readPos = 0;
    int m_writePos = 0;
};

#endif
