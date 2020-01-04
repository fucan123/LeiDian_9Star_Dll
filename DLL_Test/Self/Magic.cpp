#include "Magic.h"
#include "Game.h"
#include <My/Common/mystring.h>
#include <My/Driver/KbdMou.h>
#include <stdio.h>

// ...
Magic::Magic(Game* p)
{
	m_pGame = p;
	m_dwAttackGWId = 0;

	m_dwMagicNum = 0;
	m_dwMagicInfoCount = 0;

	m_dwMagicX = 0;
	m_dwMagicY = 0;
}

// 使用技能
bool Magic::UseMagic(const char* name, DWORD x, DWORD y, DWORD gwid)
{
	MagicInfo* pInfo = GetMagicInfo(name);
	if (!pInfo) {
		_Magic_ magic;
		if (!ReadMagic(name, &magic)) {
			printf("找不到技能:%s\n", name);
			return false;
		}

		printf("添加技能:%s 冷却时间:%d秒\n", name, magic.Cd);
		pInfo = AddMagicInfo(magic.Name, magic.Id, magic.Cd);
	}
	
	if (!CheckCd(pInfo)) {
		printf("技能未冷却:%s\n", name);
		//return false;
	}
		
	pInfo->UseTime = getmillisecond();          // 使用时间
	pInfo->CdTime = pInfo->UseTime + pInfo->Cd; // 冷却时间
	//printf("UseMagic:%d %d,%d %d\n", type, v, v2, p->CallType);
	//return false;
	m_dwMagicX = 0;
	m_dwMagicY = 0;
	if (MagicIsPos(name)) {
#if 0
		if (strstr(name, "虚无空间")) {
			m_dwMagicX = x;
			m_dwMagicY = y;
			m_pGame->Keyborad(VK_F3, NULL, false);
			m_dwMagicX = 0;
			m_dwMagicY = 0;
			return true;
		}

		if (strstr(name, "星陨")) {
			m_pGame->Keyborad(VK_F1, NULL, false);
			return true;
		}
		if (strstr(name, "影魂契约")) {
#if 1
			m_pGame->Keyborad(VK_F4, NULL, false);
#else
			int click_x, click_y;
			m_pGame->GetQuickKeyClickPos(VK_F4, click_x, click_y);
			m_pGame->RightClick(click_x, click_y, m_pGame->m_hWndMagic, 2);
#endif
			return true;
		}
#endif
		if (strstr(name, "最终审判")) { // 此技能鼠标停哪打哪
			SetForegroundWindow(m_pGame->m_hWnd);
			int sx = m_pGame->m_GameWnd.Width / 2, sy = m_pGame->m_GameWnd.Height / 2;
			m_pGame->GetGameCenterPos(sx, sy);
			sy -= 20;
			printf("鼠标移动:%d,%d\n", sx, sy);
			m_pGame->MouseMove(m_pGame->m_GameWnd.Width / 2, m_pGame->m_GameWnd.Height / 2, m_pGame->m_hWnd2);
			//mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE, sx, sy, 0, 0);//点击
			
		}
		m_pGame->Call_Magic(pInfo->Id, x, y);
		return true;
	}
	else {
		m_pGame->Call_Magic(pInfo->Id, gwid);
		return true;
	}
	return false;
}

// 检查冷却时间[allow_ms=允许最大还剩此冷却]
bool Magic::CheckCd(const char* name, DWORD allow_ms, DWORD* magic_id)
{
	MagicInfo* pInfo = GetMagicInfo(name);
	return CheckCd(pInfo, allow_ms);
}

// 检查冷却时间[allow_ms=允许最大还剩此冷却]
bool Magic::CheckCd(DWORD id, DWORD allow_ms)
{
	MagicInfo* p = GetMagicInfo(id);
	if (p == nullptr)
		return true;
	if (!p->CdTime)
		return true;

	__int64 t = p->CdTime - getmillisecond();
	return t <= (__int64)allow_ms;
}

// 检查冷却时间[allow_ms=允许最大还剩此冷却]
bool Magic::CheckCd(MagicInfo* p, DWORD allow_ms)
{
	if (p == nullptr)
		return true;
	if (!p->CdTime)
		return true;

	__int64 t = p->CdTime - getmillisecond();
	return t <= (__int64)allow_ms;
}

// 获取技能信息
MagicInfo* Magic::GetMagicInfo(const char* name)
{
	for (DWORD i = 0; i < m_dwMagicInfoCount; i++) {
		if (strstr(m_ListMagicInfo[i].Name, name) != nullptr)
			return &m_ListMagicInfo[i];
	}
	return nullptr;
}

