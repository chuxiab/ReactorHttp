#include "PollDispatcher.h"

PollDispatcher::PollDispatcher(EventLoop* evLoop)
	:Dispatcher(evLoop)
{
	
	m_maxfd = 0;
	for (int i = 0; i < 1024; ++i)
	{
		m_fds[i].fd = -1;
		m_fds[i].events = 0;
		m_fds[i].revents = 0;
	}
}

PollDispatcher::~PollDispatcher()
{
}

int PollDispatcher::add()
{
	int events = 0;
	if (m_channel->getEvent() & (int)FDEvent::ReadEvent)
	{
		events |= POLLIN;
	}
	if (m_channel->getEvent() & (int)FDEvent::WriteEvent)
	{
		events |= POLLOUT;
	}

	int i = 0;
	for (; i < 1024; ++i)
	{
		if (m_fds[i].fd == -1)
		{
			m_fds[i].fd = m_channel->getSocket();
			m_fds[i].events = events;
			m_maxfd = m_maxfd > i ? m_maxfd : i;
			break;
		}
	}

	if (i >= 1024)
	{
		return -1;
	}

	return 0;
}

int PollDispatcher::remove()
{

	int i = 0;
	for (; i < 1024; ++i)
	{
		if (m_fds[i].fd == -1)
		{
			m_fds[i].fd = -1;
			m_fds[i].events = 0;
			m_fds[i].revents = 0;
			m_maxfd = m_maxfd == i ? m_maxfd - 1 : m_maxfd;
			break;
		}
	}
	m_channel->destroyCallback(const_cast<void*>(m_channel->getArg()));
	if (i >= 1024)
	{
		return -1;
	}
	return 0;
}

int PollDispatcher::modify()
{
	int events = 0;
	if (m_channel->getEvent() & (int)FDEvent::ReadEvent)
	{
		events |= POLLIN;
	}
	if (m_channel->getEvent() & (int)FDEvent::WriteEvent)
	{
		events |= POLLOUT;
	}

	int i = 0;
	for (; i < 1024; ++i)
	{
		if (m_channel->getSocket() == m_fds[i].fd)
		{
			m_fds[i].events = events;
			break;
		}
	}

	if (i >= 1024)
	{
		return -1;
	}
	return 0;
}

int PollDispatcher::dispatch(int timeout)
{

	int count = poll(m_fds, m_maxfd + 1, timeout * 5000);
	if (count == -1)
	{
		perror("poll");
		exit(0);
	}
	printf("%d\n", count);
	for (int i = 0; i <= m_maxfd; ++i)
	{
		if (m_fds[i].fd == -1)
		{
			continue;
		}

		if (m_fds[i].revents & POLLIN)
		{
			m_evLoop->eventActivate(m_fds[i].fd, (int)FDEvent::ReadEvent);
		}

		if (m_fds[i].revents & POLLOUT)
		{
			m_evLoop->eventActivate(m_fds[i].fd, (int)FDEvent::WriteEvent);
		}
	}

	return 0;
}
