#include "EventLoop.h"
#include "EpollDispatcher.h"
#include "Channel.h"
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

EventLoop::EventLoop()
	:EventLoop(string())
{
}

EventLoop::EventLoop(const string threadName)
{
    m_isQuit = false;
    m_threadID = this_thread::get_id();
    m_threadName = threadName == string() ? "MainThread" : threadName;
    m_dispatcher = new EpollDispatcher(this);
    // ����
    m_channelMap.clear();
    // map
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0,m_socketPair);
    if (ret == -1)
    {
        perror("socketpair");
        exit(0);
    }   

#if 0
    // ָ������: evLoop->socketPair[0] ��������, evLoop->socketPair[1] ��������
    Channel* channel = new Channel(m_socketPair[1], (int)FDEvent::ReadEvent,
        readLocalMessage, nullptr, nullptr, this);
#else
    auto obj = bind(&EventLoop::readLocalMessage, this);
    Channel* channel = new Channel(m_socketPair[1], (int)FDEvent::ReadEvent,
        obj, nullptr, nullptr, this);

    // channel ��ӵ��������
#endif
    addTask(channel, ElemType::ADD);

}

EventLoop::~EventLoop()
{
}

int EventLoop::run()
{
    // �Ƚ��߳�ID�Ƿ�����
    if (m_threadID != this_thread::get_id())
    {
        return -1;
    }
    // ѭ�������¼�����
    while (!m_isQuit)
    {
        m_dispatcher->dispatch();    // ��ʱʱ�� 2s
        eventLoopProcessTask();
    }
    return 0;

}

int EventLoop::eventActivate(int fd, int event)
{
    if (fd < 0)
    {
        return -1;
    }
    // ȡ��channel
    struct Channel* channel = m_channelMap[fd];
 
    if (event & (int)FDEvent::ReadEvent && channel->readCallback)
    {
        channel->readCallback(const_cast<void*>(channel->getArg()));
    }
    if (event & (int)FDEvent::WriteEvent && channel->writeCallback)
    {
        channel->writeCallback(const_cast<void*>(channel->getArg()));
    }
    return 0;
}

int EventLoop::addTask(Channel* channel, ElemType type)
{
    // ����, ����������Դ
    m_mutex.lock();
    // �����½ڵ�
    ChannelElement* node = (ChannelElement*)malloc(sizeof(ChannelElement));
    node->channel = channel;
    node->type = type;
    // ����Ϊ��
    m_taskQ.push(node);  // add     // ����

    m_mutex.unlock();
    // ����ڵ�
    /*
    * ϸ��:
    *   1. ��������ڵ�����: �����ǵ�ǰ�߳�Ҳ�����������߳�(���߳�)
    *       1). �޸�fd���¼�, ��ǰ���̷߳���, ��ǰ���̴߳���
    *       2). ����µ�fd, �������ڵ�Ĳ����������̷߳����
    *   2. ���������̴߳����������, ��Ҫ�ɵ�ǰ�����߳�ȡ����
    */
    if (m_threadID == this_thread::get_id())
    {
        // ��ǰ���߳�(�������̵߳ĽǶȷ���)
        eventLoopProcessTask();
    }
    else
    {
        // ���߳� -- �������̴߳�����������е�����
        // 1. ���߳��ڹ��� 2. ���̱߳�������:select, poll, epoll
        taskWakeup();
    }
    return 0;
}

int EventLoop::eventLoopProcessTask()
{
    
    // ȡ��ͷ���
    while (!m_taskQ.empty())
    {
        m_mutex.lock();
        ChannelElement* node = m_taskQ.front();
        m_taskQ.pop();
        m_mutex.unlock();
       
        if (node->type == ElemType::ADD)
        {
            // ���
            add(node->channel);
        }
        else if (node->type == ElemType::DELETE)
        {
            // ɾ��
            remove(node->channel);
        }
        else if (node->type == ElemType::MODIFY)
        {
            // �޸�
            modify(node->channel);
        }
        delete node;
        
    }

    return 0;
}

int EventLoop::add(Channel* channel)
{
    int fd = channel->getSocket();
    
    // �ҵ�fd��Ӧ������Ԫ��λ��, ���洢
    if (!m_channelMap.count(fd))
    {
        m_channelMap[fd] = channel;
        m_dispatcher->setChannel(channel);
        int ret = m_dispatcher->add();
        return ret;
    }
    return 0;
}

int EventLoop::remove(Channel* channel)
{
    int fd = channel->getSocket();
    if (!m_channelMap.count(fd))
    {
        return -1;
    }

    m_dispatcher->setChannel(channel);
    int ret = m_dispatcher->remove();
    return 0;
}

int EventLoop::modify(Channel* channel)
{

    int fd = channel->getSocket();
    if (!m_channelMap.count(fd))
    {
        return -1;
    }

    m_dispatcher->setChannel(channel);
    int ret = m_dispatcher->modify();
    return 0;
}

int EventLoop::freeChannel(Channel* channel)
{
    auto it = m_channelMap.find(channel->getSocket());
    if (it != m_channelMap.end())
    {
        m_channelMap.erase(it);
        close(channel->getSocket());
        delete channel;
    }
    return 0;
}

int EventLoop::readMessage()
{
    char buf[1024] = { 0 };
    int len = read(m_socketPair[1], buf, sizeof buf);
    return 0;
}

int EventLoop::readLocalMessage(void* arg)
{
    EventLoop* evLoop = static_cast<EventLoop*>(arg);
    char buf[1024] = { 0 };
    int len = read(evLoop->m_socketPair[1], buf, sizeof buf);
    return 0;
}

void EventLoop::taskWakeup()
{
    const char* msg = "����Ҫ��Ϊ�����һ�ı����δ�ʦ";
    write(m_socketPair[0], msg, strlen(msg));
}

