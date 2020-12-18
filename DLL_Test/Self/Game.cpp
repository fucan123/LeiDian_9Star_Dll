#include "Home.h"

#include "GameClient.h"
#include <My/Common/Explode.h>
#include <My/Common/mystring.h>
#include <My/Common/func.h>
#include <My/Driver/KbdMou.h>
#include <My/Common/C.h>
#include <stdio.h>
#include <time.h>

#include "Game.h"
#include "GameConf.h"
#include "GameProc.h"
#include "Item.h"
#include "Talk.h"

// 游戏g_pObjHero全局变量(CHero类this指针)
DWORD g_pObjHero = NULL;
// 游戏g_objPlayerSet全局变量
DWORD g_objPlayerSet = NULL;

Game* Game::self = NULL;   // Game自身
// ...
Game::Game()
{
	self = this;
	m_iGetPosTime = 0;
	m_iNoAutoSelect = 0;
	m_iTalkOpen = 0;
	m_iDebug = 1;
	m_iQiaZhuS = 15;

	ZeroMemory(&m_GameModAddr, sizeof(GameModAddr));
	ZeroMemory(&m_GameAddr, sizeof(GameAddr));
	ZeroMemory(&m_GameCall, sizeof(GameCall));
	ZeroMemory(&m_Account, sizeof(m_Account));
	//m_Account.IsBig = true;

	m_nVerFail = 0;

	m_bIsReadEnd = true;
	m_pReadBuffer = new BYTE[1024 * 1024 * 10];

	m_pHome     = new Home(this);
	m_pClient   = new GameClient(this);
	m_pClient->SetSelf(m_pClient);

	m_pItem     = new Item(this);      // 物品类
	m_pTalk     = new Talk(this);      // 对话类
	//m_pMove     = new Move(this);      // 移动类
	//m_pGuaiWu   = new GuaiWu(this);    // 怪物类
	//m_pMagic    = new Magic(this);     // 技能类
	//m_pPet      = new Pet(this);       // 宠物类

	m_pGameConf = new GameConf(this);  // 游戏配置类
	m_pGameProc = new GameProc(this);  // 执行过程类
}

// >>>
Game::~Game()
{
	delete m_pReadBuffer;

	delete m_pClient;
	//delete m_pItem;
	//delete m_pTalk;
	//delete m_pMove;
	//delete m_pGuaiWu;
	//delete m_pMagic;
	//delete m_pPet;

	delete m_pGameProc;

	Drv_DisConnectDriver();
}

// 验证
void Game::VerifyServer()
{
	int n = 0;
	while (true) {
		if (++n == 60) {
			if (m_pHome->Verify()) {
				//printf("验证成功.\n");
				m_nVerFail = 0;
			}
			else {
				//printf("验证失败.\n");
				if (++m_nVerFail >= 10) { }
			}
			n = 0;
		}
		
		Sleep(5000);
	}
}

// 关闭游戏
void Game::Close(bool leave_team)
{
	if (leave_team) {
		if (Call_IsTeamLeader())
			Call_TeamDismiss();
		else
			Call_TeamLeave();
	}

	m_pClient->Send(SCK_CLOSE, true);
	m_pGameProc->Wait(3 * 1000);
	::printf("关闭游戏\n");
	ExitProcess(0);
	TerminateProcess(GetCurrentProcess(), 0);
	//TerminateProcess(m_hGameProcess, 0);
}

// 连接管理服务
void Game::Connect(const char* host, USHORT port)
{
	::printf("准备连接服务器[%s:%d]\n", host, port);
	m_pClient->Connect(host, port);
}

// 登录游戏帐号
void Game::Login()
{
	m_Account.IsLogin = 0;

	if (m_iNoAutoSelect) { // 自己选择登录
		::printf("手动登录...\n");
		return;
	}

	::printf("准备登录...\n");
#if 1
	Sleep(1500);
	LeftClick(510, 550); // 进入正式版
	Sleep(1500);

	SelectServer(0xff);
#endif

	InputUserPwd();
}

// 选择区服
void Game::SelectServer(int flag)
{
	HWND h = ::FindWindow(NULL, L"360安全卫士");
	SetForegroundWindow(h);
	Sleep(150);

	int x, y;
	if (flag & 0x01) {
		GetSerBigClickPos(x, y);
		LeftClick(x, y); // 选择大区
		Sleep(1500);
	}
	if (flag & 0x02) {
		GetSerSmallClickPos(x, y);
		LeftClick(x, y); // 选择小区
	}
}

// 输入帐号密码
void Game::InputUserPwd(bool input_user)
{
	ReadGameMemory(0x100);
start:
	if (0 && m_GameAddr.SmallServer) {
		Sleep(1000);
		char ser[16], ch[8];
		if (GetSmallSerNum(ser, ch)) {
			if (strcmp(ser, "神祗") == 0)
				strcpy(ser, "神祇");

			char* s_ser = strstr((char*)m_GameAddr.SmallServer, ser);
			char* s_ch = strstr((char*)m_GameAddr.SmallServer, ch);
			::printf("%s[%08x] %s[%08X]\n", ser, s_ser, ch, s_ch);
			if (!s_ser || !s_ch) {
				//28,730
				::printf("选择的小区不符合 %s!=%s\n", m_GameAddr.SmallServer, m_Account.SerSmall);
				Sleep(1000);
				LeftClick(28, 730); // 返回
				Sleep(1000);
				SelectServer(0x02);
				goto start;
			}
			else {
				::printf("选择小区正确 %s==%s\n", m_GameAddr.SmallServer, m_Account.SerSmall);
			}
		}
	}

	HWND edit = FindWindowEx(m_hWndPic, NULL, NULL, NULL);
	int i;
	if (input_user) {
		Sleep(2000);
		LeftClick(300, 265); // 点击帐号框
		SetForegroundWindow(m_hWnd);
		Sleep(1000);

		char save_name[32] = {0};
		GetWindowTextA(edit, save_name, sizeof(save_name));
		::printf("保存帐号:%s\n", save_name);
		if (strcmp(save_name, m_Account.Name) != 0) { // 保存的不一样
			int eq_num = 0; // 前面相同字符
			int length = strlen(save_name) < strlen(m_Account.Name) ? strlen(save_name) : strlen(m_Account.Name);
			for (int idx = 0; idx < length; idx++) {
				if (save_name[idx] != m_Account.Name[idx])
					break;
				eq_num++;
			}

			int back_num = strlen(save_name) - eq_num;
			for (i = 0; i < back_num; i++) {
				Keyborad(VK_BACK);
				Sleep(200);
			}
			for (i = eq_num; i < strlen(m_Account.Name); i++) {
				Keyborad(m_Account.Name[i]);
				Sleep(200);
			}
		}		
	}

	LeftClick(300, 305); // 点击密码框
	Sleep(1000);
	for (i = 0; i < strlen(m_Account.Pwd); i++) {
		Keyborad(m_Account.Pwd[i]);
		Sleep(200);
	}

	LeftClick(265, 430); // 进入
}

// 初始化
bool Game::Init(DWORD hook_tid, SetAccountProc_Func set_account_proc)
{
	::printf("Run:%08X\n", &Game::Call_Run);
	m_dwHookTid = hook_tid;
	m_SetAccountProc = set_account_proc;

	FindGameWnd();

	m_hGameProcess = GetCurrentProcess();
	FindAllModAddr();
	FindCoorAddr();

#if 0
	while (true) {
		printf("while (true7).\n");
		Sleep(5000);
	}
#endif

	m_Account.IsLogin = 1;

	WaitGameInit(60);

	DWORD pid;
	GetWindowThreadProcessId(m_hWnd, &pid);

	::printf("开始读取游戏数据...\n");
	m_pClient->SendMsg("读取游戏数据...");

	FindAllCall();
	//while (true) Sleep(1000);
	FindBagAddr();

	
	
	GetWindowRect(m_hWnd, &m_GameWnd.Rect);
	m_GameWnd.Width = m_GameWnd.Rect.right - m_GameWnd.Rect.left;
	m_GameWnd.Height = m_GameWnd.Rect.bottom - m_GameWnd.Rect.top;
	::printf("窗口句柄:%0x %d %d %.2\n", m_hWnd, m_GameWnd.Width, m_GameWnd.Height, m_fScale);
	::printf("游戏画面窗口:%08X %.2f\n", m_hWndPic, m_fScale);
	
#if 1
	
	/*
	if (!ReadGameMemory()) {
		//INLOG("无法获得人物血量地址！！！");
		return false;
	}
	*/
	
	while (false) {
		printf("while (true7).\n");
		Sleep(5000);
	}
	while (0 && !m_GameAddr.ItemPtr) {
		::printf("获取地面物品地址...\n");
		ReadGameMemory();
		Sleep(5000);
	}
	while (0 && !m_GameAddr.MapName) {
		::printf("获取地图名称地址..\n");
		ReadGameMemory();
		Sleep(5000);
	}
#if 1
	while (!m_GameAddr.Life) {
		::printf("重新读取血量地址...\n");
		m_pClient->SendMsg("重新读取血量...");
		ReadGameMemory();
		Sleep(3000);
	}
#endif
#endif
	
	//m_pMagic->ReadMagic(nullptr, nullptr, false);

	m_pClient->SendMsg("读取游戏数据完成");

	//::printf("连接键盘和鼠标驱动%s\n", Drv_ConnectDriver()?"成功":"失败");

	while (false) {
		printf("while (true8).\n");
		Sleep(5000);
	}
	
	return true;
}

// 等待游戏初始化完毕
void Game::WaitGameInit(int wait_s)
{
	m_pClient->SendMsg("等待游戏初始化完成");
	for (int i = 0; i < wait_s; i++) {
		HWND child, parent;
		if (FindButtonWnd(BUTTON_ID_CLOSEMENU, child, parent, "x")) {
			SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_CLOSEMENU, BN_CLICKED), (LPARAM)child); // 关闭它
			::printf("---------游戏数据已初始化完毕---------\n");
			m_pClient->SendMsg("游戏数据已初始化完毕");
			break;
		}

		::printf("等待游戏数据初始完成, 还剩%02d秒[%d秒].\n", wait_s - i, wait_s);
		Sleep(1000);
	}

	m_pGameProc->Button(BUTTON_ID_CANCEL,    1500);
	m_pGameProc->Button(BUTTON_ID_SURE,      1500);
	m_pGameProc->Button(BUTTON_ID_CLOSEMENU, 1500);
}

// 设置路径
void Game::SetPath(const char * path)
{
	strcpy(m_chPath, path);
	printf("配置文件目录:%s\n", m_chPath);
}

// 设置是否是大号
void Game::SetAccount(const char* ser_big, const char* ser_small, const char* name, const char* pwd, int role_no, int getxl, int big, int getxl_logout, int create_team, int out_no_xl)
{
	if (m_SetAccountProc)
		m_SetAccountProc(big);

	strcpy(m_Account.SerBig, ser_big);
	strcpy(m_Account.SerSmall, ser_small);
	strcpy(m_Account.Name, name);
	strcpy(m_Account.Pwd, pwd);
	m_Account.RoleNo = role_no - 1;
	m_Account.IsGetXL = getxl;
	m_Account.IsBig = big;
	m_Account.GetXLLogout = getxl_logout;
	m_Account.CreateTeam = create_team;
	m_Account.OutNoGoXL = out_no_xl;
	m_Account.IsReady = true;
	if (big && m_iDebug) {
		AllocConsole();
		freopen("CON", "w", stdout);
		m_pGameProc->OpenLogFile();
	}

	::printf("游戏服务器:%s.%s\n", ser_big, ser_small);
	::printf("帐号:%s|%s 第%d个角色 %s\n", name, pwd, role_no, big?"大号":"小号");
	if (getxl_logout)
		::printf("领取项链完后退出\n");
	if (create_team)
		::printf("需要创建队伍\n");

	Login();
}

