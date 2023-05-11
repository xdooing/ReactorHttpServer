#include<assert.h>
#include"ThreadPool.h"

ThreadPool::ThreadPool(EventLoop* mainLoop,int num){
    m_mainLoop = mainLoop;
    m_workThread.clear();
    m_isStart = false;
    m_threadNum = num;
    m_index = 0;
}

ThreadPool::~ThreadPool(){
    for(auto item : m_workThread){
        delete item;
    }
}

void ThreadPool::run(){
    assert(!m_isStart);
    if(m_mainLoop->getThreadID() != this_thread::get_id()){
        exit(-1);
    }
    m_isStart = true;
    if(m_threadNum > 0){
        for(int i = 0; i < m_threadNum; ++i){
            WorkerThread* subThread = new WorkerThread(i);
            subThread->run();
            m_workThread.push_back(subThread);
        }
    }
}

EventLoop* ThreadPool::getEvLoop(){
    assert(m_isStart);  // 确保线程池是运行的
    if(m_mainLoop->getThreadID() != this_thread::get_id()){
        exit(-1);
    }
    EventLoop* evLoop = m_mainLoop;
    if(m_threadNum > 0){
        evLoop = m_workThread[m_index]->getEvLoop();
        m_index = ++m_index % m_threadNum;
    }
    return evLoop;
}