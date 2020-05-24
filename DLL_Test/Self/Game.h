#pragma once
#include "GameStruct.h"

typedef LRESULT(CALLBACK *Game_KbdProc)(int nCode, WPARAM wParam, LPARAM lParam);
typedef LRESULT(CALLBACK *Game_MouseProc)(int nCode, WPARAM wParam, LPARAM lParam);
typedef void (CALLBACK* SetAccountProc_Func)(int big);

// 搜索模块中方法名称需要的信息
typedef struct search_mod_func_msg
{
	const char*   Name;    // 搜索的名称
	const char*   Substr;  // 名称中包含或不包含的字符串
	DWORD         Flag;    // 0-不包含 1-包含
	DWORD*        Save;    // 保存地址
	const char*   Remark;  // 说明
} SearchModFuncMsg;

typedef struct _account_
{
	char SerBig[32];   // 游戏大区
	char SerSmall[32]; // 游戏小区
	char Name[32];     // 帐号
	char Pwd[32];      // 密码
	char Role[16];     // 角色名字
	int  RoleNo;       // 选择哪个角色登录
	int  IsGetXL;      // 是否获取了项链
	int  IsBig;        // 是否大号
	int  IsLogin;      // 是否登录了
	int  LoginTime;    // 登录时间
	int  IsOffLine;    // 掉线了
	int  GetXLTime;    // 领取项链时间
	int  GetXLLogout;  // 领取项链后是否退出
	int  CreateTeam;   // 是否创建队伍
	bool IsReady;      // 是否已准备好
	bool IsMeOpenFB;   // 是否是自己开副本的
} Account;

// 游戏g_pObjHero全局变量(CHero类this指针)
extern DWORD g_pObjHero;
// 游戏g_objPlayerSet全局变量
extern DWORD g_objPlayerSet;

class Home;
class GameClient;
class GameConf;
class GameProc;
class Item;
class Talk;
class Move;
class GuaiWu;
class Magic;
class Pet;

class Game
{
public:
	// ...
	Game();
	// >>>
	~Game();
	// 验证
	void VerifyServer();
	// 关闭游戏
	void Close(bool leave_team=true);
	// 连接管理服务
	void Connect(const char* host, USHORT port);
	// 登录游戏帐号
	void Login();
	// 选择区服
	void SelectServer(int flag=0xff);
	// 输入帐号密码
	void InputUserPwd(bool input_user=true);
	// 初始化
	bool Init(DWORD hook_tid=0, SetAccountProc_Func set_account_proc=NULL);
	// 等待游戏初始化完毕
	void WaitGameInit(int wait_s);
	// 设置路径
	void SetPath(const char* path);
	// 设置是否是大号
	void SetAccount(const char* ser_big, const char* ser_small, const char* name, const char* pwd, int role_no, int getxl, int big, int getxl_logout, int create_team);
	// 设置卡住重启时间
	void SetQiaZhuS(int second);
	// 设置是否掉线
	void SetOffLine(int v);
	// 进程是否是魔域
	bool IsMoYu();
	// 是否登录了
	bool IsLogin();
	// 是否掉线了 
	bool IsOffLine();
	// 比较登录帐号
	bool CmpLoginAccount(const char* name);
	// 是否大号
	bool IsBig();
	// 是否在副本
	bool IsInFB();
	// 是否在指定地图
	bool IsInMap(const char* name);
	// 是否在指定区域坐标 allow=误差
	bool IsInArea(int x, int y, int allow=10);
	// 是否不在指定区域坐标 allow=误差
	bool IsNotInArea(int x, int y, int allow = 10);
	// 是否已获取了项链
	bool IsGetXL();
	// 运行
	void Run();
	// 停止
	void Stop(bool v = true);
	// 获取游戏中心位置在屏幕上的坐标
	void GetGameCenterPos(int& x, int& y);
	// 获得游戏窗口
	HWND FindGameWnd();
	// 获取所有模块地址
	void FindAllModAddr();
	// 获取游戏所有CALL
	void FindAllCall();
	// 获取user32 ret 04地址
	DWORD FindUser32Ret04();
	// 获取NPC对话函数
	DWORD FindNPCTalkCall();
	// 获取副本邀请队伍选择框函数
	DWORD FindTeamChkCall();
	// 数字按键函数
	DWORD FindKeyNumCall();
	// 获取关闭提示框函数
	DWORD FindCloseTipBoxCall();
	// 获取获取NPC基地址函数
	DWORD FindGetNpcBaseAddr();
	// 获取模块地址
	DWORD FindModAddr(LPCWSTR name);
	// 读取坐标
	bool ReadCoor(DWORD* x=NULL, DWORD* y=NULL);
	// 读取生命值
	bool ReadLife(int& life, int& life_max);
	// 读取药包数量
	bool ReadQuickKey2Num(int* nums, int length);
	// 读取包包物品
	bool ReadBag(DWORD* bag, int length);
	// 获得窗口句柄
	bool FindButtonWnd(int button_id, HWND& hwnd, HWND& parent, const char* text=nullptr);
	// 获取登录选择的小区地址
	bool FindLoginSmallServerAddr();
	// 获取坐标地址
	bool FindCoorAddr();
	// 获取移动状态地址
	bool FindMoveStaAddr();
	// 获取对话框状态地址
	bool FindTalkBoxStaAddr();
	// 获取是否选择邀请队伍状态地址
	bool FindTeamChkStaAddr();
	// 获取提示框状态地址
	bool FindTipBoxStaAddr();
	// 获取生命地址
	bool FindLifeAddr();
	// 获取背包代码
	bool FindBagAddr();
	// 获得地面物品地址的保存地址
	bool FindItemPtr();
	// 获取NPC二级对话ESI寄存器数值
	bool FindCallNPCTalkEsi();
	// 获取宠物列表基地址
	bool FindPetPtrAddr();
	// 获取地图名称地址
	bool FindMapName();
	// 在某个模块中搜索函数
	DWORD SearchFuncInMode(SearchModFuncMsg* info, HANDLE hMod);
	// 在某个模块里面搜索
	DWORD SearchInMod(LPCTSTR name, DWORD* codes, DWORD length, DWORD* save, DWORD save_length = 1, DWORD step = 1);
	// 在某个模块里面搜索
	DWORD SearchInModByte(LPCTSTR name, BYTE* codes, DWORD length);
	// 搜索特征码
	DWORD SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length=1, DWORD step=4);
	// 搜索特征码
	DWORD SearchByteCode(BYTE* codes, DWORD length);
	// 读取四字节内容
	bool ReadDwordMemory(DWORD addr, DWORD& v);
	// 读取内存
	bool ReadMemory(PVOID addr, PVOID save, DWORD length);
	// 读取游戏内存
	bool ReadGameMemory(DWORD flag=0x01);
	// 打印日记
	void InsertLog(char* text);
	// 打印日记
	void InsertLog(wchar_t* text);
	// 获取大区点击坐标
	void GetSerBigClickPos(int& x, int& y);
	// 获取小区点击坐标
	void GetSerSmallClickPos(int& x, int& y);
	// 获取小区中的数字
	bool GetSmallSerNum(char ser[], char num[]);
	// 获取技能快捷键按键点击坐标
	void GetQuickKeyClickPos(int key, int& x, int& y);
