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

// ��Ϸg_pObjHeroȫ�ֱ���(CHero��thisָ��)
DWORD g_pObjHero = NULL;
// ��Ϸg_objPlayerSetȫ�ֱ���
DWORD g_objPlayerSet = NULL;

Game* Game::self = NULL;   // Game����
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

	m_pItem     = new Item(this);      // ��Ʒ��
	m_pTalk     = new Talk(this);      // �Ի���
	//m_pMove     = new Move(this);      // �ƶ���
	//m_pGuaiWu   = new GuaiWu(this);    // ������
	//m_pMagic    = new Magic(this);     // ������
	//m_pPet      = new Pet(this);       // ������

	m_pGameConf = new GameConf(this);  // ��Ϸ������
	m_pGameProc = new GameProc(this);  // ִ�й�����
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

// ��֤
void Game::VerifyServer()
{
	int n = 0;
	while (true) {
		if (++n == 60) {
			if (m_pHome->Verify()) {
				//printf("��֤�ɹ�.\n");
				m_nVerFail = 0;
			}
			else {
				//printf("��֤ʧ��.\n");
				if (++m_nVerFail >= 10) { }
			}
			n = 0;
		}
		
		Sleep(5000);
	}
}

// �ر���Ϸ
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
	::printf("�ر���Ϸ\n");
	ExitProcess(0);
	TerminateProcess(GetCurrentProcess(), 0);
	//TerminateProcess(m_hGameProcess, 0);
}

// ���ӹ������
void Game::Connect(const char* host, USHORT port)
{
	::printf("׼�����ӷ�����[%s:%d]\n", host, port);
	m_pClient->Connect(host, port);
}

// ��¼��Ϸ�ʺ�
void Game::Login()
{
	m_Account.IsLogin = 0;

	if (m_iNoAutoSelect) { // �Լ�ѡ���¼
		::printf("�ֶ���¼...\n");
		return;
	}

	::printf("׼����¼...\n");
#if 1
	Sleep(1500);
	LeftClick(510, 550); // ������ʽ��
	Sleep(1500);

	SelectServer(0xff);
#endif

	InputUserPwd();
}

// ѡ������
void Game::SelectServer(int flag)
{
	HWND h = ::FindWindow(NULL, L"360��ȫ��ʿ");
	SetForegroundWindow(h);
	Sleep(150);

	int x, y;
	if (flag & 0x01) {
		GetSerBigClickPos(x, y);
		LeftClick(x, y); // ѡ�����
		Sleep(1500);
	}
	if (flag & 0x02) {
		GetSerSmallClickPos(x, y);
		LeftClick(x, y); // ѡ��С��
	}
}

// �����ʺ�����
void Game::InputUserPwd(bool input_user)
{
	ReadGameMemory(0x100);
start:
	if (0 && m_GameAddr.SmallServer) {
		Sleep(1000);
		char ser[16], ch[8];
		if (GetSmallSerNum(ser, ch)) {
			if (strcmp(ser, "����") == 0)
				strcpy(ser, "��o");

			char* s_ser = strstr((char*)m_GameAddr.SmallServer, ser);
			char* s_ch = strstr((char*)m_GameAddr.SmallServer, ch);
			::printf("%s[%08x] %s[%08X]\n", ser, s_ser, ch, s_ch);
			if (!s_ser || !s_ch) {
				//28,730
				::printf("ѡ���С�������� %s!=%s\n", m_GameAddr.SmallServer, m_Account.SerSmall);
				Sleep(1000);
				LeftClick(28, 730); // ����
				Sleep(1000);
				SelectServer(0x02);
				goto start;
			}
			else {
				::printf("ѡ��С����ȷ %s==%s\n", m_GameAddr.SmallServer, m_Account.SerSmall);
			}
		}
	}

	HWND edit = FindWindowEx(m_hWndPic, NULL, NULL, NULL);
	int i;
	if (input_user) {
		Sleep(2000);
		LeftClick(300, 265); // ����ʺſ�
		SetForegroundWindow(m_hWnd);
		Sleep(1000);

		char save_name[32] = {0};
		GetWindowTextA(edit, save_name, sizeof(save_name));
		::printf("�����ʺ�:%s\n", save_name);
		if (strcmp(save_name, m_Account.Name) != 0) { // ����Ĳ�һ��
			int eq_num = 0; // ǰ����ͬ�ַ�
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

	LeftClick(300, 305); // ��������
	Sleep(1000);
	for (i = 0; i < strlen(m_Account.Pwd); i++) {
		Keyborad(m_Account.Pwd[i]);
		Sleep(200);
	}

	LeftClick(265, 430); // ����
}

// ��ʼ��
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

	::printf("��ʼ��ȡ��Ϸ����...\n");
	m_pClient->SendMsg("��ȡ��Ϸ����...");

	FindAllCall();
	//while (true) Sleep(1000);
	FindBagAddr();

	
	
	GetWindowRect(m_hWnd, &m_GameWnd.Rect);
	m_GameWnd.Width = m_GameWnd.Rect.right - m_GameWnd.Rect.left;
	m_GameWnd.Height = m_GameWnd.Rect.bottom - m_GameWnd.Rect.top;
	::printf("���ھ��:%0x %d %d %.2\n", m_hWnd, m_GameWnd.Width, m_GameWnd.Height, m_fScale);
	::printf("��Ϸ���洰��:%08X %.2f\n", m_hWndPic, m_fScale);
	
#if 1
	
	/*
	if (!ReadGameMemory()) {
		//INLOG("�޷��������Ѫ����ַ������");
		return false;
	}
	*/
	
	while (false) {
		printf("while (true7).\n");
		Sleep(5000);
	}
	while (0 && !m_GameAddr.ItemPtr) {
		::printf("��ȡ������Ʒ��ַ...\n");
		ReadGameMemory();
		Sleep(5000);
	}
	while (0 && !m_GameAddr.MapName) {
		::printf("��ȡ��ͼ���Ƶ�ַ..\n");
		ReadGameMemory();
		Sleep(5000);
	}
#if 1
	while (!m_GameAddr.Life) {
		::printf("���¶�ȡѪ����ַ...\n");
		m_pClient->SendMsg("���¶�ȡѪ��...");
		ReadGameMemory();
		Sleep(3000);
	}
#endif
#endif
	
	//m_pMagic->ReadMagic(nullptr, nullptr, false);

	m_pClient->SendMsg("��ȡ��Ϸ�������");

	//::printf("���Ӽ��̺��������%s\n", Drv_ConnectDriver()?"�ɹ�":"ʧ��");

	while (false) {
		printf("while (true8).\n");
		Sleep(5000);
	}
	
	return true;
}

// �ȴ���Ϸ��ʼ�����
void Game::WaitGameInit(int wait_s)
{
	m_pClient->SendMsg("�ȴ���Ϸ��ʼ�����");
	for (int i = 0; i < wait_s; i++) {
		HWND child, parent;
		if (FindButtonWnd(BUTTON_ID_CLOSEMENU, child, parent, "x")) {
			SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_CLOSEMENU, BN_CLICKED), (LPARAM)child); // �ر���
			::printf("---------��Ϸ�����ѳ�ʼ�����---------\n");
			m_pClient->SendMsg("��Ϸ�����ѳ�ʼ�����");
			break;
		}

		::printf("�ȴ���Ϸ���ݳ�ʼ���, ��ʣ%02d��[%d��].\n", wait_s - i, wait_s);
		Sleep(1000);
	}

	m_pGameProc->Button(BUTTON_ID_CANCEL,    1500);
	m_pGameProc->Button(BUTTON_ID_SURE,      1500);
	m_pGameProc->Button(BUTTON_ID_CLOSEMENU, 1500);
}

