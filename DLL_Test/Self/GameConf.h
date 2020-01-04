#pragma once
#include "GameStruct.h"
#include <Windows.h>

#define MAX_CONF_ITEMS   16

class Game;
class GameConf
{
public:
	// ... 
	GameConf(Game* p);
	// ��ȡ�����ļ�
	bool ReadConf(const char* path);
private:
	// ��ȡ��������
	void ReadPetOut(const char* data);
	// ��ȡ��ʰ��Ʒ
	void ReadPickUp(const char* data);
	// ��ȡ������Ʒ
	void ReadSell(const char* data);
	// ��ȡ������Ʒ
	void ReadCheckIn(const char* data);
	// ��ȡ��������
	void ReadSetting(const char* data);
	// ת����Ʒ����
	ITEM_TYPE TransFormItemType(const char* item_name);
public:
	Game* m_pGame;

	// ����������
	struct {
		DWORD No[3];
		DWORD Length;
	} m_stPetOut;

	// ��ʰ��Ʒ�б�
	struct {
		ConfItemInfo PickUps[MAX_CONF_ITEMS];
		DWORD        Length;
	} m_stPickUp;

	// ������Ʒ�б�
	struct {
		ConfItemInfo Sells[MAX_CONF_ITEMS];
		DWORD        Length;
	} m_stSell;

	// ������Ʒ�б�
	struct {
		ConfItemInfo CheckIns[MAX_CONF_ITEMS];
		DWORD        Length;
	} m_stCheckIn;

	// ��������
	struct {
		char FBFile[64]; // ���������ļ�
	} m_Setting;
};