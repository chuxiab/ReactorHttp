#pragma once
#include "Buffer.h"
#include <string>
#include <map>
#include <functional>

using namespace std;

enum class StatusCode
{
	Unknown,
	OK = 200,
	MovePermanently = 301,
	MovedTemporarily = 302,
	BadRequest = 400,
	NotFound = 404
};

class HttpResponse
{
public:
	HttpResponse();
	~HttpResponse();
	function<void(const string, Buffer*, int)> sendDataFunc;

	//�����Ӧͷ
	void addHeader(const string key, const string value);
	//��֯http��Ӧ����
	void prepareMsg(Buffer* sendBuf, int socket);

	inline void set_fileName(string fileName)
	{
		m_fileName = fileName;
	}

	inline void setStatusCode(StatusCode code)
	{
		m_statusCode = code;
	}

private:
	//״̬�У�״̬�룬 ״̬����
	StatusCode m_statusCode;
	string m_fileName;
	map<string, string> m_headers;
	const map<int, string> m_info = {
		{200, "OK"},
		{301, "MovedPermanently"},
		{302, "MovedTemporarily"},
		{400, "BadRequest"},
		{404, "NotFound"}
	};

};