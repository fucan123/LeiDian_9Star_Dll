#pragma once
#include <tlhelp32.h>
#include <psapi.h>
#include <My/Win32/Peb.h>
#include <My/Win32/Hook.h>

#define MOD_3drole         L"3drole.dll"
#define MOD_sound          L"sound.dll"
#define ADDR_COOR_X_OFFSET  0x11E7F24   // X坐标地址在模块里面的偏移[MOD_3drole]
#define ADDR_COOR_Y_OFFSET  0x11DE150    // Y坐标地址在模块里面的偏移[MOD_3drole]
#define ADDR_MOV_STA_OFFSET 0x4A7A60      // 人物移动状态在模块里面偏移[MOD_sound]
#define ADDR_MOVE_FUNC      0x5F0208      // 人物移动函数在模块里面偏移
#define ADDR_MOVE_P         0x712E48      // mov [eax+000000B4],ecx
#define ADDR_MOVE_PP        0x712EE8      // mov [eax+000000B4],ecx
#define ADDR_SET_POS        0x7724E8      // ret 08


// 改写x坐标函数入口位置[相对于3drole.dll偏移]
#define CGamePlayerSet_Process_In  0x603F5C  0x72D4D0
// 改写x坐标函数位置[相对于3drole.dll偏移]
#define CGamePlayerSet_Process     0x606A0B
// 改写x坐标[MAP]函数位置[相对于Process_In偏移]
#define CGamePlayerSet_Process2    0x28F2
// 改写x坐标[MAP]函数位置[相对于3drole.dll偏移]
#define CGamePlayerSet_Process3    0x72D789 05AB065C
// 620687
// 789435
// 789435-22F 3drole.Run
// AB9F
// 621379 CHero.Run

// 76643C Command eax==059305A8
// 62BE70 Command CHero
// 72D4D0 Process

// mov ecx,dword ptr ds:[0xF03500]
// mov ecx,dword ptr ds:[ecx]
/*
mov ecx, 058B05A8
mov ecx,dword ptr ds:[0xF03500]
mov ecx,dword ptr ds:[ecx]
pushf
mov eax, [esp]
and [esp],0FFFFFF6E
mov [esp], eax
popf
push 0
push 320
push 320
call 029C1379
点地图 CAS位置0
*/

#define MODE_3dgamemap      L"3dgamemap.dll"
#define ADDR_MOVE_FUNC2     0x342E2

#define IAIMANAGER_ADDR     0x00CA9690
#define IAIMANAGER_ADDR2    0x00CF3870
#define IAIMANAGER_ADDR3    0x00CB2B99

#define NOW_X               300

#define NTQIP_OFFSET 0x05

typedef DWORD (WINAPI *PlayerSetMove)(DWORD x, DWORD y);

typedef struct
{
	LONG   ExitStatus;
	UINT32 PebBaseAddress;
	UINT32 AffinityMask;
	UINT32 BasePriority;
	UINT32 UniqueProcessId;
	UINT32 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;


DWORD WINAPI Run(LPVOID);
DWORD WINAPI ForeachModules();
DWORD WINAPI ConnectServer(LPVOID);
void  CALLBACK SetAccountProc(int big);
DWORD WINAPI ReLogin(LPVOID);
int GetProcessThreadList(); //进程的ID
LRESULT CALLBACK CldKeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
DWORD WINAPI KeyBoardHook(LPVOID);
LRESULT CALLBACK KeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
LONG WINAPI VectorHandler(struct _EXCEPTION_POINTERS* ExceptionInfo);
VOID SetVectorBreak(PVOID address, bool set=true);
DWORD WINAPI MyMove(DWORD x, DWORD y);
DWORD WINAPI OldMove(DWORD x, DWORD y);
BOOL CheckDllInProcess(DWORD dwPID, LPCTSTR szDllPath, HMODULE* hMod);

NTSTATUS WINAPI MyNtQueryInformationProcess
(HANDLE ProcessHandle, ULONG ProcessInformationClass,
	PVOID ProcessInformation, UINT32 ProcessInformationLength,
	UINT32* ReturnLength);
