#include "HttpResponse.h"

HttpResponse::HttpResponse()
{
	m_statusCode = StatusCode::Unknown;
	m_headers.clear();
	sendDataFunc = nullptr;
	m_fileName = string();
}

HttpResponse::~HttpResponse()
{

}

void HttpResponse::addHeader(const string key, const string value)
{
	if (key.empty() || value.empty())
	{
		return;
	}

	m_headers.insert(make_pair(key, value));
}

void HttpResponse::prepareMsg(Buffer* sendBuf, int socket)
{
	char tmp[1024] = { 0 };
	int code = static_cast<int>(m_statusCode);

	sprintf(tmp, "HTTP/1.1 %d %s\r\n", code, m_info.at(code).data());
	sendBuf->appendString(tmp);

	for (auto it = m_headers.begin(); it != m_headers.end(); ++it)
	{
		sprintf(tmp, "%d: %s\r\n", (*it).first, (*it).second.data());
	}

	sendBuf->appendString(tmp);

	sendBuf->appendString("\r\n");

#ifndef MSG_SEND_AUTO
	sendBuf->sendData(socket);
#endif
	sendDataFunc(m_fileName, sendBuf, socket);
}




