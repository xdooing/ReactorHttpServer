#pragma once
#include"Channel.h"
#include"EventLoop.h"
#include"Dispatcher.h"
#include<sys/select.h>


class SelectDispatcher :public Dispatcher {
public:
	SelectDispatcher(EventLoop* evLoop);
	~SelectDispatcher();

	// 添加
	int add()override;
	// 删除
	int remove()override;
	// 修改
	int modify()override;
	// 事件检测
	int dispatch(int timeout = 2)override;

private:
	void setFdSet();
	void clearFdSet();
private:
	const int m_maxSize = 1024;
	fd_set m_readSet;
	fd_set m_writeSet;
};