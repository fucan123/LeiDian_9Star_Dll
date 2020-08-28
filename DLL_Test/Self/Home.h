#pragma once

#include "HttpClient.h"
#include <string>

#define HOME_HOST      "137.59.149.38"
//#define HOME_HOST      L"www.fuzhu.com"
#define HOME_GAME_FLAG "1"

class Game;
class Home;

typedef bool (Home::*func_valid)();

class Home
{
public:
	Home(Game* p);
	// 设置是否免费
	void SetFree(bool v);
	// 是否有效
	bool IsValid();
	bool IsValidS();
	// 验证
	bool Verify();
	// 获得此机器有效时间（秒）
	int  GetExpire();
	// 解析返回结果
	void Parse(const char* msg, char key_yh[], int key_yh_length, HttpClient& http);
	// 获得返回结果中的值
	bool GetValue(char* key, char value[], int length);
	// 获得返回结果中的值
	int  GetValue(char* key, int length, int default_value=0);
	// 获得返回结果中的值
	int  GetValue(const char* key, int length, int default_value = 0);
public:
	Game* m_pGame;

	char m_NoUse[128];

	int m_nVerifyNum = 0;
	// 返回结果指针
	char m_pRepsone[256];
	// 验证时间（秒）
	int   m_iVerifyTime;
	// 过期时间
	int m_iExpire;
	// 到期时间
	int   m_iEndTime;

	char m_MachineId[33];
	int  m_Status;
	int  m_Error;

	// 是否免费
	bool m_bFree = false;

	// 网站返回内容
	std::string m_sResult;
	// 提示内容
	char m_MsgStr[128];
};