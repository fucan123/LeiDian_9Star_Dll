#include "Talk.h"
#include "Game.h"
#include <stdio.h>
#include <My/Common/mystring.h>

// ...
Talk::Talk(Game * p)
{
	m_pGame = p;
	InitData();
}

// 初始化数据
void Talk::InitData()
{
	m_bIsSearchNPC = false;
	ClearSearched();
}

// 清除搜索了的
void Talk::ClearSearched()
{
	m_dwIsSearchCount = 0;
}

// NPC
void Talk::NPC(DWORD npc_id)
{
	m_pGame->Call_NPC(npc_id);
}

// NPC
DWORD Talk::NPC(const char* name)
{
	printf("Talk::NPC:%s\n", name);
	DWORD npc_id = 0;
	for (int i = 1; i <= 2; i++) {
		printf("第%d次寻找NPC\n", i);
		npc_id = GetNPCId(name);
		if (npc_id) {
			NPC(npc_id);
			break;
#if 0
			if (WaitTalkBoxOpen()) {
				break;
			}
			else {
				printf("NPC可能无效.\n");
			}
#endif
		}
		Sleep(500);
	}
	return npc_id;
}

// NPC对话选择项
void Talk::NPCTalk(DWORD no, bool close)
{
	m_pGame->Call_NPCTalk(no, close);
}

// NPC对话状态[对话框是否打开]
bool Talk::NPCTalkStatus()
{
	if (!m_pGame->m_GameAddr.TalKBoxSta)
		return false;

	DWORD v = 0;
	m_pGame->ReadDwordMemory(m_pGame->m_GameAddr.TalKBoxSta, v);
	return v != 0;
}

// 是否选择邀请队伍
bool Talk::CheckTeamSta()
{
	// +29C0是TeamChkSta偏移头 +100是选择框状态
	DWORD v = 0;
	m_pGame->ReadDwordMemory(m_pGame->m_GameAddr.TeamChkSta + 0x29C0 + 0x100, v);
	return v != 0;
}

// 提示框是否打开
bool Talk::TipBoxStatus()
{
	if (!m_pGame->m_GameAddr.TipBoxSta)
		return false;

	DWORD v = 0;
	m_pGame->ReadDwordMemory(m_pGame->m_GameAddr.TipBoxSta + 0xA0, v);
	return v != 0;
}

// 关闭提示框
void Talk::CloseTipBox(int close)
{
	//m_pGame->Call_CloseTipBox(close);
}

// 等待对话框打开
bool Talk::WaitTalkBoxOpen()
{
	int i = 0;
	while (!NPCTalkStatus()) {
		if (i++ == 10) { // 超过10秒
			return false;
		}
		Sleep(100);
	}
	return true;
}

// 获取NPC信息
bool Talk::GetNPCInfo(PVOID addr, Player& info, bool isid)
{
	// 906,1052
	ZeroMemory(info.Name, sizeof(info.Name));
	DWORD ptr = (DWORD)addr;
	if (isid) {
		return m_pGame->ReadDwordMemory(ptr + 0xEC, info.Id);
	}
	else {
		if (!m_pGame->ReadDwordMemory(ptr + 0x9C, info.X))
			return false;
		if (!m_pGame->ReadDwordMemory(ptr + 0xA0, info.Y))
			return false;
		if (!m_pGame->ReadDwordMemory(ptr + 0xEC, info.Id))
			return false;
		if (!m_pGame->ReadMemory((PVOID)(ptr + 0x760), info.Name, sizeof(info.Name)))
			return false;

		info.Name[sizeof(info.Name) - 1] = 0;
		return true;
	}
}

// 检查NPC是否存在
bool Talk::CheckNPC(DWORD npc_id)
{
	if (!npc_id)
		return false;

	DWORD dwCount = ReadNPCCount();
	for (DWORD i = 0; i < dwCount; i++) {
		GamePlayer* pGamePlayer = (GamePlayer*)m_pGame->Call_GetPlayerByIndex(i);
		Player player;
		if (!GetNPCInfo(pGamePlayer, player)) {
			printf("无法获取NPC信息(%d) %08X\n", GetLastError(), pGamePlayer);
			continue;
		}
		if (player.Id == npc_id)
			return true;
	}

	return false;
}

// 获取NPCID
DWORD Talk::GetNPCId(const char* name)
{
	GamePlayer* p = NULL;
	ReadNPC(name, &p, 1, true);
	if (!p)
		return 0;

	Player player;
	if (!GetNPCInfo(p, player, true)) {
		printf("无法获取NPC信息[GetNPCId](%d) %08X\n", GetLastError(), p);
		return 0;
	}
	return player.Id;
}

// 读取NPC数量
DWORD Talk::ReadNPCCount()
{
#if 1
	return m_pGame->Call_GetPlayerAmount();
#else
	if (!g_objPlayerSet)
		return 0;

	DWORD count = 0;
	m_pGame->ReadDwordMemory(g_objPlayerSet + 0x14 + 0x2C, count);
	return count;
#endif
}

