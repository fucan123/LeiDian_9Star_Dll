#include "GameClient.h"
#include "Game.h"
#include "GameProc.h"
#include "Item.h"
#include <stdio.h>

#define RECV_NAME(var,data,len)  char var[32]; ZeroMemory(var,sizeof(var)); memcpy(var,data,len); 
#define RECV_ROLE(var,data,len)  char var[16]; ZeroMemory(var,sizeof(var)); memcpy(var,data,len); 

// 自身指针
GameClient* GameClient::self = nullptr;

// ...
GameClient::GameClient(Game* p)
{
	m_pGame = p;
}

// 设置自身指针
void GameClient::SetSelf(GameClient* p)
{
	self = p;
}

void GameClient::Connect(const char* host, USHORT port)
{
	m_Client.onconnect_error = OnConnectError;
	m_Client.onconect = OnConnect;
	m_Client.onread = OnRead;
	m_Client.onclose = OnClose;
	m_Client.onerror = OnError;

	while (true) {
		m_Client.Connect(host, port);
		Sleep(5000);
		printf("与服务器连接断开, 准备重连.\n");
	}
}

void GameClient::ReConnect(const char * host, USHORT port)
{
	Sleep(5000);
	printf("重连服务器...\n");
	Connect(host, port);
}

// 要登录的帐号
void GameClient::Account(const char * data, int len)
{
	char path[255], ser_big[32], ser_small[32], name[32], pwd[32];
	ZeroMemory(path, sizeof(path));
	ZeroMemory(ser_big, sizeof(name));
	ZeroMemory(ser_small, sizeof(name));
	ZeroMemory(name, sizeof(name));
	ZeroMemory(pwd, sizeof(pwd));

	const char* tmp = data;

	int path_len = P2INT(tmp);
	memcpy(path, tmp + 4, path_len);
	tmp += 4 + path_len;

	int ser_big_len = P2INT(tmp);
	memcpy(ser_big, tmp + 4, ser_big_len);
	tmp += 4 + ser_big_len;

	int ser_small_len = P2INT(tmp);
	memcpy(ser_small, tmp + 4, ser_small_len);
	tmp += 4 + ser_small_len;

	int name_len = P2INT(tmp);
	memcpy(name, tmp + 4, name_len);
	tmp += 4 + name_len;

	int pwd_len = P2INT(tmp);
	memcpy(pwd, tmp + 4, pwd_len);
	tmp += 4 + pwd_len;

	int role_no = P2INT(tmp);
	int getxl = P2INT(tmp + 4);
	int big = P2INT(tmp + 8);
	int getxl_logout = P2INT(tmp + 0x0C);
	int create_team = P2INT(tmp + 0x10);
	self->m_pGame->SetAccount(ser_big, ser_small, name, pwd, role_no, getxl, big, getxl_logout, create_team);
	self->m_pGame->SetPath(path);
	self->Send(SCK_LOGIN, true); // 发送可以登录
}

// 已经可以组队
void GameClient::CanTeam(const char* data, int len)
{
	int index = P2INT(data); // 服务端帐号列表索引
	RECV_ROLE(role, data + 4, len - 4);
	int result = m_pGame->m_pGameProc->ViteInTeam(role);

	if (result != -1) {
		// 告诉服务端发送邀请结果
		m_Client.ClearSendString();
		m_Client.SetInt(index); // 原样返回给她
		m_Client.SetContent(&result, 4);
		Send(SCK_INTEAM, false);
	}
}

// 可以入队了
void GameClient::InTeam()
{
	m_pGame->m_pGameProc->InTeam();

	// 告诉服务端已准备好可以进副本了
	Send(SCK_CANINFB, true);
}

// 可以进副本了
void GameClient::CanInFB()
{
}

// 开启副本
void GameClient::OpenFB()
{
	m_pGame->m_pGameProc->GoFBDoor();
	if (m_pGame->m_pGameProc->GoInFB()) {
		Send(SCK_INFB, true);
	}
}

