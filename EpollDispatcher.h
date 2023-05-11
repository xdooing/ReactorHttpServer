#pragma once
#include<sys/epoll.h>
#include"Dispatcher.h"

class EpollDispatcher:public Dispatcher{
public:
    EpollDispatcher(EventLoop*evLoop);
    ~EpollDispatcher();

     // 处理方式
    int add()override;
    int remove()override;
    int modify()override;
    int dispatch(int timeout = 2) override;
private:
    int epollctl(int op);
private:
    int m_epfd;
    struct epoll_event* m_events;
    const int m_maxNode = 520;
};

