#pragma once
#include <functional>

// 定义函数指针
//typedef int(*handleFunc)(void* arg);

// 定义文件描述符的读写事件
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
    // 回调函数的参数
private:
    // 文件描述符
    int m_fd;
    // 事件
    int m_events;
    void* m_arg;
    // 回调函数
};