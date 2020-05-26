#pragma once
#include <WinSock2.h>
#include <string>

#define HTTP_POST 0
#define HTTP_GET  1
#define HTTP_PUT  2

typedef DWORD HTTP_STATUS;

class HttpClient
{
public:
	HttpClient() { }
	HTTP_STATUS Request(const char* host, const char* path, int port, std::string& result, const char* post_con = nullptr, int type = HTTP_GET);
	int HexToInt(const char* str, int length = 0);
	void CharToHext(char* save, char* str, int length);
	std::string UTF8ToGB(const char* str);
public:
	// ÊÇ·ñ×ª³ÉGB2312
	bool m_GB2312 = false;
};