// 进入副本
void GameClient::InFB()
{
	m_pGame->m_pGameProc->AgreeInFB();
}

// 出去副本
void GameClient::OutFB()
{
	if (!m_pGame->IsBig()) {
		m_pGame->m_pGameProc->GoOutFB("卡利亚堡传送门");
		//Sleep(1000);
		//m_pGame->m_pItem->UseSelfItem("星辰之眼", 250, 500, 10);
	}	
}

// 小号进入副本
void GameClient::SmallInFB()
{
	if (!m_pGame->IsBig()) {
		m_pGame->m_pGameProc->ContinueInFB();
	}
}

// 小号出去副本
void GameClient::SmallOutFB()
{
	if (!m_pGame->IsBig()) {
		SendMsg("暂时出副本");
		m_pGame->m_pGameProc->GoOutFB("卡利亚堡传送门", false, true);
		Sleep(1000);
		m_pGame->m_pItem->UseSelfItem("星辰之眼", 250, 500, 10);
	}	
}

// 获取进副本项链数量
void GameClient::GetXL()
{
	bool free_in = false; // 是否可以免费进
	SYSTEMTIME stLocal;
	::GetLocalTime(&stLocal);
	if (stLocal.wHour >= 20 && stLocal.wHour <= 23) { // 20点到24点可以免费进
		free_in = stLocal.wHour == 23 ? stLocal.wMinute < 59 : true; // 至少需要1分钟
	}

	if (!free_in) {
		// 获取项链[爱娜祈祷项链]
		m_pGame->m_pGameProc->GoGetXiangLian();
		int count = m_pGame->m_pItem->GetSelfItemCountByName("爱娜祈祷项链");
		printf("拥有项链数量:%d\n", count);

		char msg[32];
		sprintf_s(msg, "拥有项链数量[new]:%d\n", count);
		SendMsg(msg);

		m_Client.ClearSendString();
		m_Client.SetInt(count);
		Send(SCK_GETXL, false);
	}
	else {
		int count = m_pGame->m_pItem->GetSelfItemCountByName("卡利亚堡钥匙");
		printf("拥有钥匙数量:%d\n", count);

		char msg[32];
		sprintf_s(msg, "拥有钥匙数量:%d\n", count);
		SendMsg(msg);

		m_Client.ClearSendString();
		m_Client.SetInt(count);
		Send(SCK_GETXL, false);
	}
}

// 关闭游戏
void GameClient::CloseGame(bool leave_team)
{
	m_pGame->Close(leave_team);
}

// 发送消息
int GameClient::SendMsg(const char * v, const char * v2)
{
	m_Client.ClearSendString();
	m_Client.SetContent((void*)v, strlen(v));
	if (v2) {
		m_Client.SetContent((void*)"->", 2);
		m_Client.SetContent((void*)v2, strlen(v2));
	}
	return Send(SCK_MSG, false);
}

// 发送消息
int GameClient::SendMsg2(const char * v)
{
	m_Client.ClearSendString();
	m_Client.SetContent((void*)v, strlen(v));
	return Send(SCK_MSG2, false);
}

// 通知已游戏窗口打开
int GameClient::SendOpen(float scale, HWND wnd, RECT& rect)
{
	m_Client.ClearSendString();
	m_Client.SetContent(&scale, sizeof(float));
	m_Client.SetContent(&wnd, sizeof(HWND));
	m_Client.SetContent(&rect, sizeof(RECT));
	return Send(SCK_OPEN, false);
}

// 通知游戏已上线
int GameClient::SendInGame(const char* name, const char* role)
{
	int name_len = strlen(name);
	int role_len = strlen(role);

	m_Client.ClearSendString();
	m_Client.SetInt(name_len);
	m_Client.SetContent((void*)name, name_len);
	m_Client.SetInt(role_len);
	m_Client.SetContent((void*)role, role_len);
	return Send(SCK_INGAME);
}

// 发送捡了什么物品
int GameClient::SendPickUpItem(const char * name)
{
	m_Client.ClearSendString();
	m_Client.SetContent((void*)name, strlen(name));
	return Send(SCK_PICKUPITEM, false);
}

