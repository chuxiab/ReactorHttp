#include "Buffer.h"
#include <cstring>
#include <strings.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>

Buffer::Buffer(int size)
{
	m_data = (char*)malloc(size);
	m_capacity = size;
	m_readPos = m_writePos = 0;
	memset(m_data, 0, size);
}

Buffer::~Buffer()
{
	if (m_data != nullptr)
	{
		free(m_data);
	}
}

void Buffer::extendRoom(int size)
{
	if (writeableSize() >= size)
	{
		return;
	}

	if (writeableSize() + m_readPos >= size)
	{
		int readable = readableSize();
		memcpy(m_data, m_data + m_readPos, readable);
		m_readPos = 0;
		m_writePos = readable;
	}
	else
	{
		void* temp = realloc(m_data, m_capacity + size);
		if (temp == NULL)
		{
			return;
		}
		memset(temp + m_capacity, 0, size);

		m_data = static_cast<char*>(temp);
		m_capacity += size;	
	}
}

int Buffer::appendString(const char* data, int size)
{
	if (data == nullptr || size <= 0)
	{
		return -1;
	}
	extendRoom(size);

	memcpy(m_data + m_writePos, data, size);
	m_writePos += size;
	return 0;
}

int Buffer::appendString(const char* data)
{
	int len = strlen(data);
	int ret = appendString(data, len);
	return ret;
}

int Buffer::appendString(const string data)
{
	int ret = appendString(data.data());
	return ret;
}

int Buffer::socketRead(int fd)
{
	struct iovec vec[2];
	int writeable = writeableSize();
	vec[0].iov_base = m_data + m_writePos;
	vec[0].iov_len = writeable;

	char* tmpbuf = (char*)malloc(40960);
	vec[1].iov_base = tmpbuf;
	vec[1].iov_len = 40960;

	int result = readv(fd, vec, 2);
	if (result == -1)
	{
		return -1;
	}
	else if (result <= writeable)
	{
		m_writePos += result;
	}
	else
	{
		m_writePos = m_capacity;
		appendString(tmpbuf, result - writeable);
	}
	free(tmpbuf);
	return result;
}

char* Buffer::findCRLF()
{
	//strstr 遇到\0就结束了memmem 大的块匹配字符串
	char* ptr = (char*)memmem(m_data + m_readPos, readableSize(), "\r\n", 2);
	return ptr;
}

int Buffer::sendData(int fd)
{
	int readable = readableSize();
	if (readable > 0)
	{
		int count = send(fd, m_data + m_readPos, readable, MSG_NOSIGNAL);
		if (count > 0)
		{
			m_readPos += count;
			usleep(1);
		}
		return count;
	}
	return 0;
}
