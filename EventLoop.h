#pragma once
#include <stdbool.h>
#include "Dispatcher.h"
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <map>

using namespace std;

// ����ýڵ��е�channel�ķ�ʽ
enum class ElemType : char{ ADD, DELETE, MODIFY };
// ����������еĽڵ�
struct ChannelElement
{
    ElemType type;   // ��δ���ýڵ��е�channel
    Channel* channel;
};

class Dispatcher;

class EventLoop
{
public:
    EventLoop();
    EventLoop(const string threadName);
    ~EventLoop();

    //������Ӧ��
    int run();
    // ����𼤻���ļ�fd
    int eventActivate(int fd, int event);
    // ��������������
    int addTask(Channel* channel, ElemType type);
    // ������������е�����
    int eventLoopProcessTask();
    // ����dispatcher�еĽڵ�
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
    // �������
    queue<ChannelElement*> m_taskQ;
    // map
    map<int, Channel*> m_channelMap;
    // �߳�id, name, mutex
    thread::id m_threadID;
    string m_threadName;
    mutex m_mutex;
    int m_socketPair[2];  // �洢����ͨ�ŵ�fd ͨ��socketpair ��ʼ��

};

