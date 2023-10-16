#pragma once
#include <stdbool.h>
#include "HttpResponse.h"
#include "Buffer.h"
#include <iostream>
#include <string>
#include <map>
#include <functional>

using namespace std;

enum class ProcessState : char
{
	ParseReqLine,
	ParseReqHeaders,
	ParseReqBody,
	ParseReqDone
};

class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();

	//����
	void reset();


	void addHeader(const string key, const string value);

	string httpRequestGetHeader(const string key);
	//����������
	bool parseHttpRequestLine(Buffer* readBuf);
	//��������ͷ
	bool parseHttpRequestHeader(Buffer* readBuf);
	//����http ����Э��
	bool parseHttpRequest(Buffer* readBuf,
		HttpResponse* response, Buffer* sendBuf, int socket);
	//����http ����
	bool processHttpRequest(HttpResponse* response);
	//�������ĸ�ʽ
	string decodeMsg(string from);
	const string getFileType(const string name);
	
	static void sendFile(const string fileName, Buffer* sendBuf, int cfd);

	static void sendDir(string fileName,Buffer* sendBuf, int cfd);

	inline void setMethod(string method)
	{
		m_method = method;
	}

	inline void seturl(string url)
	{
		m_url = url;
	}

	inline void setVersion(string version)
	{
		m_version = version;
	}

	inline ProcessState getState()
	{
		return m_curState;
	}

	inline void setState(ProcessState state)
	{
		m_curState = state;	
	}
private:
	char* splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback);
	int hexToDec(char c);

private:
	string m_method;
	string m_url;
	string m_version;
	map<string, string> m_reqHeaders;
	ProcessState m_curState;
};