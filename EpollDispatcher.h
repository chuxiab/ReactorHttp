#pragma once
#include "Dispatcher.h"
#include <string>
#include <sys/epoll.h>

using namespace std;
class EventLoop;


class EpollDispatcher : public Dispatcher
{
public:
    EpollDispatcher(EventLoop* evLoop);
    ~EpollDispatcher();

    int add() override;
    // 删除
    int remove() override;
    // 修改
    int modify() override;
    // 事件监测
    int dispatch(int timeout = 2) override; // 单位: s

private:
    int m_epfd;
    struct epoll_event* m_events;
    const int m_maxNode = 520;

    int epollCtl(int op);

};