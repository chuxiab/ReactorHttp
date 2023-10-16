#pragma once
#include "Dispatcher.h"
#include <string>
#include <poll.h>

using namespace std;


class PollDispatcher : public Dispatcher
{
public:
    PollDispatcher(EventLoop* evLoop);
    ~PollDispatcher();

    int add() override;
    // ɾ��
    int remove() override;
    // �޸�
    int modify() override;
    // �¼����
    int dispatch(int timeout = 2) override; // ��λ: s

private:
    int m_maxfd;
    struct pollfd m_fds[1024];

};