// 设置卡住重启时间
void Game::SetQiaZhuS(int second)
{
	m_iQiaZhuS = second;
	::printf("卡住%d秒后重启\n", m_iQiaZhuS);
}

// 设置是否掉线
void Game::SetOffLine(int v)
{
	m_Account.IsOffLine = v;
}

// 进程是否是魔域
bool Game::IsMoYu()
{
	wchar_t name[32] = { 0 };
	GetProcessName(name, GetCurrentProcessId());
	::wprintf(L"%ws = %ws\n", name, L"soul.exe");
	return wcsstr(name, L"soul.exe") != nullptr || wcsstr(name, L"SOUL.exe") != nullptr;
}

// 是否登录了
bool Game::IsLogin()
{
	int length = strlen((const char*)(m_GameModAddr.Mod3DRole + ADDR_ACCOUNT_NAME));
	if (length > 0) {
		strcpy(m_Account.Name, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ACCOUNT_NAME));
		strcpy(m_Account.Role, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ROLE_NAME));
		m_Account.LoginTime = time(nullptr);
	}
	return length > 0;
}

// 是否掉线了 
bool Game::IsOffLine()
{
	return m_Account.IsOffLine;
}

// 比较登录帐号
bool Game::CmpLoginAccount(const char* name)
{
	return strcmp(name, m_Account.Name) == 0;
}

// 是否大号
bool Game::IsBig()
{
	//return true;
	return m_Account.IsBig;
}

// 是否在副本
bool Game::IsInFB()
{
	DWORD x = 0, y = 0;
	ReadCoor(&x, &y);
	if (x >= 882 && x <= 930 && y >= 1055 && y < 1115) // 刚进副本区域
		return true;

	return IsInMap("阿拉玛的哭泣");
}

// 是否在指定地图
bool Game::IsInMap(const char* name)
{
	if (!m_GameAddr.MapName)
		return false;

	char map_name[32] = { 0 };
	if (ReadMemory((PVOID)m_GameAddr.MapName, map_name, sizeof(map_name))) { // 读取地图名称
		return strcmp(map_name, name) == 0;
	}
	return false;
}

// 是否在指定区域坐标 allow=误差
bool Game::IsInArea(int x, int y, int allow)
{
	ReadCoor();
	int cx = (int)m_dwX - x;
	int cy = (int)m_dwY - y;

	return abs(cx) <= allow && abs(cy) <= allow;
}

// 是否不在指定区域坐标 allow=误差
bool Game::IsNotInArea(int x, int y, int allow)
{
	ReadCoor();
	int cx = (int)m_dwX - x;
	int cy = (int)m_dwY - y;

	//::printf("IsNotInArea:%d,%d %d,%d\n", m_dwX, m_dwY, x, y);
	return abs(cx) > allow || abs(cy) > allow;
}

// 是否已获取了项链
bool Game::IsGetXL()
{
	if (m_Account.IsGetXL) {
		tm t;
		time_t get_time = m_Account.GetXLTime;
		gmtime_s(&t, &get_time);

		tm t2;
		time_t now_time = time(nullptr);
		gmtime_s(&t2, &now_time);

		if (t.tm_mday != t2.tm_mday) { // 昨天领取的
			m_Account.IsGetXL = 0;
		}
	}
	return m_Account.IsGetXL != 0;
}

// 运行
void Game::Run()
{
	if (0 && !IsBig())
		return;

	if (m_iTalkOpen) {
		::printf("-----------开启了自动喊话-----------\n");
		return;
	}

	m_pGameProc->Run();
}

// 获取游戏中心位置在屏幕上的坐标
void Game::GetGameCenterPos(int & x, int & y)
{
	GetWindowRect(m_hWnd, &m_GameWnd.Rect);
	m_GameWnd.Width = m_GameWnd.Rect.right - m_GameWnd.Rect.left;
	m_GameWnd.Height = m_GameWnd.Rect.bottom - m_GameWnd.Rect.top;

	x = m_GameWnd.Rect.left + (m_GameWnd.Width / 2);
	y = m_GameWnd.Rect.top + (m_GameWnd.Height / 2);
}

// 获得游戏窗口
HWND Game::FindGameWnd()
{
	while (true) {
		DWORD pid = GetCurrentProcessId();
		::EnumChildWindows(m_hWnd, EnumProc, (LPARAM)&pid);
		if (pid != GetCurrentProcessId()) {
			m_hWnd = (HWND)pid;
			while (true) {
				m_hWnd2 = FindWindowEx(m_hWnd, NULL, NULL, NULL);
				if (m_hWnd2) {
					while (true) {
						m_hWndPic = FindWindowEx(m_hWnd2, NULL, NULL, NULL);
						if (m_hWndPic)
							break;
						Sleep(1000);
					}
					break;
				}
				Sleep(1000);
			}
			break;
		}
		Sleep(5000);
	}
	
	m_fScale = ::GetScale();
	GetWindowRect(m_hWnd, &m_GameWnd.Rect);
	m_GameWnd.Width = m_GameWnd.Rect.right - m_GameWnd.Rect.left;
	m_GameWnd.Height = m_GameWnd.Rect.bottom - m_GameWnd.Rect.top;
	::printf("窗口句柄:%0x->%0x->%0x %d %d %.2\n", m_hWnd, m_hWnd2, m_hWndPic, m_GameWnd.Width, m_GameWnd.Height, m_fScale);
	
#if 0
	int n = 0;
	while (true) {
		n++;
		HWND child, parent;
		FindButtonWnd(BUTTON_ID_MAGIC, child, parent, "技能");

		HWND talkWnd = FindWindowEx(m_hWnd2, NULL, NULL, NULL);
		::printf("%d.游戏对话窗口:%08X,%08X\n", n, talkWnd, child);
		Sleep(1000);

		if (n >= 60) {
			if ((n % 60) == 0) {
				printf("点击F1\n");
				ClickMagic(VK_F1);
			}
			if ((n % 25) == 0) {
				printf("点击用卡利亚堡钥匙开启进入\n");
				SelectTalkNo("钥匙开启副本");
			}
		}
	}
#endif

	m_pClient->SendOpen(m_fScale, m_hWnd, m_GameWnd.Rect); // 通知窗口已打开
	return m_hWnd;
}

// 获取所有模块地址
void Game::FindAllModAddr()
{
	m_GameModAddr.Mod3DRole = FindModAddr(MOD_3drole);
	m_GameModAddr.Mod3DGameMap = FindModAddr(MOD_3dgamemap);
}

// 获取游戏所有CALL
void Game::FindAllCall()
{
	SearchModFuncMsg info[] = {
		{"?g_pObjHero",            NULL,  0, &g_pObjHero,                  "g_pObjHero全局变量"},
		{"?g_objPlayerSet",        NULL,  0, &g_objPlayerSet,              "g_objPlayerSet全局变量"},
		{"?ReBorn@CHero",          NULL,  0, &m_GameCall.ReBorn,           "人物复活"},
		{"?Run@CHero",             NULL,  0, &m_GameCall.Run,              "移动函数"},
		{"?Talk@CHero",            NULL,  0, &m_GameCall.Talk,             "喊话函数"},
		{"?ActiveNpc@CHero",       NULL,  0, &m_GameCall.ActiveNpc,        "NPC对话"},
		{"?GetInstance@ITaskManager",NULL,0, &m_GameCall.ITaskGetInstance, "NPC对话选择"},
		{"?GetPackageItemAmount@CHero", NULL, 0,   &m_GameCall.GetPkAgeItemAmount,  "获取仓库物品指针"},
		{"?GetPackageItemByIndex@CHero",  NULL, 0, &m_GameCall.GetPkageItemByIndex, "获取仓库物品指针"},
		{"?GetItemAmount@CHero", NULL,    0, &m_GameCall.GetItemAmount,  "获取物品数量"},
		{"?GetItemByIndex@CHero",  NULL,  0, &m_GameCall.GetItemByIndex,   "获取物品指针"},
		{"?UseItem@CHero",         NULL,  0, &m_GameCall.UseItem,          "使用物品"},
		{"?DropItem@CHero",        NULL,  0, &m_GameCall.DropItem,         "丢弃物品"},
		{"?PickUpItem@CHero",      NULL,  0, &m_GameCall.PickUpItem,       "捡拾物品"},
		{"?SellItem@CHero",        NULL,  0, &m_GameCall.SellItem,         "卖物品"},
		{"?SaveMoney@CHero",       NULL,  0, &m_GameCall.SaveMoney,        "存钱"},
		{"?CheckInItem@CHero",     NULL,  0, &m_GameCall.CheckInItem,      "存入远程仓库"},
		{"?CheckOutItem@CHero",    NULL,  0, &m_GameCall.CheckOutItem,     "取出远程仓库"},
		{"?OpenBank@CHero",        NULL,  0, &m_GameCall.OpenBank,         "打开远程仓库"},
		{"?TransmByMemoryStone@CHero",    NULL,  0, &m_GameCall.TransmByMemoryStone, "使用可传送物品"},
		{"?MagicAttack@CHero",     "POS", 0, &m_GameCall.MagicAttack_GWID, "使用技能-怪物ID "},
		{"?MagicAttack@CHero",     "POS", 1, &m_GameCall.MagicAttack_XY,   "使用技能-XY坐标"},
		{"?CallEudenmon@CHero",    NULL,  0, &m_GameCall.CallEudenmon,     "宠物出征"},
		{"?KillEudenmon@CHero",    NULL,  0, &m_GameCall.KillEudenmon,     "宠物召回"},
		{"?AttachEudemon@CHero",   NULL,  0, &m_GameCall.AttachEudemon,    "宠物合体"},
		{"?UnAttachEudemon@CHero", NULL,  0, &m_GameCall.UnAttachEudemon,  "宠物解体"},
		{"?GetData@CPlayer",       NULL,  0, &m_GameCall.SetRealLife,      "CPlayer::GetData"},
		{"?QueryInterface_RemoteTeam@CHero", NULL,  0, &m_GameCall.QueryInf_RemoteTeam,"获取远程邀请人物基址"},
		{"?IsHaveTeam@CHero",      NULL,  0, &m_GameCall.IsHaveTeam,       "是否有队伍"},
		{"?IsTeamLeader@CHero",    NULL,  0, &m_GameCall.IsTeamLeader,     "是否是队长"},
		{"?TeamCreate@CHero",      NULL,  0, &m_GameCall.TeamCreate,       "创建队伍"},
		{"?TeamDismiss@CHero",     NULL,  0, &m_GameCall.TeamDismiss,      "离开队伍[队长]"},
		{"?TeamLeave@CHero",       NULL,  0, &m_GameCall.TeamLeave,        "离开队伍[队员]"},
		{"?TeamInvite@CHero",      NULL, 0, &m_GameCall.TeamInvite,        "邀请加入"},
		{"?SetAutoJoinStatus@CHero",NULL, 0, &m_GameCall.TeamAutoJoin,     "自动加入"},
		{"?GetMagicAmount@CHero",   NULL, 0, &m_GameCall.GetMagicAmount,   "技能数量"},
		{"?GetMagicByIndex@CHero",  NULL, 0, &m_GameCall.GetMagicByIndex,  "技能基址地址"},

		{"?GetPlayerAmount@CGamePlayerSet",  NULL,0, &m_GameCall.GetPlayerAmount,  "获取人物数量"},
		{"?GetPlayerByIndex@CGamePlayerSet", NULL,0, &m_GameCall.GetPlayerByIndex, "获取人物基址"},
		{"?Process@CGamePlayerSet",          NULL,0, &m_GameCall.HookProcess,      "人物坐标设置"},
	};

	DWORD length = sizeof(info) / sizeof(SearchModFuncMsg);
	for (DWORD i = 0; i < length; i++) {
		SearchFuncInMode(&info[i], (HANDLE)m_GameModAddr.Mod3DRole);
	}

	::printf("\n--------------------------------\n");
	m_GameCall.User32Ret04 = FindUser32Ret04();
	m_GameCall.NPCTalk = FindNPCTalkCall();
	m_GameCall.TeamChk = FindTeamChkCall();
	//m_GameCall.KeyNum = FindKeyNumCall();
	//m_GameCall.CloseTipBox = FindCloseTipBoxCall();
	//m_GameCall.GetNpcBaseAddr = FindGetNpcBaseAddr();
	::printf("\n--------------------------------\n");

	m_Ecx = P2DW(g_pObjHero);
}