public:
	// 验证失败次数
	int m_nVerFail = 0;

	DWORD m_dwHookTid;
	HOOKPROC m_HookKeyProc;
	SetAccountProc_Func m_SetAccountProc;
	// 游戏窗口
	HWND  m_hWnd = NULL;
	// 游戏窗口2
	HWND  m_hWnd2 = NULL;
	// 游戏画面窗口
	HWND  m_hWndPic = NULL;
	// 物品快捷键窗口
	HWND  m_hWndItem= NULL;
	// 技能快捷键窗口
	HWND  m_hWndMagic = NULL;
	// 游戏进程ID
	DWORD m_dwPid = 0;
	// 游戏窗口缩放
	FLOAT m_fScale;
	// 游戏权限句柄
	HANDLE   m_hGameProcess = NULL; 
	// 游戏模块地址
	GameModAddr m_GameModAddr;
	// 游戏地址
	GameAddr m_GameAddr;
	// 游戏CALL
	GameCall m_GameCall;
	// 游戏窗口信息
	GameWnd m_GameWnd;

	DWORD   m_Ecx;

	// 配置文件目录路径
	char   m_chPath[255];
	// 当前角色信息
	Account m_Account;
	// 当前X坐标
	DWORD   m_dwX = 0;
	// 当前Y坐标
	DWORD   m_dwY = 0;
	// 当前Y坐标
	DWORD   m_dwLifeMax = 0;
	// 获取坐标的时间
	int     m_iGetPosTime = 0;
	// 上次坐标不一样时间
	int     m_iFlagPosTime = 0;
	// 是否手动选区
	int     m_iNoAutoSelect = 0;
	// 是否喊话开启
	int     m_iTalkOpen = 0;
	// 是否打开控制台
	int     m_iDebug = 1;
	// 卡住多久重启
	int     m_iQiaZhuS = 15;

	// 怪物数量
	DWORD   m_dwGuaiWuCount;

	// 是否读取完毕
	bool  m_bIsReadEnd;
	// 读取内存块的大小
	DWORD m_dwReadSize;
	// 读取基地址
	DWORD m_dwReadBase;
	// 读取内容临时内存
	BYTE* m_pReadBuffer;

	// Home家
	Home* m_pHome;
	// 连接服务器类
	GameClient* m_pClient;
	// 游戏的配置类
	GameConf* m_pGameConf;
	// 游戏过程处理类
	GameProc* m_pGameProc;
	// 物品类
	Item*     m_pItem;
	// 对话类
	Talk*     m_pTalk;
	// 移动类
	//Move*     m_pMove;
	// 怪物类
	//GuaiWu*   m_pGuaiWu;
	// 技能类
	//Magic*    m_pMagic;
	// 宠物类
	//Pet*      m_pPet;

	// 自身
	static Game* self;

	bool m_bMd = false;

	CHAR m_Test[0x1000];
