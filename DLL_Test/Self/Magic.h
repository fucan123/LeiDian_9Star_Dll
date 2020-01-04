#pragma once
#include "GameStruct.h"

#define MAX_MAGIC_COUNT 100

// 技能调用哪个CALL
enum MagicCallType{
	MCT_POS  = 0x00, // 调用传坐标CALL
	MCT_GWID = 0x01, // 调用传怪物IDCALL
};

typedef struct game_magic
{
	DWORD First;         // 不知道是什么
	DWORD Id;            // 技能ID
	DWORD UnKnow;        // 不知道
	CHAR* Name;          // 技能名字
	BYTE  UNknow2[0x4C]; // 不知道为什么
	DWORD Cd;            // 冷却时间
} GameMagic;

typedef struct _magic_
{
	DWORD Id;
	DWORD Cd;
	CHAR  Name[32];
} _Magic_;

typedef struct magic_info
{
	CHAR          Name[32];   // 技能名称
	DWORD         Id;         // 技能Id
	DWORD         Cd;         // 所需冷却时间[毫秒]
	__int64       UseTime;    // 使用时间[毫秒]
	__int64       CdTime;     // 冷却时间
	
} MagicInfo;

class Game;

class Magic
{
public:
	// ...
	Magic(Game* p);
	// 使用技能
	bool UseMagic(const char* name, DWORD x, DWORD y, DWORD gwid=0);
	// 检查冷却时间[allow_ms=允许最大还剩此冷却]
	bool CheckCd(const char* name, DWORD allow_ms=0, DWORD* magic_id=nullptr);
	// 检查冷却时间[allow_ms=允许最大还剩此冷却]
	bool CheckCd(DWORD id, DWORD allow_ms=0);
	// 检查冷却时间[allow_ms=允许最大还剩此冷却]
	bool CheckCd(MagicInfo* p, DWORD allow_ms = 0);
	// 获取技能信息
	MagicInfo* GetMagicInfo(const char* name);
	// 获取技能信息
	MagicInfo* GetMagicInfo(DWORD Id);
	// 技能是否为坐标技能
	bool MagicIsPos(const char* name);
	// 读取技能数量
	int ReadMagicLength(bool flag);
	// 读取技能
	bool ReadMagic(const char* name=nullptr, _Magic_* save = nullptr, bool flag=true);
private:
	// 添加技能信息
	MagicInfo* AddMagicInfo(CHAR* name, DWORD id, DWORD cd);
public:
	// 游戏类
	Game* m_pGame;
	// 技能熟练
	DWORD m_dwMagicNum;
	// 技能信息列表数量
	DWORD m_dwMagicInfoCount;
	// 技能信息列表
	MagicInfo m_ListMagicInfo[MAX_MAGIC_COUNT];
	// 正在攻击的怪物Id
	DWORD m_dwAttackGWId;
	// 技能坐标X
	DWORD m_dwMagicX;
	// 技能坐标Y
	DWORD m_dwMagicY;

	CHAR m_Test[0x1000];
};