#include"Channel.h"

Channel::Channel(int fd,FDEvent events,handFunc readFunc,handFunc writeFunc,handFunc destoryFunc,void* arg){
    m_fd = fd;
    m_events = static_cast<int>(events);
    m_arg = arg;
    readCallback = readFunc;
    writeCallback = writeFunc;
    deatoryCallback = destoryFunc;
}



