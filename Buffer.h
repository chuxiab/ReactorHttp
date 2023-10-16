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

	//����
	void extendRoom(int size);

	//д�ڴ�ֱ��д
	int appendString(const char* data, int size);
	int appendString(const char* data);
	int appendString(const string data);
	int socketRead(int fd);

	//����\r\n ȡ��1��
	char* findCRLF();

	int sendData(int fd);
	//�õ�ʣ���д���ڴ�����
	inline int writeableSize()
	{
		return m_capacity - m_writePos;
	}

	//�õ�ʣ��ɶ����ڴ�����
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
