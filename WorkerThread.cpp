#include"WorkerThread.h"
#include<iostream>

WorkerThread::WorkerThread(int index){           
    m_thread = nullptr;         
    // m_threadID = this_thread::get_id();
    m_threadID = thread::id();
    m_name = "SubThread-" + to_string(index);
    m_evLoop = nullptr;      
}

// 其实子线程会一直存活到程序退出
WorkerThread::~WorkerThread(){
    if(m_thread != nullptr){
        delete m_thread;
    }
}

void WorkerThread::run(){
    m_thread = new thread(&WorkerThread::running,this);
    // 使用条件变量,确保子线程m_evLoop被创建出
    unique_lock<mutex> locker(m_mutex);
    while(m_evLoop == nullptr){
        m_conn.wait(locker);
    }
}

void WorkerThread::running(){
    // 互斥锁保护公共资源 m_evLoop
    m_mutex.lock();
    m_evLoop = new EventLoop(m_name);
    m_mutex.unlock();
    // 唤醒被条件变量阻塞的线程
    m_conn.notify_one();
    m_evLoop->run();
}  