// 读取NPC
DWORD Talk::ReadNPC(const char* name, GamePlayer** save, DWORD save_count, bool no_repeat)
{
	//212,502 865,500
	DWORD dwSaveCount = 0;
	DWORD dwCount = ReadNPCCount();
	DWORD _ecx = g_objPlayerSet + 0x14;
	DWORD func = m_pGame->m_GameCall.GetNpcBaseAddr;
	printf("周围角色数量:%d\n", dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
#if 1
		GamePlayer* pGamePlayer = (GamePlayer*)m_pGame->Call_GetPlayerByIndex(i);
#else
		GamePlayer* pGamePlayer = (GamePlayer*)m_pGame->Call_GetBaseAddr(i, _ecx);
#endif
		Player player;
		if (!GetNPCInfo(pGamePlayer, player)) {
			printf("无法获取NPC信息(%d) %08X\n", GetLastError(), pGamePlayer);
			continue;
		}
		if (player.Id == 0x00) {
			printf("角色[%08X].Id==0:%s %08X 坐标:%d,%d\n", pGamePlayer, player.Name, player.Id, player.X, player.Y);
			continue;
		}	

		DWORD life = player.Life;
		printf("角色[%08X]:%s %08X 坐标:%d,%d\n", pGamePlayer, player.Name, player.Id, player.X, player.Y);
		if (!save)
			continue;

		bool is_repeat = false;
		bool is_save = true;
		if (name) {
			is_save = false;
			if (strcmp(player.Name, name) == 0) {
				//printf("NPC[%08X]:%s %08X\n", addr, npc_name, npc_id);
				if (no_repeat) { // 不允许重复
					for (DWORD j = 0; j < m_dwIsSearchCount; j++) { // 已经搜索了的 不再使用
						if (m_ListIsSearch[j] == player.Id) {
							printf("角色重复:%s %08X\n", player.Name, player.Id);
							is_repeat = true;
							break;
						}
					}
				}
				is_save = true;
			}
		}
		if (is_save) {
			save[dwSaveCount++] = pGamePlayer;
			if (is_repeat) { // 重复的数据
				dwSaveCount--;
			}
			else {
				if (no_repeat) { // 不允许重复
					m_ListIsSearch[m_dwIsSearchCount++] = pGamePlayer->Id;
				}
			}
			if (dwSaveCount == save_count)
				break;
		}
	}
	return dwSaveCount;
}

// 获取角色当前血量
DWORD Talk::GetLife(GamePlayer* p)
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
		mov  eax, [ebp + 0xC]       // mov eax,dword ptr ss:[ebp+0xC]
		mov  eax, [eax + esi * 8]   // 血量
		pop  ebp
	}
}

// 获取远程邀请人物信息
DWORD Talk::ReadRemotePlayer(const char* name)
{
	for (int i = 0; ; i++) {
		DWORD addr = m_pGame->Call_QueryRemoteTeam(i);
		//printf("%d. %08X\n",i , addr);
		//continue;
		if (!addr) {
			printf("没有了\n");
			break;
		}

		const char* name = (const char*)P2DW(addr + 0x08);
		printf("%d.%s\n", i, name);
	}
	return 0;
}

// 读取怪物
bool Talk::ReadNPC_Old()
{
	//printf("搜索NPC\n");
	// 搜索方法:找到NPCID->CE搜索查找合适地址
	// 4:0x03E5D678 4:* 4:* 4:* 4:* 4:0x00 4:0x00 4:0xFFFFFFDB
	// eax=25B74A38 ecx=0361BE48
	// CRole::IsPlayer esi==0656B710 edx=260679DC ebx=25F96DE8 25F96D68
	DWORD codes[] = {
		0x1234D678, 0x00000011, 0x00000011, 0x00000011,
		0x00000011, 0x00000000, 0x00000000, 0xFFFFFFDB,
	};

	DWORD address[16];
	DWORD count = m_pGame->SearchCode(codes, sizeof(codes) / sizeof(DWORD), address, 16);
	if (count) {
		//printf("NPC列表:%d\n", count);
		DWORD num = 0, near_index = 0xff, near_dist = 0;
		for (DWORD i = 0; i < count; i++) {
			try {
				DWORD dwRepeatId = 0;
				char* name = (char*)((DWORD)address[i] + 0x50);
				DWORD id = PtrToDword(address[i] + 0xD0);
				GameNPC* pNPC = (GameNPC*)address[i];
				if (1 || id) {
					printf("%02d[%08X].%s[%08X]\n", i + 1, pNPC, name, id);
					num++;
				}

				if (m_bIsSearchNPC) {
					if (id && strcmp(m_chSearchNPCName, name) == 0) {
						bool no_search = true;
						for (DWORD j = 0; j < m_dwIsSearchCount; j++) { // 已经搜索了的 不再使用
							if (m_ListIsSearch[j] == id) {
								printf("NPCID:%08X重复\n", id);
								no_search = false;
								dwRepeatId = id;
								break;
							}
						}
						if (no_search) {
							printf("NPCID:%08X\n", id);
							m_dwSearchNPCId = id;
							m_bIsSearchNPC = false;
							m_ListIsSearch[m_dwIsSearchCount++] = id;
							return false;
						}
					}
				}
				if (dwRepeatId)
					m_dwSearchNPCId = dwRepeatId;
			}
			catch (void*) {
			}
		}

	}

	return true;
}

