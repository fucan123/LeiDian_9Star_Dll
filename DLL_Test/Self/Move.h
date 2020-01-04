#pragma once
#include "GameStruct.h"

class Game;

class Move
{
public:
	// ...
	Move(Game* p);

	// 初始化数据
	void InitData();
	// 移动
	bool Run(DWORD x, DWORD y);
	// 设置移动位置
	void SetMove(DWORD x, DWORD y);
	// 清除移动数据
	void ClearMove();
	// 是否达到终点
	bool IsMoveEnd();
	// 是否移动
	bool IsMove();
public:
	// 游戏类
	Game* m_pGame;
	// 当前位置X
	DWORD m_dwX = 0;
	// 当前位置Y
	DWORD m_dwY = 0;
	// 上次位置X
	DWORD m_dwLastX = 0;
	// 上次位置Y
	DWORD m_dwLastY = 0;
	// 移动位置X
	DWORD m_dwMvX;
	// 移动位置Y
	DWORD m_dwMvY;
	// 判断移动结束时间
	DWORD m_dwIsEndTime;
	// 判断是否移动时间
	DWORD m_dwIsMvTime;
	// 用于比较的X
	DWORD m_dwCmpX;
	// 用于比较的Y
	DWORD m_dwCmpY;
	// 移动时间
	DWORD m_dwMvTime;
	// 获取坐标时间
	int   m_iGetPosTime;
	// 获取坐标标记时间
	int   m_iFlagPosTime;

	CHAR m_Test[0x1000];
};