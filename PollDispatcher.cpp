#include"Dispatcher.h"
#include<poll.h>
#include<stdlib.h>
#include<stdio.h>
#include "PollDispatcher.h"


PollDispatcher::PollDispatcher(EventLoop* evLoop) :Dispatcher(evLoop)
{
	m_maxfd = 0;
	m_fds = new struct pollfd[m_maxNode];
	for (int i = 0; i < m_maxNode; ++i) {
		m_fds[i].fd = -1;
		m_fds[i].events = 0;
		m_fds[i].revents = 0;
	}
}

PollDispatcher::~PollDispatcher()
{
	delete[] m_fds;
}

int PollDispatcher::add()
{
	int events = 0;
	if (m_channel->get_events() & (int)FDEvent::ReadEvent) {
		events |= POLLIN;
	}
	if (m_channel->get_events() & (int)FDEvent::WriteEvent) {
		events |= POLLOUT;
	}
	int i = 0;
	for (; i < m_maxNode; ++i) {
		if (m_fds[i].fd == -1) {
			//说明是空闲的，没有被占用
			m_fds[i].events = events;
			m_fds[i].fd = m_channel->get_fd();
			m_maxfd = i > m_maxfd ? i : m_maxfd;
			break;
		}
	}
	if (i >= m_maxNode) {
		return -1;
	}
	return 0;
}

int PollDispatcher::remove()
{
	int i = 0;
	for (; i < m_maxNode; ++i) {
		if (m_fds[i].fd == m_channel->get_fd()) {
			m_fds[i].events = 0;
			m_fds[i].revents = 0;// 把这个也重置一下
			m_fds[i].fd = -1;
			break;
		}
	}
	// 通过 channel 释放 TcpConnection 资源(方式是回调函数)
	m_channel->deatoryCallback(const_cast<void*>(m_channel->get_arg()));

	if (i >= m_maxNode) {
		return -1;
	}
	return 0;
}

int PollDispatcher::modify()
{
	int events = 0;
	if (m_channel->get_events() & (int)FDEvent::ReadEvent) {
		events |= POLLIN;
	}
	if (m_channel->get_events() & (int)FDEvent::WriteEvent) {
		events |= POLLOUT;
	}
	int i = 0;
	for (; i < m_maxNode; ++i) {
		if (m_fds[i].fd == m_channel->get_fd()) {
			m_fds[i].events = events;
			break;
		}
	}
	if (i >= m_maxNode) {
		return -1;
	}
	return 0;
}

int PollDispatcher::dispatch(int timeout)
{
	int count = poll(m_fds, m_maxfd + 1, timeout * 1000);
	if (count == -1) {
		perror("poll error");
		exit(-1);
	}
	// 我们并不知道数组中哪一个文件描述符被激活，因此需要依次判断
	for (int i = 0; i <= m_maxfd; ++i) {
		if (m_fds[i].fd == -1) {
			continue;
		}
		if (m_fds[i].revents & POLLIN) {
			m_evLoop->eventActive(m_fds[i].fd, (int)FDEvent::ReadEvent);
		}
		if (m_fds[i].revents & POLLOUT) {
			m_evLoop->eventActive(m_fds[i].fd, (int)FDEvent::WriteEvent);
		}
	}
	return 0;
}