// 获取user32 ret 04地址
DWORD Game::FindUser32Ret04()
{
	BYTE codes[] = {
		//0x5d,             // pop ebp
		0xc2, 0x04, 0x00  // ret 0004
	};

	DWORD address = SearchInModByte(L"user32", codes, sizeof(codes));
	if (address) {
		::printf("FindUser32Ret04:%08X\n", address);
	}
	return address;
}

// 获取NPC对话函数
DWORD Game::FindNPCTalkCall()
{
	// 从mov ecx,dword ptr ds:[ecx+esi+0x1710]开始搜索
	BYTE codes[] = {
		0x8b,0x8c,0x31,0x11,0x11,0x11,0x11,  0x8b,0x92,0x11,0x11,0x11,0x11,
		0x6a,0x00,  0x51,  0x8b,0xc8,  0xff,0xd2,  0x6a,0x00,  0x8b,0xce,
	};

	DWORD address = SearchInModByte(L"soul.exe", codes, sizeof(codes));
	if (address) {
		DWORD tmp = address;
		DWORD offset = P2DW(address + 0x19);
		address += offset + 0x1D;
		::printf("CallNPC对话选择2:%08X %08X\n", address, tmp);

		BYTE* data = (BYTE*)tmp;
		for (int i = 0; i < 0x1000; i++) {
			if (IsBadReadPtr(data + i, 1))
				break;

			if (//data[i] == 0x5d // pop ebp
				data[i] == 0xc2 && data[i + 1] == 0x0c && data[i + 2] == 0x00) { // ret 000c
				m_GameCall.SoulRet0C = (DWORD)(data + i);
				::printf("SoulRet0C:%08X\n", m_GameCall.SoulRet0C);
				break;
			}
		}
	}
	return address;
}

// 获取副本邀请队伍选择框函数
DWORD Game::FindTeamChkCall()
{
	return 123;
#if 0
	// 从push 0xFFFFFF开始搜索
	BYTE codes[] = {
		0x68,0xff,0xff,0xff,0x00,  0x8b,0xc8,  0x8b,0x82,0x11,0x11,0x11,0x11,
		0x68,0x11,0x11,0x11,0x11,  0xff,0xd0,  0x8b,0x8e,0x11,0x11,0x11,0x11,
		0x50,  0xe8,0x11,0x11,0x11,0x11,  0x6a,0x01,  0x8b,0x8e,0x11,0x11,0x11,0x11,
	};

	DWORD address = SearchInModByte(L"soul.exe", codes, sizeof(codes));
	if (address) {
		::printf("FindTeamChkCall %08X\n", address);
		DWORD tmp = address;
		m_GameAddr.TeamChkOffset = P2DW(address + 0x24);
		DWORD offset = P2DW(address + 0x29);
		address += offset + 0x2D;
		::printf("Call副本邀请队伍选择:%08X %08X\n", address, tmp);
		::printf("副本邀请队伍选择ESI偏移:%08X\n", m_GameAddr.TeamChkOffset);
	}
	return address;
#endif
}

// 数字按键函数
DWORD Game::FindKeyNumCall()
{
	return 0;
#if 0
	// 从push 0x00开始搜索
	DWORD codes[] = {
		0x006A006A, 0x1234006A, 0x1234006A, 0x016A068B,
		0x1234EB50, 0x51006A0E
	};

	DWORD address = 0;
	if (SearchInMod(L"soul.exe", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		DWORD tmp = address;
		m_GameAddr.KeyNumEcxPtr = P2DW(address + 0x1A);
		DWORD offset = P2DW(address + 0x1F);
		address += offset + 0x23;
		::printf("Call数字按键:%08X %08X\n", address, tmp);
		::printf("数字按键ECX指针地址:%08X\n", m_GameAddr.KeyNumEcxPtr);
	}
	return address;
#endif
}

// 获取关闭提示框函数
DWORD Game::FindCloseTipBoxCall()
{
	return 123;
#if 0
	// mov dword ptr ds:[ebx+0xA0],esi
	// 搜索方法->CE提示框状态地址下修改断点->找到修改地址->OD下断点, 查看调用堆栈即可找到
	DWORD codes[] = {
		0x51EC8B55, 0x8B575653, 0xFF83087D, 0x0FD98B0A,
		0x00000011, 0x0C751234
	};

	DWORD address = 0;
	if (SearchInMod(L"soul.exe", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		::printf("Call关闭提示框函数地址:%08X\n", address);
	}
	return address;
#endif
}

// 获取获取NPC基地址函数
DWORD Game::FindGetNpcBaseAddr()
{
	// 搜索方法 找到NPC基址->下访问断点，点击NPC身上的断点, 找到合适断点下断
	// [在CGamePlayerSet::Process里面]mov dword ptr ds:[ecx+0x44],edx
	// 里面寻找即可找到 add ecx,0x14
	DWORD codes[] = {
		0x83EC8B55, 0x4D8924EC, 0x08458BDC, 0xE04D8D50,
		0xF0558D51, 0xDC4D8B52
	};

	DWORD address = 0;
	if (SearchInMod(L"3drole.dll", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		::printf("Call获取获取NPC基地址:%08X\n", address);
	}
	return address;
}

// 获取模块地址
DWORD Game::FindModAddr(LPCWSTR name)
{
	HANDLE hMod = NULL;
	while (hMod == NULL) {
		hMod = GetModuleHandle(name);
		::wprintf(L"%ws:%08X\n", name, hMod);
	}
	return (DWORD)hMod;
}

// 获取登录选择的小区地址
bool Game::FindLoginSmallServerAddr()
{
	// 2C0
	// 4:* 4:0x00 4:0x10 4:* 4:0xFFFFFFFF 4:0x00 4:0x00 4:0x00 4:0x7D0 4:0x7D0 4:0x00 4:0x00
	DWORD codes[] = {
		0x00000011, 0x00000000, 0x00000010, 0x00000011,
		0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000,
		0x000007D0, 0x000007D0, 0x00000000, 0x00000000
	};

	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.SmallServer = address + 0x2C0;
		::printf("选择的小区：%08X %s\n", m_GameAddr.SmallServer, m_GameAddr.SmallServer);
	}
	return address > 0;
}

// 获取坐标地址
bool Game::FindCoorAddr()
{
	while (!IsLogin()) {
		::printf("等待登录游戏！！！\n");
		Sleep(5000);
	}

	try {
		DWORD p;
		//ReadDwordMemory(0x123, p);
	}
	catch (...) {
		::printf("错误PPPPPPPPPPPPPPPPPPPPPPPPP\n");
	}

	m_GameAddr.CoorX = m_GameModAddr.Mod3DRole + ADDR_COOR_X_OFFSET;
	m_GameAddr.CoorY = m_GameModAddr.Mod3DRole + ADDR_COOR_Y_OFFSET;
	while (PtrToDword(m_GameAddr.CoorX) == 0 || PtrToDword(m_GameAddr.CoorY) == 0) {
		::printf("正在进入游戏！！！\n");
		HWND child, parent;
		if (FindButtonWnd(BUTTON_ID_LOGIN, child, parent)) { // 有选择人物登入按钮
			//::printf("发现选择角色按钮\n");
			if (1 && m_Account.RoleNo > 0) {
				::printf("选择角色:%d\n", m_Account.RoleNo + 1);
				HWND child2, parent2;
				int btn_id = BUTTON_ID_ROLENO + m_Account.RoleNo;
				FindButtonWnd(btn_id, child2, parent2, "角色");
				SendMessageW(parent2, WM_COMMAND, MAKEWPARAM(btn_id, BN_CLICKED), (LPARAM)child2); // 选择角色
				
				RECT rect;
				GetWindowRect(child2, &rect);
				//Drv_LeftClick(float(rect.left + 20) / m_fScale, float(rect.top + 20) / m_fScale);
				::printf("按钮ID:%X %08X %08X %d %d %d %d\n", btn_id, child2, parent2, rect.left, rect.top, rect.right, rect.bottom);
				Sleep(500);
				GetWindowRect(child, &rect);
				//Drv_LeftClick(float(rect.left + 10) / m_fScale, float(rect.top + 10) / m_fScale);
				::printf("按钮ID2:%X %08X %08X %d %d %d %d\n", btn_id, child, parent, rect.left, rect.top, rect.right, rect.bottom);
				SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_LOGIN, BN_CLICKED), (LPARAM)child); // 登入
			}
		}
		Sleep(1500);
	}
	::printf("人物坐标:%d,%d\n", PtrToDword(m_GameAddr.CoorX), PtrToDword(m_GameAddr.CoorY));

	HWND parent;
	FindButtonWnd(BUTTON_ID_MAGIC, m_hWndItem, parent);
	FindButtonWnd(BUTTON_ID_MAGIC, m_hWndMagic, parent);

	strcpy(m_Account.Name, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ACCOUNT_NAME));
	strcpy(m_Account.Role, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ROLE_NAME));
	::printf("帐号:%s 角色:%s\n", m_Account.Name, m_Account.Role);
	m_pClient->SendInGame(m_Account.Name, m_Account.Role); // 通知人物已上线
	return true;
}

// 获取移动状态地址
bool Game::FindMoveStaAddr()
{
#if 0
	DWORD codes[] = {
		0x000064, 0x000064, 0x000000, 0x000000, 0x0FD308, 0x000000, 0x000000, 0x000001,
		0x0F0084, 0x000000, 0x000000, 0x000000, 0x0000FF, 0x021084, 0x06B6C1, 0x000000,
	};
	//+0xB0
#else
	// 4:0x043D4C74 4:0x00 4:0xFFFFFFFF 4:0x01 4:0x00 4:0x00 4:0x01E0FDDC 4:0x00
	// 4:0x00 4:0x02 4:0x00 4:0x00 4:0x06 4:0x00 4:0x64 4:0x64 4:0x00 4:0x00 4:0x00 4:0x00
	// +19C 77D8
#if 0
	DWORD codes[] = {
		
		0x12344C74, 0x00000000, 0xFFFFFFFF, 0x00000001,
		0x00000000, 0x00000000, 0x1234FDDC, 0x00000000
	};
#else
	DWORD codes[] = {
		0x00000000, 0x00000002, 0x00000000, 0x00000000,
		0x00000011, 0x00000000, 0x00000064, 0x00000064,
		0x00000000, 0x00000000, 0x00000011, 0x00000011
	};
#endif
#endif;
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		//printf("address=%08X\n", address);
		if ((address & 0xff) == 0x08) {
			//m_GameAddr.MovSta = address + 0x190;
			m_GameAddr.MovSta = address + 0x30;
			::printf("移动状态地址：%08X\n", m_GameAddr.MovSta);
		}
	}
	return address > 0;
}

