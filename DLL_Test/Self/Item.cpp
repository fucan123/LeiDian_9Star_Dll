#include "GameClient.h"
#include "Item.h"
#include "Game.h"
#include <stdio.h>
#include <time.h>
#include <My/Common/mystring.h>

// ...
Item::Item(Game* p)
{
	m_pGame = p;
	InitData();
}

// 初始化数据
void Item::InitData()
{
	m_i64DropTime = 0;
	ZeroMemory(m_Yao, sizeof(m_Yao));
	ZeroMemory(m_Bao, sizeof(m_Bao));
}

// 获取药数量
DWORD Item::GetYaoCount(int* p_index)
{
	int count = 0;
	return count;
}

// 获取包数量
DWORD Item::GetBaoCount(int* p_index)
{
	int count = 0;
	for (int i = 0; i < YAO_BAO_L; i++) {
		if (m_Bao[i] != 0) {
			if (p_index)
				*p_index = i;

			count++;
		}
	}
	return count;
}

// 使用药
DWORD Item::UseYao()
{
	DWORD id = 0;

	return id;
}

// 使用药包
DWORD Item::UseBao()
{
	DWORD id = 0;

	return id;
}

// 读取药包
int Item::ReadYaoBao(int flag)
{
	DWORD index = 0;

	return index;
}

// 读取自己拥有物品
DWORD Item::ReadSelfItems(GameSelfItem ** save, DWORD save_length)
{
	// 搜索方法->CE查找物品数量->下访问断点, 获得基址->基址下访问断点得到偏移
	DWORD* pItem = NULL, dwCount = 0;
#if 0
	__asm
	{
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov eax, [eax]
		mov eax, [eax + 0x2234]     // [eax+0x10]为背包物品地址 是一个数组 长度为[eax+0x30]
		mov edx, [eax + 0x10]       // [eax+0x10]物品地址指针
		mov dword ptr[pItem], edx
		mov edx, [eax + 0x30]       // 物品数量
		mov dword ptr[dwCount], edx
	}

	DWORD p = PtrToDword(BASE_DS_OFFSET);
	p = PtrToDword(p);
	p = PtrToDword(p + 0x2234);
	pItem = (DWORD*)PtrToDword(p + 0x10);
	dwCount = PtrToDword(p + 0x30);

	DWORD p;
	if (m_pGame->ReadDwordMemory(g_pObjHero, p)) {
		if (m_pGame->ReadDwordMemory(p + 0x224C, p)) {
			m_pGame->ReadDwordMemory(p + 0x10, (DWORD&)pItem);
			m_pGame->ReadDwordMemory(p + 0x30, (DWORD&)dwCount);
		}
	}

	if (!pItem)
		return 0;

	for (DWORD i = 0; i < dwCount; i++, pItem++) {
		if (i >= save_length)
			break;

		if (!m_pGame->ReadDwordMemory((DWORD)pItem, (DWORD&)save[i])) {
			save[i] = nullptr;
		}
	}
#endif

	
	dwCount = m_pGame->Call_GetItemAmount();
	//printf("pItem:%08X dwCount:%d\n", pItem, dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
		if (i >= save_length)
			break;

		save[i] = (GameSelfItem*)m_pGame->Call_GetItemByIndex(i);
	}

	return dwCount;
}

// 获取自己物品信息
bool Item::GetSelfItemInfo(PVOID addr, SelfItem& item)
{
	ZeroMemory(item.Name, sizeof(item.Name));
	DWORD ptr = (DWORD)addr;
	if (!m_pGame->ReadDwordMemory(ptr + 4, item.Id))
		return false;
	if (!m_pGame->ReadDwordMemory(ptr + 12, item.Type))
		return false;
	if (!m_pGame->ReadMemory((PVOID)(ptr + 16), item.Name, sizeof(item.Name)))
		return false;
	if (!m_pGame->ReadDwordMemory(ptr + 788, item.ToX))
		return false;
	if (!m_pGame->ReadDwordMemory(ptr + 792, item.ToY))
		return false;

	item.Name[sizeof(item.Name) - 1] = 0;
	return true;
}

