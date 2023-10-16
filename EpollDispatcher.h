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
    // ɾ��
    int remove() override;
    // �޸�
    int modify() override;
    // �¼����
    int dispatch(int timeout = 2) override; // ��λ: s

private:
    int m_epfd;
    struct epoll_event* m_events;
    const int m_maxNode = 520;

    int epollCtl(int op);

};