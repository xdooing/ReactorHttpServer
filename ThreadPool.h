#pragma once
#include<vector>
#include"EventLoop.h"
#include"WorkerThread.h"




class ThreadPool{
public:
    ThreadPool(EventLoop* mainLoop,int num);
    ~ThreadPool();
    void run();
    // 获取一个子线程的反应堆，处理具体任务的时候使用
    EventLoop* getEvLoop();
private:
    EventLoop* m_mainLoop;
    vector<WorkerThread*> m_workThread;
    bool m_isStart;
    int m_threadNum;
    int m_index;
};