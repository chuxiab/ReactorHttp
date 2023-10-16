#include "Channel.h"

Channel::Channel(int fd, int events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg)
	:m_fd(fd), m_events(events), readCallback(readFunc), writeCallback(writeFunc), destroyCallback(destroyFunc), m_arg(arg)
{
	m_fd = fd;
	m_events = events;
	readCallback = readFunc;
	writeCallback = writeFunc;
	destroyCallback = destroyFunc;
	m_arg = arg;
}

void Channel::writeEventEnable(bool flag)
{
	if (flag)
		this->m_events |= static_cast<int>(FDEvent::WriteEvent);
	else
		this->m_events &= ~static_cast<int>(FDEvent::WriteEvent);
}

bool Channel::isWriteEventEnable()
{
	return this->m_events & static_cast<int>(FDEvent::WriteEvent);
}


