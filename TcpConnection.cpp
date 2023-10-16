#include "TcpConnection.h"
#include "Log.h"

TcpConnection::TcpConnection(int fd, EventLoop* evLoop)
{
	m_evLoop = evLoop;
	m_readBuf = new Buffer(10240);
	m_writeBuf = new Buffer(10240);

	m_request = new HttpRequest;
	m_response = new HttpResponse;

	m_name = "Connection-" + to_string(fd);
	m_channel = new Channel(fd, (int)FDEvent::ReadEvent, processRead, processWrite, destroy, this);
	evLoop->addTask(m_channel, ElemType::ADD);
}

TcpConnection::~TcpConnection()
{
	if (m_readBuf && m_readBuf->readableSize() == 0 && m_writeBuf && m_writeBuf->readableSize() == 0)
	{
		delete m_readBuf;
		delete m_writeBuf;
		delete m_request;
		delete m_response;
		m_evLoop->freeChannel(m_channel);
	}
	Debug("连接断开, 释放资源, gameover, connName: %s", m_name);
}

int TcpConnection::processRead(void* arg)
{
	printf("7777777777\n");
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	int socket = conn->m_channel->getSocket();
	int count = conn->m_readBuf->socketRead(socket);

	char* a = conn->m_readBuf->getData();
	for (int i = 0; i < conn->m_readBuf->readableSize(); ++i)
		printf("%c", *(a + i));
	printf("\n");

	if (count > 0)
	{
#ifdef MSG_SEND_AUTO
		conn->m_channel->writeEventEnable(true);
		conn->m_evLoop->addTask(conn->m_channel, ElemType::Modify);
#endif
		bool flag = conn->m_request->parseHttpRequest(conn->m_readBuf, conn->m_response, conn->m_writeBuf, socket);
		
		if (!flag)
		{
			string msg = "Http/1.1 400 Bad Request\r\n\r\n";
			conn->m_writeBuf->appendString(msg);
		}
	}
	else
	{
//#ifdef MSG_SEND_AUTO
//		conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);
//#endif



	}
#ifndef MSG_SEND_AUTO
		conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);
#endif
	return 0;
}

int TcpConnection::processWrite(void* arg)
{
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	int count = conn->m_writeBuf->sendData(conn->m_channel->getSocket());

	if (count > 0)
	{
		if (conn->m_writeBuf->readableSize() == 0)
		{
			conn->m_channel->writeEventEnable(false);
			conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);
			conn->m_evLoop->addTask(conn->m_channel, ElemType::MODIFY);
			
		}
	}
	return 0;
}

int TcpConnection::destroy(void* arg)
{
	TcpConnection* conn = static_cast<TcpConnection*>(arg);

	if (conn != nullptr)
	{
		delete conn;
	}
	return 0;
}


