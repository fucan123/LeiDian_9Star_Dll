#include "GameClient.h"
#include "GameProc.h"
#include "Game.h"
#include "GameConf.h"
#include "Move.h"
#include "Item.h"
#include "GuaiWu.h"
#include "Talk.h"
#include "Magic.h"
#include "Pet.h"

#include <ShlObj_core.h>
#include <My/Common/mystring.h>
#include <My/Common/func.h>
#include <My/Common/C.h>
#include <psapi.h>
#include <stdio.h>
#include <time.h>

#if 1
#if 0
#define SMSG_D(v) m_pGame->m_pClient->SendMsg2(v)
#else
#define SMSG_D(v) WriteLog(v);
#endif
#define SMSG_DP(p,...) { sprintf_s(p,__VA_ARGS__);SMSG_D(p); }
#else
#define SMSG_D(v) 
#define SMSG_DP(v) 
#endif
#define SMSG(v) SendMsg(v)
#define SMSG_P(p,...) { sprintf_s(p,__VA_ARGS__);SMSG(p); }
#define SMSG_N(n,...) {char _s[n]; SMSG_P(_s,__VA_ARGS__); }

// !!!
GameProc::GameProc(Game* pGame)
{
	m_pGame = pGame;
	m_pGameStep = new GameStep;
}

// 初始化数据
void GameProc::InitData()
{
	m_bStop = false;
	m_bPause = false;
	m_bReStart = false;
	m_i64SearchKairi = 0;
	m_bLockGoFB = false;
	m_bAtFB = false;
	m_iNeedYao = 6;
	m_iNeedBao = 6;
	m_pStepCopy = nullptr;

	ZeroMemory(&m_stLastStepInfo, sizeof(m_stLastStepInfo));
	m_pGame->m_pMove->InitData();
	m_pGame->m_pItem->InitData();
	m_pGame->m_pTalk->InitData();
	m_pGame->m_pGuaiWu->InitData();
	m_pGame->m_pPet->InitData();
}

// 取消所有旗帜按钮
void GameProc::CancelAllButton()
{
	if (!m_pGame->IsBig()) {
		for (int i = 1; i <= 10; i++) {
			if (Button(BUTTON_ID_TEAMFLAG)) {
				Sleep(2000);
				Button(BUTTON_ID_CANCEL, 1500);
			}
			else {
				break;
			}
		}
	}
	
	Button(BUTTON_ID_CANCEL, 1500);
	Button(BUTTON_ID_CLOSEMENU, 1500);
}

// 执行
void GameProc::Exec()
{
#if 0
	while (true) {
		//m_pGame->m_pMagic->UseMagic("虚无空间", 66, 60);
		//m_pGame->Keyborad2(VK_F4, NULL);
		m_pGame->ReadCoor();
		m_pGame->m_pTalk->NPC("尤兰达");
		Sleep(10000);
	}
#endif

	int have_team = m_pGame->Call_IsHaveTeam();
	int leader = 0;
	int have_team2 = have_team;
	GoLeiMing();
	
	CancelAllButton();
	if (m_pGame->IsInFB()) { // 在副本里面
		if (have_team) { // 有队伍, 继续刷
			m_bAtFB = true;
			return;
		}
		else {
			GoOutFB("卡利亚堡传送门", false);
		}
	}

	if (m_pGame->m_Account.CreateTeam) {
		if (!have_team) {
			m_pGame->Call_TeamCreate(); // 需要创建队伍
			Sleep(1000);
		}
		have_team = m_pGame->Call_IsHaveTeam();
		leader = m_pGame->Call_IsTeamLeader();
		printf("是否有队伍:%d 队长:%d\n", have_team, leader);
	}
	if (!m_pGame->m_Account.IsBig) {
		printf("是否领了项链:%d\n", m_pGame->m_Account.IsGetXL);
		if (!m_pGame->m_Account.IsGetXL) {
			printf("去领项链\n");
			GoGetXiangLian(); // 领取项链
		}
	}
		
	GoFBDoor(); // 去副本门口
	if (!have_team && !leader) {
		// 告诉可以组队了
		m_pGame->m_pClient->Send(SCK_CANTEAM, true);
	}
	if (have_team2 || leader) {
		m_pGame->m_pClient->Send(SCK_CANINFB, true);
	}
}

