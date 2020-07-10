#include "Home.h"
#include "Game.h"
#include <time.h>
#include <My/Common/MachineID.h>
#include <My/Common/Des.h>
#include <My/Common/Explode.h>
#include <My/Common/C.h>

Home::Home(Game* p)
{
	m_pGame = p;

	m_bFree = false;

	m_iVerifyTime = 0;
	m_iExpire = 0;
	m_iEndTime = 0;
	m_Status = 0;
	m_Error = 0;

	m_nVerifyNum = 0;

	MachineID mac;
	mac.GetMachineID(m_MachineId);
	m_MachineId[32] = 0;
	//printf("机器码:%s\n", m_MachineId);
}

// 设置是否免费
void Home::SetFree(bool v)
{
	m_bFree = v;
}

bool Home::IsValid()
{
#ifdef _DEBUG
	return true;
#endif
#if 1
	if (m_bFree)
		return true;
#endif
	if (!m_iVerifyTime || !m_iExpire || !m_iEndTime)
		return false;

	int now_time = time(NULL);
	if (now_time < m_iVerifyTime) { // 当前时间比验证时间还早 可能修改了系统时间
		return false;
	}
	//printf("%d --- %d(%d)\n", now_time, m_iEndTime, m_iEndTime - now_time);
	return now_time <= m_iEndTime;
}

// ...
bool Home::IsValidS()
{
	return false;
}


bool Home::Verify()
{
	std::string result;
	HttpClient http;

	int now_time = GetTickCount();
	int now_yu = (now_time >> 2) & 0x88;
	int mod = ((now_time >> 6) & 0xff) ^ now_yu;

	char key_yh[8] = { mod ^ 6, mod ^ 8, mod ^ 9, mod ^ 16, mod ^ 89, mod, 9, mod ^ 2 };
	char key[17], param[128], encryptParam[256], encryptParam2[1024], postCon[1024];
	memset(param, 0, sizeof(param));
	memset(encryptParam, 0, sizeof(encryptParam));
	memset(encryptParam2, 0, sizeof(encryptParam2));
	memset(postCon, 0, sizeof(postCon));

	RandStr(key, 16, 16, -1);
	sprintf_s(param, "game=%s&machine_id=%s&tm=%d", HOME_GAME_FLAG, m_MachineId, time(nullptr));
	DesEncrypt(encryptParam, (char*)"phpcpp999888666b", param, strlen(param));
	//printf("encryptParam:%s\n", encryptParam);
	int encry_param_len = strlen(encryptParam);
	int i = 0, j = 0, n = 0;
	for (; i < encry_param_len; i++) {
		encryptParam[i] ^= key_yh[j];
		if (++j >= sizeof(key_yh))
			j = 0;
	}
	//printf("encryptParam[0]:%02x %d\n", encryptParam[0] & 0xff, encryptParam[0]);
	http.CharToHext(encryptParam2, encryptParam, encry_param_len);
	sprintf_s(postCon, "p=%s&tm=%d", encryptParam2, now_time);

	char path[32];
	sprintf_s(path, "/verify_n?t=%d", now_time);
	if (!http.Request(HOME_HOST, "/verify_n", 80, result, postCon, HTTP_POST))
		return false;

	Parse(result.c_str(), key_yh, sizeof(key_yh), http);
	time_t a;

	//MessageBox(NULL, m_MsgStr, L"XXX", MB_OK);
	//printf("%d, %s 内容->%s %s %d\n", m_Error, m_MsgStr, m_pRepsone, result.c_str(), result.length());

	return m_Error == 0;
}

int Home::GetExpire()
{
	return m_iExpire = GetValue("expire:", 10);
}

void Home::Parse(const char* msg, char key_yh[], int key_yh_length, HttpClient& http)
{
	// 格式为(MSG||DES加密字符)
	// DES加密字符解密为(状态||机器码||过期日期[时间戳]||还剩时间[秒])

	char a = 'a', e = 'E', r = 'r', o = 'o', dian='.';
	char unknow_str[] = { 'U', 'n', 'k', 'n', o, 'w', dian, 0 };
	strcpy(m_MsgStr, unknow_str);

	char error_str[] = {e, r, r, o, r, dian, 0};
	char* msgStr = (char*)msg;
	char* desStr = strstr(msgStr, "||");
	if (!desStr) {
		m_Error = 1;
		return;
	}

	desStr += 2;

	int i = 0, j = 0, n = 0;

	//printf("desStr:%s\n", desStr);
	char key[17];
	strcpy(key, "phpcpp999888666b");

	char yhStr[512];
	for (; i < strlen(desStr); i += 2, n++) {
		yhStr[n] = http.HexToInt(desStr + i, 2) & 0xff;

		//printf("%c%c:%02x ", *(desStr + i), *(desStr + i + 1), yhStr[n]&0xff);
		yhStr[n] ^= key_yh[j];

		if (++j >= key_yh_length)
			j = 0;
	}
	yhStr[n] = 0;
	DesDecrypt(m_pRepsone, key, yhStr, strlen(yhStr), true);
	//printf("m_pRepsone:%s\n", m_pRepsone);

	Explode arr("||", m_pRepsone);
	if (arr.GetCount() != 5) {
		m_Error = 1;
		//printf("arr.GetCount() != 5\n");
		return;
	}

	if (arr.GetValue2Int(1) == 0) {
		//printf("arr.GetValue2Int(1) == 0\n");
		m_Error = 1;
		return;
	}

	//printf("MsgPtr:%p --- %c\n", msgPtr, *statusPtr);
	if (strcmp(arr[2], m_MachineId) != 0) {
		//printf("strcmp(arr[2], m_MachineId) != 0\n");
		m_Error = 1;
		return;
	}

	m_Error = 0;
}

bool Home::GetValue(char* key, char value[], int length)
{
	char* ptr = strstr(m_pRepsone, key);
	if (!ptr) {
		value[0] = 0;
		return false;
	}

	ptr += strlen(key);
	for (int i = 0; i < length; i++) {
		if (ptr[i] == ' ') {
			length = i;
			break;
		}
		value[i] = ptr[i];
	}
	value[length] = 0;

	return true;
}

int Home::GetValue(char* key, int length, int default_value)
{
	return GetValue((const char*)key, length, default_value);
}

int Home::GetValue(const char* key, int length, int default_value)
{
	char* ptr = strstr(m_pRepsone, key);
	if (!ptr) {
		return default_value;
	}
	//printf("%s\n", key);
	int value = 0;
	ptr += strlen(key);
	for (int i = 0; i < length; i++) {
		if (ptr[i] == '-') {
			return 0;
		}
		if (ptr[i] == ' ') {
			break;
		}
		if (ptr[i] >= '0' && ptr[i] <= '9') {
			//printf("%c\n", ptr[i]);
			value = value * 10 + (ptr[i] - '0');
		}
	}

	return value;
}