// 获取物品ID
DWORD Item::GetSelfItem(const char* name, DWORD to_x, DWORD to_y, DWORD allow_to)
{
	DWORD dwId = 0;
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		if (!items[i])
			continue;

		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("无法获取背包物品信息(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}

		if (strstr(info.Name, name)) {
			bool result = true;
			if (to_x && to_y) {
				if (allow_to) { // 允许误差范围
					int cx = (int)info.ToX - (int)to_x;
					int cy = (int)info.ToY - (int)to_y;
					result = abs(cx) <= allow_to && abs(cy) <= allow_to;
				}
				else {
					result = info.ToX == to_x && info.ToY == to_y;
				}
			}
			if (result) {
				dwId = info.Id;
				break;
			}
		}
	}
	return dwId;
}

// 根据物品类型获取物品ID
DWORD Item::GetSelfItemIdByType(ITEM_TYPE type)
{
	// 03C5F19E
	DWORD dwId = 0;
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("无法获取背包物品信息(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}
		if (items[i] && info.Type == type) {
			dwId = info.Id;
			break;
		}
	}
	return dwId;
}

// 根据物品名称获取物品数量
DWORD Item::GetSelfItemCountByName(const char * name)
{
	DWORD dwNum = 0;
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("无法获取背包物品信息(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}
		if (strcmp(info.Name, name) == 0)
			dwNum++;
	}
	return dwNum;
}

// 根据物品类型获取物品数量
DWORD Item::GetSelfItemCountByType(ITEM_TYPE type)
{
	DWORD dwNum = 0;
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("无法获取背包物品信息(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}
		if (items[i] && info.Type == type)
			dwNum++;
	}
	return dwNum;
}

// 使用物品
DWORD Item::UseSelfItem(const char * name, DWORD to_x, DWORD to_y, DWORD allow_to)
{
	return UseSelfItem(GetSelfItem(name, to_x, to_y, allow_to), (to_x!=0 && to_y!=0));
}

// 使用物品
DWORD Item::UseSelfItem(DWORD item_id, bool flag)
{
	if (!item_id)
		return 0;

	if (!flag)
		m_pGame->Call_UseItem(item_id);
	else
		m_pGame->Call_TransmByMemoryStone(item_id);
	return item_id;
}

// 使用物品
DWORD Item::UseSelfItemByType(ITEM_TYPE type, DWORD use_count)
{
	DWORD dwCount = 0;
	for (DWORD i = 0; i < use_count; i++) {
		if (UseSelfItem(GetSelfItemIdByType(type)))
			dwCount++;
		if (i > 0)
			Sleep(100);
	}
	return dwCount;
}

// 售卖物品
DWORD Item::SellSelfItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwSellCount = 0;
	
	return dwSellCount;
}

// 存入物品
DWORD Item::CheckInSelfItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwCheckInCount = 0;
	
	return dwCheckInCount;
}

// 丢弃物品
DWORD Item::DropSelfItem(DWORD item_id)
{
	WaitCanDrop();

	m_i64DropTime = getmillisecond();
	m_pGame->Call_DropItem(item_id);
	return 0;
}

// 丢弃拥有物品
DWORD Item::DropSelfItemByName(const char* name, DWORD live_count)
{
	DWORD dwDropNum = 0; // 丢弃的数量

	return dwDropNum;
}

// 丢弃拥有物品
DWORD Item::DropSelfItemByType(ITEM_TYPE type, DWORD live_count)
{
	DWORD dwDropNum = 0; // 丢弃的数量
	
	return dwDropNum;
}

// 获取仓库物品数量
DWORD Item::GetSaveItemCount(const char* name)
{
	return ReadSaveItems(nullptr, 0, name);
}

// 取出物品
DWORD Item::CheckOutItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwCheckOutCount = 0;
	
	return dwCheckOutCount;
}

// 取出物品
DWORD Item::CheckOutItem(const char* name, DWORD out_count)
{
	DWORD dwCount = 0; // ReadSaveItems(items, sizeof(items), name);

	return dwCount;
}