// 运行
void GameProc::Run()
{
	//return;
	//m_pGame->KeyDown('I');
	//Sleep(100);
	//m_pGame->KeyUp('I');
	char log[64];
	//INLOGVARN(32, "反：%.2f", asin(0.5f));
#if RUNRUN == 0
	return;
#endif
#if 0
	m_bPause = true;
	printf("选择配置文件：A:魔域副本流程.txt B:魔域副本流程2.txt\n");
	while (m_bPause) {
		Sleep(500);
	}
#endif
	m_pGame->m_pGameConf->ReadConf(m_pGame->m_chPath);
	if (!m_pGameStep->InitSteps(m_pGame->m_chPath, m_pGame->m_pGameConf->m_Setting.FBFile)) {
		printf("初始化步骤失败，无法继续运行！！！\n");
		return;
	}
start:
	InitData();
	//m_bPause = true;
	if (m_bPause) {
		printf("按C开始\n");
	}
	while (m_bPause) {
		Sleep(500);
	}
	//ReadQuickKey2Num();
	//INLOGVARP(log, "快捷键上物品数量:%d %d", m_QuickKey2Nums[0], m_QuickKey2Nums[1]);
	m_dwKairi = 0;
	int n = 0;
	int exec_time = time(nullptr) + 2;
	while (true) {
		int c = exec_time - time(nullptr);
		if (c >= 0 && n != c) {
			printf("准备开始执行[%02d秒]\n", c);
		}
		n = c;
		if (c < 0) {
			break;
		}
		Sleep(500);
	}

	printf("开始执行流程\n");
	m_pGame->m_pPet->PetOut(m_pGame->m_pGameConf->m_stPetOut.No, m_pGame->m_pGameConf->m_stPetOut.Length, true);
	Exec();
	//Drv_MouseMovAbsolute(m_pGame->m_GameWnd.Rect.left, m_pGame->m_GameWnd.Rect.top);
	//INLOGVARN(32, "Step:%08x Stop:%d", step, m_bStop);
	bool send_at = false, send_out = true;
	int start_time = time(nullptr);
	while (true) {
		if (m_bAtFB || m_pGame->IsInFB()) {
			if (!send_at) {
				m_pGame->m_pClient->Send(SCK_ATFB, true); // 通知服务端已在副本里
				send_at = true;
			}
			
			InitData();
			ExecInFB(); // 执行副本流程

			//m_pGame->ReadCoor();
		}
		else {
			send_at = false;
		}
		Sleep(1000);
	}
	//GoLeiMing();
	//while (ExecStep(m_pGameStep->m_Step));
end:
	int second = time(nullptr) - start_time;
	printf("完成，总用时:%02d分%02d秒\n", second/60, second%60);
	if (m_bReStart) {
		printf("GameProc::Run重新加载...\n");
		Run();
	}
	else {
		m_pGameStep->ResetStep();
		m_bPause = true;
		//goto start;
	}
}

// 邀请入队
int GameProc::ViteInTeam(const char* name)
{
	if (strcmp(name, m_pGame->m_Account.Role) == 0)
		return -1;
	if (!m_pGame->Call_IsTeamLeader()) // 不是队长
		return -1;

	printf("邀请玩家:%s入队\n", name);
	for (int i = 0; i < 10; i++) {
		DWORD id = m_pGame->m_pTalk->GetNPCId(name);
		if (id) {
			m_pGame->Call_TeamInvite(id);
			printf("已邀请玩家:%s入队\n", name);
			return 1;
		}
		Sleep(1000);
	}
	printf("玩家:%s找不到\n", name);
	return 0;
}

// 入队
void GameProc::InTeam()
{
	for (int i = 1; i <= 10; i++) {
		printf("第%d次寻找入队旗帜按钮\n", i);
		if (Button(BUTTON_ID_TEAMFLAG))
			break;

		Sleep(1000);
	}

	Sleep(1000);
	Button(BUTTON_ID_INTEAM,    1500, "同意");
	Button(BUTTON_ID_CLOSEMENU, 0);
	printf("同意入队完成\n");
	SendMsg("同意入队");
}

// 邀请进副本
void GameProc::ViteInFB()
{
	printf("邀请进副本\n");
	Wait(2 * 1000);
	m_pGame->Call_CheckTeam();
	Wait(1 * 1000);
	for (int i = 0; i < 5; i++) {
		if (Button(BUTTON_ID_SURE))
			break;
		Sleep(1000);
	}

	m_bAtFB = true;
	SendMsg("邀请进副本");
}

// 同意进副本
void GameProc::AgreeInFB()
{
	if (m_bAtFB) {
		SendMsg("已经在副本, 不用同意");
		return;
	}
	
	int now_time = time(nullptr);
	while (m_bLockGoFB) {
		if ((time(nullptr) - now_time) > 60)
			break;

		printf("等待同意进入副本\n");
		Sleep(1500);
	}
	printf("同意进入副本\n");
	SendMsg("同意进入副本");
	m_pGame->m_Account.IsMeOpenFB = false;
	for (int i = 0; i < 3; i++) {
		Sleep(1000);
		if (Button(BUTTON_ID_INFB, 0, "同意")) {
			Wait(3 * 1000);
			m_bAtFB = true;
			printf("已同意\n");
			return;
		}	
	}
	printf("找不到同意按钮\n");
	SendMsg("找不到同意进副本按钮");
}

// 神殿去雷鸣大陆流程
void GameProc::GoLeiMing()
{
	if (!m_pGame->IsInMap("神殿")) {
		printf("不在神殿里面\n");
		return;
	}

	SendMsg("去雷鸣大陆");
	printf("在神殿里面\n");
	while (ExecStep(m_pGameStep->m_GoLeiMingStep));
	m_pGameStep->ResetStep();
}

