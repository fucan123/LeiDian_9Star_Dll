#pragma once
#include "GameStruct.h"

class Game;

class Move
{
public:
	// ...
	Move(Game* p);

	// ��ʼ������
	void InitData();
	// �ƶ�
	bool Run(DWORD x, DWORD y);
	// �����ƶ�λ��
	void SetMove(DWORD x, DWORD y);
	// ����ƶ�����
	void ClearMove();
	// �Ƿ�ﵽ�յ�
	bool IsMoveEnd();
	// �Ƿ��ƶ�
	bool IsMove();
public:
	// ��Ϸ��
	Game* m_pGame;
	// ��ǰλ��X
	DWORD m_dwX = 0;
	// ��ǰλ��Y
	DWORD m_dwY = 0;
	// �ϴ�λ��X
	DWORD m_dwLastX = 0;
	// �ϴ�λ��Y
	DWORD m_dwLastY = 0;
	// �ƶ�λ��X
	DWORD m_dwMvX;
	// �ƶ�λ��Y
	DWORD m_dwMvY;
	// �ж��ƶ�����ʱ��
	DWORD m_dwIsEndTime;
	// �ж��Ƿ��ƶ�ʱ��
	DWORD m_dwIsMvTime;
	// ���ڱȽϵ�X
	DWORD m_dwCmpX;
	// ���ڱȽϵ�Y
	DWORD m_dwCmpY;
	// �ƶ�ʱ��
	DWORD m_dwMvTime;
	// ��ȡ����ʱ��
	int   m_iGetPosTime;
	// ��ȡ������ʱ��
	int   m_iFlagPosTime;

	CHAR m_Test[0x1000];
};