// 获取对话框状态地址
bool Game::FindTalkBoxStaAddr()
{
	// 4:0x018664FC 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	DWORD codes[] = {
		0x018664FC, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.CallNpcTalkEsi = address;
		m_GameAddr.TalKBoxSta = address + 0xB4;
		::printf("NPC二级对话ESI:%08X\n", m_GameAddr.CallNpcTalkEsi);
		::printf("对话框状态地址：%08X\n", m_GameAddr.TalKBoxSta);
	}
	return address > 0;
}

// 获取是否选择邀请队伍状态地址
bool Game::FindTeamChkStaAddr()
{
	return false;
#if 0
	// 3104
	// 4:0x01764D2C 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	// 11F88BAC
	DWORD codes[] = {
		0x01764D2C, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0; // 这个地址是Call选择是否邀请队伍函数中esi的值 mov ecx,dword ptr ds:[esi+0x1DC]
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.TeamChkSta = address; // +2F68是TeamChkSta偏移头 再+134是选择框状态[0162976C]
		::printf("邀请队伍状态地址：%08X\n", m_GameAddr.TeamChkSta);
	}
	return address > 0;
#endif
}

// 获取提示框状态地址
bool Game::FindTipBoxStaAddr()
{
	return false;
#if 0
	// 4:0x00F39588 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	DWORD codes[] = {
		0x00F39588, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.TipBoxSta = address;
		::printf("提示框状态地址：%08X\n", m_GameAddr.TipBoxSta);
	}
	return address > 0;
#endif
}

// 获取生命地址
bool Game::FindLifeAddr()
{
#if 0
	m_GameAddr.Life = m_GameModAddr.Mod3DRole + ADDR_LIFE_OFFSET;
	m_GameAddr.LifeMax = m_GameModAddr.Mod3DRole + ADDR_LIFEMAX_OFFSET;
	::printf("找到生命地址：%08X\n", m_GameAddr.Life);
	::printf("找到生命上限地址：%08X\n", m_GameAddr.LifeMax);
	return true;
#else
	// 4:* 4:0x00 4:* 4:0x03 4:0x0A 4:0x18 4:0x29 4:0x00
	DWORD codes[] = {
		0x00000003, 0x0000000A, 0x00000018, 0x00000029, 0x00000000
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		m_GameAddr.Life = address + 0x18;
		m_GameAddr.LifeMax = m_GameModAddr.Mod3DRole + ADDR_LIFEMAX_OFFSET;
		::printf("找到生命地址：%08X\n", m_GameAddr.Life);
		::printf("找到生命上限地址：%08X\n", m_GameAddr.LifeMax);
	}
	return address > 0;
#endif
}

// 获取背包代码
bool Game::FindBagAddr()
{
	// Soul.exe+C8C12C
	// Soul.exe + CA97C8
	// 4:0x281 4:0x1E6 4:0xA04 4:0x05 4:0x05 4:0x1E1E
	// 023C78D5 
	// 搜索方法->CE查找物品数量->下访问断点, 获得基址->基址下访问断点得到偏移
	// CHero::GetItem函数里面 具体哪个位置需要配合上面的查看
	try {
		DWORD p, count;
#if 0
		__asm
		{
			mov eax, g_pObjHero
			mov eax, [eax]
			mov eax, [eax + 0x26C4]     // 2234
			mov edx, [eax + 0x10]       // 物品地址指针
			mov dword ptr[p], edx
			mov edx, [eax + 0x20]       // 物品数量
			mov dword ptr[count], edx
		}
#else
		p = Call_GetItemByIndex(0);
		count = Call_GetItemAmount();
#endif
		::printf("物品指针:%08X 数量:%d\n", p, count);
		//m_GameAddr.Bag = p;

		return p != 0;
	}
	catch (...) {
		::printf("Game::FindBagAddr失败!\n");
	}
	return false;
}

// 获得地面物品地址的保存地址
bool Game::FindItemPtr()
{
	// 4:0x00 4:0x04 4:0x80000000 4:0x80000000 4:0x7FFFFFFF 4:0x7FFFFFFF 4:* 4:0x01 4:0x19 4:0x00400000
	DWORD codes[] = {
		0x00000000, 0x00000004, 0x80000000, 0x80000000,
		0x7FFFFFFF, 0x7FFFFFFF, 0x00000011, 0x00000011,
	};
	DWORD address;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.ItemPtr = address + 0x38;
		INLOGVARN(32, "地面物品地址保存地址:%08X", m_GameAddr.ItemPtr);
		::printf("地面物品地址保存地址:%08X\n", m_GameAddr.ItemPtr);
	}

	return address != 0;
}

// 获取NPC二级对话ESI寄存器数值
bool Game::FindCallNPCTalkEsi()
{
	return true;
	// 0x00F6FE30
	DWORD codes[] = {
		0x00FB1B10, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.CallNpcTalkEsi = address;
		::printf("NPC二级对话ESI:%08X\n", m_GameAddr.CallNpcTalkEsi);
	}

	return address != 0;
}

// 获取宠物列表基地址
bool Game::FindPetPtrAddr()
{
	return false;
#if 0
	// 搜索方法 先找出宠物血量->CE下血量访问断点, 找到基址, 查看特征码
	// 宠物ID[773610E3宠物名字:800750]->获得宠物ID地址->下访问断点->找到偏移
	// 4:0x00F855F8 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	// 774D077B[猫右] 77521F67[娜迦]
	// 775E02AF
	// 1F0B0
#if 0
	DWORD address = NULL;
	__asm
	{
		mov ecx, dword ptr ds : [BASE_PET_OFFSET]
		mov ecx, dword ptr ds : [ecx + 0x50C]
		mov ecx, dword ptr ds : [ecx + 0x458]
		mov dword ptr[address], ecx
	}
	m_GameAddr.PetPtr = address;
	::printf("宠物列表基址:%08X\n", address);
#else
	// 4:0x017BEF6C 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	DWORD codes[] = {
		0x017BEF6C, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022,
		//0x00000022, 0x00000000, 0x00000010, 0x00000022,
		//0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000,
		//0x0000016B, 0x0000016B, 0x00000000, 0x00000000,
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.PetPtr = address;
		::printf("宠物列表基址:%08X [%08X] %08X\n", address, PtrToDword(address), codes);
	}
#endif
	return address != 0;
#endif
}

// 获取地图名称地址
bool Game::FindMapName()
{
	// 4:* 4:0x3E8 4:0x3E8 4:0x3E8 4:0x00 4:0x00200000 4:0x00120080 4:0x00 4:0x00 4:0x01 4:0xFFFFFFFF 4:0xFFFFFFFF
	DWORD codes[] = {
		0x00000011, 0x00000022, 0x00000022, 0x00001102,
		0x00000011, 0x00000022, 0x00000022, 0x00000000,
		0x00000000, 0x00000001, 0x00000022, 0xFFFFFFFF,
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		if (address < 0x60000000) {
			m_GameAddr.MapName = address + 0x30;
			char* name = (char*)m_GameAddr.MapName;
			::printf("地图名称地址:%08X[%s] %d\n", m_GameAddr.MapName, name, name[0]);

			if (name[0] == 0 || name[0] == 0xff) {
				//m_pClient->SendMsg("没有找到地图名称, 重启游戏机.|red");
				//Close(false);
			}
			else {
				char str[32];
				sprintf_s(str, "地图名称:%s", name);
				m_pClient->SendMsg(str);
			}
		}
	}
	return address != 0;
}

// 在某个模块中搜索函数
DWORD Game::SearchFuncInMode(SearchModFuncMsg* info, HANDLE hMod)
{
	//得到DOS头
	IMAGE_DOS_HEADER* dosheader = (PIMAGE_DOS_HEADER)hMod;
	//效验是否DOS头
	if (dosheader->e_magic != IMAGE_DOS_SIGNATURE)return NULL;

	//NT头
	PIMAGE_NT_HEADERS32 NtHdr = (PIMAGE_NT_HEADERS32)((CHAR*)hMod + dosheader->e_lfanew);
	//效验是否PE头
	if (NtHdr->Signature != IMAGE_NT_SIGNATURE)return NULL;

	//得到PE选项头
	IMAGE_OPTIONAL_HEADER32* opthdr = (PIMAGE_OPTIONAL_HEADER32)((PBYTE)hMod + dosheader->e_lfanew + 24);
	//得到导出表
	IMAGE_EXPORT_DIRECTORY* pExportTable = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)hMod + opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	//得到函数地址列表
	PDWORD arrayOfFunctionAddresses = (PDWORD)((PBYTE)hMod + pExportTable->AddressOfFunctions);
	//得到函数名称列表
	PDWORD arrayOfFunctionNames = (PDWORD)((PBYTE)hMod + pExportTable->AddressOfNames);
	//得到函数序号
	PWORD arrayOfFunctionOrdinals = (PWORD)((PBYTE)hMod + pExportTable->AddressOfNameOrdinals);
	//导出表基地址
	DWORD Base = pExportTable->Base;
	//循环导出表
	for (DWORD x = 0; x < pExportTable->NumberOfNames; x++)			//导出函数有名称 编号之分，导出函数总数=名称导出+编号导出，这里是循环导出名称的函数
	{
		//得到函数名 
		PSTR functionName = (PSTR)((PBYTE)hMod + arrayOfFunctionNames[x]);
		if (strstr(functionName, info->Name)) { // 存在此名称
			bool find = true;
			if (info->Substr) { // 搜索子串
				PSTR sub = strstr(functionName, info->Substr);
				if ((sub && !info->Flag) || (!sub && info->Flag)) { // 不符合要求
					find = false;
				}
			}
			if (find == true) {
				DWORD functionOrdinal = arrayOfFunctionOrdinals[x];
				*info->Save = (DWORD)hMod + arrayOfFunctionAddresses[functionOrdinal];
				::printf("Call%s:%08X:%s\n", info->Remark, *info->Save, functionName);
				goto end;
			}
		}
	}
end:
	return *(info->Save);
}

// 在某个模块里面搜索
DWORD Game::SearchInMod(LPCTSTR name, DWORD * codes, DWORD length, DWORD * save, DWORD save_length, DWORD step)
{
	DWORD result = 0;
	DWORD dwSize = 0;
	HANDLE hMod = GetModuleBaseAddr(GetCurrentProcessId(), name, &dwSize);
	//::printf("hMod:%08X %08X\n", hMod, dwSize);

	if (dwSize) {
		DWORD page = 0x1000;
		DWORD addr = 0;
		while (addr < dwSize) {
			m_dwReadBase = (DWORD)hMod + addr;
			m_dwReadSize = page;
			if (ReadProcessMemory(m_hGameProcess, (LPVOID)m_dwReadBase, m_pReadBuffer, m_dwReadSize, NULL)) {
				if (SearchCode(codes, length, save, save_length, step)) {
					//::printf("SearchInMod.....\n");
					result = save[0];
					break;
				}
			}
			addr += page;
		}
	}

	return result;
}

// 在某个模块里面搜索
DWORD Game::SearchInModByte(LPCTSTR name, BYTE * codes, DWORD length)
{
	DWORD result = 0;
	DWORD dwSize = 0;
	HANDLE hMod = GetModuleBaseAddr(GetCurrentProcessId(), name, &dwSize);
	//::printf("hMod:%08X %08X\n", hMod, dwSize);

	if (dwSize) {
		DWORD page = 0x1000;
		DWORD addr = 0;
		while (addr < dwSize) {
			m_dwReadBase = (DWORD)hMod + addr;
			m_dwReadSize = page;
			if (ReadProcessMemory(m_hGameProcess, (LPVOID)m_dwReadBase, m_pReadBuffer, m_dwReadSize, NULL)) {
				result = SearchByteCode(codes, length);
				if (result) {
					break;
				}
			}
			addr += page;
		}
	}

	return result;
}

