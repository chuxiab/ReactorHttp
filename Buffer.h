#pragma once
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

class Buffer
{
public:
	Buffer(int size);
	~Buffer();

	//扩容
	void extendRoom(int size);

	//写内存直接写
	int appendString(const char* data, int size);
	int appendString(const char* data);
	int appendString(const string data);
	int socketRead(int fd);

	//根据\r\n 取出1行
	char* findCRLF();

	int sendData(int fd);
	//得到剩余可写的内存容量
	inline int writeableSize()
	{
		return m_capacity - m_writePos;
	}

	//得到剩余可读的内存容量
	inline int readableSize()
	{
		return m_writePos - m_readPos;
	}

	inline char* getData()
	{
		return m_data + m_readPos;
	}

	inline void addReadPos(int x)
	{
		m_readPos += x;
	}

private:
	char* m_data;
	int m_capacity;
	int m_readPos = 0;
	int m_writePos = 0; 
};
