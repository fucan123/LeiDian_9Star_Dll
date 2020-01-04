#pragma once
#include "../stdafx.h"

//  最大支持多少个函数HOOK
#define MAX_HOOK_FUNC_NUM 16

typedef struct _hook_data {
	LPVOID address;
	BYTE   oldCode[12];
	SIZE_T size; // 改变的字节大小
	BYTE   flag; // 0-没有HOOK 1-已HOOK
} HK_DATA;

class Hook {
private :
	HWND m_hWnd;
	BYTE m_hkLen;
	HK_DATA m_hkData[MAX_HOOK_FUNC_NUM];
private:
	Hook();
public:
	static Hook* self;
public:
	// 设置hwnd
	void SetHwnd(HWND hwnd);
	// 附加地址为address的函数 跳转到func
	void Attach(LPVOID address, LPVOID func);
	// 取消地址为address的附加
	void Detach(LPVOID address);
	// 自身实例对象
	static Hook* GetInstance();
	// 删除自身实例对象
	static void  DeleteInstance();
};