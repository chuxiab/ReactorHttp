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

	//重置
	void reset();


	void addHeader(const string key, const string value);

	string httpRequestGetHeader(const string key);
	//解析请求行
	bool parseHttpRequestLine(Buffer* readBuf);
	//解析请求头
	bool parseHttpRequestHeader(Buffer* readBuf);
	//解析http 请求协议
	bool parseHttpRequest(Buffer* readBuf,
		HttpResponse* response, Buffer* sendBuf, int socket);
	//处理http 请求
	bool processHttpRequest(HttpResponse* response);
	//处理中文格式
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