// 搜索特征码
DWORD Game::SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length, DWORD step)
{
	if (length == 0 || save_length == 0)
		return 0;

	DWORD count = 0;
	for (DWORD i = 0; i < m_dwReadSize; i += step) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		if (addr == (DWORD)codes) { // 就是自己
			//::printf("搜索到了自己:%08X\n", codes);
			return 0;
		}

		DWORD* dw = (DWORD*)(m_pReadBuffer + i);
		bool result = true;
		for (DWORD j = 0; j < length; j++) {
			if (codes[j] == 0x11) { // 不检查
				result = true;
			}
			else if (codes[j] == 0x22) { // 需要此值不为0
				if (dw[j] == 0) {
					result = false;
					break;
				}
			}
			else if (((codes[j] & 0xffff0000) == 0x12340000)) { // 低2字节相等
				if ((dw[j]&0x0000ffff) != (codes[j]&0x0000ffff)) {
					result = false;
					break;
				}
				else {
					//::printf("%08X\n", dw[j]);
				}
			}
			else if (((codes[j] & 0x0000ffff) == 0x00001234)) { // 高2字节相等
				if ((dw[j] & 0xffff0000) != (codes[j] & 0xffff0000)) {
					result = false;
					break;
				}
			}
			else {
				if ((codes[j] & 0xff00) == 0x1100) { // 比较两个地址数值相等 最低8位为比较索引
					//::printf("%X:%X %08X:%08X\n", j, codes[j] & 0xff, dw[j], dw[codes[j] & 0xff]);
					if (dw[j] != dw[codes[j] & 0xff]) {
						result = false;
						break;
					}
				}
				else if (dw[j] != codes[j]) { // 两个数值不相等
					result = false;
					break;
				}
			}
		}

		if (result) {
			save[count++] = addr;
			//::printf("地址:%08X   %08X\n", addr, codes);
			if (count == save_length)
				break;
		}
	}

	return count;
}

// 搜索特征码
DWORD Game::SearchByteCode(BYTE * codes, DWORD length)
{
	if (length == 0)
		return 0;

	for (DWORD i = 0; i < m_dwReadSize; i++) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		if (addr == (DWORD)codes) { // 就是自己
			//::printf("搜索到了自己:%08X\n", codes);
			return 0;
		}

		BYTE* data = (m_pReadBuffer + i);
		bool result = true;
		for (DWORD j = 0; j < length; j++) {
			if (codes[j] == 0x11) { // 不检查
				result = true;
			}
			else if (codes[j] == 0x22) { // 需要此值不为0
				if (data[j] == 0) {
					result = false;
					break;
				}
			}
			else if (codes[j] != data[j]) {
				result = false;
				break;
			}
		}

		if (result)
			return addr;
	}

	return 0;
}

// 读取四字节内容
bool Game::ReadDwordMemory(DWORD addr, DWORD& v)
{
	return ReadMemory((PVOID)addr, &v, 4);
}

// 读取内存
bool Game::ReadMemory(PVOID addr, PVOID save, DWORD length)
{
	if (IsBadReadPtr(addr, length)) {
		::printf("错误地址:%08X %d\n", addr, length);
		return false;
	}

	bool mod = false;
	DWORD oldProtect;
#if 0
	mod = VirtualProtect(addr, length, PAGE_EXECUTE_READWRITE, &oldProtect);
	if (!mod)
		::printf("!VirtualProtect\n");
#endif

	DWORD dwRead = 0;
	bool result = ReadProcessMemory(m_hGameProcess, addr, save, length, &dwRead);
	//::printf("ReadProcessMemory:%d %08X %d Read:%d 数值:%d(%08X)\n", GetLastError(), addr, result, dwRead, *(DWORD*)save, *(DWORD*)save);

	if (!result || dwRead != length) {
		::printf("ReadProcessMemory错误:%d %08X %d\n", GetLastError(), addr, result);
		if (GetLastError() == 6) {
			m_pGameProc->WriteLog("GetLastError() == 6", true);
			Sleep(100);
			//CloseHandle(m_hGameProcess);
			m_pGameProc->WriteLog("关闭m_hGameProcess", true);
			Sleep(100);
			m_hGameProcess = GetCurrentProcess();
			return ReadProcessMemory(m_hGameProcess, addr, save, length, NULL);
		}
	}

	if (mod)
		VirtualProtect(addr, length, oldProtect, &oldProtect);
	return result;
}

// 读取坐标
bool Game::ReadCoor(DWORD* x, DWORD* y)
{
#if 0
	if (x) {
		*x = m_dwX;
	}
	if (y) {
		*y = m_dwY;
	}
	return true;
#endif
start:
	DWORD vx = 0, vy = 0;
	if (!ReadDwordMemory(m_GameAddr.CoorX, vx)) {
		::printf("无法读取坐标X(%d) %08X\n", GetLastError(), m_GameAddr.CoorX);
		return false;
	}
	if (!ReadDwordMemory(m_GameAddr.CoorY, vy)) {
		::printf("无法读取坐标Y(%d) %08X\n", GetLastError(), m_GameAddr.CoorY);
		return false;
	}

	m_dwX = vx;
	m_dwY = vy;

	if (x) {
		*x = m_dwX;
	}
	if (y) {
		*y = m_dwY;
	}

	return true;
}

// 读取生命值
bool Game::ReadLife(int& life, int& life_max)
{
	ReadDwordMemory(m_GameAddr.Life, (DWORD&)life);
	life_max = m_dwLifeMax;
	if (!m_dwLifeMax) {
		ReadDwordMemory(m_GameAddr.LifeMax, (DWORD&)life_max);
		m_dwLifeMax = life_max;
	}
	
	return true;	
}

// 读取药包数量
bool Game::ReadQuickKey2Num(int* nums, int length)
{
	return false;
#if 0
	if (!m_GameAddr.QuickKey2Num) {
		memset(nums, 0, length);
		return false;
	}

	return ReadProcessMemory(m_hGameProcess, (PVOID)m_GameAddr.QuickKey2Num, nums, length, NULL);
#endif
}

// 读取包包物品
bool Game::ReadBag(DWORD* bag, int length)
{
	return ReadProcessMemory(m_hGameProcess, (PVOID)m_GameAddr.Bag, bag, length, NULL);
}

// 获得窗口句柄
bool Game::FindButtonWnd(int button_id, HWND& hwnd, HWND& parent, const char* text)
{
	HWND wnds[] = { (HWND)button_id, NULL, (HWND)text };
	::EnumChildWindows(m_hWnd, EnumChildProc, (LPARAM)wnds);
	hwnd = wnds[0];
	parent = wnds[1];
	return parent != NULL;
}