// 去领取项链
void GameProc::GoGetXiangLian(bool is_get)
{
	int x = 250, y = 500;
	if (m_pGame->IsGetXL())
		goto end;

	SendMsg("去领项链");
	if (m_pGame->IsInArea(x, y))
		goto get;

	for (int j = 0; j < 5; j++) {
		if (!m_pGame->m_pItem->UseSelfItem("星辰之眼", x, y, 10)) {
			SendMsg("没有星辰之眼, 无法领项链");
			goto end;
		}
		for (int i = 0; i < 5; i++) {
			Sleep(1000);
			if (m_pGame->IsInArea(x, y))
				goto get;
		}
	}

get:
	if (!is_get)
		return;

	m_pGame->m_pTalk->NPC("幻宠师·艾黎");
	Wait(2 * 1000);
	m_pGame->m_pTalk->NPCTalk(0x03); // 领取特制经验球

	// 每天领3次项链
	m_pGame->m_pTalk->NPC("幻宠师·艾黎");
	Wait(2 * 1000);
	m_pGame->m_pTalk->NPCTalk(0x04);

	Wait(1 * 1000);
	m_pGame->m_pTalk->NPCTalk(0xff);

	m_pGame->m_Account.IsGetXL = 1;
	m_pGame->m_Account.GetXLTime = time(nullptr);

end:
	if (m_pGame->m_Account.GetXLLogout) { // 领完项链后退出
		SendMsg("已领取项链");
		Sleep(500);
		m_pGame->Close(false);
	}	
}

// 去副本门口
void GameProc::GoFBDoor()
{
	SendMsg("去副本门口");
	int x = 865, y = 500;
	if (m_pGame->IsInArea(x, y))
		return;
	for (int j = 0; j < 50; j++) {
		if (!m_pGame->m_pItem->UseSelfItem("星辰之眼", x, y, 10)) {
			printf("没有星辰之眼\n");
			break;
		}
		for (int i = 0; i < 5; i++) {
			Sleep(1000);
			if (m_pGame->IsInArea(x, y))
				return;
		}
	}

	SendMsg("未能去副本门口, 或没有星辰之眼(865,500)");
}

// 进副本
bool GameProc::GoInFB()
{
	if (0 && m_pGame->m_Account.IsMeOpenFB) {
		printf("等待再次进入副本\n");
		SendMsg("25秒后开启副本");
		Wait(30 * 1000);
	}

	int select_no = 0x01;
	int select_count = 1000;
	if (m_pGame->IsBig()) { // 大号进的
		if (!m_pGame->m_pItem->GetSelfItemCountByName("卡利亚堡钥匙")) { // 没有副本钥匙
			m_pGame->m_pClient->Send(SCK_NOYAOSI, true);
			SendMsg("无卡利亚堡钥匙");
			return false;
		}
		SYSTEMTIME stLocal;
		::GetLocalTime(&stLocal);
		if (stLocal.wHour >= 20 && stLocal.wHour <= 23) { // 在免费时间段里面
			select_no = 0x00; // 第一个选项
			select_count = 3; // 试三次
			if (!m_pGame->m_Account.IsMeOpenFB) {
				Wait(30 * 1000);
			}
		}
	}

	SendMsg("开启副本");
	while (true) {
		for (int j = 0; j < select_count; j++) {
			if ((j % 5) == 0) {
				GoGetXiangLian(false);
				GoFBDoor();
			}

			if (IsNeedAddLife() == -1) {
				ReBorn(1);
				Sleep(1000);
			}

			m_pGame->m_pTalk->NPC("卡利亚堡传送门");
			Wait(1 * 1000);
			m_pGame->m_pTalk->NPCTalk(select_no);
			Wait(1 * 1000);
			m_pGame->m_pTalk->NPCTalk(0x00);
			Wait(5 * 1000);

			if (m_pGame->IsInFB()) {
				Sleep(1000);
				ViteInFB();
				m_pGame->m_Account.IsMeOpenFB = true;
				return true;
			}

			char log[64];
			sprintf_s(log, "开启副本, 第%d次尝试", j + 1);
			SendMsg(log);
			Sleep(1000);
		}
		if (select_no == 0x00) {
			select_no = 0x01;
			select_count = 1000;
		}
		else {
			break;
		}
	}
	
	return false;
}

// 继续副本
void GameProc::ContinueInFB()
{
	GoFBDoor();
	Wait(1 * 1000);
	SendMsg("继续呆副本");
	printf("继续副本\n");
	for (int j = 0; j < 3; j++) {
		m_pGame->m_pTalk->NPC("卡利亚堡传送门");
		Wait(1 * 1000);
		m_pGame->m_pTalk->NPCTalk(0x00);
		Wait(1 * 1000);
		m_pGame->m_pTalk->NPCTalk(0x00);
		Wait(5 * 1000);

		if (m_pGame->IsInFB()) {
			m_bAtFB = true;
			break;
		}
		Sleep(1000);
	}
	printf("继续副本完成\n");
}

// 出副本
void GameProc::GoOutFB(const char* name, bool notify, bool reborn)
{
	printf("GoOutFB:%d\n", m_pGame->IsBig());

	if (reborn && !m_pGame->IsBig()) { // 小号需要复活
		while (IsNeedAddLife() == -1) {
			Sleep(500);
			m_pGame->Call_ReBoren(); // 复活
			Sleep(500);
		}
	}
	if (!m_pGame->IsInFB())
		goto end;

	for (int i = 0; true; i++) { // 试五次
		if (!m_pGame->IsInFB()) { // 出去了
			break;
		}

		printf("重新第%d次对话出副本\n", i + 1);
		m_pGame->m_pTalk->NPC(name);
		Wait(1 * 1000);
		m_pGame->m_pTalk->NPCTalk(0x00);
		Wait(1 * 1000);
		m_pGame->m_pTalk->NPCTalk(0x00);
		Wait(1 * 1000);
		m_pGame->m_pTalk->NPCTalk(0x00);
		Wait(1 * 1000);
	}
end:
	m_bAtFB = false;
	if (notify && !m_bSendOut) {
		SendMsg("出副本");
		m_bSendOut = true;
		m_pGame->m_pClient->Send(SCK_OUTFB, true); // 通知出副本
	}	
}

