#include "Pet.h"
#include "Game.h"
#include "Item.h"
#include "Magic.h"
#include <stdio.h>
#include <time.h>

// ...
Pet::Pet(Game* p)
{
	m_pGame = p;
}

// 初始化数据
void Pet::InitData()
{
	ZeroMemory(m_PetRecord, sizeof(m_PetRecord));
}

// 宠物出征
bool Pet::PetOut(DWORD* nums, DWORD length, bool fuck)
{
	DWORD dwCount = ReadPetList();
	for (DWORD i = 0; i < dwCount; i++) {
		for (DWORD j = 0; j < length; j++) {
			if (i == nums[j]) {
				if (j > 0) {
					Sleep(500);
				}
				m_pGame->Call_PetOut(m_GamePet[i].Id);      // 宠物出征
				if (fuck) {
					Sleep(500);
					m_pGame->Call_PetFuck(m_GamePet[i].Id); // 宠物合体
				}
				break;
			}
		}
	}
	return dwCount > 0;
}

// 宠物合体
bool Pet::PetFuck(DWORD* nums, DWORD length)
{
	DWORD dwCount = ReadPetList();
	for (DWORD i = 0; i < dwCount; i++) {
		for (DWORD j = 0; j < length; j++) {
			if (i == nums[j]) {
				if (j > 0) {
					Sleep(300);
				}
				m_pGame->Call_PetFuck(m_GamePet[i].Id); // 宠物合体
				break;
			}
		}
	}
	return dwCount > 0;
}

// 宠物解体
bool Pet::PetUnFuck(DWORD* nums, DWORD length)
{
	DWORD dwCount = ReadPetList();
	for (DWORD i = 0; i < dwCount; i++) {
		for (DWORD j = 0; j < length; j++) {
			if (i == nums[j]) {
				if (j > 0) {
					Sleep(100);
				}
				m_pGame->Call_PetUnFuck(m_GamePet[i].Id);      // 宠物出征
			}
		}
	}
	return dwCount > 0;
}

// 宠物技能
void Pet::Magic(char key)
{
	// 赋值地址EAX=00D475ED EDX=00D4766A ESI=00D47698
	// 调用函数地址00D476F5
	// mov eax,dword ptr ds:[0x10CFDE8]
	// lea ebx,dword ptr ds:[eax+0x2D8] mov edx,ebx mov eax,dword ptr ds:[edx+0x8] 
	// lea edx,dword ptr ds:[eax+edi*4]
	// mov edx,dword ptr ds:[0x1099594] 这个是基数[在赋值EDX处] 数字按键需要+此+1 lea ecx,dword ptr ds:[edx+eax+0x1]
	DWORD v = 9;
	DWORD no =  v + (key - '1') + 1;
	m_pGame->Call_PetMagic(no);
}

// 复活所有没有血量宠物
DWORD Pet::Revive()
{
	DWORD dwCount = ReadPetList(0);
#if 0
	if (dwCount == 1) {
		printf("使用【生命祈祷】%08X\n", m_GamePet[0].Id);
		m_pGame->m_pMagic->UseMagic("生命祈祷", 0, 0, m_GamePet[0].Id); // 单个复活
	}
#endif
	if (dwCount == 0)
		return dwCount;

	if (dwCount >= 1) { // 有药使用药
		printf("需要复活宠物数量:%d\n", dwCount);
		if (m_pGame->m_pItem->GetSelfItemCountByType(速效圣兽灵药)) { // 拥有此药
			m_pGame->m_pItem->UseSelfItemByType(速效圣兽灵药);
		}
		else { // 技能复活全部
			printf("使用【神圣复苏】%08X\n", m_GamePet[0].Id);
			m_pGame->m_pMagic->UseMagic("神圣复苏", 0, 0, m_GamePet[0].Id); // 全部复活
		}
	}

	// 合体
	DWORD no[] = { 0, 1, 2 };
	dwCount = PetFuck(no, sizeof(no)/sizeof(DWORD));

	return dwCount;
}

// 是否需要复活
bool Pet::IsNeedRevive(int index, DWORD life, DWORD life_max)
{
	if (index >= 3)
		return false;

	int now_time = time(nullptr);
	PetRecord* p = &m_PetRecord[index];

	bool result = false;
	if (life == 0) {
		result = true;
		goto end;
	}

	if (p->Time == 0) // 第一次
		goto end;
	if (!life_max && p->Life > 2000) // 找不到最大生命值
		goto end;
	if (life_max == p->Life) // 满血
		goto end;
	if (life == p->Life) { // 跟上次生命一样
		if ((now_time - p->Time) >= 12) { // 超过12秒还一样
			result = true;
			goto end;
		}
		else { // 不更新信息
			goto ret;
		}
	}
end:
	p->Life = life;
	p->Time = now_time;
ret:
	return result;
}

// 读取拥有宠物
DWORD Pet::ReadPetList(DWORD life)
{
	if (!m_pGame->m_GameAddr.PetPtr) {
		printf("找不到宠物地址\n");
		return 0;
	}

	m_dwPetCount = 0;
	DWORD addr = m_pGame->m_GameAddr.PetPtr;
	for (DWORD i = 0; i < 5; i++) {
		// mov edx,dword ptr ds:[edi+ebx*4+0x398]
		// mov esi, dword ptr ds : [edx+0x16C]
		// edi=addr ebx=索引 esi=宠物ID
		// mov ecx, [edi+ebx*4+0x2C0]
		// ecx=血量
		DWORD edx = 0;
		if (!m_pGame->ReadDwordMemory(addr + i * 4 + 0xB18, edx))
			continue;
		DWORD esi = 0;
		if (!m_pGame->ReadDwordMemory(edx + 0x184, esi))
			continue;
		
		DWORD ecx = 0;
		if (!m_pGame->ReadDwordMemory(addr + i * 4 + 0x518, ecx))
			continue;
		
		// 找到最大血量->得到基址->基址下断点可得到偏移
		DWORD life_max = 0;
		if (m_pGame->ReadDwordMemory(addr + i * 4 + 0x698, edx)) { // 这个是血量信息那边的基址728
			m_pGame->ReadDwordMemory(edx + 0xE8, life_max);
		}

		if (!esi) { // 没有ID
			break;
		}
		if (life == 0 && !IsNeedRevive(i, ecx, life_max)) // 获取没有血量宠物
			continue;

		m_GamePet[m_dwPetCount].Id = esi;
		m_GamePet[m_dwPetCount].Life = ecx;
		m_dwPetCount++;

		printf("%d 宠物:%08X %d\n", m_dwPetCount, esi, ecx);
	}
	return m_dwPetCount;
}
