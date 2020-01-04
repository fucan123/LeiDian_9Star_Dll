#pragma once
#include "GameStep.h"
#include "GameStruct.h"
#include <My/Common/Link.hpp>
#include <Windows.h>
#include <vector>
#include <fstream>

using namespace std;

#define ONE_COOR_PIX     50   // 一坐标占用像数
#define ONE_MAX_MOV_COOR 8    // 一次最大可移动8距离坐标
#define ONE_MOV_MAX_MS   1000 // 一次移动最大允许时间(毫秒)

#define BAG_NUM         40       // 包包容量
#define WUPIN_YAOBAO    0x0B5593 // 药包
#define WUPIN_XIANGLIAN 0x0B5B24 // 爱娜祈祷项链
#define WUPIN_YAO       0x0F6982 // 药
#define WUPIN_LINGYAO   0x0F943E // 速效圣兽灵药

#define RUNRUN 1
struct _step_;

class Game;
class GameStep;
class GameProc
{
public:
	// !!!
	GameProc(Game* pGame);

	// 初始化数据
	void InitData();
	// 取消所有旗帜按钮
	void CancelAllButton();
	// 执行
	void Exec();
	// 运行
	void Run();
	// 邀请入队
	int ViteInTeam(const char* name);
	// 入队
	void InTeam();
	// 邀请进副本
	void ViteInFB();
	// 同意进副本
	void AgreeInFB();
	// 神殿去雷鸣大陆流程
	void GoLeiMing();
	// 去领取项链
	void GoGetXiangLian(bool is_get=true);
	// 去副本门口
	void GoFBDoor();
	// 进副本
	bool GoInFB();
	// 继续副本
	void ContinueInFB();
	// 出副本
	void GoOutFB(const char* name, bool notify=true, bool reborn=true);
	// 执行副本流程
	void ExecInFB();
#if USE_MY_LINK
	// 执行流程
	bool ExecStep(Link<_step_*>& link);
#else
	// 执行流程
	bool ExecStep(vector<_step_*>& link);
#endif
	// 步骤是否已执行完毕
	bool StepIsComplete();
	// 移动
	void Move();
	// NCP
	void NPC();
	// 最后一个对话的NPC
	void NPCLast();
	// 选择
	void Select();
	// 技能
	void Magic();
	// 技能-宠物
	void MagicPet();
	// 狂甩
	void Crazy();
	// 清怪
	void Clear();
	// 捡物
	void PickUp();
	// 存钱
	void SaveMoney();
	// 存物
	DWORD CheckIn(bool in=true);
	// 使用物品
	void UseItem();
	// 扔物品
	void DropItem();
	// 售卖物品
	void SellItem();
	// 按钮
	void Button();
	// 按钮
	bool Button(int button_id, DWORD sleep_ms=0, const char* name=nullptr);
	// 等待
	void Wait();
	// 等待
	void Wait(DWORD ms);
	// 小号
	void Small();
	// 复活
	void ReBorn(int flag=0);
	// 是否检查此NPC对话完成
	bool IsCheckNPC(const char* name);
	// 是否在副本
	bool IsInFB();
	// 读取人物坐标
	bool ReadCoor();
	// 读取人物血量
	bool ReadLife();
	// 读取快捷键上面物品数量
	bool ReadQuickKey2Num();
	// 读取包包物品
	bool ReadBag();
	// 是否需要加血量
	int  IsNeedAddLife();
	// 加血
	void AddLife();
	// 发送信息给服务端
	void SendMsg(const char* v, const char* v2=nullptr);
	// 停止
	void Stop(bool v=true);
	// 打开日记文件
	void OpenLogFile();
	// 写入日记
	void WriteLog(const char* log, bool flush=true);
public:
	// 游戏指针
	Game*     m_pGame;
	// 游戏步骤指针
	GameStep* m_pGameStep;
	// 正在执行的步骤
	_step_* m_pStep;
	// 正在执行的步骤[副本]
	_step_* m_pStepCopy;
	// 选择哪个配置文件
	int  m_iChkFile = 2;
	// 是否停止
	bool  m_bStop = false;
	// 是否暂停
	bool  m_bPause = false;
	// 是否重新开始
	bool  m_bReStart = false;
	// 是否狂甩
	bool m_bIsCrazy = false;
	// 狂甩技能
	char m_CrazyMagic[32];

	// 是否清掉凯瑞
	DWORD m_dwKairi = 0;
	// 搜索凯瑞时间
	__int64 m_i64SearchKairi;

	// 上一次执行步骤的相关信息
	struct {
		DWORD MvX;         // 移动X
		DWORD MvY;         // 移动Y
		DWORD NPCId;       // NPCId
		CHAR  NPCName[32]; // NPC名称
		CHAR  Magic[32];   // 技能
		bool  IsOut;       // 是否出副本
	} m_stLastStepInfo;
	// 人物坐标
	DWORD   m_iCoorX = 0;
	DWORD   m_iCoorY = 0;
	// 人物移动到的坐标
	int   m_iMovCoorX = 0;
	int   m_iMovCoorY = 0;
	// 人物血量
	int   m_iLife = 0;
	int   m_iLifeMax = 0;
	// 快捷键上物品数量
	int   m_QuickKey2Nums[2];
	// 包包物品
	DWORD m_dwBag[BAG_NUM];
	// 加血时间
	DWORD m_dwAddLifeTime = 0;

	// 是否锁定进入副本
	bool m_bLockGoFB = false;
	// 是否在副本里面
	bool m_bAtFB = false;
	// 保留治疗药数量
	int  m_iNeedYao = 6;
	// 保留治疗包数量
	int  m_iNeedBao = 6;

	// 是否最后BOSS
	bool m_bLastBoss = false;
	// 是否发送出去副本
	bool m_bSendOut = false;

	ofstream m_LogFile;
};