// 执行副本流程
void GameProc::ExecInFB()
{
	m_bSendOut = false;
	if (m_pGame->IsBig()) {
		printf("执行副本流程\n");
		SendMsg("开始刷副本");
		int start_time = time(nullptr);
		
		// 出征宠物
		m_pGame->m_pPet->PetOut(m_pGame->m_pGameConf->m_stPetOut.No, m_pGame->m_pGameConf->m_stPetOut.Length, true);
		// 存钱钱
		SaveMoney();

		while (ExecStep(m_pGameStep->m_Step)); // 大号刷副本
		m_bLockGoFB = false;

		int second = time(nullptr) - start_time;
		printf("执行副本流程完成，总用时:%02d分%02d秒\n", second / 60, second % 60);
		char log[64];
		sprintf_s(log, "完成刷副本，总用时:%02d分%02d秒", second / 60, second % 60);
		SendMsg(log);

		m_pGameStep->ResetStep(0);
		GoFBDoor();
	}
}

#if USE_MY_LINK
// 执行流程
bool GameProc::ExecStep(Link<_step_*>& link)
#else
bool GameProc::ExecStep(vector<_step_*>& link)
#endif
{
	m_pStep = m_pGameStep->Current(link);
	if (!m_pStep)
		return false;

	_step_* m_pTmpStep = m_pStep; // 临时的
	if (m_pStepCopy) { // 已经执行到的步骤
		if (m_pStep == m_pStepCopy) {
			m_pStepCopy = nullptr;
		}
		else {
			if (m_pStep->OpCode != OP_MOVE && m_pStep->OpCode != OP_MAGIC) { // 不是移动或放技能
				return m_pGameStep->CompleteExec(link) != nullptr;
			}
		}
	}

	if (m_pStep->OpCode != OP_NPC && m_pStep->OpCode != OP_SELECT) {
		SMSG_D("复活宠物");
		// 复活所有没有血量宠物
		if (m_pGame->m_pPet->Revive()) {
			Wait(1 * 1000);
		}
		SMSG_D("复活宠物->完成");
	}

	if (m_pStep->OpCode != OP_SELECT && m_pStep->OpCode != OP_MAGIC && m_pStep->OpCode != OP_MAGIC_PET) { // 丢掉药
		m_pGame->m_pItem->DropSelfItemByType(速效治疗包, m_iNeedBao);
		if (m_iNeedYao < 6) {
			m_pGame->m_pItem->DropSelfItemByType(速效治疗药水, m_iNeedYao);
		}
	}
	
	char msg[128];
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
		printf("流程->移动:%d.%d|%d.%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (m_pStep->X2 && m_pStep->Y2) {
			m_pStep->X = MyRand(m_pStep->X, m_pStep->X2, GetTickCount() % 100);
			m_pStep->Y = MyRand(m_pStep->Y, m_pStep->Y2, (GetTickCount() % 100) * 2);

			m_pStep->X2 = 0;
			m_pStep->Y2 = 0;

			printf("实际移动位置:%d,%d\n", m_pStep->X, m_pStep->Y);
		}
		Move();
		break;
	case OP_MOVEFAR:
		printf("流程->传送:%d.%d\n", m_pStep->X, m_pStep->Y);
		SMSG_DP(msg, "流程->传送.%d,%d", m_pStep->X, m_pStep->Y);
		Move();
		break;
	case OP_NPC:
		printf("流程->NPC:%s\n", m_pStep->NPCName);
		SMSG_DP(msg, "流程->NPC.%s 坐标{%d,%d}", m_pStep->NPCName, m_stLastStepInfo.MvX, m_stLastStepInfo.MvY);
		NPC();
		break;
	case OP_SELECT:
		printf("流程->选项:%d\n", m_pStep->SelectNo);
		SMSG_DP(msg, "流程->选项:%d", m_pStep->SelectNo);
		Select();
		break;
	case OP_MAGIC:
		printf("流程->技能:%s\n", m_pStep->Magic);
		SMSG_DP(msg, "流程->技能.%s", m_pStep->Magic);
		Magic();
		break;
	case OP_MAGIC_PET:
		printf("流程->宠物技能:%s\n", m_pStep->Magic);
		SMSG_DP(msg, "流程->宠物技能:%s", m_pStep->Magic);
		MagicPet();
		break;
	case OP_CRAZY:
		SMSG_D("流程->疯狂");
		Crazy();
		break;
	case OP_CLEAR:
		SMSG_D("流程->清理");
		Clear();
		break;
	case OP_PICKUP:
		printf("流程->捡拾物品\n");
		SMSG_D("流程->捡拾物品");
		PickUp();
		break;
	case OP_CHECKIN:
		printf("流程->存入物品\n");
		SMSG_D("流程->存入物品");
		CheckIn();
		break;
	case OP_USEITEM:
		printf("流程->使用物品\n");
		SMSG_D("流程->使用物品");
		UseItem();
		break;
	case OP_DROPITEM:
		printf("流程->丢弃物品:%s\n", m_pStep->Name);
		SMSG_DP(msg, "流程->丢弃物品:%s", m_pStep->Name);
		DropItem();
		break;
	case OP_SELL:
		printf("流程->售卖物品\n");
		SMSG_D("流程->售卖物品");
		SellItem();
		break;
	case OP_BUTTON:
		printf("流程->按钮:%08X\n", m_pStep->ButtonId);
		SMSG_D("流程->按钮");
		Button();
		break;
	case OP_WAIT:
		printf("流程->等待:%d %s\n", m_pStep->WaitMs / 1000, m_pStep->Magic);
		SMSG_DP(msg, "流程->等待:%d", m_pStep->WaitMs / 1000);
		Wait();
		break;
	case OP_SMALL:
		if (m_pStep->SmallV == -1) {
			printf("流程->小号:未知操作\n");
		}
		else {
			printf("流程->小号:%d %s\n", m_pStep->SmallV, m_pStep->SmallV == 0 ? "出副本" : "进副本");
			Small();
		}
		break;
	default:
		SMSG_D("流程->未知");
		break;
	}

	SMSG_D("流程->执行完成");

	int drop_i = 0;
	int move_far_i = 0;
	do {
		do {
			if (m_bStop || m_bReStart)
				return false;
			if (m_bPause)
				Sleep(500);
		} while (m_bPause);

		Sleep(100);

		if (m_pStep->OpCode != OP_DROPITEM) {
			SMSG_D("判断加血");
			int life_flag = IsNeedAddLife();
			if (life_flag == 1) { // 需要加血
				SMSG_D("加血");
				AddLife();
				SMSG_D("加血->完成");
			}
			else if (life_flag == -1) { // 需要复活
				ReBorn(); // 复活
				m_pStepCopy = m_pStep; // 保存当前执行副本
				m_pGameStep->ResetStep(0); // 重置到第一步
				printf("重置到第一步\n");
				return true;
			}
			SMSG_D("判断加血->完成");
		}
		
		//SMSG_D("判断流程");
		bool complete = StepIsComplete();
		//SMSG_D("判断流程->完成");
		if (m_pStep->OpCode == OP_MOVEFAR) {
			if (++move_far_i == 300) {
				printf("传送超时\n");
				complete = true;
			}
		}

		if (!m_pStep) {
			SMSG_D("!m_pStep");
			printf("!m_pStep\n");
			while (true) Sleep(1000);
		}
		if (m_pStep != m_pTmpStep) {
			SMSG_D("m_pStep != m_pTmpStep");
			printf("m_pStep != m_pTmpStep\n");
			while (true) Sleep(1000);
		}

		if (complete) { // 已完成此步骤
			SMSG_D("流程->已完成此步骤");
			if (m_pStep->OpCode != OP_NPC && m_pStep->OpCode != OP_SELECT && m_pStep->OpCode != OP_WAIT) {
				//if (m_pGame->m_pTalk->NPCTalkStatus()) // 对话框还是打开的
				//	m_pGame->m_pTalk->NPCTalk(0xff);   // 关掉它
			}
			
			_step_* next = m_pGameStep->CompleteExec(link);
			if (m_pStep->OpCode != OP_WAIT && next) {
				if (next->OpCode == OP_MOVE) {
					Sleep(50);
				}
				else {
					Sleep(50);
				}
			}
			return next != nullptr;
		}
	} while (true);

	return false;
}