// 发送数据
int GameClient::Send()
{
	int ret = send(m_Socket, m_Client.GetSendString(), m_Client.GetSendLength(), 0);
	//printf("发送:%s(%d) %d\n", m_Client.GetSendString(), m_Client.GetSendLength(), ret);
	return ret;
}

// 发送数据
int GameClient::Send(SOCKET_OPCODE opcode, bool clear)
{
	if (clear) {
		m_Client.ClearSendString();
	}
	m_Client.MakeSendString(opcode);
	return Send();
}


void GameClient::OnConnectError(const char* host, USHORT port)
{
	//printf("连接不上服务器！\n");
	//self->ReConnect(host, port);
}

void GameClient::OnConnect(const char* host, USHORT port, SOCKET s)
{
	printf("连接服务器成功[%s:%d]\n", host, port);
	self->m_Socket = s;
	if (strlen(self->m_pGame->m_Account.Role) > 0) {
		self->SendInGame(self->m_pGame->m_Account.Name, self->m_pGame->m_Account.Role);
	}
}

void GameClient::OnRead(const char* host, USHORT port, int opcode, const char* data, int len)
{
#if 0
	if (opcode != SCK_PING)
		printf("接受>>指令:%d 数据长度：%d\n", opcode, len);
#endif
	if (self->m_pGame->m_nVerFail >= 10) {
		//::MessageBoxA(NULL, "self->m_pGame->m_nVerFail >= 10", "...", MB_OK);
		return;
	}

	/*for (int i = 0; i < 4; i++) {
		printf("%02x ", data[i] & 0xff);
	}
	printf("\n");*/
	switch (opcode)
	{
	case SCK_PING:
		self->Send(SCK_PING, true); // 回应服务端
		break;
	case SCK_ACCOUNT:
		self->Account(data, len);
		break;
	case SCK_CANTEAM: // 可以组队了
		self->CanTeam(data, len);
		break;
	case SCK_INTEAM:  // 可以入队了
		self->InTeam();
		break;
	case SCK_CANINFB: // 可以进去副本
		self->CanInFB();
		break;
	case SCK_OPENFB:  // 开启副本
		self->OpenFB();
		break;
	case SCK_INFB:    // 进去副本
		self->InFB();
		break;
	case SCK_OUTFB:   // 出去副本
		self->OutFB();
		break;
	case SCK_SMALLINFB:   // 小号进去副本
		printf("SCK_SMALLINFB 小号进去副本\n");
		self->SmallInFB();
		break;
	case SCK_SMALLOUTFB:  // 小号出去副本
		printf("SCK_SMALLOUTFB 小号出去副本\n");
		self->SmallOutFB();
		break;
	case SCK_GETXL:   // 获取项链
		self->GetXL();
		break;
	case SCK_QIAZHUS: // 卡住多久重启
		self->m_pGame->SetQiaZhuS(P2INT(data));
		break;
	case SCK_DEBUG: // 是否显示调试
		self->m_pGame->m_iDebug = P2INT(data);
		break;
	case SCK_NOAUOTOSLT: // 是否手动选区
		self->m_pGame->m_iNoAutoSelect = P2INT(data);
		break;
	case SCK_TALK_OPEN:  // 自动喊话开启
		self->m_pGame->m_iTalkOpen = P2INT(data);
		break;
	case SCK_TALK:       // 自动喊话
		self->m_pGame->Call_Talk(data + 8, P2INT(data));
		break;
	case SCK_CLOSE:   // 关闭游戏
		self->CloseGame(len > 0);		
		break;
	default:
		break;
	}
}

void GameClient::OnClose(const char* host, USHORT port)
{
	printf("服务器关闭[%s:%d]！\n", host, port);
	self->m_Socket = 0;
	//self->ReConnect(host, port);
}

void GameClient::OnError(const char* host, USHORT port)
{
	printf("连接发送错误[%s:%d]！\n", host, port);
	self->m_Socket = 0;
}