// 获取仓库物品数量
DWORD Item::GetSaveItemAllCount()
{
#if 1
	m_pGame->Call_GetPackageItemAmount();
#else
	// 搜索方式: 找到仓库数量指针，下段得到基址，一路追溯ecx
	ASM_STORE_ECX();
	__asm
	{
		mov ecx, g_pObjHero
		mov ecx, dword ptr ds : [ecx]
		// 下面在CHero::DelPackageItem里面找(应该是第一个)
		mov ecx, dword ptr ds : [ecx + 0x252C]
		// 下面在CPackage::DelItem里面找(应该是第一个)
		add ecx, 4
		// 下面是下段直接可得到的偏移
		mov eax, [ecx + 0x2C]
	}
	ASM_RESTORE_ECX();

	DWORD p = PtrToDword(g_pObjHero);
	p = PtrToDword(p + 0x2544);
	return PtrToDword(p + 4 + 0x2C);
#endif
}

// 读取仓库物品
DWORD Item::ReadSaveItems(GameSelfItem** save, DWORD save_length, const char* name)
{
	if (!m_pGame->m_GameCall.GetPkageItemByIndex)
		return 0;

	DWORD dwIndex = 0;
	// 搜索方式: 取物品函数下段, 一路追溯可找到 call dword ptr ds:[CHero::GetPackageItemByIndex]
	DWORD dwCount = GetSaveItemAllCount();
	//DWORD* f = (DWORD*)this->ReadSaveItems;
	printf("仓库物品数量:%d %08X\n", dwCount, &Item::ReadSaveItems);
	if (dwCount > 180)
		return 0;

	for (DWORD i = 0; i < dwCount; i++) {
		if (save_length > 0 && i >= save_length)
			break;

		GameSelfItem* p = (GameSelfItem*)m_pGame->Call_GetPackageItemByIndex(i);
		bool is_save = name ? strstr(p->Name, name) != nullptr : true;
		if (save && is_save)
			save[dwIndex] = p;
		if (is_save)
			dwIndex++;

		//printf("仓库物品[%08X]:%s[%08X]\n", p, p->Name, p->Id);
		//printf("仓库物品[%08X]:%s[%08X]\n", p, p->Name, p->Id);
	}
	return dwIndex;
}

// 是否可以扔物品
bool Item::IsCanDrop()
{
	if (!m_i64DropTime)
		return true;

	__int64 ms = getmillisecond();
	return ms >= (m_i64DropTime + 800);
}

// 等待到可以扔物品
void Item::WaitCanDrop()
{
	while (!IsCanDrop()) {
		Sleep(100);
	}
}

// 获取地面物品信息
bool Item::GetGroundItemInfo(PVOID addr, GameGroundItem& item)
{
	return m_pGame->ReadMemory(addr, &item, sizeof(GameGroundItem));
}