// 步骤是否已执行完毕
bool GameProc::StepIsComplete()
{
	bool result = false;
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
		SMSG_D("判断是否移动到终点&是否移动", true);
		if (m_pGame->m_pMove->IsMoveEnd()) { // 已到指定位置
			result = true;
			goto end;
		}
		SMSG_D("判断是否移动到终点->完成", true);
		if (!m_pGame->m_pMove->IsMove()) {   // 已经停止移动
			m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
		}
		SMSG_D("判断是否移动->完成", true);
		break;
	case OP_MOVEFAR:
		if (m_pGame->IsNotInArea(m_pStep->X, m_pStep->Y, 50)) { // 已不在此区域
			result = true;
			goto end;
		}
		if (m_pGame->m_pMove->IsMoveEnd()) { // 已到指定位置
			m_pGame->m_pMove->Run(m_pStep->X - 5, m_pStep->Y);
		}
		if (!m_pGame->m_pMove->IsMove()) {   // 已经停止移动
			m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
		}
		break;
	case OP_NPC:
		result = m_pGame->m_pTalk->NPCTalkStatus();
		result = true;
		if (!result) { // 对话框没有打开
			m_pGame->m_pTalk->NPC(m_pStep->NPCName);
		}
		break;
	case OP_SELECT:
		result = true;
		break;
	case OP_MAGIC:
		result = true;
		break;
	case OP_CRAZY:
		result = true;
		break;
	case OP_CLEAR:
		printf("清怪:%08X\n", m_pStep->Magic);
		result = m_pGame->m_pGuaiWu->Clear(m_pStep->Magic, 8, 8); // 清除5*5范围内的怪物
		break;
	case OP_PICKUP:
		result = true;
		break;
	case OP_WAIT:
		result = true;
		break;
	default:
		result = true;
		break;
	}
end:
	return result;
}

