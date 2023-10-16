#pragma once
#include <functional>

// ���庯��ָ��
//typedef int(*handleFunc)(void* arg);

// �����ļ��������Ķ�д�¼�
enum class FDEvent
{
    TimeOut = 0x01,
    ReadEvent = 0x02,
    WriteEvent = 0x04
};

class Channel
{
public:
    using handleFunc = std::function<int(void*)>;

    Channel(int fd, int events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg);
    void writeEventEnable(bool flag);
    bool isWriteEventEnable();

    inline int getEvent()
    {
        return m_events;
    }

    inline int getSocket()
    {
        return m_fd;
    }

    inline const void* getArg()
    {
        return m_arg;
    }

    handleFunc readCallback;
    handleFunc writeCallback;
    handleFunc destroyCallback;
    // �ص������Ĳ���
private:
    // �ļ�������
    int m_fd;
    // �¼�
    int m_events;
    void* m_arg;
    // �ص�����
};