public:
	// 键盘钩子
	static LRESULT CALLBACK CldKeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
	// 枚举窗口
	static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam);
	// 枚举子窗口
	static BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
	// 按键
	void Keyborad(int key, HWND hwnd=NULL, bool tra=true);
	// 按键
	void Keyborad2(int key, HWND hwnd = NULL);
	// 单击
	void LeftClick(int x, int y, HWND hwnd = NULL);
	// 右击
	void RightClick(int x, int y, HWND hwnd = NULL, int num=1);
	// 鼠标移动
	void MouseMove(int x, int y, HWND hwnd=NULL);
	// 选择对话选项
	void SelectTalkNo(const char* name);
	// 选择对话选项
	void SelectTalkNo(int x, int y);
	// 点击技能栏
	void ClickMagic(char key);

	// 构造栈
	void FakeStackFrame(DWORD stack[], DWORD frames[], DWORD length);
	// 人物复活
	void Call_ReBoren(int flag=0);
	// 人物移动函数
	void Call_Run(int x, int y);
	// 喊话CALL
	void Call_Talk(const char* msg, int type=0);
	// NPC对话
	void Call_NPC(int npc_id);
	// NPC二级对话
	void Call_NPCTalk(int no, bool close=true);
	// 关闭提示框
	void Call_CloseTipBox(int close=1);
	// 获取仓库物品指针
	DWORD Call_GetPackageItemAmount();
	// 获取仓库物品指针
	DWORD Call_GetPackageItemByIndex(int index);
	// 获取物品数量
	DWORD Call_GetItemAmount(int flag=0);
	// 获取物品指针
	DWORD Call_GetItemByIndex(int index, int flag=0);
	// 使用物品
	void Call_UseItem(int item_id);
	// 扔物品
	void Call_DropItem(int item_id);
	// 捡物品
	void Call_PickUpItem(DWORD id, DWORD x, DWORD y);
	// 卖东西
	void Call_SellItem(int item_id);
	// 存钱
	void Call_SaveMoney(int money);
	// 存入远程仓库
	void Call_CheckInItem(int item_id);
	// 取出仓库物品
	void Call_CheckOutItem(int item_id);
	// 使用可传送物品
	void Call_TransmByMemoryStone(int item_id);
	// 放技能
	void Call_Magic(int magic_id, int guaiwu_id);
	// 放技能
	void Call_Magic(int magic_id, int x, int y);
	// 宠物出征
	void Call_PetOut(int pet_id);
	// 宠物召回
	void Call_PetIn(int pet_id);
	// 宠物合体
	void Call_PetFuck(int pet_id);
	// 宠物解体
	void Call_PetUnFuck(int pet_id);
	// 宠物技能 key_no=按键索引 1=0 2=1 ...
	void Call_PetMagic(int key_no);
	// 获取远程邀请人物信息
	DWORD Call_QueryRemoteTeam(int no);
	// 是否有队伍
	int Call_IsHaveTeam();
	// 是否是队长
	int Call_IsTeamLeader();
	// 创建队伍
	void Call_TeamCreate();
	// 离开队伍[队长]
	void Call_TeamDismiss();
	// 离开队伍[队员]
	void Call_TeamLeave();
	// 邀请入队
	void Call_TeamInvite(int player_id);
	// 自动组队
	void Call_TeamAutoJoin(int open=1);
	// 是否选择邀请队伍
	void Call_CheckTeam(int v=1);
	// 获取技能数量
	DWORD Call_GetMagicAmount();
	// 获取技能地址
	DWORD Call_GetMagicByIndex(int index);
	// 获取周围玩家数量
	DWORD Call_GetPlayerAmount();
	// 获取周围玩家地址
	DWORD Call_GetPlayerByIndex(int index);
	// 获取基址[不知道什么可以获取]
	DWORD Call_GetBaseAddr(int index, DWORD _ecx);
};