// 读取游戏内存
bool Game::ReadGameMemory(DWORD flag)
{
	m_dwGuaiWuCount = 0; // 重置怪物数量
	m_bIsReadEnd = false;

	MEMORY_BASIC_INFORMATION mbi;
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	DWORD_PTR MaxPtr = 0x70000000; // 最大读取内存地址
	DWORD_PTR max = 0;


	__int64 ms = getmillisecond();
	DWORD_PTR ReadAddress = 0x00000000;
	ULONG count = 0, failed = 0;
	//::printf("fuck\n");
	while (ReadAddress < MaxPtr)
	{
		SIZE_T r = VirtualQueryEx(m_hGameProcess, (LPCVOID)ReadAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
		SIZE_T rSize = 0;

		//::printf("r:%d\n", r);
		//::printf("ReadAddress:%08X - %08X-----%X\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
		if (r == 0) {
			::printf("r:%d -- %p\n", (int)r, ReadAddress);
			break;
		}
		K32GetMappedFileNameA;
#if 0
		if (flag & 0x100) {
			if (mbi.State == MEM_COMMIT &&
				(mbi.Type == MEM_IMAGE || mbi.Type == MEM_MAPPED)) {
				char image[MAX_PATH] = { 0 };
				GetMappedFileNameA(m_hGameProcess, (LPVOID)ReadAddress, image, MAX_PATH);
				::printf("%p-%p %s %08X %08X\n", ReadAddress, ReadAddress + mbi.RegionSize, image, mbi.Type, &Game::Run);
				if (strlen(image) > 0) {
					if (strstr(image, "MyLogic.dll")) {
						m_bMd = true;
					}
					else if (m_bMd) {
						m_bMd = false;
					}
				}
				else {
					
				}
			}
		}
#endif
		if (mbi.Type != MEM_PRIVATE && mbi.Protect != PAGE_READWRITE) {
			//::printf("%p-%p ===跳过, 大小:%d\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
			goto _c;
		}
		else {
			DWORD pTmpReadAddress = ReadAddress;
			DWORD dwOneReadSize = 0x1000; // 每次读取数量
			DWORD dwReadSize = 0x00;      // 已读取数量
			while (dwReadSize < mbi.RegionSize) {
				m_dwReadBase = pTmpReadAddress;
				m_dwReadSize = (dwReadSize + dwOneReadSize) <= mbi.RegionSize 
					? dwOneReadSize : mbi.RegionSize - dwReadSize;

				SIZE_T szReadSize = 0;
				if (ReadProcessMemory(m_hGameProcess, (LPVOID)pTmpReadAddress, m_pReadBuffer, m_dwReadSize, &szReadSize)) {
					if (flag & 0x100) {
						FindLoginSmallServerAddr();
					}

					//::printf("flag:%08X %p-%p\n", flag, ReadAddress, ReadAddress + mbi.RegionSize);

					if (flag & 0x01) {
						if (!m_GameAddr.MovSta) {
							FindMoveStaAddr();
						}
						if (!m_GameAddr.TalKBoxSta) {
							FindTalkBoxStaAddr();
						}
						if (!m_GameAddr.TipBoxSta) {
							FindTipBoxStaAddr();
						}
						if (!m_GameAddr.TeamChkSta) {
							FindTeamChkStaAddr();
						}
						if (!m_GameAddr.Life) {
							FindLifeAddr();
						}
						if (!m_GameAddr.CallNpcTalkEsi) {
							FindCallNPCTalkEsi();
						}
						if (!m_GameAddr.ItemPtr) {
							FindItemPtr();
						}
						if (!m_GameAddr.PetPtr) {
							FindPetPtrAddr();
						}
						if (!m_GameAddr.MapName) {
							FindMapName();
						}
					}
					if (!flag)
						goto end;
				}
				else {
					//::printf("%p-%p ===读取失败, 大小:%d, 错误码:%d\n", pTmpReadAddress, pTmpReadAddress + m_dwReadSize, (int)m_dwReadSize, GetLastError());
				}

				dwReadSize += m_dwReadSize;
				pTmpReadAddress += m_dwReadSize;
			}

			count++;
		}
	_c:
		// 读取地址增加
		ReadAddress += mbi.RegionSize;
		//Sleep(8);
		//::printf("%016X---内存大小2:%08X\n", ReadAddress, mbi.RegionSize);
		// 扫0x10000000字节内存 休眠100毫秒
	}
end:
	__int64 ms2 = getmillisecond();
	::printf("读取完内存用时:%d毫秒\n", ms2 - ms);
	m_bIsReadEnd = true;
	return true;
}

// 打印日记
void Game::InsertLog(char * text)
{
	//g_dlg->InsertLog(text);
}

// 打印日记
void Game::InsertLog(wchar_t * text)
{
	//g_dlg->InsertLog(text);
}

// 获取大区点击坐标
void Game::GetSerBigClickPos(int& x, int& y)
{
	int vx = 200, vy = 33;
	Explode arr("-", m_Account.SerBig);
	printf("arr:%d-%d\n", arr.GetValue2Int(0), arr.GetValue2Int(1));
	SET_VAR2(x, arr.GetValue2Int(0) * vx - vx + 125, y, arr.GetValue2Int(1) * vy - vy + 135);
	printf("x:%d y:%d\n", x, y);
}

// 获取小区点击坐标
void Game::GetSerSmallClickPos(int & x, int & y)
{
	int vy = 38;
	int n = atoi(m_Account.SerSmall);
	printf("n:%d\n", n);
	SET_VAR2(x, 515, y, n * vy - vy + 125);
	printf("x:%d y:%d\n", x, y);
}

// 获取小区中的数字
bool Game::GetSmallSerNum(char ser[], char num[])
{
	char* first = nullptr;
	const char ch[10][3] = {"一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};
	for (int i = 0; i < 10; i++) {
		char* find = strstr(m_Account.SerSmall, ch[i]);
		if (find) {
			if (first == nullptr) {
				first = find;
				strcpy(num, ch[i]);
				::printf("first:%08X, %s\n", first, num);
			}
			else if (find < first) {
				first = find;
				strcpy(num, ch[i]);
				::printf("first:%08X, %s\n", first, num);
			}
		}
	}
	if (first) {
		int j = 0;
		char* s = m_Account.SerSmall;
		for (; s < first; s++, j++) {
			ser[j] = *s;
		}
		ser[j] = 0;
	}
	return first != nullptr;
}

// 获取快捷键按键点击坐标
void Game::GetQuickKeyClickPos(int key, int & x, int & y)
{
	if (key == VK_F1 || key == '1') {
		SET_VAR2(x, 13, y, 10);
		return;
	}
	if (key == VK_F2 || key == '2') {
		SET_VAR2(x, 50, y, 10);
		return;
	}
	if (key == VK_F3 || key == '3') {
		SET_VAR2(x, 85, y, 10);
		return;
	}
	if (key == VK_F4 || key == '4') {
		SET_VAR2(x, 122, y, 10);
		return;
	}
	if (key == VK_F5 || key == '5') {
		SET_VAR2(x, 155, y, 10);
		return;
	}
}

LRESULT Game::CldKeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

// 枚举窗口
BOOL Game::EnumProc(HWND hWnd, LPARAM lParam)
{
	DWORD thepid = *(DWORD*)lParam;
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);
	//::printf("HWND:%08X pid:%d thepid:%d\n", hWnd, pid, thepid);
	if (pid == thepid) {
		char name[64];
		GetWindowTextA(hWnd, name, sizeof(name));
		if (strcmp(name, WND_TITLE_A) == 0) {
			*(HWND*)lParam = hWnd;
			return FALSE;
		}
		
	}
	return TRUE;
}

// 枚举子窗口
BOOL Game::EnumChildProc(HWND hWnd, LPARAM lParam)
{
	HWND* param = (HWND*)lParam;
	HWND hWnd_Child = ::GetDlgItem(hWnd, (int)param[0]);
	if (hWnd_Child) { // 找到了子窗口
		if (param[2]) { // 如果搜索按钮文字
			char text[32];
			GetWindowTextA(hWnd_Child, text, sizeof(text)); // 获取按钮文字
			if (strstr(text, (const char*)param[2]) == nullptr)
				return TRUE;
		}

		param[0] = hWnd_Child; // 子窗口句柄 
		param[1] = hWnd;       // 父窗口句柄
		
		return FALSE;
	}
	return TRUE;
}

// 按键
void Game::Keyborad(int key, HWND hwnd, bool tra)
{
	SetForegroundWindow(m_hWnd);

	if (hwnd == NULL)
		hwnd = m_hWndPic;

	bool is_caps = key >= 'A' && key <= 'Z';
	if (tra) {
		if (key >= 'a' && key <= 'z')
			key -= 32;
	}
	

	LPARAM lParam = (MapVirtualKey(key, 0) << 16) + 1;
#if 1
	if (is_caps) {
		keybd_event(VK_SHIFT, MapVirtualKey(VK_SHIFT, 0), 0, 0);
		Sleep(100);
	}
	keybd_event(key, MapVirtualKey(key, 0), 0, 0);
	Sleep(100);
	keybd_event(key, MapVirtualKey(key, 0), KEYEVENTF_KEYUP, 0);
	if (is_caps) {
		Sleep(100);
		keybd_event(VK_SHIFT, MapVirtualKey(VK_SHIFT, 0), KEYEVENTF_KEYUP, 0);
	}
#else
	SendMessageA(m_hWnd, 0x36E, 0x02, 0x015FFA18);
	Sleep(10);
	SendMessageA(m_hWnd, 0x36E, 0x00, 0x015FF9F0);

	LPARAM lParam = (MapVirtualKey(key, 0) << 16) + 1;
	PostMessageA(hwnd, WM_KEYDOWN, key, lParam);
#if 1
	Sleep(50);
	lParam |= 0xC0000001;
	PostMessageA(hwnd, WM_KEYUP, key, lParam);
#endif
#endif
	//::printf("按键:%d %08X %08X %d\n", key, lParam, hwnd, GetLastError());
}

// 按键
void Game::Keyborad2(int key, HWND hwnd)
{
	if (hwnd == NULL)
		hwnd = m_hWnd2;

	LPARAM lParam = (MapVirtualKey(key, 0) << 16) + 1;
	PostMessageA(hwnd, WM_KEYDOWN, key, lParam);
	Sleep(50);
	lParam |= 0xC0000001;
	PostMessageA(hwnd, WM_KEYUP, key, lParam);

	hwnd = m_hWnd;
	Sleep(100);
	lParam = (MapVirtualKey(key, 0) << 16) + 1;
	PostMessageA(hwnd, WM_KEYDOWN, key, lParam);
	Sleep(50);
	lParam |= 0xC0000001;
	PostMessageA(hwnd, WM_KEYUP, key, lParam);

	hwnd = m_hWndMagic;
	Sleep(100);
	lParam = (MapVirtualKey(key, 0) << 16) + 1;
	PostMessageA(hwnd, WM_KEYDOWN, key, lParam);
	Sleep(50);
	lParam |= 0xC0000001;
	PostMessageA(hwnd, WM_KEYUP, key, lParam);
	::printf("按键:%C[%d] %08X %08X %08X\n", key, key, m_hWnd, m_hWnd2, m_hWndMagic);
	
}

void Game::LeftClick(int x, int y, HWND hwnd)
{
	if (hwnd == NULL)
		hwnd = m_hWndPic; 

#if 0
	SetForegroundWindow(m_hWnd);
	Sleep(200);
	RECT r;
	::GetWindowRect(hwnd, &r);

	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, r.left + x, r.top + y, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_LEFTDOWN, r.left + x, r.top + y, 0, NULL);
	Sleep(25);
	mouse_event(MOUSEEVENTF_LEFTUP, r.left + x, r.top + y, 0, NULL);

	::printf("单击完成:%d,%d %08X\n", r.left + x, r.top + y, hwnd);
#else
	PostMessageA(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	//Sleep(10);
	PostMessageA(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
	PostMessageA(hwnd, WM_LBUTTONUP, 0x00, MAKELPARAM(x, y));
	//PostMessageA(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
	//::printf("单击完成:%d,%d %08X\n", x, y, hwnd);
#endif
}

// 右击
void Game::RightClick(int x, int y, HWND hwnd, int num)
{
	if (hwnd == NULL)
		hwnd = m_hWndPic;

	for (int i = 0; i < num; i++) {
		PostMessageA(hwnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(x, y));
		PostMessageA(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
		PostMessageA(hwnd, WM_RBUTTONUP, 0x00, MAKELPARAM(x, y));
		Sleep(50);
	}
	
	::printf("右击完成:%d,%d %08X\n", x, y, hwnd);
}

// 鼠标移动
void Game::MouseMove(int x, int y, HWND hwnd)
{
	if (hwnd == NULL)
		hwnd = m_hWndPic;

	PostMessageA(hwnd, WM_MOUSEMOVE, 0x00, MAKELPARAM(x, y));
	Sleep(50);
}

// 选择对话选项
void Game::SelectTalkNo(const char* name)
{
	// 用卡利亚堡钥匙开启进入(50,126-235,135)
	// 用爱娜祈祷项链开启进入(50,162-235,168)
	int x = MyRand(50, 235), y = 0;
	if (strcmp(name, "钥匙开启副本") == 0) {
		y = MyRand(126, 135);
	}
	else if (strcmp(name, "项链开启副本") == 0) {
		y = MyRand(162, 168);
	}
	else if (strcmp(name, "继续副本") == 0) {
	}
	else if (strcmp(name, "离开副本") == 0) {
	}
	else if (strcmp(name, "领取特球") == 0) {
	}
	else if (strcmp(name, "领取项链") == 0) {
	}

	SelectTalkNo(x, y);
}

// 选择对话选项
void Game::SelectTalkNo(int x, int y)
{
	HWND talkWnd = FindWindowEx(m_hWnd2, NULL, NULL, NULL);
	RECT rect;
	::GetWindowRect(talkWnd, &rect);

	x += rect.left;
	y += rect.top;
	::SetCursorPos(x, y);
	Sleep(100);
	LeftClick(x, y, talkWnd);
	printf("选择对话选项:%d,%d\n", x, y);
}

void Game::ClickMagic(char key)
{
	// 技能F1(3,10-20,20)
	// 技能F2(35,10-55,20)

	int x = MyRand(3, 20), y = MyRand(10, 20);
	if (key == VK_F1) {
		x = MyRand(3, 20);
	}
	if (key == VK_F2) {
		x = MyRand(35, 55);
	}
	else {
		int offset = (key - VK_F1) * 25;
		x += offset;
	}

	HWND child, parent;
	FindButtonWnd(BUTTON_ID_MAGIC, child, parent, "技能");
	RightClick(x, y, child);
}

// 构造栈
void Game::FakeStackFrame(DWORD stack[], DWORD frames[], DWORD length)
{
	DWORD i = 0, index = 0;
	for (; i < length; i++) {
		stack[index] = (DWORD)&(stack[index + 2]);
		stack[index + 1] = frames[i];
		index += 2;
	}
}

// 人物复活
void Game::Call_ReBoren(int flag)
{
	::printf("人物复活\n");
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC1 func = (_FUNC1)m_GameCall.ReBorn;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push flag
			push soul
			jmp func
		}
	}
	else {
		func(0x00);
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
}

// 人物移动函数
void Game::Call_Run(int x, int y)
{
#if 0
	::printf("移动:%d,%d\n", x, y);
	// 4:0x6AEC8B55 4:0xBE7468FF 4:0xA1640326 4:0x00000000 4:0x25896450 4:0x00000000 4:0x04D8EC81 4:0x56530000
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC3 func = (_FUNC3)m_GameCall.Run;
	//::printf("准备调用移动函数\n");
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push 0x00
			push y
			push x
			push soul
			jmp func
		}
	}
	else {
		func(x, y, 0x00);
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
	//::printf("移动函数调用完成\n");
#endif
}

// 喊话CALL[type 0=公共频道 1=私人 2=队伍]
void Game::Call_Talk(const char* msg, int type)
{
#if 0
	DWORD arg = 0, func = m_GameCall.Talk;
	if (!func)
		return;

	printf("喊话内容:%s (%d)\n", msg, type);
	if (type == 1) { // 公共频道
		arg = 0x07D7;
	}
	else {
		return;
	}

	char* text = (char*)(m_GameAddr.CoorX + 0x30);
	// 15A62D0
	ZeroMemory((PVOID)text, 0x30);
	memcpy((PVOID)text, msg, strlen(msg));

	ASM_STORE_ECX();
	ASM_SET_ECX();
	// B7859a
	__asm {
		push 0x00
		push arg
		push 0x00FFFF00
		push 0x00       // 0x015A63F8 // 可能是变量地址
		push text       // 喊话内容[固定地址]
		push 0x00       // 0x015A63D8 // 可能是变量地址
		call func
	}

	ASM_FAKE_STACK_END();
	ASM_RESTORE_ECX();
#endif
}


_declspec (naked) DWORD __stdcall Naked_NPC(DWORD& ri, DWORD func, int npc_id, DWORD addr, DWORD(&stack)[32])
{
	ASM_SET_ECX();
	__asm
	{
		push ebp;
		mov ebp, esp;

		mov[esp + 8], ebp;

		push 0x00;
		push 0x00;
		push 0x00;
		push _end_;

		push 0x00;
		push npc_id;
		push addr;
		mov eax, func;
		mov ebp, stack;
		jmp eax;
	_end_:
		mov ebp, [esp + 8];

		mov esp, ebp;
		pop ebp;
		ret 20;
	}
}

// NPC对话
void Game::Call_NPC(int npc_id)
{
	//INLOGVARN(32, "打开NPC:%08X\n", npc_id);
	::printf("打开NPC:%08X！！！\n", npc_id);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC2 func = (_FUNC2)m_GameCall.ActiveNpc;
	ASM_STORE_ECX();
	//ASM_SET_ECX();
	if (user32 && soul) {
		DWORD stack[32];
		ZeroMemory(stack, sizeof(stack));
		DWORD frame[] = { 0x6FFDBECC, 0x6FFDBA62, 0x0068122A, 0x6FFDA656, 0x6FFDA8E2, 0x6FFD85CD, 0x74DFBEFB, 0x74DF83DA, 0x74DF7C8E };
		FakeStackFrame(stack, frame, sizeof(frame) / sizeof(DWORD));

#if 1
		DWORD ri;
		Naked_NPC(ri, (DWORD)func, npc_id, soul, stack);
#else
		__asm
		{
			push 0x00
			push npc_id
			push soul
			jmp func
		}
#endif
	}
	else {
		func(npc_id, 0x00);
	}
_end_:
	ASM_FAKE_STACK_END();
	ASM_RESTORE_ECX();
}

// NPC二级对话
void Game::Call_NPCTalk(int no, bool close)
{
	//INLOGVARN(32, "对话NPC：%d\n", no);
	// eax=038735B8-2550000=13235B8
	// edi=0343DDD8-2550000=EEDDD8
	// esi=05DD5314
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC_R func = (_FUNC_R)m_GameCall.ITaskGetInstance;
	DWORD _eax = func();
	DWORD _esi = m_GameAddr.CallNpcTalkEsi;
	::printf("NPC对话选择 no:%d _eax:%08X _edi:%08X _esi:%08X\n", no, _eax, PtrToDword(_eax), _esi);

	ASM_STORE_ECX();
	_FUNC2 func2 = (_FUNC2)PtrToDword((PtrToDword(_eax) + 0x138));
	__asm { 
		mov ecx, _eax
	}
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_mid_, user32, _ebp_);
		__asm
		{
			push 0x00
			push no
			push soul
			jmp func2
		}
	}
	else {
		func2(no, 0x00);
	}
