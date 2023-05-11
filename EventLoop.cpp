#include<sys/socket.h>
#include<unistd.h>
#include<assert.h>
#include<string.h>
#include"EventLoop.h"
#include"EpollDispatcher.h"
#include"PollDispatcher.h"
#include"SelectDispatcher.h"



EventLoop::EventLoop():EventLoop(string()){

}

EventLoop::EventLoop(const string threadname){
    // m_dispatcher = new EpollDispatcher(this);
    // m_dispatcher = new PollDispatcher(this);
    m_dispatcher = new SelectDispatcher(this);

    m_isQuit = true;
    m_channelMap.clear();
    m_threadID = this_thread::get_id();           
    m_threadName = threadname == string() ? "mainThread" : threadname;    
    int ret = socketpair(AF_UNIX,SOCK_STREAM,0,m_socketPair);
    if(ret == -1){
        perror("socketpair error");
        exit(-1);
    }

    // m_socketPair[0] 发送数据  m_socketPair[1] 接收数据  
    auto obj = bind(&EventLoop::readMessage,this);
    Channel* channel = new Channel(m_socketPair[1],FDEvent::ReadEvent,obj,nullptr,nullptr,this);
    addTask(channel,ElemType::ADD);
}

EventLoop::~EventLoop(){
    
}

int EventLoop::run(){
    m_isQuit = false;
    if(m_threadID != this_thread::get_id()){
        return -1;
    }
    while(!m_isQuit){  
        m_dispatcher->dispatch();
        processTaskQ();
    }
    return 0;
}

int EventLoop::eventActive(int fd,int event){
    if(fd < 0){
        return -1;
    }
    Channel* channel = m_channelMap[fd];
    assert(channel->get_fd() == fd);
    if(event & (int)FDEvent::ReadEvent && channel->readCallback){
        channel->readCallback(const_cast<void*>(channel->get_arg()));
    }
    if(event & (int)FDEvent::WriteEvent && channel->writeCallback){
        channel->writeCallback(const_cast<void*>(channel->get_arg()));
    }
    return 0;
}

int EventLoop::addTask(Channel* channel,ElemType type){
    // 互斥锁保护队列
    m_mutex.lock();
    ChannelElem* node = new ChannelElem;
    node->channel = channel;
    node->type = type;
    m_taskQ.push(node);
    m_mutex.unlock();
    
    // 处理结点--子线程处理/主线程处理
    if(m_threadID == this_thread::get_id()){
        processTaskQ();
    }
    else{
        // 若为主线程，则通知子线程，解除子线程阻塞
        wakeUp();
    }
    return 0;  
}

int EventLoop::processTaskQ(){
    while(!m_taskQ.empty()){
        m_mutex.lock();
        ChannelElem* node = m_taskQ.front();
        m_taskQ.pop();
        m_mutex.unlock();
        Channel* channel = node->channel;
        if(node->type == ElemType::ADD){
            add(channel);
        }
        else if(node->type == ElemType::DELETE){
            remove(channel);
        }
        else if(node->type == ElemType::MODIFY){
            modify(channel);
        }
        delete node;
    }
    return 0;
}

int EventLoop::add(Channel* channel){
    int fd = channel->get_fd();
    // 存储到map
    auto it = m_channelMap.find(fd);
    if(it == m_channelMap.end()){
        m_channelMap.insert(make_pair(fd,channel));
        m_dispatcher->setChannel(channel);
        // 加入检测集合
        int ret = m_dispatcher->add();
        return ret;
    }
    return 0;
}

int EventLoop::remove(Channel* channel){
    int fd = channel->get_fd();
    auto it = m_channelMap.find(fd);
    if(it == m_channelMap.end()){
        return -1;
    }
    m_dispatcher->setChannel(channel);
    int ret = m_dispatcher->remove();
    return ret;
}

int EventLoop::modify(Channel* channel){
    int fd = channel->get_fd();
    auto it = m_channelMap.find(fd);
    if(it == m_channelMap.end()){
        return -1;
    }
    m_dispatcher->setChannel(channel);
    int ret = m_dispatcher->modify();
    return ret;
}

int EventLoop::freeChannel(Channel* channel){
    auto it = m_channelMap.find(channel->get_fd());
    if(it != m_channelMap.end()){
        m_channelMap.erase(it);
        close(channel->get_fd());
        delete channel;
    }
    return 0;
}

int EventLoop::readMessage(){
    char buf[256];
    read(m_socketPair[1],buf,sizeof buf);
    return 0;
}

void EventLoop::wakeUp(){
    const char* msg = "上班了！";
    write(m_socketPair[0],msg,strlen(msg));
}