#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"
#include "Channel.h"

class TcpServer
{
public:
	TcpServer(unsigned short port, int m_threadNum);
	~TcpServer();

	void setListen();
	void run();
	
	static int acceptConnection(void* arg);
private:
	unsigned short m_port;
	int m_lfd;
	int m_threadNum;
	EventLoop* m_mainLoop;
	ThreadPool* m_threadPool;
};
