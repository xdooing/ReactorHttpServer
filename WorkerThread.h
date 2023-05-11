#pragma once
#include<thread>
#include<mutex>
#include<condition_variable>
#include"EventLoop.h"
using namespace std;



class WorkerThread{
public:
    WorkerThread(int index);
    ~WorkerThread();
    void run();                  // 启动线程
    inline EventLoop* getEvLoop(){return m_evLoop;}
private:
    void running();             // 子线程回调函数
private:
    mutex m_mutex;              // 互斥锁
    condition_variable m_conn;  // 条件变量
    thread* m_thread;           // 线程指针
    thread::id m_threadID;
    string m_name;
    EventLoop* m_evLoop;          // 子线程中的反应堆
};