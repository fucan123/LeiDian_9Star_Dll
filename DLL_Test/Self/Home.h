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
	// �����Ƿ����
	void SetFree(bool v);
	// �Ƿ���Ч
	bool IsValid();
	bool IsValidS();
	// ��֤
	bool Verify();
	// ��ô˻�����Чʱ�䣨�룩
	int  GetExpire();
	// �������ؽ��
	void Parse(const char* msg, char key_yh[], int key_yh_length, HttpClient& http);
	// ��÷��ؽ���е�ֵ
	bool GetValue(char* key, char value[], int length);
	// ��÷��ؽ���е�ֵ
	int  GetValue(char* key, int length, int default_value=0);
	// ��÷��ؽ���е�ֵ
	int  GetValue(const char* key, int length, int default_value = 0);
public:
	Game* m_pGame;

	char m_NoUse[128];

	int m_nVerifyNum = 0;
	// ���ؽ��ָ��
	char m_pRepsone[256];
	// ��֤ʱ�䣨�룩
	int   m_iVerifyTime;
	// ����ʱ��
	int m_iExpire;
	// ����ʱ��
	int   m_iEndTime;

	char m_MachineId[33];
	int  m_Status;
	int  m_Error;

	// �Ƿ����
	bool m_bFree = false;

	// ��վ��������
	std::string m_sResult;
	// ��ʾ����
	char m_MsgStr[128];
};