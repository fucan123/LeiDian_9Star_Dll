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

// ��ʼ������
void Move::InitData()
{
	ClearMove();
}

// �ƶ�
bool Move::Run(DWORD x, DWORD y)
{
	if (x == m_dwMvX && y == m_dwMvY) {
		if ((GetTickCount() - m_dwMvTime) < 1000)
			return false;
	}

	//printf("Move::Run:%d,%d �Լ�λ�ã�%d,%d\n", x, y, m_dwX, m_dwY);
	SetMove(x, y);
	m_pGame->Call_Run(x, y);

	return true;
}

// �����ƶ�λ��
void Move::SetMove(DWORD x, DWORD y)
{
	m_pGame->ReadCoor(&m_dwLastX, &m_dwLastY); // ��ȡ��ǰ����

	m_dwMvX = x;
	m_dwMvY = y;
	m_dwMvTime = GetTickCount();

	int now_time = time(nullptr);
	if ((now_time - m_iGetPosTime) < 2) // 2���ڲ�ȡ�仯
		return;

	if (1 && m_dwCmpX && m_dwCmpY) {
		if (m_dwCmpX == m_dwLastX && m_dwCmpY == m_dwLastY) { // �������ϴ�һ��
			if ((now_time - m_iGetPosTime) < 5) { // ��ȡ����ʱ�������
				if ((now_time - m_iFlagPosTime) > 30) { // 30�����껹һ����������
					printf("���ܿ�ס�������\n");
					m_pGame->m_pGameProc->NPCLast();
				}

				int t_out = m_pGame->m_iQiaZhuS >= 30 ? m_pGame->m_iQiaZhuS : 30;
				if ((now_time - m_iFlagPosTime) >= t_out) {
					m_pGame->m_pGameProc->SendMsg("���ܿ�ס�������");
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

// ����ƶ�����
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

// �Ƿ�ﵽ�յ�
bool Move::IsMoveEnd()
{
#if 0
	DWORD ms = GetTickCount();
	if (ms < (m_dwIsEndTime + 100)) // С��500���� ���ж�
		return false;

	m_dwIsEndTime = ms;
#endif
	m_pGame->ReadCoor(&m_dwX, &m_dwY); // ��ȡ��ǰ����
	//printf("IsMoveEnd %d,%d %d,%d\n", m_dwX, m_dwY, m_dwMvX, m_dwMvY);
	return m_dwX == m_dwMvX && m_dwY == m_dwMvY;
}

// �Ƿ��ƶ�
bool Move::IsMove()
{
	DWORD ms = GetTickCount();
	if (ms < (m_dwIsMvTime + 500)) // С��500���� ���ж�
		return true;

	m_dwIsMvTime = ms;

	if (m_pGame->m_GameAddr.MovSta) { // �˵�ַ�����ƶ�״̬ 0-δ�ƶ� 1-���ƶ�
		DWORD v = 0;
		m_pGame->ReadDwordMemory(m_pGame->m_GameAddr.MovSta, v);
		return v == 1;
	}

	if (!m_dwLastX || !m_dwLastY)
		return false;

	m_pGame->ReadCoor(&m_dwX, &m_dwY); // ��ȡ��ǰ����
	if (m_dwLastX == m_dwX && m_dwLastY == m_dwY) // û���ƶ� 1��������û���κα仯
		return false;

	m_dwLastX = m_dwX;
	m_dwLastY = m_dwY;
	m_dwMvTime = ms;
	return true;
}

