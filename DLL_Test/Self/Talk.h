#pragma once
#include "GameStruct.h"

// NPC信息
typedef struct game_npc {
	DWORD First[20]; // 未知数值
	CHAR* Name;      // 名称
	DWORD Mid[35];   // 位置数值
	DWORD Id;        // ID
} GameNPC;

class Game;

class Talk
{
public:
	// ...
	Talk(Game* p);

	// 初始化数据
	void InitData();
	// 清除搜索了的
	void ClearSearched();
	// NPC
	void NPC(DWORD npc_id);
	// NPC
	DWORD NPC(const char* name);
	// NPC对话选择项
	void NPCTalk(DWORD no, bool close = true);
	// NPC对话状态[对话框是否打开]
	bool NPCTalkStatus();
	// 是否选择邀请队伍
	bool CheckTeamSta();
	// 提示框是否打开
	bool TipBoxStatus();
	// 关闭提示框
	void CloseTipBox(int close=0);
	// 等待对话框打开
	bool WaitTalkBoxOpen();
	// 获取NPC信息
	bool GetNPCInfo(PVOID addr, Player& info, bool isid=false);
	// 检查NPC是否存在
	bool CheckNPC(DWORD npc_id);
	// 获取NPCID
	DWORD GetNPCId(const char* name);
	// 读取NPC数量
	DWORD ReadNPCCount();
	// 读取NPC
	DWORD ReadNPC(const char* name=nullptr, GamePlayer** save=nullptr, DWORD save_count=0, bool no_repeat=true);
	// 获取当前角色血量
	DWORD GetLife(GamePlayer* p);
	// 获取远程邀请人物信息
	DWORD ReadRemotePlayer(const char* name = nullptr);
	// 读取周围NPC
	bool ReadNPC_Old();
public:
	// 游戏类
	Game* m_pGame;

	// 是否搜索NPCID
	bool m_bIsSearchNPC;
	// 搜索的NCP名称
	char m_chSearchNPCName[32];
	// 搜索到的NPCId
	DWORD m_dwSearchNPCId;
	// 已经搜索了的NPCId数目
	DWORD m_dwIsSearchCount;
	// 已经搜索了的NPCId列表
	DWORD m_ListIsSearch[32];

	DWORD m_Test[0x1000];
};