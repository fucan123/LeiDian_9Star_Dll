#pragma once
#include "GameStruct.h"
#include <Windows.h>

#define YAO_BAO_L 16

// 游戏自己拥有物品信息
typedef struct self_item
{
	DWORD Id;       // ID
	DWORD Type;     // 类型
	CHAR  Name[32]; // 名称
	DWORD ToX;      // 可传送的点(X)
	DWORD ToY;      // 可传送的点(Y)
} SelfItem;

class Game;
class Item
{
public:
	// ...
	Item(Game* p);

	// 初始化数据
	void InitData();
	// 获取药数量
	DWORD GetYaoCount(int* p_index=nullptr);
	// 获取包数量
	DWORD GetBaoCount(int* p_index=nullptr);
	// 使用药
	DWORD UseYao();
	// 使用药包
	DWORD UseBao();
	// 读取药包
	int   ReadYaoBao(int flag);
	// 读取自己拥有物品
	DWORD ReadSelfItems(GameSelfItem** save, DWORD save_length);
	// 获取自己物品信息
	bool GetSelfItemInfo(PVOID addr, SelfItem& item);
	// 获取物品ID
	DWORD GetSelfItem(const char* name, DWORD to_x=0, DWORD to_y=0, DWORD allow_to=0);
	// 根据物品类型获取物品ID
	DWORD GetSelfItemIdByType(ITEM_TYPE type);
	// 根据物品名称获取物品数量
	DWORD GetSelfItemCountByName(const char* name);
	// 根据物品类型获取物品数量
	DWORD GetSelfItemCountByType(ITEM_TYPE type);
	// 使用物品
	DWORD UseSelfItem(const char* name, DWORD to_x=0, DWORD to_y=0, DWORD allow_to=0);
	// 使用物品
	DWORD UseSelfItem(DWORD item_id, bool flag=0);
	// 使用物品
	DWORD UseSelfItemByType(ITEM_TYPE type, DWORD use_count=1);
	// 售卖物品
	DWORD SellSelfItem(ConfItemInfo* items, DWORD length);
	// 存入物品
	DWORD CheckInSelfItem(ConfItemInfo* items, DWORD length);
	// 丢弃物品
	DWORD DropSelfItem(DWORD item_id);
	// 丢弃拥有物品
	DWORD DropSelfItemByName(const char* name, DWORD live_count);
	// 丢弃拥有物品
	DWORD DropSelfItemByType(ITEM_TYPE type, DWORD live_count);
	// 获取仓库物品数量
	DWORD GetSaveItemCount(const char* name);
	// 取出物品
	DWORD CheckOutItem(ConfItemInfo* items, DWORD length);
	// 取出物品
	DWORD CheckOutItem(const char* name, DWORD out_count=0);
	// 获取仓库物品数量
	DWORD GetSaveItemAllCount();
	// 读取仓库物品
	DWORD ReadSaveItems(GameSelfItem** save, DWORD save_length, const char* name=nullptr);
	// 是否可以扔物品
	bool IsCanDrop();
	// 等待到可以扔物品
	void WaitCanDrop();
	// 获取地面物品信息
	bool GetGroundItemInfo(PVOID addr, GameGroundItem& item);
	// 读取地面物品
	DWORD ReadGroundItems(GameGroundItem** save, DWORD save_length);
	// 地面是否还有此物品
	bool GroundHasItem(DWORD item_id);
	// 捡物品
	DWORD PickUpItem(ConfItemInfo* items, DWORD length);
	// 是否已捡起来
	bool IsPickUp(DWORD item_id);
public:
	// 游戏类
	Game* m_pGame;

	// 药ID
	DWORD m_Yao[YAO_BAO_L];
	// 药包ID
	DWORD m_Bao[YAO_BAO_L];

	// 临时变量
	DWORD m_dwTmp;

	// 扔物品时间
	__int64 m_i64DropTime;

	CHAR m_Test[0x1000];
};