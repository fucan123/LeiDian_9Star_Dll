#include "GameClient.h"
#include "Game.h"
#include "GameProc.h"
#include "Item.h"
#include <stdio.h>

#define RECV_NAME(var,data,len)  char var[32]; ZeroMemory(var,sizeof(var)); memcpy(var,data,len); 
#define RECV_ROLE(var,data,len)  char var[16]; ZeroMemory(var,sizeof(var)); memcpy(var,data,len); 

// ����ָ��
GameClient* GameClient::self = nullptr;

// ...
GameClient::GameClient(Game* p)
{
	m_pGame = p;
}

// ��������ָ��
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
		printf("����������ӶϿ�, ׼������.\n");
	}
}

void GameClient::ReConnect(const char * host, USHORT port)
{
	Sleep(5000);
	printf("����������...\n");
	Connect(host, port);
}

// Ҫ��¼���ʺ�
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
	self->Send(SCK_LOGIN, true); // ���Ϳ��Ե�¼
}

// �Ѿ��������
void GameClient::CanTeam(const char* data, int len)
{
	int index = P2INT(data); // ������ʺ��б�����
	RECV_ROLE(role, data + 4, len - 4);
	int result = m_pGame->m_pGameProc->ViteInTeam(role);

	if (result != -1) {
		// ���߷���˷���������
		m_Client.ClearSendString();
		m_Client.SetInt(index); // ԭ�����ظ���
		m_Client.SetContent(&result, 4);
		Send(SCK_INTEAM, false);
	}
}

// ���������
void GameClient::InTeam()
{
	m_pGame->m_pGameProc->InTeam();

	// ���߷������׼���ÿ��Խ�������
	Send(SCK_CANINFB, true);
}

// ���Խ�������
void GameClient::CanInFB()
{
}

// ��������
void GameClient::OpenFB()
{
	m_pGame->m_pGameProc->GoFBDoor();
	if (m_pGame->m_pGameProc->GoInFB()) {
		Send(SCK_INFB, true);
	}
}

// ���븱��
void GameClient::InFB()
{
	m_pGame->m_pGameProc->AgreeInFB();
}

// ��ȥ����
void GameClient::OutFB()
{
	if (!m_pGame->IsBig()) {
		m_pGame->m_pGameProc->GoOutFB("�����Ǳ�������");
		//Sleep(1000);
		//m_pGame->m_pItem->UseSelfItem("�ǳ�֮��", 250, 500, 10);
	}	
}

// С�Ž��븱��
void GameClient::SmallInFB()
{
	if (!m_pGame->IsBig()) {
		m_pGame->m_pGameProc->ContinueInFB();
	}
}

// С�ų�ȥ����
void GameClient::SmallOutFB()
{
	if (!m_pGame->IsBig()) {
		SendMsg("��ʱ������");
		m_pGame->m_pGameProc->GoOutFB("�����Ǳ�������", false, true);
		Sleep(1000);
		m_pGame->m_pItem->UseSelfItem("�ǳ�֮��", 250, 500, 10);
	}	
}

// ��ȡ��������������
void GameClient::GetXL()
{
	bool free_in = false; // �Ƿ������ѽ�
	SYSTEMTIME stLocal;
	::GetLocalTime(&stLocal);
	if (stLocal.wHour >= 20 && stLocal.wHour <= 23) { // 20�㵽24�������ѽ�
		free_in = stLocal.wHour == 23 ? stLocal.wMinute < 59 : true; // ������Ҫ1����
	}

	if (!free_in) {
		// ��ȡ����[����������]
		m_pGame->m_pGameProc->GoGetXiangLian();
		int count = m_pGame->m_pItem->GetSelfItemCountByName("����������");
		printf("ӵ����������:%d\n", count);

		char msg[32];
		sprintf_s(msg, "ӵ����������[new]:%d\n", count);
		SendMsg(msg);

		m_Client.ClearSendString();
		m_Client.SetInt(count);
		Send(SCK_GETXL, false);
	}
	else {
		int count = m_pGame->m_pItem->GetSelfItemCountByName("�����Ǳ�Կ��");
		printf("ӵ��Կ������:%d\n", count);

		char msg[32];
		sprintf_s(msg, "ӵ��Կ������:%d\n", count);
		SendMsg(msg);

		m_Client.ClearSendString();
		m_Client.SetInt(count);
		Send(SCK_GETXL, false);
	}
}