// ����·��
void Game::SetPath(const char * path)
{
	strcpy(m_chPath, path);
	printf("�����ļ�Ŀ¼:%s\n", m_chPath);
}

// �����Ƿ��Ǵ��
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

	::printf("��Ϸ������:%s.%s\n", ser_big, ser_small);
	::printf("�ʺ�:%s|%s ��%d����ɫ %s\n", name, pwd, role_no, big?"���":"С��");
	if (getxl_logout)
		::printf("��ȡ��������˳�\n");
	if (create_team)
		::printf("��Ҫ��������\n");

	Login();
}

// ���ÿ�ס����ʱ��
void Game::SetQiaZhuS(int second)
{
	m_iQiaZhuS = second;
	::printf("��ס%d�������\n", m_iQiaZhuS);
}

// �����Ƿ����
void Game::SetOffLine(int v)
{
	m_Account.IsOffLine = v;
}

// �����Ƿ���ħ��
bool Game::IsMoYu()
{
	wchar_t name[32] = { 0 };
	GetProcessName(name, GetCurrentProcessId());
	::wprintf(L"%ws = %ws\n", name, L"soul.exe");
	return wcsstr(name, L"soul.exe") != nullptr || wcsstr(name, L"SOUL.exe") != nullptr;
}

// �Ƿ��¼��
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

// �Ƿ������ 
bool Game::IsOffLine()
{
	return m_Account.IsOffLine;
}

// �Ƚϵ�¼�ʺ�
bool Game::CmpLoginAccount(const char* name)
{
	return strcmp(name, m_Account.Name) == 0;
}

// �Ƿ���
bool Game::IsBig()
{
	//return true;
	return m_Account.IsBig;
}

// �Ƿ��ڸ���
bool Game::IsInFB()
{
	DWORD x = 0, y = 0;
	ReadCoor(&x, &y);
	if (x >= 882 && x <= 930 && y >= 1055 && y < 1115) // �ս���������
		return true;

	return IsInMap("������Ŀ���");
}

// �Ƿ���ָ����ͼ
bool Game::IsInMap(const char* name)
{
	if (!m_GameAddr.MapName)
		return false;

	char map_name[32] = { 0 };
	if (ReadMemory((PVOID)m_GameAddr.MapName, map_name, sizeof(map_name))) { // ��ȡ��ͼ����
		return strcmp(map_name, name) == 0;
	}
	return false;
}

// �Ƿ���ָ���������� allow=���
bool Game::IsInArea(int x, int y, int allow)
{
	ReadCoor();
	int cx = (int)m_dwX - x;
	int cy = (int)m_dwY - y;

	return abs(cx) <= allow && abs(cy) <= allow;
}

// �Ƿ���ָ���������� allow=���
bool Game::IsNotInArea(int x, int y, int allow)
{
	ReadCoor();
	int cx = (int)m_dwX - x;
	int cy = (int)m_dwY - y;

	//::printf("IsNotInArea:%d,%d %d,%d\n", m_dwX, m_dwY, x, y);
	return abs(cx) > allow || abs(cy) > allow;
}

// �Ƿ��ѻ�ȡ������
bool Game::IsGetXL()
{
	if (m_Account.IsGetXL) {
		tm t;
		time_t get_time = m_Account.GetXLTime;
		gmtime_s(&t, &get_time);

		tm t2;
		time_t now_time = time(nullptr);
		gmtime_s(&t2, &now_time);

		if (t.tm_mday != t2.tm_mday) { // ������ȡ��
			m_Account.IsGetXL = 0;
		}
	}
	return m_Account.IsGetXL != 0;
}

// ����
void Game::Run()
{
	if (0 && !IsBig())
		return;

	if (m_iTalkOpen) {
		::printf("-----------�������Զ�����-----------\n");
		return;
	}

	m_pGameProc->Run();
}

// ��ȡ��Ϸ����λ������Ļ�ϵ�����
void Game::GetGameCenterPos(int & x, int & y)
{
	GetWindowRect(m_hWnd, &m_GameWnd.Rect);
	m_GameWnd.Width = m_GameWnd.Rect.right - m_GameWnd.Rect.left;
	m_GameWnd.Height = m_GameWnd.Rect.bottom - m_GameWnd.Rect.top;

	x = m_GameWnd.Rect.left + (m_GameWnd.Width / 2);
	y = m_GameWnd.Rect.top + (m_GameWnd.Height / 2);
}

// �����Ϸ����
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
	::printf("���ھ��:%0x->%0x->%0x %d %d %.2\n", m_hWnd, m_hWnd2, m_hWndPic, m_GameWnd.Width, m_GameWnd.Height, m_fScale);
	
#if 0
	int n = 0;
	while (true) {
		n++;
		HWND child, parent;
		FindButtonWnd(BUTTON_ID_MAGIC, child, parent, "����");

		HWND talkWnd = FindWindowEx(m_hWnd2, NULL, NULL, NULL);
		::printf("%d.��Ϸ�Ի�����:%08X,%08X\n", n, talkWnd, child);
		Sleep(1000);

		if (n >= 60) {
			if ((n % 60) == 0) {
				printf("���F1\n");
				ClickMagic(VK_F1);
			}
			if ((n % 25) == 0) {
				printf("����ÿ����Ǳ�Կ�׿�������\n");
				SelectTalkNo("Կ�׿�������");
			}
		}
	}
#endif

	m_pClient->SendOpen(m_fScale, m_hWnd, m_GameWnd.Rect); // ֪ͨ�����Ѵ�
	return m_hWnd;
}

// ��ȡ����ģ���ַ
void Game::FindAllModAddr()
{
	m_GameModAddr.Mod3DRole = FindModAddr(MOD_3drole);
	m_GameModAddr.Mod3DGameMap = FindModAddr(MOD_3dgamemap);
}