// 获取技能信息
MagicInfo* Magic::GetMagicInfo(DWORD id)
{
	for (DWORD i = 0; i < m_dwMagicInfoCount; i++) {
		if (m_ListMagicInfo[i].Id == id)
			return &m_ListMagicInfo[i];
	}
	return nullptr;
}

// 技能是否为坐标技能
bool Magic::MagicIsPos(const char* name)
{
	return strstr(name, "生命祈祷") == nullptr
		&& strstr(name, "神圣复苏") == nullptr;
}

// 读取技能数量
int Magic::ReadMagicLength(bool flag)
{
#if 1
	return m_pGame->Call_GetMagicAmount();
#else
	__asm
	{
		mov ecx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [ecx]
		add ecx, 0x28FC
		mov eax, dword ptr ds : [ecx + 0x8]
		sub eax, dword ptr ds : [ecx + 0x4]
		sar eax, 0x2
	}

	DWORD p = 0, n = 0, n2 = 0;
	if (m_pGame->ReadDwordMemory(g_pObjHero, p)) {
		p += 0x2914;
		if (m_pGame->ReadDwordMemory(p + 0x08, n)
			&& m_pGame->ReadDwordMemory(p + 0x04, n2)) {
			n = (n - n2) >> 2;
		}
	}
	return n;
#endif
}

// 读取技能
bool Magic::ReadMagic(const char* name, _Magic_* save, bool flag)
{
	// 4F60 51E6
	// CHero::GetMagic里面寻找
	int length = ReadMagicLength(flag);
	if (name == nullptr) {
		printf("技能数量:%d[%08X]\n", length, length);
	}
	for (int i = 0; i < length; i++) {
#if 1
		DWORD base = m_pGame->Call_GetMagicByIndex(i);
		DWORD p = 0;
		if (!m_pGame->ReadDwordMemory(base + 0x28, p))
			continue;
#else
		GameMagic* pMagic;
		__asm
		{
			mov ecx, dword ptr ds : [BASE_DS_OFFSET]
			mov ecx, dword ptr ds : [ecx]
			add ecx, 0x28FC
			mov eax, dword ptr ds : [ecx + 0x4]
			mov ecx, dword ptr[i]
			lea eax, dword ptr ds : [eax + ecx * 4]
			mov eax, dword ptr ds : [eax]
			mov eax, dword ptr ds : [eax + 0x28]
			mov dword ptr[p], eax
			//mov eax, dword ptr ds : [eax + 0x4]       // 这个就是放技能函数要传的参数
			//mov dword ptr [magic], eax
		}

		DWORD p = 0, n = 0;
		if (m_pGame->ReadDwordMemory(g_pObjHero, p)) {
			p += 0x2914;
			if (m_pGame->ReadDwordMemory(p + 0x04, p)) {
				if (m_pGame->ReadDwordMemory(p + (i * 4), p)) {
					m_pGame->ReadDwordMemory(p + 0x28, p);
				}
			}
			
		}
		if (!p)
			break;
#endif
		if (save) {
			bool is_ok = false;
			char* p_name;
			if (m_pGame->ReadDwordMemory(p + 0x0C, (DWORD&)p_name)) {
				if (m_pGame->ReadMemory(p_name, save->Name, sizeof(save->Name))) {
					save->Name[31] = 0;
					if (strstr(save->Name, name)) {
						is_ok = true;
					}
				}
			}
			if (is_ok) {
				if (!m_pGame->ReadDwordMemory(p + 0x04, save->Id))
					return false;
				if (!m_pGame->ReadDwordMemory(p + 0x5C, save->Cd))
					return false;

				return true;
			}
		}
		else {
			GameMagic* pMagic  = (GameMagic*)p;
			printf("%08X.%s:%08X 冷却时间:%d\n", pMagic, pMagic->Name, pMagic->Id, pMagic->Cd);
			AddMagicInfo(pMagic->Name, pMagic->Id, pMagic->Cd);
		}
	}
	return false;
}

// 添加技能信息
MagicInfo* Magic::AddMagicInfo(CHAR* name, DWORD id, DWORD cd)
{
	MagicInfo* p = &m_ListMagicInfo[m_dwMagicInfoCount];
	strcpy(p->Name, name);
	p->Id = id;
	p->Cd = (cd + 1) * 1000;
	p->UseTime = 0;
	p->CdTime = 0;
	m_dwMagicInfoCount++;

	return p;
}
