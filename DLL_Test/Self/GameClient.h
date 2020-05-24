#pragma once
#include <My/Socket/Client.h>

enum SOCKET_OPCODE
{
	SCK_PING = 0x00,
	SCK_MSG,         // 信息
	SCK_MSG2,        // 信息
	SCK_OPEN,        // 游戏窗口已打开
	SCK_ACCOUNT,     // 帐号信息 0-3位=选择哪个角色, 4-7=是否是大号
	SCK_LOGIN,       // 可以登录帐号了
	SCK_INGAME,      // 已进入游戏
	SCK_CANTEAM,     // 可以组队了
	SCK_INTEAM,      // 入队
	SCK_CANINFB,     // 可以进副本了
	SCK_OPENFB,      // 开启副本
	SCK_INFB,        // 进入副本
	SCK_OUTFB,       // 出去副本
	SCK_ATFB,        // 是否已在副本
	SCK_SMALLOUTFB,  // 小号出副本
	SCK_SMALLINFB,   // 小号进副本
	SCK_GETXL,       // 获取进副本项链数量
	SCK_NOYAOSI,     // 小号没有钥匙
	SCK_PICKUPITEM,  // 捡物
	SCK_REBORN,      // 复活
	SCK_QIAZHUS,     // 卡住多久重启
	SCK_OFFLINE,     // 游戏下线了
	SCK_CLOSE,       // 关闭游戏

	SCK_DEBUG,       // 是否调试
	SCK_NOAUOTOSLT,  // 不自动选区
	SCK_TALK,        // 喊话
	SCK_TALK_OPEN,   // 喊话开启
};

class Game;
class GameClient
{
public:
	// ...
	GameClient(Game* p);
	// 设置自身指针
	void SetSelf(GameClient* p);

	void Connect(const char* host, USHORT port);
	void ReConnect(const char* host, USHORT port);
	
	// 要登录的帐号
	void Account(const char* data, int len);
	// 已经可以组队
	void CanTeam(const char* data, int len);
	// 可以入队了
	void InTeam();
	// 可以进副本了
	void CanInFB();
	// 开启副本
	void OpenFB();
	// 进入副本
	void InFB();
	// 出去副本
	void OutFB();
	// 小号进入副本
	void SmallInFB();
	// 小号出去副本
	void SmallOutFB();
	// 获取进副本项链数量 
	void GetXL();
	// 关闭游戏
	void CloseGame(bool leav_team);

	// 发送消息
	int SendMsg(const char* v, const char* v2=nullptr);
	// 发送消息
	int SendMsg2(const char* v);
	// 通知已游戏窗口打开
	int SendOpen(float scale, HWND wnd, RECT& rect);
	// 通知游戏已上线
	int SendInGame(const char* name, const char* role);
	// 发送捡了什么物品
	int SendPickUpItem(const char* name);
	// 发送数据
	int  Send();
	// 发送数据
	int  Send(SOCKET_OPCODE, bool clear=false);

	static void OnConnectError(const char*, USHORT);
	static void OnConnect(const char*, USHORT, SOCKET);
	static void OnRead(const char*, USHORT, int, const char*, int);
	static void OnClose(const char*, USHORT);
	static void OnError(const char*, USHORT);
public:
	// 自身指针
	static GameClient* self;

	// Game类指针
	Game* m_pGame;

	// Cleint类
	Client m_Client;
	// 服务器Socket
	SOCKET m_Socket = 0;
};