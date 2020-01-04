#include "Move.h"
#include "Game.h"
#include "GameProc.h"
#include <stdio.h>
#include <time.h>
#include <My/Common/mystring.h>

// ...
Move::Move(Game * p)
{
	m_pGame = p;
	InitData();
}

// 初始化数据
void Move::InitData()
{
	ClearMove();
}

// 移动
bool Move::Run(DWORD x, DWORD y)
{
	if (x == m_dwMvX && y == m_dwMvY) {
		if ((GetTickCount() - m_dwMvTime) < 1000)
			return false;
	}

	//printf("Move::Run:%d,%d 自己位置：%d,%d\n", x, y, m_dwX, m_dwY);
	SetMove(x, y);
	m_pGame->Call_Run(x, y);

	return true;
}

// 设置移动位置
void Move::SetMove(DWORD x, DWORD y)
{
	m_pGame->ReadCoor(&m_dwLastX, &m_dwLastY); // 读取当前坐标

	m_dwMvX = x;
	m_dwMvY = y;
	m_dwMvTime = GetTickCount();

	int now_time = time(nullptr);
	if ((now_time - m_iGetPosTime) < 2) // 2秒内不取变化
		return;

	if (1 && m_dwCmpX && m_dwCmpY) {
		if (m_dwCmpX == m_dwLastX && m_dwCmpY == m_dwLastY) { // 坐标与上次一样
			if ((now_time - m_iGetPosTime) < 5) { // 获取坐标时间在最近
				if ((now_time - m_iFlagPosTime) > 30) { // 30秒坐标还一样当作掉线
					printf("可能卡住或掉线了\n");
					m_pGame->m_pGameProc->NPCLast();
				}

				int t_out = m_pGame->m_iQiaZhuS >= 30 ? m_pGame->m_iQiaZhuS : 30;
				if ((now_time - m_iFlagPosTime) >= t_out) {
					m_pGame->m_pGameProc->SendMsg("可能卡住或掉线了");
					m_pGame->Close(false);
				}
			}
		}
		else {
			m_iFlagPosTime = now_time;
		}
	}
	else {
		m_iFlagPosTime = now_time;
	}

	m_dwCmpX = m_dwLastX;
	m_dwCmpY = m_dwLastY;
	m_iGetPosTime = now_time;
}

// 清除移动数据
void Move::ClearMove()
{
	m_dwX = 0;
	m_dwY = 0;
	m_dwLastX = 0;
	m_dwLastY = 0;
	m_dwMvX = 0;
	m_dwMvY = 0;

	m_dwIsEndTime = 0;
	m_dwIsMvTime = 0;
	m_dwMvTime = 0;

	m_dwCmpX = 0;
	m_dwCmpY = 0;
	m_iGetPosTime = 0;
	m_iFlagPosTime = 0;
}

// 是否达到终点
bool Move::IsMoveEnd()
{
#if 0
	DWORD ms = GetTickCount();
	if (ms < (m_dwIsEndTime + 100)) // 小于500豪秒 不判断
		return false;

	m_dwIsEndTime = ms;
#endif
	m_pGame->ReadCoor(&m_dwX, &m_dwY); // 读取当前坐标
	//printf("IsMoveEnd %d,%d %d,%d\n", m_dwX, m_dwY, m_dwMvX, m_dwMvY);
	return m_dwX == m_dwMvX && m_dwY == m_dwMvY;
}

// 是否移动
bool Move::IsMove()
{
	DWORD ms = GetTickCount();
	if (ms < (m_dwIsMvTime + 500)) // 小于500豪秒 不判断
		return true;

	m_dwIsMvTime = ms;

	if (m_pGame->m_GameAddr.MovSta) { // 此地址保存移动状态 0-未移动 1-在移动
		DWORD v = 0;
		m_pGame->ReadDwordMemory(m_pGame->m_GameAddr.MovSta, v);
		return v == 1;
	}

	if (!m_dwLastX || !m_dwLastY)
		return false;

	m_pGame->ReadCoor(&m_dwX, &m_dwY); // 读取当前坐标
	if (m_dwLastX == m_dwX && m_dwLastY == m_dwY) // 没有移动 1秒内坐标没有任何变化
		return false;

	m_dwLastX = m_dwX;
	m_dwLastY = m_dwY;
	m_dwMvTime = ms;
	return true;
}