_mid_:
	ASM_FAKE_STACK_END()
	if (0 && m_GameCall.NPCTalk) {
		_FUNC1 func3 = (_FUNC1)m_GameCall.NPCTalk;
		_asm {
			mov ecx, _esi
		}
		if (user32 && soul) {
			//::printf("user32&&soul\n");
			ASM_FAKE_STACK(_end_, user32, _ebp_);
			__asm
			{
				push 0x00
				push soul
				jmp func3
			}
		}
		else {
			func3(0x00);
		}
	}
	else {
		if (close) {
			m_pGameProc->Button(BUTTON_ID_CLOSEMENU, 0, "C");
		}
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
}

// 关闭提示框
void Game::Call_CloseTipBox(int close)
{
#if 0
	::printf("关闭提示框:%d\n", close);
	DWORD _ecx = m_GameAddr.TipBoxSta;
	_FUNC3 func = (_FUNC3)m_GameCall.CloseTipBox;
	__asm { mov ecx, _ecx };
	func(0x0A, close, 0x00);
#endif
}

// 获取仓库物品数量
DWORD Game::Call_GetPackageItemAmount()
{
	_FUNC1_R func = (_FUNC1_R)m_GameCall.GetPkAgeItemAmount;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD count = func(0x0A);
	ASM_RESTORE_ECX();
	return count;
}

// 获取仓库物品指针
DWORD Game::Call_GetPackageItemByIndex(int index)
{
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC2_R func = (_FUNC2_R)m_GameCall.GetPkageItemByIndex;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD r = 0;
	if (user32 && soul) {
		//::printf("user32&&soul->Call_GetPackageItemByIndex\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push 0x0A
			push index
			push soul
			jmp func
		}
	}
	else {
		func(index, 0x0A);
	}
_end_:
	ASM_FAKE_STACK_END();
	ASM_RESTORE_ECX();
	__asm {
		mov r, eax
	}
	return r;
}

// 获取物品数量
DWORD Game::Call_GetItemAmount(int flag)
{
	_FUNC1_R func = (_FUNC1_R)m_GameCall.GetItemAmount;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD count = func(flag);
	ASM_RESTORE_ECX();
	return count;
}

// 获取物品指针
DWORD Game::Call_GetItemByIndex(int index, int flag)
{
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC2_R func = (_FUNC2_R)m_GameCall.GetItemByIndex;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD r = 0;
	if (user32 && soul) {
		//::printf("user32&&soul->Call_GetItemByIndex\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push flag
			push index
			push soul
			jmp func
		}
	}
	else {
		func(index, flag);
	}
_end_:
	ASM_FAKE_STACK_END();
	ASM_RESTORE_ECX();
	__asm {
		mov r, eax
	}
	//::printf("r:%08X\n", r);
	return r;
}
	

// 使用物品
void Game::Call_UseItem(int item_id)
{
	::printf("使用物品:%08X\n", item_id);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC5 func = (_FUNC5)m_GameCall.UseItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push 0x00
			push 0xFFFFFFFF
			push 0xFFFFFFFF
			push 0x00
			push item_id
			push soul
			jmp func
		}
	}
	else {
		func(item_id, 0x00, 0xFFFFFFFF, 0xFFFFFFFF, 0x00);
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
}

// 扔物品
void Game::Call_DropItem(int item_id)
{
#if 0
	::printf("丢弃物品:%08X\n", item_id);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC3 func = (_FUNC3)m_GameCall.DropItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push 0x00
			push 0x00
			push item_id
			push soul
			jmp func
		}
	}
	else {
		func(item_id, 0x00, 0x00);
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
	// 4:0x00 4:0x04 4:0x80000000 4:0x80000000 4:0x7FFFFFFF 4:0x7FFFFFFF 4:* 4:0x01 4:0x19 4:0x00400000
#endif
}


#if 0
_declspec (naked) DWORD __stdcall Naked_PickUpItem(DWORD& ri, DWORD func, int id, int x, int y, DWORD addr, DWORD(&stack)[32])
{
	ASM_SET_ECX();
	__asm
	{
		push ebp;
		mov ebp, esp;

		mov[esp + 8], ebp;

		push 0x00;
		push 0x00;
		push 0x00;
		push _end_;

		push y;
		push x;
		push id;
		push addr;

		mov eax, func;
		mov ebp, stack;
		jmp eax;
	_end_:
		mov ebp, [esp + 8];

		mov esp, ebp;
		pop ebp;
		ret 28;
	}
}
#endif

// 捡物品
void Game::Call_PickUpItem(DWORD id, DWORD x, DWORD y)
{
#if 0
	::printf("拾取物品:%08X %d,%d！！！\n", id, x, y);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC3 func = (_FUNC3)m_GameCall.PickUpItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		DWORD stack[32];
		ZeroMemory(stack, sizeof(stack));
		DWORD frame[] = { 0x0060D262, 0x6FFDBECC, 0x6FFDBA62, 0x0068122A, 0x6FFDA656, 0x6FFDA8E2, 0x6FFD85CD,
			0x74DFBEFB, 0x74DF83DA, 0x74DF7C8E , 0x74DDA613, 0x74DD66FE };
		FakeStackFrame(stack, frame, sizeof(frame) / sizeof(DWORD));

#if 1
		DWORD ri;
		Naked_PickUpItem(ri, (DWORD)func, id, x, y, soul, stack);
#else
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push y
			push x
			push id
			push soul
			jmp func
		}
#endif
	}
	else {
		func(id, x, y);
	}
_end_:
	ASM_FAKE_STACK_END()
		ASM_RESTORE_ECX();
#endif
}

// 卖东西
void Game::Call_SellItem(int item_id)
{
#if 0
	::printf("卖东西:%08X\n", item_id);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC1 func = (_FUNC1)m_GameCall.SellItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push item_id
			push soul
			jmp func
		}
	}
	else {
		func(item_id);
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
#endif
}

// 存钱
void Game::Call_SaveMoney(int money)
{
#if 0
	::printf("存钱:%d\n", money);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC1 func = (_FUNC1)m_GameCall.SaveMoney;
	if (!func)
		return;

	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push money
			push soul
			jmp func
		}
	}
	else {
		func(money);
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
#endif
}

// 存入远程仓库
void Game::Call_CheckInItem(int item_id)
{
#if 0
	//::printf("存入物品:%08X\n", item_id);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC2 func = (_FUNC2)m_GameCall.CheckInItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push 0x0A
			push item_id
			push soul
			jmp func
		}
	}
	else {
		func(item_id, 0x0A);
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
#endif
}

// 取出仓库物品
void Game::Call_CheckOutItem(int item_id)
{
#if 0
	::printf("取出仓库物品:%08X\n", item_id);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC3 func = (_FUNC3)m_GameCall.CheckOutItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push 0x00
			push 0x0A
			push item_id
			push soul
			jmp func
		}
	}
	else {
		func(item_id, 0x0A, 0x00);
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
#endif
}

// 使用可传送物品
void Game::Call_TransmByMemoryStone(int item_id)
{
	::printf("使用可传送物品:%08X\n", item_id);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC1 func = (_FUNC1)m_GameCall.TransmByMemoryStone;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push item_id
			push soul
			jmp func
		}
	}
	else {
		func(item_id);
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
}

#if 0
_declspec (naked) DWORD __stdcall Naked_Magic_GW(DWORD& ri, DWORD func, int magic_id, int guaiwu_id, DWORD addr, DWORD(&stack)[32])
{
	ASM_SET_ECX();
	__asm
	{
		push ebp;
		mov ebp, esp;

		mov[esp + 8], ebp;

		push 0x00;
		push 0x00;
		push 0x00;
		push _end_;

		push 0x00;
		push 0x00;
		push guaiwu_id;
		push magic_id;
		push addr;

		mov eax, func;
		mov ebp, stack;
		jmp eax;
	_end_:
		mov ebp, [esp + 8];

		mov esp, ebp;
		pop ebp;
		ret 24;
	}
}
#endif
// 放技能
void Game::Call_Magic(int magic_id, int guaiwu_id)
{
#if 0
	::printf("技能怪物ID:%08X %08X！！！\n", magic_id, guaiwu_id);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC4 func = (_FUNC4)m_GameCall.MagicAttack_GWID;
	ASM_STORE_ECX();
	//ASM_SET_ECX();
	if (user32 && soul) {
		DWORD stack[32];
		ZeroMemory(stack, sizeof(stack));
		DWORD frame[] = { 0x005F5D9E, 0x74DEA6EE, 0x74DFA661, 0x74DFA566, 0x7719CD3D };
		FakeStackFrame(stack, frame, sizeof(frame) / sizeof(DWORD));
#if 1
		DWORD ri;
		Naked_Magic_GW(ri, (DWORD)func, magic_id, guaiwu_id, soul, stack);
#else
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push 0x00
			push 0x00
			push guaiwu_id
			push magic_id
			push soul
			jmp func
		}
#endif
	}
	else {
		func(magic_id, guaiwu_id, 0x00, 0x00);
	}
