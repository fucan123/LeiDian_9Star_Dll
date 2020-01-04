#pragma once
#include "GameStruct.h"

// 宠物结构体
typedef struct game_pet {
	DWORD Id;   // ID
	DWORD Life; // 血量
} GamePet;

typedef struct pet_record {
	DWORD Life; // 血量
	int   Time; // 记录时间
} PetRecord;

class Game;
// 宠物类
class Pet
{
public:
	// ...
	Pet(Game* p);

	// 初始化数据
	void InitData();
	// 宠物出征
	bool  PetOut(DWORD* nums, DWORD length, bool fuck=true);
	// 宠物合体
	bool  PetFuck(DWORD* nums, DWORD length);
	// 宠物解体
	bool  PetUnFuck(DWORD* nums, DWORD length);
	// 宠物技能
	void  Magic(char key);
	// 复活所有没有血量宠物
	DWORD Revive();
	// 是否需要复活
	bool IsNeedRevive(int index, DWORD life, DWORD life_max);
	// 读取拥有宠物
	DWORD ReadPetList(DWORD life=1);
public:
	// 游戏类
	Game* m_pGame;
	
	// 拥有宠物数量
	DWORD   m_dwPetCount;
	// 宠物结构列表
	GamePet m_GamePet[8];
	// 宠物血量记录
	PetRecord m_PetRecord[3];

	CHAR m_Test[0x1000];
};