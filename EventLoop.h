#pragma once
#include<thread>
#include<queue>
#include<map>
#include<mutex>
#include"Dispatcher.h"
#include"Channel.h"

class Dispatcher;

enum class ElemType :char {ADD,DELETE,MODIFY};

// 任务队列节点
struct ChannelElem{
    Channel* channel;
    ElemType type;
};


class EventLoop{
public:
    EventLoop();
    EventLoop(const string threadname);
    ~EventLoop();

    // 启动反应堆
    int run();
    // 处理激活的文件描述符
    int eventActive(int fd,int event);
    // 添加节点
    int addTask(Channel* channel,ElemType type);
    // 处理任务队列中的任务
    int processTaskQ();
    // 处理结点
    int add(Channel* channel);
    int remove(Channel* channel);
    int modify(Channel* channel);
    // 释放节点中的channel
    int freeChannel(Channel* channel);
    // 唤醒阻塞
    int readMessage();
    // 返回线程id
    inline thread::id getThreadID(){ return m_threadID;}

private:
    void wakeUp();
private:
    bool m_isQuit;
    Dispatcher* m_dispatcher;        // 父类指针
    queue<ChannelElem*> m_taskQ;     // 任务队列
    map<int,Channel*> m_channelMap;  // 映射关系
    thread::id m_threadID;           // 反应堆所属线程的ID
    string m_threadName;
    mutex m_mutex;                   // 互斥锁
    int m_socketPair[2];             // 本地通信fd 唤醒阻塞
};