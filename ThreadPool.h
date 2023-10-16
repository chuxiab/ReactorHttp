#pragma once
#include <vector>
#include "WorkerThread.h"
#include "EventLoop.h"

using namespace std;

class ThreadPool
{
public:
	ThreadPool(EventLoop* mainLoop, int count);
	~ThreadPool();
	void run();
	EventLoop* takeWorkerEventLoop();

private:
	//主线程的反应堆模型
	EventLoop* m_mainLoop;
	bool m_isStart;
	int m_threadNum;
	vector<WorkerThread*> m_workerThreads;
	int m_index;
};

