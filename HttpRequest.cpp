#include "HttpRequest.h"
#include "Log.h"
#include <functional>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>

HttpRequest::HttpRequest()
{
	reset();
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::reset()
{
	m_curState = ProcessState::ParseReqLine;
	m_method = m_url = m_version = string();
	m_reqHeaders.clear();
}

void HttpRequest::addHeader(const string key, const string value)
{
	if (key.empty() && value.empty())
	{
		return;
	}
	m_reqHeaders.insert(make_pair(key, value));
}

string HttpRequest::httpRequestGetHeader(const string key)
{
	if (!m_reqHeaders.count(key))
		return string();
	
	return m_reqHeaders[key];
}

bool HttpRequest::parseHttpRequestLine(Buffer* readBuf)
{
	char* end = readBuf->findCRLF();
	char* start = readBuf->getData();
	
	int lineSize = end - start;

	if (lineSize > 0)
	{
		char* tmp = (char*)malloc(lineSize + 1);
		auto methodFunc = bind(&HttpRequest::setMethod, this, placeholders::_1);
		start = splitRequestLine(start, end, " ", methodFunc);
		auto urlFunc = bind(&HttpRequest::seturl, this, placeholders::_1);
		start = splitRequestLine(start, end, " ", urlFunc);
		auto versionFunc = bind(&HttpRequest::setVersion, this, placeholders::_1);
		start = splitRequestLine(start, end, nullptr, versionFunc);
		
		readBuf->addReadPos(lineSize + 2);
		setState(ProcessState::ParseReqHeaders);
		return true;
	}
	return false;
}

bool HttpRequest::parseHttpRequestHeader(Buffer* readBuf)
{
	char* end = readBuf->findCRLF();

	if (end != nullptr)
	{
		char* start = readBuf->getData();
		int lineSize = end - start;

		char* middle = static_cast<char*>(memmem(start, lineSize, ": ", 2));
		if (middle != nullptr)
		{
			int keylen = middle - start;
			int valuelen = end - middle - 2;
			if (keylen > 0 && valuelen > 0)
			{
				string key(start, keylen);
				string value(middle + 2, valuelen);
				addHeader(key, value);
			}
			readBuf->addReadPos(lineSize + 2);
		}
		else
		{
			//\r\n 跳过
			readBuf->addReadPos(2);
			setState(ProcessState::ParseReqDone);
		}
		return true;
	}
	
	
	return false;
}

bool HttpRequest::parseHttpRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket)
{
	bool flag = true;
	while (m_curState != ProcessState::ParseReqDone)
	{
		switch (m_curState)
		{
		case ProcessState::ParseReqLine :
			flag = parseHttpRequestLine(readBuf);
			break;
		case ProcessState::ParseReqHeaders:
			flag = parseHttpRequestHeader(readBuf);
		case ProcessState::ParseReqBody :
				break;
		default: 
			break;
		}
		if (!flag) return flag;

		if (m_curState == ProcessState::ParseReqDone)
		{
			processHttpRequest(response);
			response->prepareMsg(sendBuf, socket);
		}
	}
	m_curState = ProcessState::ParseReqLine;
	return flag;
}

bool HttpRequest::processHttpRequest(HttpResponse* response)
{
	if (strcasecmp(m_method.data(), "get") != 0)
	{
		return -1;
	}
	
	m_url = decodeMsg(m_url);

	const char* file = nullptr;
	if (strcmp(m_url.data(), "/") == 0)
	{
		file = "./";
	}
	else
	{
		file = m_url.data() + 1;
	}

	struct stat s;
	int ret = stat(file, &s);
	if (ret == -1)
	{
		response->setStatusCode(StatusCode::NotFound);
		response->set_fileName("404.html");

		response->addHeader("Content-type", getFileType("*.html"));
		response->sendDataFunc = sendFile;
		return 0;
	}

	response->set_fileName(file);
	response->setStatusCode(StatusCode::OK);
	
	if (S_ISDIR(s.st_mode))
	{
		response->addHeader("Content-type", getFileType(file));
		response->sendDataFunc = sendDir;
		
	}
	else if (S_ISREG(s.st_mode))
	{
		response->addHeader("Content-type", getFileType(file));
		response->addHeader("Content-length", to_string(s.st_size));
		response->sendDataFunc = sendFile;
	}

	return false;
}

