#include "GuaiWu.h"
#include "Game.h"
#include "Talk.h"
#include <stdio.h>
#include <My/Common/mystring.h>

// ...
GuaiWu::GuaiWu(Game * p)
{
	m_pGame = p;
	InitData();
}

// 初始化数据
void GuaiWu::InitData()
{
	m_bSearchName = false;
	InitAttack();
}

// 自身周围是否有怪物
bool GuaiWu::HasInArea(DWORD cx, DWORD cy)
{
	m_bIsClear = true;
	m_dwCX = cx;
	m_dwCY = cy;
	m_pGame->ReadGameMemory(0x10); // 获取怪物列表
	m_bIsClear = false;
	bool result = m_pAttack != nullptr;
	InitAttack();
	return result;
}

// 怪物是否在自身区域内 x,y=自身范围(IN),怪物坐标(OUT)
bool GuaiWu::IsInArea(const char* name, IN OUT DWORD& x, IN OUT DWORD& y)
{
	//printf("要搜索的怪物:%s %d,%d\n", name, x, y);
	GamePlayer* p = NULL;
	m_pGame->m_pTalk->ReadNPC(name, &p, 1, false);
	if (!p)
		return false;

	m_pGame->ReadCoor();
	DWORD cx = abs((int)m_pGame->m_dwX - (int)p->X);
	DWORD cy = abs((int)m_pGame->m_dwY - (int)p->Y);
	bool result = cx <= x && cy <= y;
	if (result) {
		x = p->X;
		y = p->Y;
	}
	return result;
}

// 清理怪物
bool GuaiWu::Clear(const char* magic_name, DWORD cx, DWORD cy)
{
	return false;
	m_bIsClear = true;
	m_dwCX = cx;
	m_dwCY = cy;

	while (true) { // 已清理完毕
		if (IsIgnoreAttack()) // 忽略正在攻击怪物
			m_pGame->ReadGameMemory(0x10); // 获取怪物列表

		if (!m_pAttack)
			break;

		try {
			printf("GuaiWu::Clear->UseMagic:%s %d,%d\n", magic_name, m_pAttack->X, m_pAttack->Y);
			m_pGame->m_pMagic->UseMagic(magic_name, m_pAttack->X, m_pAttack->Y, m_pAttack->Id);
		}
		catch (...) {
			printf("GuaiWu::Clear怪物无效\n");
			break;
		}
		Sleep(600);
	}

	m_bIsClear = false;
	return true;
}

// 初始化被攻击的怪物
void GuaiWu::InitAttack()
{
	m_pAttack = nullptr;
	m_i64AttackTime = 0;
}

// 设置被攻击怪物
void GuaiWu::SetAttack(GamePlayer * p)
{
	m_pAttack = p;
	m_i64AttackTime = getmillisecond();
}

// 是否忽略正在攻击怪物
bool GuaiWu::IsIgnoreAttack()
{
	try {
		if (!m_pAttack)  // 没有正在攻击的
			throw nullptr;
		if (!m_pAttack->Id || !m_pAttack->X || !m_pAttack->Y) { // 信息无效
			throw nullptr;
		}
		if (!GetLife(m_pAttack, 0)) { // 无血量
			printf("GuaiWu::IsIgnoreAttack怪物血量为0\n");
			throw nullptr;
		}
		return false;
	}
	catch (...) {
		InitAttack();
		return true;
	}
}

// 读取怪物
bool GuaiWu::ReadGuaiWu()
{
	// 037F3C05
	// 037D81B0
	// 4:* 4:0x00 4:0xFFFFFFFF 4:0x01 4:0x00 4:0x00 4:* 4:0x00
	DWORD codes[] = {
		0x1234AA40, 0x00000000, 0xFFFFFFFF, 0x00000001,
		0x00000000, 0x00000000, 0x1234F320, 0x00000000
	};

	//偏移15CC处+0C是血量地址

	DWORD address[16];
	DWORD count = m_pGame->SearchCode(codes, sizeof(codes) / sizeof(DWORD), address, 16);

	if (count) {
		m_pGame->ReadCoor();
		//printf("\n---------------------------\n");
		//printf("[%d]怪物数量：%d(%d)\n", (int)time(nullptr), count, m_dwGuaiWuCount + count);
		DWORD num = 0, near_index = 0xff, near_dist = 0;
		for (DWORD i = 0; i < count; i++) {
			//if (address[i] != ((DWORD)m_pGuaiWus[i + m_dwGuaiWuCount])) {
				//printf("不相等%d!=%d\n", i, i + m_dwGuaiWuCount);
			//}
			try {
				GamePlayer* pGuaiWu = (GamePlayer*)address[i];
				//printf("怪物地址:%08X\n", pGuaiWu);
				//continue;
				if (pGuaiWu->X > 0 && pGuaiWu->Y > 0 && pGuaiWu->Type) {
					char* name = (char*)((DWORD)address[i] + 0x520);
					DWORD life = GetLife(pGuaiWu, i);

					printf("%02d[%08X].%s[%08X]: x:%X[%d] y:%X[%d] 类型:%X 血量:%d\n", i + 1, pGuaiWu, name, pGuaiWu->Id, pGuaiWu->X, pGuaiWu->X, pGuaiWu->Y, pGuaiWu->Y, pGuaiWu->Type, life);

					DWORD cx = abs((int)m_pGame->m_dwX - (int)pGuaiWu->X);
					DWORD cy = abs((int)m_pGame->m_dwY - (int)pGuaiWu->Y);

					if (m_bSearchName) { // 搜索怪物名称
						if (cx <= m_dwCX && cy <= m_dwCY) {         // 在搜索范围内
							if (strcmp(name, m_sSearChName) == 0) { // 符合名字一致
								printf("搜索到的怪物:%08X, %d,%d\n", pGuaiWu->Id, pGuaiWu->X, pGuaiWu->Y);
								m_dwCX = pGuaiWu->X;
								m_dwCY = pGuaiWu->Y;
								m_bSearchName = false;
								return false;
							}
						}
					}
					if (m_bIsClear) { // 清怪
						printf("%d,%d %d,%d\n", cx, cy, m_dwCX, m_dwCY);
						if (cx <= m_dwCX && cy <= m_dwCY) { // 在攻击范围内
							printf("选择攻击怪物:%08X\n", pGuaiWu->Id);
							m_pAttack = pGuaiWu;
							return false;
						}
					}
					num++;
				}
			}
			catch (void*) {
			}
		}

	}

	return true;
}

// 获取怪物当前血量
DWORD GuaiWu::GetLife(GamePlayer* p, DWORD no)
{
	// 搜索方式:CE找出怪物血量, 下访问断点->OD访问地址下断点, 调用堆栈返回到CPlayer::GetData->调试即可
	// 直接得到CPlayer::GetData里面查看
	__asm {
		push ebp
		mov  ebp, dword ptr p
		mov  ebp, [ebp + 0x15FC]    // mov ecx,dword ptr ds:[edx+0x15FC]
		mov  esi, [ebp + 0x8]       // mov esi,dword ptr ss:[ebp+0x8]
		sub  esi, 1                 // sub esi,ecx[ecx应该固定是1]
		dec  esi                    // dec esi
		mov  eax, dword ptr no
		mov  edx, 0
		sub  eax, edx
		mov  esi, eax
		sar  esi, 1
		mov  eax, [ebp + 0xC]       // mov eax,dword ptr ss:[ebp+0xC]
		mov  eax, [eax + esi * 8]   // 血量
		pop  ebp
	}
}
