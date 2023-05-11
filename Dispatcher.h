#pragma once
#include "Channel.h"
#include"EventLoop.h"

class EventLoop;

class Dispatcher{
public:
    Dispatcher(EventLoop* evLoop);
    virtual ~Dispatcher();

    // 处理方式
    virtual int add();
    virtual int remove();
    virtual int modify();
    virtual int dispatch(int timeout = 2); // 事件检测

    // 指定channel 子类会使用到
    inline void setChannel(Channel* channel) {m_channel = channel;}

protected:
    Channel* m_channel;
    EventLoop* m_evLoop;
};