// ��ȡ��Ϸ����CALL
void Game::FindAllCall()
{
	SearchModFuncMsg info[] = {
		{"?g_pObjHero",            NULL,  0, &g_pObjHero,                  "g_pObjHeroȫ�ֱ���"},
		{"?g_objPlayerSet",        NULL,  0, &g_objPlayerSet,              "g_objPlayerSetȫ�ֱ���"},
		{"?ReBorn@CHero",          NULL,  0, &m_GameCall.ReBorn,           "���︴��"},
		{"?Run@CHero",             NULL,  0, &m_GameCall.Run,              "�ƶ�����"},
		{"?Talk@CHero",            NULL,  0, &m_GameCall.Talk,             "��������"},
		{"?ActiveNpc@CHero",       NULL,  0, &m_GameCall.ActiveNpc,        "NPC�Ի�"},
		{"?GetInstance@ITaskManager",NULL,0, &m_GameCall.ITaskGetInstance, "NPC�Ի�ѡ��"},
		{"?GetPackageItemAmount@CHero", NULL, 0,   &m_GameCall.GetPkAgeItemAmount,  "��ȡ�ֿ���Ʒָ��"},
		{"?GetPackageItemByIndex@CHero",  NULL, 0, &m_GameCall.GetPkageItemByIndex, "��ȡ�ֿ���Ʒָ��"},
		{"?GetItemAmount@CHero", NULL,    0, &m_GameCall.GetItemAmount,  "��ȡ��Ʒ����"},
		{"?GetItemByIndex@CHero",  NULL,  0, &m_GameCall.GetItemByIndex,   "��ȡ��Ʒָ��"},
		{"?UseItem@CHero",         NULL,  0, &m_GameCall.UseItem,          "ʹ����Ʒ"},
		{"?DropItem@CHero",        NULL,  0, &m_GameCall.DropItem,         "������Ʒ"},
		{"?PickUpItem@CHero",      NULL,  0, &m_GameCall.PickUpItem,       "��ʰ��Ʒ"},
		{"?SellItem@CHero",        NULL,  0, &m_GameCall.SellItem,         "����Ʒ"},
		{"?SaveMoney@CHero",       NULL,  0, &m_GameCall.SaveMoney,        "��Ǯ"},
		{"?CheckInItem@CHero",     NULL,  0, &m_GameCall.CheckInItem,      "����Զ�ֿ̲�"},
		{"?CheckOutItem@CHero",    NULL,  0, &m_GameCall.CheckOutItem,     "ȡ��Զ�ֿ̲�"},
		{"?OpenBank@CHero",        NULL,  0, &m_GameCall.OpenBank,         "��Զ�ֿ̲�"},
		{"?TransmByMemoryStone@CHero",    NULL,  0, &m_GameCall.TransmByMemoryStone, "ʹ�ÿɴ�����Ʒ"},
		{"?MagicAttack@CHero",     "POS", 0, &m_GameCall.MagicAttack_GWID, "ʹ�ü���-����ID "},
		{"?MagicAttack@CHero",     "POS", 1, &m_GameCall.MagicAttack_XY,   "ʹ�ü���-XY����"},
		{"?CallEudenmon@CHero",    NULL,  0, &m_GameCall.CallEudenmon,     "�������"},
		{"?KillEudenmon@CHero",    NULL,  0, &m_GameCall.KillEudenmon,     "�����ٻ�"},
		{"?AttachEudemon@CHero",   NULL,  0, &m_GameCall.AttachEudemon,    "�������"},
		{"?UnAttachEudemon@CHero", NULL,  0, &m_GameCall.UnAttachEudemon,  "�������"},
		{"?GetData@CPlayer",       NULL,  0, &m_GameCall.SetRealLife,      "CPlayer::GetData"},
		{"?QueryInterface_RemoteTeam@CHero", NULL,  0, &m_GameCall.QueryInf_RemoteTeam,"��ȡԶ�����������ַ"},
		{"?IsHaveTeam@CHero",      NULL,  0, &m_GameCall.IsHaveTeam,       "�Ƿ��ж���"},
		{"?IsTeamLeader@CHero",    NULL,  0, &m_GameCall.IsTeamLeader,     "�Ƿ��Ƕӳ�"},
		{"?TeamCreate@CHero",      NULL,  0, &m_GameCall.TeamCreate,       "��������"},
		{"?TeamDismiss@CHero",     NULL,  0, &m_GameCall.TeamDismiss,      "�뿪����[�ӳ�]"},
		{"?TeamLeave@CHero",       NULL,  0, &m_GameCall.TeamLeave,        "�뿪����[��Ա]"},
		{"?TeamInvite@CHero",      NULL, 0, &m_GameCall.TeamInvite,        "�������"},
		{"?SetAutoJoinStatus@CHero",NULL, 0, &m_GameCall.TeamAutoJoin,     "�Զ�����"},
		{"?GetMagicAmount@CHero",   NULL, 0, &m_GameCall.GetMagicAmount,   "��������"},
		{"?GetMagicByIndex@CHero",  NULL, 0, &m_GameCall.GetMagicByIndex,  "���ܻ�ַ��ַ"},

		{"?GetPlayerAmount@CGamePlayerSet",  NULL,0, &m_GameCall.GetPlayerAmount,  "��ȡ��������"},
		{"?GetPlayerByIndex@CGamePlayerSet", NULL,0, &m_GameCall.GetPlayerByIndex, "��ȡ�����ַ"},
		{"?Process@CGamePlayerSet",          NULL,0, &m_GameCall.HookProcess,      "������������"},
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

// ��ȡuser32 ret 04��ַ
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

// ��ȡNPC�Ի�����
DWORD Game::FindNPCTalkCall()
{
	// ��mov ecx,dword ptr ds:[ecx+esi+0x1710]��ʼ����
	BYTE codes[] = {
		0x8b,0x8c,0x31,0x11,0x11,0x11,0x11,  0x8b,0x92,0x11,0x11,0x11,0x11,
		0x6a,0x00,  0x51,  0x8b,0xc8,  0xff,0xd2,  0x6a,0x00,  0x8b,0xce,
	};

	DWORD address = SearchInModByte(L"soul.exe", codes, sizeof(codes));
	if (address) {
		DWORD tmp = address;
		DWORD offset = P2DW(address + 0x19);
		address += offset + 0x1D;
		::printf("CallNPC�Ի�ѡ��2:%08X %08X\n", address, tmp);

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

// ��ȡ�����������ѡ�����
DWORD Game::FindTeamChkCall()
{
	return 123;
#if 0
	// ��push 0xFFFFFF��ʼ����
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
		::printf("Call�����������ѡ��:%08X %08X\n", address, tmp);
		::printf("�����������ѡ��ESIƫ��:%08X\n", m_GameAddr.TeamChkOffset);
	}
	return address;
#endif
}

// ���ְ�������
DWORD Game::FindKeyNumCall()
{
	return 0;
#if 0
	// ��push 0x00��ʼ����
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
		::printf("Call���ְ���:%08X %08X\n", address, tmp);
		::printf("���ְ���ECXָ���ַ:%08X\n", m_GameAddr.KeyNumEcxPtr);
	}
	return address;
#endif
}

// ��ȡ�ر���ʾ����
DWORD Game::FindCloseTipBoxCall()
{
	return 123;
#if 0
	// mov dword ptr ds:[ebx+0xA0],esi
	// ��������->CE��ʾ��״̬��ַ���޸Ķϵ�->�ҵ��޸ĵ�ַ->OD�¶ϵ�, �鿴���ö�ջ�����ҵ�
	DWORD codes[] = {
		0x51EC8B55, 0x8B575653, 0xFF83087D, 0x0FD98B0A,
		0x00000011, 0x0C751234
	};

	DWORD address = 0;
	if (SearchInMod(L"soul.exe", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		::printf("Call�ر���ʾ������ַ:%08X\n", address);
	}
	return address;
#endif
}

// ��ȡ��ȡNPC����ַ����
DWORD Game::FindGetNpcBaseAddr()
{
	// �������� �ҵ�NPC��ַ->�·��ʶϵ㣬���NPC���ϵĶϵ�, �ҵ����ʶϵ��¶�
	// [��CGamePlayerSet::Process����]mov dword ptr ds:[ecx+0x44],edx
	// ����Ѱ�Ҽ����ҵ� add ecx,0x14
	DWORD codes[] = {
		0x83EC8B55, 0x4D8924EC, 0x08458BDC, 0xE04D8D50,
		0xF0558D51, 0xDC4D8B52
	};

	DWORD address = 0;
	if (SearchInMod(L"3drole.dll", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		::printf("Call��ȡ��ȡNPC����ַ:%08X\n", address);
	}
	return address;
}

// ��ȡģ���ַ
DWORD Game::FindModAddr(LPCWSTR name)
{
	HANDLE hMod = NULL;
	while (hMod == NULL) {
		hMod = GetModuleHandle(name);
		::wprintf(L"%ws:%08X\n", name, hMod);
	}
	return (DWORD)hMod;
}

// ��ȡ��¼ѡ���С����ַ
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
		::printf("ѡ���С����%08X %s\n", m_GameAddr.SmallServer, m_GameAddr.SmallServer);
	}
	return address > 0;
}

// ��ȡ�����ַ
bool Game::FindCoorAddr()
{
	while (!IsLogin()) {
		::printf("�ȴ���¼��Ϸ������\n");
		Sleep(5000);
	}

	try {
		DWORD p;
		//ReadDwordMemory(0x123, p);
	}
	catch (...) {
		::printf("����PPPPPPPPPPPPPPPPPPPPPPPPP\n");
	}

	m_GameAddr.CoorX = m_GameModAddr.Mod3DRole + ADDR_COOR_X_OFFSET;
	m_GameAddr.CoorY = m_GameModAddr.Mod3DRole + ADDR_COOR_Y_OFFSET;
	while (PtrToDword(m_GameAddr.CoorX) == 0 || PtrToDword(m_GameAddr.CoorY) == 0) {
		::printf("���ڽ�����Ϸ������\n");
		HWND child, parent;
		if (FindButtonWnd(BUTTON_ID_LOGIN, child, parent)) { // ��ѡ��������밴ť
			//::printf("����ѡ���ɫ��ť\n");
			if (1 && m_Account.RoleNo > 0) {
				::printf("ѡ���ɫ:%d\n", m_Account.RoleNo + 1);
				HWND child2, parent2;
				int btn_id = BUTTON_ID_ROLENO + m_Account.RoleNo;
				FindButtonWnd(btn_id, child2, parent2, "��ɫ");
				SendMessageW(parent2, WM_COMMAND, MAKEWPARAM(btn_id, BN_CLICKED), (LPARAM)child2); // ѡ���ɫ
				
				RECT rect;
				GetWindowRect(child2, &rect);
				//Drv_LeftClick(float(rect.left + 20) / m_fScale, float(rect.top + 20) / m_fScale);
				::printf("��ťID:%X %08X %08X %d %d %d %d\n", btn_id, child2, parent2, rect.left, rect.top, rect.right, rect.bottom);
				Sleep(500);
				GetWindowRect(child, &rect);
				//Drv_LeftClick(float(rect.left + 10) / m_fScale, float(rect.top + 10) / m_fScale);
				::printf("��ťID2:%X %08X %08X %d %d %d %d\n", btn_id, child, parent, rect.left, rect.top, rect.right, rect.bottom);
				SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_LOGIN, BN_CLICKED), (LPARAM)child); // ����
			}
		}
		Sleep(1500);
	}
	::printf("��������:%d,%d\n", PtrToDword(m_GameAddr.CoorX), PtrToDword(m_GameAddr.CoorY));

	HWND parent;
	FindButtonWnd(BUTTON_ID_MAGIC, m_hWndItem, parent);
	FindButtonWnd(BUTTON_ID_MAGIC, m_hWndMagic, parent);

	strcpy(m_Account.Name, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ACCOUNT_NAME));
	strcpy(m_Account.Role, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ROLE_NAME));
	::printf("�ʺ�:%s ��ɫ:%s\n", m_Account.Name, m_Account.Role);
	m_pClient->SendInGame(m_Account.Name, m_Account.Role); // ֪ͨ����������
	return true;
}

