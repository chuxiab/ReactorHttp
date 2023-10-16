#pragma once
#include <stdbool.h>
#include "Dispatcher.h"
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <map>

using namespace std;

// 处理该节点中的channel的方式
enum class ElemType : char{ ADD, DELETE, MODIFY };
// 定义任务队列的节点
struct ChannelElement
{
    ElemType type;   // 如何处理该节点中的channel
    Channel* channel;
};

class Dispatcher;

class EventLoop
{
public:
    EventLoop();
    EventLoop(const string threadName);
    ~EventLoop();

    //启动反应堆
    int run();
    // 处理别激活的文件fd
    int eventActivate(int fd, int event);
    // 添加任务到任务队列
    int addTask(Channel* channel, ElemType type);
    // 处理任务队列中的任务
    int eventLoopProcessTask();
    // 处理dispatcher中的节点
    int add(Channel* channel);
    int remove(Channel* channel);
    int modify(Channel* channel);
    int freeChannel(Channel* channel);
    int readMessage();
    static int readLocalMessage(void* arg);

    inline thread::id getThreadID()
    {
        return m_threadID;
    }

    inline string getThreadName() 
    {
        return m_threadName;
    }

private:
    void taskWakeup();

private:
    bool m_isQuit;
    Dispatcher* m_dispatcher;
    // 任务队列
    queue<ChannelElement*> m_taskQ;
    // map
    map<int, Channel*> m_channelMap;
    // 线程id, name, mutex
    thread::id m_threadID;
    string m_threadName;
    mutex m_mutex;
    int m_socketPair[2];  // 存储本地通信的fd 通过socketpair 初始化

};

