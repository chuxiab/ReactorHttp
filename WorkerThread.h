#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include "EventLoop.h"

using namespace std;

//定义子线程对应的结构体
class WorkerThread
{
public:
	WorkerThread(int index);
	~WorkerThread();
	void run();

	inline EventLoop* getEvLoop()
	{
		return m_evLoop;
	}
private:
	void running();

private:
	thread* m_thread;
	thread::id m_threadID;
	string m_name;
	mutex m_mutex;
	condition_variable m_cond;
	EventLoop* m_evLoop;
};
