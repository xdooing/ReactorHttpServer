#pragma once
#include<functional>
using namespace std;

//枚举：定义读写事件
enum class FDEvent{
    ReadEvent = 0x02, // 二进制10
    WriteEvent = 0x04 // 二进制100
};


class Channel{
public:
    // 回调函数
    using handFunc = function<int(void*)>;
    Channel(int fd,FDEvent events,handFunc readFunc,handFunc writeFunc,handFunc destoryFunc,void* arg);

    handFunc readCallback;
    handFunc writeCallback;
    handFunc deatoryCallback;

    inline int get_fd(){return m_fd;}
    inline int get_events(){return m_events;}
    inline const void*get_arg(){return m_arg;}

private:
    int m_fd;
    int m_events;
    void* m_arg;
};