string HttpRequest::decodeMsg(string msg)
{
	string str = string();
	const char* from = msg.data();
	for (; *from != '\0'; ++from)
	{
		// isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
		// Linux%E5%86%85%E6%A0%B8.jpg
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
		{
			// 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
			// B2 == 178
			// 将3个字符, 变成了一个字符, 这个字符就是原始数据
			str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));

			// 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
			from += 2;
		}
		else
		{
			// 字符拷贝, 赋值
			str.append(1, *from);
		}
	}
	str.append(1, '\0');
	return str;
}

const string HttpRequest::getFileType(const string name)
{
	// a.jpg a.mp4 a.html
		// 自右向左查找‘.’字符, 如不存在返回NULL
	const char* dot = strrchr(name.data(), '.');
	if (dot == NULL)
		return "text/plain; charset=utf-8";	// 纯文本
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";
}

void HttpRequest::sendFile(const string fileName, Buffer* sendBuf, int cfd)
{
	int fd = open(fileName.data(), O_RDONLY);
	assert(fd > 0);

#if 1
	while (1)
	{
		char buf[1024] = { 0 };
		int len = read(fd, buf, sizeof buf);
		if (len > 0)
		{
			sendBuf->appendString(buf, len);
#ifndef MSG_SEND_AUTO
			sendBuf->sendData(cfd);
#endif
		}
		else if (len == 0)
		{
			break;
		}
		else
		{
			close(fd);
			break;
		}
	}
#else
	off_t offset = 0;
	int size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	while (offset < size)
	{
		int ret = sendFile(cfd, fd, &offset, size - offset);
		printf("ret value : %d\n", ret);
		if (ret == -1 && errno == EAGAIN)
		{
			printf("no datas\n");
		}
	}
#endif
	close(fd);
}

void HttpRequest::sendDir(string dirName, Buffer* sendBuf, int cfd)
{
	char buf[4096] = { 0 };
	sprintf(buf, "<html><head><title>%s</title><meta charset=\"UTF-8\"></head><body><table>", dirName.data());
	struct dirent** namelist;
	int num = scandir(dirName.data(), &namelist, NULL, alphasort);
	for (int i = 0; i < num; ++i)
	{
		// 取出文件名 namelist 指向的是一个指针数组 struct dirent* tmp[]
		char* name = namelist[i]->d_name;
		struct stat st;
		char subPath[1024] = { 0 };
		sprintf(subPath, "%s/%s", dirName.data(), name);
		stat(subPath, &st);
		if (S_ISDIR(st.st_mode))
		{
			// a标签 <a href="">name</a>
			sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
				name, name, st.st_size);
		}
		else
		{
			sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
				name, name, st.st_size);
		}
		// send(cfd, buf, strlen(buf), 0);
		sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
		sendBuf->sendData(cfd);
#endif
		memset(buf, 0, sizeof(buf));
		free(namelist[i]);
	}
	sprintf(buf, "</table></body></html>");
	// send(cfd, buf, strlen(buf), 0);
	sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
	sendBuf->sendData(cfd);
#endif
	free(namelist);
}

char* HttpRequest::splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback)
{
	char* space = const_cast<char*>(end);
	if (sub != nullptr)
	{
		space = static_cast<char*>(memmem(start, end - start, sub, strlen(sub)));
		assert(space != nullptr);
	}
	int len = space - start;
	callback(string(start, len));
	return space + 1;
}

int HttpRequest::hexToDec(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0;
}