// ��ȡ�ƶ�״̬��ַ
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
			::printf("�ƶ�״̬��ַ��%08X\n", m_GameAddr.MovSta);
		}
	}
	return address > 0;
}

// ��ȡ�Ի���״̬��ַ
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
		::printf("NPC�����Ի�ESI:%08X\n", m_GameAddr.CallNpcTalkEsi);
		::printf("�Ի���״̬��ַ��%08X\n", m_GameAddr.TalKBoxSta);
	}
	return address > 0;
}

// ��ȡ�Ƿ�ѡ���������״̬��ַ
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
	DWORD address = 0; // �����ַ��Callѡ���Ƿ�������麯����esi��ֵ mov ecx,dword ptr ds:[esi+0x1DC]
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.TeamChkSta = address; // +2F68��TeamChkStaƫ��ͷ ��+134��ѡ���״̬[0162976C]
		::printf("�������״̬��ַ��%08X\n", m_GameAddr.TeamChkSta);
	}
	return address > 0;
#endif
}

// ��ȡ��ʾ��״̬��ַ
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
		::printf("��ʾ��״̬��ַ��%08X\n", m_GameAddr.TipBoxSta);
	}
	return address > 0;
#endif
}

// ��ȡ������ַ
bool Game::FindLifeAddr()
{
#if 0
	m_GameAddr.Life = m_GameModAddr.Mod3DRole + ADDR_LIFE_OFFSET;
	m_GameAddr.LifeMax = m_GameModAddr.Mod3DRole + ADDR_LIFEMAX_OFFSET;
	::printf("�ҵ�������ַ��%08X\n", m_GameAddr.Life);
	::printf("�ҵ��������޵�ַ��%08X\n", m_GameAddr.LifeMax);
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
		::printf("�ҵ�������ַ��%08X\n", m_GameAddr.Life);
		::printf("�ҵ��������޵�ַ��%08X\n", m_GameAddr.LifeMax);
	}
	return address > 0;
