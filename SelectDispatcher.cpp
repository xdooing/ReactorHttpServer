#include"Dispatcher.h"
#include<sys/select.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include "SelectDispatcher.h"


SelectDispatcher::SelectDispatcher(EventLoop* evLoop) :Dispatcher(evLoop)
{
	FD_ZERO(&m_readSet);
	FD_ZERO(&m_writeSet);
}

SelectDispatcher::~SelectDispatcher()
{
}

int SelectDispatcher::add()
{
	if (m_channel->get_fd() >= m_maxSize) {
		return -1;
	}
	setFdSet();
	return 0;
}

int SelectDispatcher::remove()
{
	clearFdSet();
	m_channel->deatoryCallback(const_cast<void*>(m_channel->get_arg()));
	return 0;
}

int SelectDispatcher::modify()
{
	setFdSet();
	clearFdSet();
	return 0;
}

int SelectDispatcher::dispatch(int timeout)
{
	struct timeval val;
	val.tv_sec = timeout;
	val.tv_usec = 0;  //微妙虽然用不到，但是还是需要初始化，因为最后在进行时间统计的时候，是二者加起来的

	//使用select之前，需要对原始数据进行备份
	fd_set rdtmp = m_readSet;
	fd_set wrtmp = m_writeSet;
	int count = select(m_maxSize, &rdtmp, &wrtmp, NULL, &val);
	if (count == -1) {
		perror("select error");
		exit(-1);
	}
	for (int i = 0; i < m_maxSize; ++i) {
		if (FD_ISSET(i, &rdtmp)) {
			m_evLoop->eventActive(i, (int)FDEvent::ReadEvent);
		}
		if (FD_ISSET(i, &wrtmp)) {
			m_evLoop->eventActive(i, (int)FDEvent::WriteEvent);
		}
	}

	return 0;
}

void SelectDispatcher::setFdSet()
{
	if (m_channel->get_events() & (int)FDEvent::ReadEvent) {
		FD_SET(m_channel->get_fd(), &m_readSet);
	}
	if (m_channel->get_events() & (int)FDEvent::WriteEvent) {
		FD_SET(m_channel->get_fd(), &m_writeSet);
	}
}

void SelectDispatcher::clearFdSet()
{
	if (m_channel->get_events() & (int)FDEvent::ReadEvent) {
		FD_CLR(m_channel->get_fd(), &m_readSet);
	}
	if (m_channel->get_events() & (int)FDEvent::WriteEvent) {
		FD_CLR(m_channel->get_fd(), &m_writeSet);
	}
}