// �ر���Ϸ
void GameClient::CloseGame(bool leave_team)
{
	m_pGame->Close(leave_team);
}

// ������Ϣ
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

// ������Ϣ
int GameClient::SendMsg2(const char * v)
{
	m_Client.ClearSendString();
	m_Client.SetContent((void*)v, strlen(v));
	return Send(SCK_MSG2, false);
}

// ֪ͨ����Ϸ���ڴ�
int GameClient::SendOpen(float scale, HWND wnd, RECT& rect)
{
	m_Client.ClearSendString();
	m_Client.SetContent(&scale, sizeof(float));
	m_Client.SetContent(&wnd, sizeof(HWND));
	m_Client.SetContent(&rect, sizeof(RECT));
	return Send(SCK_OPEN, false);
}

// ֪ͨ��Ϸ������
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

// ���ͼ���ʲô��Ʒ
int GameClient::SendPickUpItem(const char * name)
{
	m_Client.ClearSendString();
	m_Client.SetContent((void*)name, strlen(name));
	return Send(SCK_PICKUPITEM, false);
}

// ��������
int GameClient::Send()
{
	int ret = send(m_Socket, m_Client.GetSendString(), m_Client.GetSendLength(), 0);
	//printf("����:%s(%d) %d\n", m_Client.GetSendString(), m_Client.GetSendLength(), ret);
	return ret;
}

// ��������
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
	//printf("���Ӳ��Ϸ�������\n");
	//self->ReConnect(host, port);
}

void GameClient::OnConnect(const char* host, USHORT port, SOCKET s)
{
	printf("���ӷ������ɹ�[%s:%d]\n", host, port);
	self->m_Socket = s;
	if (strlen(self->m_pGame->m_Account.Role) > 0) {
		self->SendInGame(self->m_pGame->m_Account.Name, self->m_pGame->m_Account.Role);
	}
}

void GameClient::OnRead(const char* host, USHORT port, int opcode, const char* data, int len)
{
#if 0
	if (opcode != SCK_PING)
		printf("����>>ָ��:%d ���ݳ��ȣ�%d\n", opcode, len);
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
		self->Send(SCK_PING, true); // ��Ӧ�����
		break;
	case SCK_ACCOUNT:
		self->Account(data, len);
		break;
	case SCK_CANTEAM: // ���������
		self->CanTeam(data, len);
		break;
	case SCK_INTEAM:  // ���������
		self->InTeam();
		break;
	case SCK_CANINFB: // ���Խ�ȥ����
		self->CanInFB();
		break;
	case SCK_OPENFB:  // ��������
		self->OpenFB();
		break;
	case SCK_INFB:    // ��ȥ����
		self->InFB();
		break;
	case SCK_OUTFB:   // ��ȥ����
		self->OutFB();
		break;
	case SCK_SMALLINFB:   // С�Ž�ȥ����
		printf("SCK_SMALLINFB С�Ž�ȥ����\n");
		self->SmallInFB();
		break;
	case SCK_SMALLOUTFB:  // С�ų�ȥ����
		printf("SCK_SMALLOUTFB С�ų�ȥ����\n");
		self->SmallOutFB();
		break;
	case SCK_GETXL:   // ��ȡ����
		self->GetXL();
		break;
	case SCK_QIAZHUS: // ��ס�������
		self->m_pGame->SetQiaZhuS(P2INT(data));
		break;
	case SCK_DEBUG: // �Ƿ���ʾ����
		self->m_pGame->m_iDebug = P2INT(data);
		break;
	case SCK_NOAUOTOSLT: // �Ƿ��ֶ�ѡ��
		self->m_pGame->m_iNoAutoSelect = P2INT(data);
		break;
	case SCK_TALK_OPEN:  // �Զ���������
		self->m_pGame->m_iTalkOpen = P2INT(data);
		break;
	case SCK_TALK:       // �Զ�����
		self->m_pGame->Call_Talk(data + 8, P2INT(data));
		break;
	case SCK_CLOSE:   // �ر���Ϸ
		self->CloseGame(len > 0);		
		break;
	default:
		break;
	}
}

void GameClient::OnClose(const char* host, USHORT port)
{
	printf("�������ر�[%s:%d]��\n", host, port);
	self->m_Socket = 0;
	//self->ReConnect(host, port);
}

void GameClient::OnError(const char* host, USHORT port)
{
	printf("���ӷ��ʹ���[%s:%d]��\n", host, port);
	self->m_Socket = 0;
}