#endif
}

// ��ȡ��������
bool Game::FindBagAddr()
{
	// Soul.exe+C8C12C
	// Soul.exe + CA97C8
	// 4:0x281 4:0x1E6 4:0xA04 4:0x05 4:0x05 4:0x1E1E
	// 023C78D5 
	// ��������->CE������Ʒ����->�·��ʶϵ�, ��û�ַ->��ַ�·��ʶϵ�õ�ƫ��
	// CHero::GetItem�������� �����ĸ�λ����Ҫ�������Ĳ鿴
	try {
		DWORD p, count;
#if 0
		__asm
		{
			mov eax, g_pObjHero
			mov eax, [eax]
			mov eax, [eax + 0x26C4]     // 2234
			mov edx, [eax + 0x10]       // ��Ʒ��ַָ��
			mov dword ptr[p], edx
			mov edx, [eax + 0x20]       // ��Ʒ����
			mov dword ptr[count], edx
		}
#else
		p = Call_GetItemByIndex(0);
		count = Call_GetItemAmount();
#endif
		::printf("��Ʒָ��:%08X ����:%d\n", p, count);
		//m_GameAddr.Bag = p;

		return p != 0;
	}
	catch (...) {
		::printf("Game::FindBagAddrʧ��!\n");
	}
	return false;
}

// ��õ�����Ʒ��ַ�ı����ַ
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
		INLOGVARN(32, "������Ʒ��ַ�����ַ:%08X", m_GameAddr.ItemPtr);
		::printf("������Ʒ��ַ�����ַ:%08X\n", m_GameAddr.ItemPtr);
	}

	return address != 0;
}

// ��ȡNPC�����Ի�ESI�Ĵ�����ֵ
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
		::printf("NPC�����Ի�ESI:%08X\n", m_GameAddr.CallNpcTalkEsi);
	}

	return address != 0;
}

// ��ȡ�����б����ַ
bool Game::FindPetPtrAddr()
{
	return false;
#if 0
	// �������� ���ҳ�����Ѫ��->CE��Ѫ�����ʶϵ�, �ҵ���ַ, �鿴������
	// ����ID[773610E3��������:800750]->��ó���ID��ַ->�·��ʶϵ�->�ҵ�ƫ��
	// 4:0x00F855F8 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	// 774D077B[è��] 77521F67[����]
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
	::printf("�����б��ַ:%08X\n", address);
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
		::printf("�����б��ַ:%08X [%08X] %08X\n", address, PtrToDword(address), codes);
	}
#endif
	return address != 0;
#endif
}

// ��ȡ��ͼ���Ƶ�ַ
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
			::printf("��ͼ���Ƶ�ַ:%08X[%s] %d\n", m_GameAddr.MapName, name, name[0]);

			if (name[0] == 0 || name[0] == 0xff) {
				//m_pClient->SendMsg("û���ҵ���ͼ����, ������Ϸ��.|red");
				//Close(false);
			}
			else {
				char str[32];
				sprintf_s(str, "��ͼ����:%s", name);
				m_pClient->SendMsg(str);
			}
		}
	}
	return address != 0;
}