// 移动
void GameProc::Move()
{
	if (m_pStep->X >= 255 && m_pStep->X <= 265 && 
		m_pStep->Y >= 50 && m_pStep->Y <= 60) { // 通知出去
		m_bSendOut = true;
		m_pGame->m_pClient->Send(SCK_OUTFB, true); // 通知出副本
	}

	m_stLastStepInfo.MvX = m_pStep->X;
	m_stLastStepInfo.MvY = m_pStep->Y;
	m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
}

// NPC
void GameProc::NPC()
{
	// 500 - 930
	// 560 - 1120
	// https://31d7f5.link.yunpan.360.cn/lk/surl_yL2uvtfBesv#/-0
	if (0
		&& (strcmp(m_pStep->NPCName, "贪求缚灵柱") == 0
		|| strcmp(m_pStep->NPCName, "怨恨缚灵柱") == 0
		|| strcmp(m_pStep->NPCName, "迷幻缚灵柱") == 0
		|| strcmp(m_pStep->NPCName, "离爱缚灵柱") == 0)) {
		
	}
	else {
		if (strcmp(m_pStep->NPCName, "传送门") == 0) {
			m_bLockGoFB = true; // 等待后面卖东西
			printf("对话.传送门[NPC]\n");
			GoOutFB("传送门");
			return;
		}
		if (strcmp(m_pStep->NPCName, "爱娜的灵魂") == 0) { // 对话了爱娜的灵魂
			m_iNeedYao = 3;
			m_iNeedBao = 2;
		}

		m_bLastBoss = strcmp(m_pStep->NPCName, "阿拉玛的怨念") == 0;
		DWORD npc_id = m_pGame->m_pTalk->NPC(m_pStep->NPCName);
		if (!npc_id) { // 没有找到NPC
			if (strcmp(m_pStep->NPCName, "爱娜的灵魂") == 0) { // 这里容易出错
				for (int i = 0; i < 3; i++) {
					DWORD id = m_pGame->m_pTalk->NPC("封印的灵魂");
					Wait(1 * 1000);
					m_pGame->m_pTalk->NPCTalk(0x00);
					Wait(1 * 1000);
					if (id) {
						npc_id = m_pGame->m_pTalk->NPC(m_pStep->NPCName);
						break;
					}
				}
			}
		}

		m_stLastStepInfo.NPCId = npc_id;
		strcpy(m_stLastStepInfo.NPCName, m_pStep->NPCName);
	}
}

// 最后一个对话的NPC
void GameProc::NPCLast()
{
	m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCName);
	Wait(1 * 1000);
	m_pGame->m_pTalk->NPCTalk(0x00);
	Wait(500);
}

// 选择
void GameProc::Select()
{
	if (!m_stLastStepInfo.NPCId) // 没有与NPC对话, 没必要再选择
		return;

	for (DWORD i = 1; i <= m_pStep->OpCount; i++) {
		if (1 || i < m_pStep->OpCount) { // 不是最后一次
			if (!m_pGame->m_pTalk->WaitTalkBoxOpen()) {        // 等待对话框打开超时
				printf("重新对话NPC\n");
				m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCId); // 重新与NPC对话
				if (m_pGame->m_pTalk->WaitTalkBoxOpen()); // NPC有效
					//i--;
			}
		}
		printf("第(%d)次选择\n", i);
		m_pGame->m_pTalk->NPCTalk(m_pStep->SelectNo);

		if (i > 1) {
			if (IsNeedAddLife() == 1) {
				AddLife();
			}
			Sleep(500);
		}
	}
	if (m_pStep->OpCount == 1 && m_pStep->SelectNo != 0xff) {
		if (IsCheckNPC(m_stLastStepInfo.NPCName)) {
			int ks = 0;
			if (m_pGame->m_iQiaZhuS > 0) {
				ks = m_pGame->m_iQiaZhuS * 2;
			}
			int zm = ks ? 0x00ffffff : 5;
			for (int z = 1; z < zm; z++) {
				Sleep(500);
				printf("%d.检查NPC:%s %08X\n", z, m_stLastStepInfo.NPCName, m_stLastStepInfo.NPCId);
				DWORD npc_id = m_pGame->m_pTalk->GetNPCId(m_stLastStepInfo.NPCName);
				if (!npc_id)
					break;

				if ((z % 2) == 0) {
					printf("NPC:%s存在, 继续对话\n", m_stLastStepInfo.NPCName);
					m_pGame->m_pTalk->NPC(npc_id); // 重新与NPC对话
					m_pGame->m_pTalk->WaitTalkBoxOpen(); // NPC有效
					m_pGame->m_pTalk->NPCTalk(m_pStep->SelectNo);
				}
				if (z > 10) {
					if ((z % 5) == 0) {
						if (IsNeedAddLife() == -1) { // 需要复活
							ReBorn(); // 复活
							m_pStepCopy = m_pGameStep->Prev(m_pGameStep->m_Step); // 保存当前执行副本
							m_pGameStep->ResetStep(0); // 重置到第一步
							printf("重置到第一步\n");
							break;
						}
					}
					if (z >= ks) {
						SMSG_N(64, "可能已卡住[%s], 重启游戏机", m_stLastStepInfo.NPCName);
						m_pGame->Close(false);
					}
				}
			}
		}
	}
}