// 读取地面物品
DWORD Item::ReadGroundItems(GameGroundItem** save, DWORD save_length)
{
	if (!m_pGame->m_GameAddr.ItemPtr) {
		printf("没有找到地面物品指针地址\n");
		return 0;
	}

	DWORD  dwCount = 0;

	DWORD* pItemsBegin = nullptr; // 地面物品列表指针首地址
	DWORD* pItemsEnd   = nullptr; // 地面物品列表指针末地址
	if (!m_pGame->ReadDwordMemory(m_pGame->m_GameAddr.ItemPtr, (DWORD&)pItemsBegin)) {
		printf("无法读取地面物品首地址(%d) %08X\n", GetLastError(), m_pGame->m_GameAddr.ItemPtr);
		return 0;
	}
	if (!m_pGame->ReadDwordMemory(m_pGame->m_GameAddr.ItemPtr + 4, (DWORD&)pItemsEnd)) {
		printf("无法读取地面物品尾地址(%d) %08X\n", GetLastError(), m_pGame->m_GameAddr.ItemPtr);
		return 0;
	}
	if (!pItemsBegin || !pItemsEnd) {
		printf("地面物品列表地址为nullptr\n");
		return 0;
	}

	dwCount = pItemsEnd - pItemsBegin;
	//printf("[%d]地面物品数量：%d\n", (int)time(nullptr), dwCount);
	if (dwCount == 0 || dwCount > 128)
		return 0;

	dwCount = 0;
	//printf("\n---------------------------\n");
	for (DWORD* p = pItemsBegin; p < pItemsEnd; p++) {
		GameGroundItem* pItem = nullptr;
		if (!m_pGame->ReadDwordMemory((DWORD)p, (DWORD&)pItem)) {
			printf("无法获取地面物品地址\n");
			continue;
		}
		if (!pItem) {
			printf("地面物品地址为nullptr\n");
			continue;
		}

		GameGroundItem item;
		if (!GetGroundItemInfo(pItem, item)) {
			printf("无法获取地面物品信息\n");
			continue;
		}
		if (item.Id == 0x00 || item.Id == 0xFFFFFFFF || item.X == 0x00 || item.Y == 0x00)
			continue;

		save[dwCount] = pItem;
		if (++dwCount == save_length)
			break;

		char* name = item.Name + 0x54;
		//printf("[%08X]物品:%s ID:%08X 物品类型:%08X X:%d Y:%d\n", pItem, name, item.Id, item.Type, item.X, item.Y);

		//Call_PickUpItem(pItem);
	}
	//printf("\n---------------------------\n");
	
	return dwCount;
}

// 地面是否还有此物品
bool Item::GroundHasItem(DWORD item_id)
{
	GameGroundItem* pItems[32];
	DWORD dwCount = ReadGroundItems(pItems, sizeof(pItems) / sizeof(GameGroundItem*));
	for (DWORD i = 0; i < dwCount; i++) {
		if (pItems[i]->Id == item_id)
			return true;
	}
	return false;
}

// 捡物品
DWORD Item::PickUpItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwPickUpCount = 0; // 捡了多少物品
	GameGroundItem* pItems[32];
	DWORD dwCount = ReadGroundItems(pItems, sizeof(pItems) / sizeof(GameGroundItem*));

	printf("周围物品数量:%d\n", dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
		GameGroundItem item;
		if (!GetGroundItemInfo(pItems[i], item)) {
			printf("无法获取地面物品信息\n");
			continue;
		}
		if (!item.Id || item.Id == 0xFFFFFFFF)
			continue;

		for (DWORD j = 0; j < length; j++) {
			bool pick_up = false;
			char* name = item.Name + 0x54; // 需要加上此偏移
			if (item.Name) { // 能读取物品名称
				char item_name[32] = { 0 };
				if (m_pGame->ReadMemory(name, item_name, sizeof(item_name))) {
					pick_up = strstr(item_name, items[j].Name) != nullptr;
				}
			}
			else {
				pick_up = items[j].Type == item.Type;
			}
				
			if (pick_up) {     // 是要捡起来的物品
				printf("捡物:%s\n", items[j].Name);
				m_pGame->Call_PickUpItem(item.Id, item.X, item.Y);  // 捡起来
				DWORD x = 0, y = 0;
				while (!IsPickUp(item.Id)) { // 等待物品捡起来
					if (++x == 15) {
						//m_pGame->Call_PickUpItem(pItems[i]);  // 捡起来
						//x = 0;
						break;
					}
					if (++y == 50) {
						break;
					}
					Sleep(500);
				}
				dwPickUpCount++;

				m_pGame->m_pClient->SendPickUpItem(items[j].Name);
				break;
			}
		}
	}

	return dwPickUpCount;
}

// 是否已捡起来
bool Item::IsPickUp(DWORD item_id)
{
	GameGroundItem* pItems[32];
	DWORD dwCount = ReadGroundItems(pItems, sizeof(pItems) / sizeof(GameGroundItem*));
	for (DWORD i = 0; i < dwCount; i++) {
		GameGroundItem item;
		if (!GetGroundItemInfo(pItems[i], item)) {
			printf("无法获取地面物品信息\n");
			continue;
		}
		if (item_id == item.Id)
			return false;
	}
	return true;
}
