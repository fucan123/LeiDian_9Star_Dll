#pragma once
#include "GameStruct.h"
#include "Magic.h"
#include <Windows.h>

class Game;

class GuaiWu
{
public:
	// ...
	GuaiWu(Game* p);

	// 初始化数据
	void InitData();
	// 自身周围是否有怪物
	bool HasInArea(DWORD cx, DWORD cy);
	// 怪物是否在自身区域内 x,y=自身范围(IN),怪物坐标(OUT)
	bool IsInArea(const char* name, IN OUT DWORD& x, IN OUT DWORD& y);
	// 清理怪物
	bool Clear(const char* magic_name, DWORD cx, DWORD cy);
	// 初始化被攻击的怪物
	void InitAttack();
	// 设置正在攻击怪物
	void SetAttack(GamePlayer* p);
	// 是否忽略正在攻击怪物
	bool IsIgnoreAttack();
	// 获取怪物当前血量
	DWORD GetLife(GamePlayer* p, DWORD no);
	// 读取怪物
	bool ReadGuaiWu();
public:
	// 游戏类
	Game* m_pGame;

	// 是否搜索怪物名称
	bool m_bSearchName;
	// 搜索的怪物名称
	char m_sSearChName[16];
	// 是否开启清理怪物
	bool m_bIsClear;
	// 攻击的X半径
	DWORD m_dwCX;
	// 攻击的Y半径
	DWORD m_dwCY;
	// 正在攻击的怪物指针
	GamePlayer* m_pAttack;
	// 攻击时间
	__int64     m_i64AttackTime;
};