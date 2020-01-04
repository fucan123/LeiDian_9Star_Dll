#include "GameConf.h"
#include "Game.h"
#include <ShlObj_core.h>
#include <stdio.h>
#include <memory.h>
#include <My/Common/mystring.h>
#include <My/Common/OpenTextFile.h>
#include <My/Common/Explode.h>

// ...
GameConf::GameConf(Game* p)
{
	m_pGame = p;

	memset(&m_stPetOut,  0, sizeof(m_stPetOut));
	memset(&m_stPickUp,  0, sizeof(m_stPickUp));
	memset(&m_stCheckIn, 0, sizeof(m_stCheckIn));
	memset(&m_Setting,   0, sizeof(m_Setting));
}

// ��ȡ�����ļ�
bool GameConf::ReadConf(const char* path)
{
	char conf_file[255];
	strcpy(conf_file, path);
	strcat(conf_file, "\\����.txt");
	printf("��Ϸ�����ļ�:%s\n", conf_file);
	OpenTextFile file;
	if (!file.Open(conf_file)) {
		printf("�Ҳ���'%s'�ļ�������\n", conf_file);
		return false;
	}

	bool pet = false, pickup = false, checkin = false, sell = false;
	int length = 0;
	char data[128];
	while ((length = file.GetLine(data, 128)) > -1) {
		if (length == 0) {
			continue;
		}
		if (strstr(data, "=")) {
			trim(data);
			ReadSetting(data);
			continue;
		}
		//printf("%d %s", length, data);
		if (data[0] == '\n' || data[0] == '\r') {
			pet = false;
			pickup = false;
			checkin = false;
			sell = false;
			continue;
		}
		if (strstr(data, "��������")) {
			pet = true;
			continue;
		}
		if (strstr(data, "��ʰ��Ʒ")) {
			pickup = true;
			continue;
		}
		if (strstr(data, "������Ʒ") || strstr(data, "������Ʒ")) {
			sell = true;
			continue;
		}
		if (strstr(data, "������Ʒ")) {
		    checkin = true;
			continue;
		}

		trim(data);
		if (pet) {
			ReadPetOut(data);
			continue;
		}
		if (pickup) {
			ReadPickUp(data);
			continue;
		}
		if (sell) {
			ReadSell(data);
			continue;
		}
		if (checkin) {
			ReadCheckIn(data);
			continue;
		}
	}

	file.Close();
	return true;
}

// ��ȡ��������
void GameConf::ReadPetOut(const char* data)
{
	if (m_stPetOut.Length == 3)
		return;

	DWORD length = m_stPetOut.Length;
	m_stPetOut.No[m_stPetOut.Length++] = atoi(data) - 1;
	printf("%d.����������:%d\n", m_stPetOut.Length, m_stPetOut.No[length] + 1);
}

// ��ȡ��ʰ��Ʒ
void GameConf::ReadPickUp(const char * data)
{
	if (m_stPickUp.Length >= MAX_CONF_ITEMS)
		return;

	DWORD length = m_stPickUp.Length;
	ITEM_TYPE type = TransFormItemType(data);
	strcpy(m_stPickUp.PickUps[length].Name, data);
	m_stPickUp.PickUps[length].Type = type;
	m_stPickUp.Length++;

	printf("%d.�Զ���ʰ��Ʒ:%s %08X\n", m_stPickUp.Length, data, type);
}

// ��ȡ������Ʒ
void GameConf::ReadSell(const char * data)
{
	if (m_stSell.Length >= MAX_CONF_ITEMS)
		return;

	DWORD length = m_stSell.Length;
	ITEM_TYPE type = TransFormItemType(data);
	strcpy(m_stSell.Sells[length].Name, data);
	m_stSell.Sells[length].Type = type;
	m_stSell.Length++;

	printf("%d.�Զ�������Ʒ:%s %08X\n", m_stSell.Length, data, type);
}

// ��ȡ������Ʒ
void GameConf::ReadCheckIn(const char* data)
{
	if (m_stCheckIn.Length >= MAX_CONF_ITEMS)
		return;

	DWORD length = m_stCheckIn.Length;
	ITEM_TYPE type = TransFormItemType(data);
	strcpy(m_stCheckIn.CheckIns[length].Name, data);
	m_stCheckIn.CheckIns[length].Type = type;
	m_stCheckIn.Length++;

	printf("%d.�Զ�������Ʒ:%s %08X\n", m_stCheckIn.Length, data, type);
}

// ��ȡ��������
void GameConf::ReadSetting(const char * data)
{
	Explode explode("=", data);
	if (strcmp(explode[0], "�����ļ�") == 0) {
		strcpy(m_Setting.FBFile, explode[1]);
	}
	printf("%s:%s\n", explode[0], explode[1]);
}

// ת����Ʒ����
ITEM_TYPE GameConf::TransFormItemType(const char * item_name)
{
	if (strcmp("��Чʥ����ҩ", item_name) == 0)
		return ��Чʥ����ҩ;
	if (strcmp("ħ�꾧ʯ", item_name) == 0)
		return ħ�꾧ʯ;
	if (strcmp("��꾧ʯ", item_name) == 0)
		return ��꾧ʯ;
	if (strcmp("��ħ��ʯ", item_name) == 0)
		return ��ħ��ʯ;
	if (strcmp("��Ч���ư�", item_name) == 0)
		return ��Ч���ư�;
	return δ֪��Ʒ;
}