// ��ĳ��ģ������������
DWORD Game::SearchFuncInMode(SearchModFuncMsg* info, HANDLE hMod)
{
	//�õ�DOSͷ
	IMAGE_DOS_HEADER* dosheader = (PIMAGE_DOS_HEADER)hMod;
	//Ч���Ƿ�DOSͷ
	if (dosheader->e_magic != IMAGE_DOS_SIGNATURE)return NULL;

	//NTͷ
	PIMAGE_NT_HEADERS32 NtHdr = (PIMAGE_NT_HEADERS32)((CHAR*)hMod + dosheader->e_lfanew);
	//Ч���Ƿ�PEͷ
	if (NtHdr->Signature != IMAGE_NT_SIGNATURE)return NULL;

	//�õ�PEѡ��ͷ
	IMAGE_OPTIONAL_HEADER32* opthdr = (PIMAGE_OPTIONAL_HEADER32)((PBYTE)hMod + dosheader->e_lfanew + 24);
	//�õ�������
	IMAGE_EXPORT_DIRECTORY* pExportTable = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)hMod + opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	//�õ�������ַ�б�
	PDWORD arrayOfFunctionAddresses = (PDWORD)((PBYTE)hMod + pExportTable->AddressOfFunctions);
	//�õ����������б�
	PDWORD arrayOfFunctionNames = (PDWORD)((PBYTE)hMod + pExportTable->AddressOfNames);
	//�õ��������
	PWORD arrayOfFunctionOrdinals = (PWORD)((PBYTE)hMod + pExportTable->AddressOfNameOrdinals);
	//���������ַ
	DWORD Base = pExportTable->Base;
	//ѭ��������
	for (DWORD x = 0; x < pExportTable->NumberOfNames; x++)			//�������������� ���֮�֣�������������=���Ƶ���+��ŵ�����������ѭ���������Ƶĺ���
	{
		//�õ������� 
		PSTR functionName = (PSTR)((PBYTE)hMod + arrayOfFunctionNames[x]);
		if (strstr(functionName, info->Name)) { // ���ڴ�����
			bool find = true;
			if (info->Substr) { // �����Ӵ�
				PSTR sub = strstr(functionName, info->Substr);
				if ((sub && !info->Flag) || (!sub && info->Flag)) { // ������Ҫ��
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

// ��ĳ��ģ����������
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

// ��ĳ��ģ����������
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

// ����������
DWORD Game::SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length, DWORD step)
{
	if (length == 0 || save_length == 0)
		return 0;

	DWORD count = 0;
	for (DWORD i = 0; i < m_dwReadSize; i += step) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		if (addr == (DWORD)codes) { // �����Լ�
			//::printf("���������Լ�:%08X\n", codes);
			return 0;
		}

		DWORD* dw = (DWORD*)(m_pReadBuffer + i);
		bool result = true;
		for (DWORD j = 0; j < length; j++) {
			if (codes[j] == 0x11) { // �����
				result = true;
			}
			else if (codes[j] == 0x22) { // ��Ҫ��ֵ��Ϊ0
				if (dw[j] == 0) {
					result = false;
					break;
				}
			}
			else if (((codes[j] & 0xffff0000) == 0x12340000)) { // ��2�ֽ����
				if ((dw[j]&0x0000ffff) != (codes[j]&0x0000ffff)) {
					result = false;
					break;
				}
				else {
					//::printf("%08X\n", dw[j]);
				}
			}
			else if (((codes[j] & 0x0000ffff) == 0x00001234)) { // ��2�ֽ����
				if ((dw[j] & 0xffff0000) != (codes[j] & 0xffff0000)) {
					result = false;
					break;
				}
			}
			else {
				if ((codes[j] & 0xff00) == 0x1100) { // �Ƚ�������ַ��ֵ��� ���8λΪ�Ƚ�����
					//::printf("%X:%X %08X:%08X\n", j, codes[j] & 0xff, dw[j], dw[codes[j] & 0xff]);
					if (dw[j] != dw[codes[j] & 0xff]) {
						result = false;
						break;
					}
				}
				else if (dw[j] != codes[j]) { // ������ֵ�����
					result = false;
					break;
				}
			}
		}

		if (result) {
			save[count++] = addr;
			//::printf("��ַ:%08X   %08X\n", addr, codes);
			if (count == save_length)
				break;
		}
	}

	return count;
}

// ����������
DWORD Game::SearchByteCode(BYTE * codes, DWORD length)
{
	if (length == 0)
		return 0;

	for (DWORD i = 0; i < m_dwReadSize; i++) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		if (addr == (DWORD)codes) { // �����Լ�
			//::printf("���������Լ�:%08X\n", codes);
			return 0;
		}

		BYTE* data = (m_pReadBuffer + i);
		bool result = true;
		for (DWORD j = 0; j < length; j++) {
			if (codes[j] == 0x11) { // �����
				result = true;
			}
			else if (codes[j] == 0x22) { // ��Ҫ��ֵ��Ϊ0
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

// ��ȡ���ֽ�����
bool Game::ReadDwordMemory(DWORD addr, DWORD& v)
{
	return ReadMemory((PVOID)addr, &v, 4);
}

// ��ȡ�ڴ�
bool Game::ReadMemory(PVOID addr, PVOID save, DWORD length)
{
	if (IsBadReadPtr(addr, length)) {
		::printf("�����ַ:%08X %d\n", addr, length);
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
	//::printf("ReadProcessMemory:%d %08X %d Read:%d ��ֵ:%d(%08X)\n", GetLastError(), addr, result, dwRead, *(DWORD*)save, *(DWORD*)save);

	if (!result || dwRead != length) {
		::printf("ReadProcessMemory����:%d %08X %d\n", GetLastError(), addr, result);
		if (GetLastError() == 6) {
			m_pGameProc->WriteLog("GetLastError() == 6", true);
			Sleep(100);
			//CloseHandle(m_hGameProcess);
			m_pGameProc->WriteLog("�ر�m_hGameProcess", true);
			Sleep(100);
			m_hGameProcess = GetCurrentProcess();
			return ReadProcessMemory(m_hGameProcess, addr, save, length, NULL);
		}
	}

	if (mod)
		VirtualProtect(addr, length, oldProtect, &oldProtect);
	return result;
}

// ��ȡ����
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
		::printf("�޷���ȡ����X(%d) %08X\n", GetLastError(), m_GameAddr.CoorX);
		return false;
	}
	if (!ReadDwordMemory(m_GameAddr.CoorY, vy)) {
		::printf("�޷���ȡ����Y(%d) %08X\n", GetLastError(), m_GameAddr.CoorY);
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

// ��ȡ����ֵ
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

// ��ȡҩ������
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

// ��ȡ������Ʒ
bool Game::ReadBag(DWORD* bag, int length)
{
	return ReadProcessMemory(m_hGameProcess, (PVOID)m_GameAddr.Bag, bag, length, NULL);
}

// ��ô��ھ��
bool Game::FindButtonWnd(int button_id, HWND& hwnd, HWND& parent, const char* text)
{
	HWND wnds[] = { (HWND)button_id, NULL, (HWND)text };
	::EnumChildWindows(m_hWnd, EnumChildProc, (LPARAM)wnds);
	hwnd = wnds[0];
	parent = wnds[1];
	return parent != NULL;
}

// ��ȡ��Ϸ�ڴ�
bool Game::ReadGameMemory(DWORD flag)
{
	m_dwGuaiWuCount = 0; // ���ù�������
	m_bIsReadEnd = false;

	MEMORY_BASIC_INFORMATION mbi;
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	DWORD_PTR MaxPtr = 0x70000000; // ����ȡ�ڴ��ַ
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
			//::printf("%p-%p ===����, ��С:%d\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
			goto _c;
		}
		else {
			DWORD pTmpReadAddress = ReadAddress;
			DWORD dwOneReadSize = 0x1000; // ÿ�ζ�ȡ����
			DWORD dwReadSize = 0x00;      // �Ѷ�ȡ����
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
					//::printf("%p-%p ===��ȡʧ��, ��С:%d, ������:%d\n", pTmpReadAddress, pTmpReadAddress + m_dwReadSize, (int)m_dwReadSize, GetLastError());
				}

				dwReadSize += m_dwReadSize;
				pTmpReadAddress += m_dwReadSize;
			}

			count++;
		}
	_c:
		// ��ȡ��ַ����
		ReadAddress += mbi.RegionSize;
		//Sleep(8);
		//::printf("%016X---�ڴ��С2:%08X\n", ReadAddress, mbi.RegionSize);
		// ɨ0x10000000�ֽ��ڴ� ����100����
	}
end:
	__int64 ms2 = getmillisecond();
	::printf("��ȡ���ڴ���ʱ:%d����\n", ms2 - ms);
	m_bIsReadEnd = true;
	return true;
}

// ��ӡ�ռ�
void Game::InsertLog(char * text)
{
	//g_dlg->InsertLog(text);
}

// ��ӡ�ռ�
void Game::InsertLog(wchar_t * text)
{
	//g_dlg->InsertLog(text);
}

// ��ȡ�����������
void Game::GetSerBigClickPos(int& x, int& y)
{
	int vx = 200, vy = 33;
	Explode arr("-", m_Account.SerBig);
	printf("arr:%d-%d\n", arr.GetValue2Int(0), arr.GetValue2Int(1));
	SET_VAR2(x, arr.GetValue2Int(0) * vx - vx + 125, y, arr.GetValue2Int(1) * vy - vy + 135);
	printf("x:%d y:%d\n", x, y);
}

// ��ȡС���������
void Game::GetSerSmallClickPos(int & x, int & y)
{
	int vy = 38;
	int n = atoi(m_Account.SerSmall);
	printf("n:%d\n", n);
	SET_VAR2(x, 515, y, n * vy - vy + 125);
	printf("x:%d y:%d\n", x, y);
}

// ��ȡС���е�����
bool Game::GetSmallSerNum(char ser[], char num[])
{
	char* first = nullptr;
	const char ch[10][3] = {"һ", "��", "��", "��", "��", "��", "��", "��", "��", "ʮ"};
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

// ��ȡ��ݼ������������
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

// ö�ٴ���
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

// ö���Ӵ���
BOOL Game::EnumChildProc(HWND hWnd, LPARAM lParam)
{
	HWND* param = (HWND*)lParam;
	HWND hWnd_Child = ::GetDlgItem(hWnd, (int)param[0]);
	if (hWnd_Child) { // �ҵ����Ӵ���
		if (param[2]) { // ���������ť����
			char text[32];
			GetWindowTextA(hWnd_Child, text, sizeof(text)); // ��ȡ��ť����
			if (strstr(text, (const char*)param[2]) == nullptr)
				return TRUE;
		}

		param[0] = hWnd_Child; // �Ӵ��ھ�� 
		param[1] = hWnd;       // �����ھ��
		
		return FALSE;
	}
	return TRUE;
}

// ����
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
	//::printf("����:%d %08X %08X %d\n", key, lParam, hwnd, GetLastError());
}

// ����
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
	::printf("����:%C[%d] %08X %08X %08X\n", key, key, m_hWnd, m_hWnd2, m_hWndMagic);
	
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

	::printf("�������:%d,%d %08X\n", r.left + x, r.top + y, hwnd);
#else
	PostMessageA(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	//Sleep(10);
	PostMessageA(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
	PostMessageA(hwnd, WM_LBUTTONUP, 0x00, MAKELPARAM(x, y));
	//PostMessageA(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
	//::printf("�������:%d,%d %08X\n", x, y, hwnd);
#endif
}

// �һ�
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
	
	::printf("�һ����:%d,%d %08X\n", x, y, hwnd);
}

// ����ƶ�
void Game::MouseMove(int x, int y, HWND hwnd)
{
	if (hwnd == NULL)
		hwnd = m_hWndPic;

	PostMessageA(hwnd, WM_MOUSEMOVE, 0x00, MAKELPARAM(x, y));
	Sleep(50);
}

// ѡ��Ի�ѡ��
void Game::SelectTalkNo(const char* name)
{
	// �ÿ����Ǳ�Կ�׿�������(50,126-235,135)
	// �ð�����������������(50,162-235,168)
	int x = MyRand(50, 235), y = 0;
	if (strcmp(name, "Կ�׿�������") == 0) {
		y = MyRand(126, 135);
	}
	else if (strcmp(name, "������������") == 0) {
		y = MyRand(162, 168);
	}
	else if (strcmp(name, "��������") == 0) {
	}
	else if (strcmp(name, "�뿪����") == 0) {
	}
	else if (strcmp(name, "��ȡ����") == 0) {
	}
	else if (strcmp(name, "��ȡ����") == 0) {
	}

	SelectTalkNo(x, y);
}

// ѡ��Ի�ѡ��
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
	printf("ѡ��Ի�ѡ��:%d,%d\n", x, y);
}

void Game::ClickMagic(char key)
{
	// ����F1(3,10-20,20)
	// ����F2(35,10-55,20)

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
	FindButtonWnd(BUTTON_ID_MAGIC, child, parent, "����");
	RightClick(x, y, child);
}

// ����ջ
void Game::FakeStackFrame(DWORD stack[], DWORD frames[], DWORD length)
{
	DWORD i = 0, index = 0;
	for (; i < length; i++) {
		stack[index] = (DWORD)&(stack[index + 2]);
		stack[index + 1] = frames[i];
		index += 2;
	}
}

// ���︴��
void Game::Call_ReBoren(int flag)
{
	::printf("���︴��\n");
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

// �����ƶ�����
void Game::Call_Run(int x, int y)
{
#if 0
	::printf("�ƶ�:%d,%d\n", x, y);
	// 4:0x6AEC8B55 4:0xBE7468FF 4:0xA1640326 4:0x00000000 4:0x25896450 4:0x00000000 4:0x04D8EC81 4:0x56530000
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC3 func = (_FUNC3)m_GameCall.Run;
	//::printf("׼�������ƶ�����\n");
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
	//::printf("�ƶ������������\n");
#endif
}

// ����CALL[type 0=����Ƶ�� 1=˽�� 2=����]
void Game::Call_Talk(const char* msg, int type)
{
#if 0
	DWORD arg = 0, func = m_GameCall.Talk;
	if (!func)
		return;

	printf("��������:%s (%d)\n", msg, type);
	if (type == 1) { // ����Ƶ��
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
		push 0x00       // 0x015A63F8 // �����Ǳ�����ַ
		push text       // ��������[�̶���ַ]
		push 0x00       // 0x015A63D8 // �����Ǳ�����ַ
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

// NPC�Ի�
void Game::Call_NPC(int npc_id)
{
	//INLOGVARN(32, "��NPC:%08X\n", npc_id);
	::printf("��NPC:%08X������\n", npc_id);
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

// NPC�����Ի�
void Game::Call_NPCTalk(int no, bool close)
{
	//INLOGVARN(32, "�Ի�NPC��%d\n", no);
	// eax=038735B8-2550000=13235B8
	// edi=0343DDD8-2550000=EEDDD8
	// esi=05DD5314
	DWORD _ebp_;
	DWORD user32 = m_GameCall.User32Ret04;
	DWORD soul = m_GameCall.SoulRet0C;
	_FUNC_R func = (_FUNC_R)m_GameCall.ITaskGetInstance;
	DWORD _eax = func();
	DWORD _esi = m_GameAddr.CallNpcTalkEsi;
	::printf("NPC�Ի�ѡ�� no:%d _eax:%08X _edi:%08X _esi:%08X\n", no, _eax, PtrToDword(_eax), _esi);

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

// �ر���ʾ��
void Game::Call_CloseTipBox(int close)
{
#if 0
	::printf("�ر���ʾ��:%d\n", close);
	DWORD _ecx = m_GameAddr.TipBoxSta;
	_FUNC3 func = (_FUNC3)m_GameCall.CloseTipBox;
	__asm { mov ecx, _ecx };
	func(0x0A, close, 0x00);
#endif
}

// ��ȡ�ֿ���Ʒ����
DWORD Game::Call_GetPackageItemAmount()
{
	_FUNC1_R func = (_FUNC1_R)m_GameCall.GetPkAgeItemAmount;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD count = func(0x0A);
	ASM_RESTORE_ECX();
	return count;
}

// ��ȡ�ֿ���Ʒָ��
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

// ��ȡ��Ʒ����
DWORD Game::Call_GetItemAmount(int flag)
{
	_FUNC1_R func = (_FUNC1_R)m_GameCall.GetItemAmount;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD count = func(flag);
	ASM_RESTORE_ECX();
	return count;
}

// ��ȡ��Ʒָ��
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
	

// ʹ����Ʒ
void Game::Call_UseItem(int item_id)
{
	::printf("ʹ����Ʒ:%08X\n", item_id);
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

// ����Ʒ
void Game::Call_DropItem(int item_id)
{
#if 0
	::printf("������Ʒ:%08X\n", item_id);
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

// ����Ʒ
void Game::Call_PickUpItem(DWORD id, DWORD x, DWORD y)
{
#if 0
	::printf("ʰȡ��Ʒ:%08X %d,%d������\n", id, x, y);
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

// ������
void Game::Call_SellItem(int item_id)
{
#if 0
	::printf("������:%08X\n", item_id);
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

// ��Ǯ
void Game::Call_SaveMoney(int money)
{
#if 0
	::printf("��Ǯ:%d\n", money);
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

// ����Զ�ֿ̲�
void Game::Call_CheckInItem(int item_id)
{
#if 0
	//::printf("������Ʒ:%08X\n", item_id);
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

// ȡ���ֿ���Ʒ
void Game::Call_CheckOutItem(int item_id)
{
#if 0
	::printf("ȡ���ֿ���Ʒ:%08X\n", item_id);
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

// ʹ�ÿɴ�����Ʒ
void Game::Call_TransmByMemoryStone(int item_id)
{
	::printf("ʹ�ÿɴ�����Ʒ:%08X\n", item_id);
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
// �ż���
void Game::Call_Magic(int magic_id, int guaiwu_id)
{
#if 0
	::printf("���ܹ���ID:%08X %08X������\n", magic_id, guaiwu_id);
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
	//::printf("����ʹ�����\n");
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

// �ż���
void Game::Call_Magic(int magic_id, int x, int y)
{
#if 0
	// 05D05020 06832EA4 mov ecx,dword ptr ds:[0xF1A518]
	::printf("����XY:%08X %d,%d������\n", magic_id, x, y);
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
	//::printf("����ʹ�����\n");
#endif
}


// �������
void Game::Call_PetOut(int pet_id)
{
	
}

// �����ٻ�
void Game::Call_PetIn(int pet_id)
{

}

// �������
void Game::Call_PetFuck(int pet_id)
{
	
}

// �������
void Game::Call_PetUnFuck(int pet_id)
{
	
}

// ���＼�� key_no=�������� 1=0 2=1 ...
void Game::Call_PetMagic(int key_no)
{
	
}

// ��ȡԶ������������Ϣ
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

// �Ƿ��ж���
int Game::Call_IsHaveTeam()
{
	if (!m_GameCall.IsHaveTeam)
		return 0;

	_FUNC_R func = (_FUNC_R)m_GameCall.IsHaveTeam;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD r = func();
	ASM_RESTORE_ECX();
	::printf("�Ƿ��ж���:%d\n", r);
	return r != 0;
}

// �Ƿ��Ƕӳ�
int Game::Call_IsTeamLeader()
{
	if (!m_GameCall.IsTeamLeader)
		return 0;

	_FUNC_R func = (_FUNC_R)m_GameCall.IsTeamLeader;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD r = func();
	ASM_RESTORE_ECX();
	::printf("�Ƿ�ӳ�:%d\n", r);
	return r;
}

// ��������
void Game::Call_TeamCreate()
{
	::printf("��������\n");
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

// �뿪����[�ӳ�]
void Game::Call_TeamDismiss()
{
	::printf("�뿪����\n");
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

// �뿪����[��Ա]
void Game::Call_TeamLeave()
{
	::printf("�뿪����\n");
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

// �������
void Game::Call_TeamInvite(int player_id)
{
	::printf("�������:%08X\n", player_id);
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

// �Զ����
void Game::Call_TeamAutoJoin(int open)
{
	::printf("�Զ����:%d\n", open);
	_FUNC1 func = (_FUNC1)m_GameCall.TeamAutoJoin;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(open);
	ASM_RESTORE_ECX();
}

// �Ƿ�ѡ���������
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
	//���붯̬�������������  
	typedef void (WINAPI *PROCSWITCHTOTHISWINDOW)(HWND, BOOL);
	PROCSWITCHTOTHISWINDOW SwitchToThisWindow;
	HMODULE hUser32 = GetModuleHandle(L"user32");
	SwitchToThisWindow = (PROCSWITCHTOTHISWINDOW)
		GetProcAddress(hUser32, "SwitchToThisWindow");

	//������ֻҪ���κ��ִ洰�ڵľ����������������ɣ�
	//�ڶ���������ʾ������ڴ�����С��״̬���Ƿ�ָ���
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
	::printf("���ھ��:%0x %d %d %d %d %.2f\n", m_hWnd, m_GameWnd.Width, m_GameWnd.Height,
		m_GameWnd.Rect.left, m_GameWnd.Rect.top, m_fScale);

	//SwitchToThisWindow(m_hWnd, TRUE);
	SetForegroundWindow(m_hWnd);
	int clx = m_GameWnd.Rect.left + 466, cly = m_GameWnd.Rect.top + 479;
	if (!SetCursorPos(clx, cly)) {
		m_pGameProc->SendMsg("�������ʧ��, �ƶ����.|red");
		mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, clx * 65536 / GetSystemMetrics(SM_CXSCREEN),
			cly * 65536 / GetSystemMetrics(SM_CYSCREEN), 0, 0);
	}
	//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, clx, cly, 0, 0);
	Sleep(500);
	HWND hWnd = parent;
	LeftClick(107, 216, hWnd);
	//mouse_event(MOUSEEVENTF_LEFTDOWN, clx, cly, 0, 0);//�������
	//mouse_event(MOUSEEVENTF_LEFTUP, clx, cly, 0, 0);//�������
	printf("���(%d %d)\n", clx, cly);

	char msg[64];
	sprintf_s(msg, "���(%d %d)\n", clx, cly);
	m_pGameProc->SendMsg(msg);

	return;
#endif

	if (!m_GameAddr.TeamChkSta)
		return;

	::printf("�����������ѡ��:%d\n", v);
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

// ��ȡ��������
DWORD Game::Call_GetMagicAmount()
{
	_FUNC5_R func = (_FUNC5_R)m_GameCall.GetMagicAmount;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD count = func(0x00, 0x00, 0x00, 0x00, 0x00);
	ASM_RESTORE_ECX();
	return count;
}

// ��ȡ���ܵ�ַ
DWORD Game::Call_GetMagicByIndex(int index)
{
	_FUNC6_R func = (_FUNC6_R)m_GameCall.GetMagicByIndex;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD ptr = func(index, 0x00, 0x00, 0x00, 0x00, 0x00);
	ASM_RESTORE_ECX();
	return ptr;
}

// ��ȡ��Χ�������
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

// ��ȡ��Χ��ҵ�ַ
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

// ��ȡ��ַ[��֪��ʲô���Ի�ȡ]
DWORD Game::Call_GetBaseAddr(int index, DWORD _ecx)
{
	_FUNC1_R func = (_FUNC1_R)m_GameCall.GetNpcBaseAddr;
	ASM_STORE_ECX();
	__asm { mov ecx, _ecx };
	DWORD p = func(index);
	ASM_RESTORE_ECX();
	return PtrToDword(p);
}
