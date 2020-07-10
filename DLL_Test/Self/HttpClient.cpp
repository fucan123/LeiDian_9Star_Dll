#include "HttpClient.h"
#include <My/Common/Des.h>
#include <stdio.h>

HTTP_STATUS HttpClient::Request(const char* host, const char* path, int port, std::string& result, const char* post_con, int type)
{
	result = "";
	HTTP_STATUS status = 0;

	WSADATA     wsaData;
	SOCKET      socket_client;
	SOCKADDR_IN server_in;
	int         iaddrSize = sizeof(SOCKADDR_IN);
	DWORD       dwThreadId;

	WSAStartup(0x0202, &wsaData);

	socket_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


	server_in.sin_family = AF_INET;
	server_in.sin_port = htons(port);

	if (!strstr(host, "com")) {
		server_in.sin_addr.S_un.S_addr = inet_addr(host);
	}
	else {
		struct hostent *host_ent = gethostbyname(host);
		char *ip;
		ip = inet_ntoa(*(struct in_addr *)*host_ent->h_addr_list);
		memcpy(&server_in.sin_addr.S_un.S_addr, host_ent->h_addr_list[0], host_ent->h_length);
	}
	if (connect(socket_client, (struct sockaddr *)&server_in, sizeof(server_in)) == SOCKET_ERROR) {
		printf("connect失败.\n");
		closesocket(socket_client);
		WSACleanup();
		return 0;
	}


	char request_type[8];
	if (type == HTTP_POST)
		strcpy(request_type, "POST");
	else if (type == HTTP_PUT)
		strcpy(request_type, "PUT");
	else
		strcpy(request_type, "GET");

	char heads[1536];
	char  heads_tmp[] =
		"%s %s HTTP/1.1\r\n" \
		"Host: %s\r\n" \
		"Content-Type: application/x-www-form-urlencoded\r\n" \
		"Content-Length: %d\r\n" \
		"Connection: Close\r\n\r\n" \
		"%s";

	sprintf_s(heads, heads_tmp, request_type, path, host, strlen(post_con), post_con);
	//printf(heads);

	send(socket_client, heads, strlen(heads), 0);

	std::string response;
	char ch, r = 0;
	while (recv(socket_client, &ch, 1, 0)) {
		if (ch == '\r' && !r) {
			r = ch;
			if (response.find("222 OK") == std::string::npos)
				return 0;
		}
		response += ch;
	}

	if (response.length() == 0)
		return 0;

	size_t res_head_end = response.find("\r\n\r\n");
	if (res_head_end == std::string::npos)
		return 0;

	size_t res_length = (size_t)response.c_str() + res_head_end + 4;
	size_t res_con_start = response.find("\r\n", res_head_end + 4);
	if (res_con_start == std::string::npos)
		return 0;
	//printf("res_length:%lld res_con_start:%lld\n", res_length, res_con_start);
	//printf(response.c_str() + res_head_end + 2);

	int con_length = HexToInt(response.c_str() + res_head_end + 4, 0);
	if (con_length == 0)
		return 0;
	//printf("长度:%d\n", HexToInt(response.c_str() + res_head_end + 4, 0))

	result.empty();
	for (size_t j = 0, i = res_con_start + 2; j < con_length; j++, i++) {
		result += response[i];
	}

	//printf(result.c_str());

	closesocket(socket_client);
	WSACleanup();

	return 1;
}

int HttpClient::HexToInt(const char* str, int length)
{
	if (str == nullptr)
		return 0;

	if (str[0] == '0') {
		if (str[1] == 'x' || str[1] == 'X')
			str += 2;
	}

	int i = 0;
	int num = 0;
	while (*str) {
		char ch = *str;
		if (ch >= '0' && ch <= '9') {
			ch = ch - '0';
		}
		else if (ch >= 'A' && ch <= 'F') {
			ch = ch - 'A' + 0x0A;
		}
		else if (ch >= 'a' && ch <= 'f') {
			ch = ch - 'a' + 0x0a;
		}
		else {
			ch = 0;
			break;
		}

		num = num * 0x10 + ch;
		if (length > 0 && (++i >= length))
			break;

		str++;
	}
	return num;
}

void HttpClient::CharToHext(char* save, char* str, int length)
{
	int i = 0, j = 0;
	for (; i < length; i++) {
		char tmp[3];
		sprintf_s(tmp, "%02x", str[i] & 0xff);
		save[j] = tmp[0];
		save[j + 1] = tmp[1];
		j += 2;
	}
}


std::string HttpClient::UTF8ToGB(const char * str)
{
	std::string result;
	WCHAR *strSrc;
	CHAR *szRes;

	//获得临时变量的大小
	int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	strSrc = new WCHAR[i + 1];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

	//获得临时变量的大小
	i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
	szRes = new CHAR[i + 1];
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

	result = szRes;
	delete[]strSrc;
	delete[]szRes;

	return result;
}