// 技能
void GameProc::Magic()
{
	if (m_pStep->WaitMs) { // 需要等待冷却
		while (!m_pGame->m_pMagic->CheckCd(m_pStep->Magic)) {
			Sleep(100);
		}
	}

	DWORD x = m_pStep->X, y = m_pStep->Y;
	if (!x || !y) { // 释放的位置坐标
		m_pGame->ReadCoor(&x, &y);
	}
	m_pGame->m_pMagic->UseMagic(m_pStep->Magic, x, y);
	strcpy(m_stLastStepInfo.Magic, m_pStep->Magic);
	Sleep(50);
}

// 技能-宠物
void GameProc::MagicPet()
{
	m_pGame->Keyborad(m_pStep->Magic[0], NULL, false);

	//m_pGame->m_pPet->Magic(m_pStep->Magic[0]);
}

// 狂甩
void GameProc::Crazy()
{
	m_bIsCrazy = strstr(m_pStep->Magic, "结束") ? false : true;
	strcpy(m_CrazyMagic, m_pStep->Magic);
	if (m_bIsCrazy) {
		printf("开启狂甩模式\n");
	}
	else {
		printf("狂甩模式结束\n");
	}
}

// 清怪
void GameProc::Clear()
{
}

// 捡物
void GameProc::PickUp()
{
	for (int i = 1; i <= 2; i++) {
		printf("第%d次捡物\n", i);
		m_pGame->m_pItem->PickUpItem(m_pGame->m_pGameConf->m_stPickUp.PickUps, 
			m_pGame->m_pGameConf->m_stPickUp.Length);
		Sleep(1000);
	}
}

// 存钱
void GameProc::SaveMoney()
{
	Button(BUTTON_ID_ROLE, 1000);           // 点击人物按钮  
	Button(BUTTON_ID_VIP, 1000);            // 点击VIP按钮   
	if (Button(BUTTON_ID_CHECKIN, 1000)) {  // 点击物品仓库按钮
		m_pGame->Call_SaveMoney(50000000);
		Wait(500);
		Button(BUTTON_ID_CLOSECKIN); // 点击关闭仓库按钮
	}
}

// 存物
DWORD GameProc::CheckIn(bool in)
{
	HWND child, parent;

	Button(BUTTON_ID_ROLE, 1000);           // 点击人物按钮  
	Button(BUTTON_ID_VIP, 1000);            // 点击VIP按钮   
	if (!Button(BUTTON_ID_CHECKIN, 1000)) { // 点击物品仓库按钮
		printf("打开仓库失败\n");
		return 0;
	}

	DWORD count = 0;
	if (in) {
		count = m_pGame->m_pItem->CheckInSelfItem(m_pGame->m_pGameConf->m_stCheckIn.CheckIns,
			m_pGame->m_pGameConf->m_stCheckIn.Length);
	}
	else {
		count = m_pGame->m_pItem->CheckOutItem(m_pGame->m_pGameConf->m_stSell.Sells,
			m_pGame->m_pGameConf->m_stSell.Length);
	}
	
	Button(BUTTON_ID_CLOSECKIN); // 点击关闭仓库按钮
	return count;
}

// 使用物品
void GameProc::UseItem()
{
	m_pGame->m_pItem->UseSelfItem(m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->Extra[0]);
}

// 扔物品
void GameProc::DropItem()
{
#if 0
	int live_count = m_pStep->Extra[0];
	if (live_count == 0) {
		if (strcmp("速效治疗药水", m_pStep->Name) == 0)
			live_count = m_iNeedYao;
		else if (strcmp("速效治疗包", m_pStep->Name) == 0)
			live_count = m_iNeedBao;
	}
	m_pGame->m_pItem->DropSelfItemByName(m_pStep->Name, live_count);
#endif
	if (strcmp("速效治疗包", m_pStep->Name) == 0) {
		m_pGame->m_pItem->ReadYaoBao(0);
		m_pGame->m_pItem->ReadYaoBao(1);
	}
}

// 售卖物品
void GameProc::SellItem()
{
	const char* name = "维德尼娜";
	int x = 295, y = 490;
	if (m_pGame->IsInArea(x, y))
		goto sell;

	if (!m_pGame->m_pItem->GetSelfItem("星辰之眼", x, y, 10)) { // 武器商
		name = "帕迪克";
		x = 260;
		y = 380;
		if (m_pGame->IsInArea(x, y))
			goto sell;
	}

	for (int j = 0; j < 3; j++) {
		m_pGame->m_pItem->UseSelfItem("星辰之眼", x, y, 10);
		for (int i = 0; i < 3; i++) {
			Sleep(1000);
			if (m_pGame->IsInArea(x, y))
				goto sell;
		}
	}

sell:
	// 对话NPC卖东西
	printf("卖东西\n");
	SendMsg("卖东西");

	m_pGame->m_pTalk->NPC(name);
	Wait(2 * 1000);
	m_pGame->m_pTalk->NPCTalk(0x00, true);

	m_pGame->m_pItem->SellSelfItem(m_pGame->m_pGameConf->m_stSell.Sells,
		m_pGame->m_pGameConf->m_stSell.Length);

	while (true) {
		DWORD count = m_pGame->m_pItem->CheckOutItem(m_pGame->m_pGameConf->m_stSell.Sells,
			m_pGame->m_pGameConf->m_stSell.Length);
		if (count == 0)
			break;

		m_pGame->m_pItem->SellSelfItem(m_pGame->m_pGameConf->m_stSell.Sells,
			m_pGame->m_pGameConf->m_stSell.Length);
	}

	Button(BUTTON_ID_CLOSESHOP, 500, "CLOSE"); // 点击关闭商店按钮

	m_bLockGoFB = false;
}

