#include<stdio.h>
#include<unistd.h>
#include"EpollDispatcher.h"
#include"Dispatcher.h"
#include<iostream>

// 委托构造函数--将evLoop保存到父类中供子类使用
EpollDispatcher::EpollDispatcher(EventLoop*evLoop):Dispatcher(evLoop){
    m_epfd = epoll_create(1);
    if(m_epfd == -1){
        perror("epoll_create error");
        exit(-1);
    }
    m_events = new struct epoll_event[m_maxNode];
}

EpollDispatcher::~EpollDispatcher(){
    close(m_epfd);
    delete[] m_events;
}

int EpollDispatcher::epollctl(int op){
    struct epoll_event ev;
    ev.data.fd = m_channel->get_fd();
    int events = 0;
    // 判断是否有读写事件
    if(m_channel->get_events() & (int)FDEvent::ReadEvent){
        events |= EPOLLIN;
    }
    if(m_channel->get_events() & (int)FDEvent::WriteEvent){
        events |= EPOLLOUT;
    }
    ev.events = events;
    int ret = epoll_ctl(m_epfd,op,m_channel->get_fd(),&ev);
    return ret;
}


int EpollDispatcher::add(){
    int ret = epollctl(EPOLL_CTL_ADD);
    if(ret == -1){
        perror("epoll_ctl_add error");
        exit(-1);
    }
    return ret;
}

int EpollDispatcher::remove(){
    int ret = epollctl(EPOLL_CTL_DEL);
    if(ret == -1){
        perror("epoll_ctl_del error");
        exit(-1);
    }
    m_channel->deatoryCallback(const_cast<void*>(m_channel->get_arg()));
    return ret;
}

int EpollDispatcher::modify(){
    int ret = epollctl(EPOLL_CTL_MOD);
    if(ret == -1){
        perror("epoll_ctl_mod error");
        exit(-1);
    }
    return ret;
}

int EpollDispatcher::dispatch(int timeout){
    int count = epoll_wait(m_epfd,m_events,m_maxNode,timeout*1000);

    // cout<< "count = "<< count <<endl;

    for(int i = 0;i < count ;++i){
        int events = m_events[i].events;
        int cfd = m_events[i].data.fd;
        if(events & EPOLLERR || events & EPOLLHUP){
            continue;
        }
        if(events & EPOLLIN){
            //反应堆处理事件
            m_evLoop->eventActive(cfd,(int)FDEvent::ReadEvent);
        }
        if(events & EPOLLOUT){
            //反应堆处理事件
            m_evLoop->eventActive(cfd,(int)FDEvent::WriteEvent);
        }
    }
    return 0;
}