_end_:
	ASM_FAKE_STACK_END();
	ASM_RESTORE_ECX();
	//::printf("技能使用完成\n");
#endif
}

#if 0
_declspec (naked) DWORD __stdcall Naked_Magic_XY(DWORD& ri, DWORD func, int magic_id, int x, int y, DWORD addr, DWORD(&stack)[32])
{
	ASM_SET_ECX();
	__asm
	{
		push ebp;
		mov ebp, esp;

		mov[esp + 8], ebp;

		push 0x00;
		push 0x00;
		push 0x00;
		push _end_;

		push 0x00;
		push 0x00;
		push y;
		push x;
		push magic_id;
		push addr;

		mov eax, func;
		mov ebp, stack;
		jmp eax;
	_end_:
		mov ebp, [esp + 8];

		mov esp, ebp;
		pop ebp;
		ret 28;
	}
}
#endif

// 放技能
void Game::Call_Magic(int magic_id, int x, int y)
{
#if 0
	// 05D05020 06832EA4 mov ecx,dword ptr ds:[0xF1A518]
	::printf("技能XY:%08X %d,%d！！！\n", magic_id, x, y);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC5 func = (_FUNC5)m_GameCall.MagicAttack_XY;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		DWORD stack[32];
		ZeroMemory(stack, sizeof(stack));
		DWORD frame[] = { 0x005F5D9E, 0x74DEA6EE, 0x74DFA661, 0x74DFA566, 0x7719CD3D };
		FakeStackFrame(stack, frame, sizeof(frame) / sizeof(DWORD));

#if 1
		DWORD ri;
		Naked_Magic_XY(ri, (DWORD)func, magic_id, x, y, soul, stack);
#else
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push 0x00
			push 0x00
			push y
			push x
			push magic_id
			push soul
			jmp func
		}
#endif
	}
	else {
		func(magic_id, x, y, 0x00, 0x00);
	}
_end_:
	ASM_FAKE_STACK_END();
	ASM_RESTORE_ECX();
	//::printf("技能使用完成\n");
#endif
}


// 宠物出征
void Game::Call_PetOut(int pet_id)
{
	
}

// 宠物召回
void Game::Call_PetIn(int pet_id)
{

}

// 宠物合体
void Game::Call_PetFuck(int pet_id)
{
	
}

// 宠物解体
void Game::Call_PetUnFuck(int pet_id)
{
	
}

// 宠物技能 key_no=按键索引 1=0 2=1 ...
void Game::Call_PetMagic(int key_no)
{
	
}

// 获取远程邀请人物信息
DWORD Game::Call_QueryRemoteTeam(int no)
{
	_FUNC_R func = (_FUNC_R)m_GameCall.QueryInf_RemoteTeam;
	ASM_SET_ECX();
	DWORD base_addr = func();
	::printf("%08X %08X %08X\n", base_addr, P2DW(base_addr), P2DW(base_addr) + 0x30);
	//return base_addr;
	_FUNC1_R func2 = (_FUNC1_R)P2DW((P2DW(base_addr) + 0x30));
	__asm mov ecx, base_addr;
	DWORD r = func2(no);
	return r;
}

// 是否有队伍
int Game::Call_IsHaveTeam()
{
	if (!m_GameCall.IsHaveTeam)
		return 0;

	_FUNC_R func = (_FUNC_R)m_GameCall.IsHaveTeam;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD r = func();
	ASM_RESTORE_ECX();
	::printf("是否有队伍:%d\n", r);
	return r != 0;
}

// 是否是队长
int Game::Call_IsTeamLeader()
{
	if (!m_GameCall.IsTeamLeader)
		return 0;

	_FUNC_R func = (_FUNC_R)m_GameCall.IsTeamLeader;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD r = func();
	ASM_RESTORE_ECX();
	::printf("是否队长:%d\n", r);
	return r;
}

// 创建队伍
void Game::Call_TeamCreate()
{
	::printf("创建队伍\n");
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC func = (_FUNC)m_GameCall.TeamCreate;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push soul
			jmp func
		}
	}
	else {
		func();
	}
_end_:
	ASM_FAKE_STACK_END();
	ASM_RESTORE_ECX();
}

// 离开队伍[队长]
void Game::Call_TeamDismiss()
{
	::printf("离开队伍\n");
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC func = (_FUNC)m_GameCall.TeamDismiss;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push soul
			jmp func
		}
	}
	else {
		func();
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
}

// 离开队伍[队员]
void Game::Call_TeamLeave()
{
	::printf("离开队伍\n");
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC func = (_FUNC)m_GameCall.TeamLeave;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push soul
			jmp func
		}
	}
	else {
		func();
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
}

// 邀请入队
void Game::Call_TeamInvite(int player_id)
{
	::printf("邀请入队:%08X\n", player_id);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC1 func = (_FUNC1)m_GameCall.TeamInvite;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push player_id
			push soul
			jmp func
		}
	}
	else {
		func(player_id);
	}
_end_:
	ASM_FAKE_STACK_END()
	ASM_RESTORE_ECX();
}

// 自动组队
void Game::Call_TeamAutoJoin(int open)
{
	::printf("自动组队:%d\n", open);
	_FUNC1 func = (_FUNC1)m_GameCall.TeamAutoJoin;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(open);
	ASM_RESTORE_ECX();
}

// 是否选择邀请队伍
void Game::Call_CheckTeam(int v)
{
#if 1
	SetForegroundWindow(m_hWnd);

#if 0
	HWND hWnd = m_hWnd;
	DWORD dwForeID;
	DWORD dwCurID;

	HWND hForeWnd = GetForegroundWindow();
	dwCurID = GetCurrentThreadId();
	dwForeID = GetWindowThreadProcessId(hForeWnd, NULL);
	AttachThreadInput(dwCurID, dwForeID, TRUE);
	ShowWindow(hWnd, SW_SHOWNORMAL);
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetForegroundWindow(hWnd);
	AttachThreadInput(dwCurID, dwForeID, FALSE);
#else
#if 1
	//必须动态加载这个函数。  
	typedef void (WINAPI *PROCSWITCHTOTHISWINDOW)(HWND, BOOL);
	PROCSWITCHTOTHISWINDOW SwitchToThisWindow;
	HMODULE hUser32 = GetModuleHandle(L"user32");
	SwitchToThisWindow = (PROCSWITCHTOTHISWINDOW)
		GetProcAddress(hUser32, "SwitchToThisWindow");

	//接下来只要用任何现存窗口的句柄调用这个函数即可，
	//第二个参数表示如果窗口处于最小化状态，是否恢复。
	SwitchToThisWindow(m_hWnd, TRUE);
#endif
#endif

	HWND child, parent;
	while (true) {
		if (FindButtonWnd(BUTTON_ID_SURE, child, parent, nullptr))
			break;

		Sleep(1000);
	}

	ShowWindow(m_hWnd, SW_SHOWNORMAL);
	SetForegroundWindow(m_hWnd);
	Sleep(1000);
	GetWindowRect(m_hWnd, &m_GameWnd.Rect);
	m_GameWnd.Width = m_GameWnd.Rect.right - m_GameWnd.Rect.left;
	m_GameWnd.Height = m_GameWnd.Rect.bottom - m_GameWnd.Rect.top;
	::printf("窗口句柄:%0x %d %d %d %d %.2f\n", m_hWnd, m_GameWnd.Width, m_GameWnd.Height,
		m_GameWnd.Rect.left, m_GameWnd.Rect.top, m_fScale);

	//SwitchToThisWindow(m_hWnd, TRUE);
	SetForegroundWindow(m_hWnd);
	int clx = m_GameWnd.Rect.left + 466, cly = m_GameWnd.Rect.top + 479;
	if (!SetCursorPos(clx, cly)) {
		m_pGameProc->SendMsg("鼠标设置失败, 移动鼠标.|red");
		mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, clx * 65536 / GetSystemMetrics(SM_CXSCREEN),
			cly * 65536 / GetSystemMetrics(SM_CYSCREEN), 0, 0);
	}
	//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, clx, cly, 0, 0);
	Sleep(500);
	HWND hWnd = parent;
	LeftClick(107, 216, hWnd);
	//mouse_event(MOUSEEVENTF_LEFTDOWN, clx, cly, 0, 0);//点下左键
	//mouse_event(MOUSEEVENTF_LEFTUP, clx, cly, 0, 0);//点下左键
	printf("点击(%d %d)\n", clx, cly);

	char msg[64];
	sprintf_s(msg, "点击(%d %d)\n", clx, cly);
	m_pGameProc->SendMsg(msg);

	return;
#endif

	if (!m_GameAddr.TeamChkSta)
		return;

	::printf("副本邀请队伍选择:%d\n", v);
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC1 func = (_FUNC1)m_GameCall.TeamChk;
	DWORD _this = P2DW(m_GameAddr.TeamChkSta + m_GameAddr.TeamChkOffset);
	ASM_STORE_ECX();
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			push v
			push soul
			mov ecx, _this
			jmp func
		}
	}
	else {
		func(v);
	}
_end_:
	ASM_FAKE_STACK_END();
	ASM_RESTORE_ECX();
}

// 获取技能数量
DWORD Game::Call_GetMagicAmount()
{
	_FUNC5_R func = (_FUNC5_R)m_GameCall.GetMagicAmount;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD count = func(0x00, 0x00, 0x00, 0x00, 0x00);
	ASM_RESTORE_ECX();
	return count;
}

// 获取技能地址
DWORD Game::Call_GetMagicByIndex(int index)
{
	_FUNC6_R func = (_FUNC6_R)m_GameCall.GetMagicByIndex;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD ptr = func(index, 0x00, 0x00, 0x00, 0x00, 0x00);
	ASM_RESTORE_ECX();
	return ptr;
}

// 获取周围玩家数量
DWORD Game::Call_GetPlayerAmount()
{
	_FUNC_R func = (_FUNC_R)m_GameCall.GetPlayerAmount;
	ASM_STORE_ECX();
	__asm { 
		mov ecx, g_objPlayerSet
	}
	DWORD count = func();
	ASM_RESTORE_ECX();
	return count;
}

// 获取周围玩家地址
DWORD Game::Call_GetPlayerByIndex(int index)
{
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC1_R func = (_FUNC1_R)m_GameCall.GetPlayerByIndex;
	ASM_STORE_ECX();
	DWORD r = 0;
	if (user32 && soul) {
		//::printf("user32&&soul\n");
		ASM_FAKE_STACK(_end_, user32, _ebp_);
		__asm
		{
			mov ecx, g_objPlayerSet
			push index
			push soul
			jmp func
		}
	}
	else {
		func(index);
	}
_end_:
	ASM_FAKE_STACK_END();
	ASM_RESTORE_ECX();
	__asm {
		mov r, eax
	}
	return r;
}

// 获取基址[不知道什么可以获取]
DWORD Game::Call_GetBaseAddr(int index, DWORD _ecx)
{
	_FUNC1_R func = (_FUNC1_R)m_GameCall.GetNpcBaseAddr;
	ASM_STORE_ECX();
	__asm { mov ecx, _ecx };
	DWORD p = func(index);
	ASM_RESTORE_ECX();
	return PtrToDword(p);
}