// 按钮
void GameProc::Button()
{
	Button(m_pStep->ButtonId);
}

// 按钮
bool GameProc::Button(int button_id, DWORD sleep_ms, const char* name)
{
	HWND child, parent;
	if (!m_pGame->FindButtonWnd(button_id, child, parent, name))
		return false;

	LRESULT result = SendMessageW(parent, WM_COMMAND, MAKEWPARAM(button_id, BN_CLICKED), (LPARAM)child); // 点击按钮
	if (sleep_ms) {
		Sleep(sleep_ms);
	}
	return result != 0;
}

// 等待
void GameProc::Wait()
{
	if (strlen(m_pStep->Magic)) { // 等待技能冷却
		while (!m_pGame->m_pMagic->CheckCd(m_pStep->Magic, m_pStep->WaitMs)) {
			Sleep(100);
		}
		strcpy(m_stLastStepInfo.Magic, m_pStep->Magic);
	}
	else { // 等待
		Wait(m_pStep->WaitMs);
	}
}

// 等待
void GameProc::Wait(DWORD ms)
{
	if (ms < 1000) {
		printf("等待%d毫秒\n", ms);
		Sleep(ms);
		return;
	}

	int n = 0;
	for (int i = 0; i < ms; i += 100) {
		int ls = (ms - i) / 1000;

		if (n != ls) {
			printf("等待%02d秒，还剩%02d秒.\n", ms / 1000, ls);
		}
		n = ls;
		Sleep(100);
	}
}

// 小号
void GameProc::Small()
{
	if (m_pStep->SmallV == 0) {
		m_pGame->m_pClient->Send(SCK_SMALLOUTFB);
	}
	if (m_pStep->SmallV == 1) {
		m_pGame->m_pClient->Send(SCK_SMALLINFB);
	}
}

// 复活
void GameProc::ReBorn(int flag)
{
	printf("没有血量了, 等待复活\n");
	SMSG_N(64, "需要复活, 位置:%d,%d", m_pStep->X, m_pStep->Y);
	Wait(25 * 1000);
	m_pGame->m_pClient->Send(SCK_REBORN, true); // 通知服务端复活了
	m_pGame->Call_ReBoren(flag);

	Sleep(1000);
	// 出征宠物
	m_pGame->m_pPet->PetOut(m_pGame->m_pGameConf->m_stPetOut.No, m_pGame->m_pGameConf->m_stPetOut.Length, true);
}

// 是否检查此NPC对话完成
bool GameProc::IsCheckNPC(const char * name)
{
	return strcmp(name, "魔封障壁") == 0 || strcmp(name, "地狱火障壁") == 0 || strcmp(name, "爆雷障壁") == 0
		|| strcmp(name, "仇之缚灵柱") == 0 || strcmp(name, "怨之缚灵柱") == 0
		|| strstr(name, "元素障壁") != nullptr;
}

// 是否在副本
bool GameProc::IsInFB()
{
	return m_pGame->IsInFB();
}

// 读取人物坐标
bool GameProc::ReadCoor()
{
	return m_pGame->ReadCoor(&m_iCoorX, &m_iCoorY);
}

// 读取人物血量
bool GameProc::ReadLife()
{
	return m_pGame->ReadLife(m_iLife, m_iLifeMax);
}

// 读取快捷键上面物品数量
bool GameProc::ReadQuickKey2Num()
{
	return m_pGame->ReadQuickKey2Num(m_QuickKey2Nums, sizeof(m_QuickKey2Nums));
}

// 读取包包物品
bool GameProc::ReadBag()
{
	return m_pGame->ReadBag(m_dwBag, sizeof(m_dwBag));
}

// 是否需要加血量
int GameProc::IsNeedAddLife()
{
	if (GetTickCount() < (m_dwAddLifeTime + 350)) // 1秒内不重复加
		return 0;
	if (!ReadLife())
		return 0;
	if (m_iLife == 0)
		return -1;
		
	return (m_iLife + 5000) <= m_iLifeMax ? 1 : 0;
}

// 加血
void GameProc::AddLife()
{
	if (m_pGame->m_pItem->UseYao()) {
		printf("加血.\n");
		m_dwAddLifeTime = GetTickCount();
	}
	else {
		printf("没有药\n");
	}
}

// 发送信息给服务端
void GameProc::SendMsg(const char * v, const char * v2)
{
	m_pGame->m_pClient->SendMsg(v, v2);
}

// 停止
void GameProc::Stop(bool v)
{
	INLOGVARN(32, "Set Stop:%d", v);
	m_bStop = v;
}

// 打开日记文件
void GameProc::OpenLogFile()
{
	return;
	char logfile[255];
	SHGetSpecialFolderPathA(0, logfile, CSIDL_DESKTOPDIRECTORY, 0);
	strcat(logfile, "\\MoYu\\日记.txt");

	m_LogFile.open(logfile);
	printf("日记:%s\n", logfile);
}

// 写入日记
void GameProc::WriteLog(const char* log, bool flush)
{
	return;
	if (!m_pGame->IsBig())
		return;

	m_LogFile << log << endl;
	if (flush) {
		m_LogFile.flush();
	}
}


