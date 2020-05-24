#pragma once
#include <My/Socket/Client.h>

enum SOCKET_OPCODE
{
	SCK_PING = 0x00,
	SCK_MSG,         // ��Ϣ
	SCK_MSG2,        // ��Ϣ
	SCK_OPEN,        // ��Ϸ�����Ѵ�
	SCK_ACCOUNT,     // �ʺ���Ϣ 0-3λ=ѡ���ĸ���ɫ, 4-7=�Ƿ��Ǵ��
	SCK_LOGIN,       // ���Ե�¼�ʺ���
	SCK_INGAME,      // �ѽ�����Ϸ
	SCK_CANTEAM,     // ���������
	SCK_INTEAM,      // ���
	SCK_CANINFB,     // ���Խ�������
	SCK_OPENFB,      // ��������
	SCK_INFB,        // ���븱��
	SCK_OUTFB,       // ��ȥ����
	SCK_ATFB,        // �Ƿ����ڸ���
	SCK_SMALLOUTFB,  // С�ų�����
	SCK_SMALLINFB,   // С�Ž�����
	SCK_GETXL,       // ��ȡ��������������
	SCK_NOYAOSI,     // С��û��Կ��
	SCK_PICKUPITEM,  // ����
	SCK_REBORN,      // ����
	SCK_QIAZHUS,     // ��ס�������
	SCK_OFFLINE,     // ��Ϸ������
	SCK_CLOSE,       // �ر���Ϸ

	SCK_DEBUG,       // �Ƿ����
	SCK_NOAUOTOSLT,  // ���Զ�ѡ��
	SCK_TALK,        // ����
	SCK_TALK_OPEN,   // ��������
};

class Game;
class GameClient
{
public:
	// ...
	GameClient(Game* p);
	// ��������ָ��
	void SetSelf(GameClient* p);

	void Connect(const char* host, USHORT port);
	void ReConnect(const char* host, USHORT port);
	
	// Ҫ��¼���ʺ�
	void Account(const char* data, int len);
	// �Ѿ��������
	void CanTeam(const char* data, int len);
	// ���������
	void InTeam();
	// ���Խ�������
	void CanInFB();
	// ��������
	void OpenFB();
	// ���븱��
	void InFB();
	// ��ȥ����
	void OutFB();
	// С�Ž��븱��
	void SmallInFB();
	// С�ų�ȥ����
	void SmallOutFB();
	// ��ȡ�������������� 
	void GetXL();
	// �ر���Ϸ
	void CloseGame(bool leav_team);

	// ������Ϣ
	int SendMsg(const char* v, const char* v2=nullptr);
	// ������Ϣ
	int SendMsg2(const char* v);
	// ֪ͨ����Ϸ���ڴ�
	int SendOpen(float scale, HWND wnd, RECT& rect);
	// ֪ͨ��Ϸ������
	int SendInGame(const char* name, const char* role);
	// ���ͼ���ʲô��Ʒ
	int SendPickUpItem(const char* name);
	// ��������
	int  Send();
	// ��������
	int  Send(SOCKET_OPCODE, bool clear=false);

	static void OnConnectError(const char*, USHORT);
	static void OnConnect(const char*, USHORT, SOCKET);
	static void OnRead(const char*, USHORT, int, const char*, int);
	static void OnClose(const char*, USHORT);
	static void OnError(const char*, USHORT);
public:
	// ����ָ��
	static GameClient* self;

	// Game��ָ��
	Game* m_pGame;

	// Cleint��
	Client m_Client;
	// ������Socket
	SOCKET m_Socket = 0;
};