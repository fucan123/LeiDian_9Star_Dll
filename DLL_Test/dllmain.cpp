// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "DLL_Test.h"
#include <stdio.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
	{
		DWORD tid = GetCurrentThreadId();
		HANDLE hTread = NULL;
#if 1
		hTread = CreateThread(NULL, NULL, Run, (LPVOID)tid, NULL, NULL);
		if (hTread) {
			CloseHandle(hTread);
		}
#else
		Run((LPVOID)tid);
#endif
#if 0
		hTread = CreateThread(NULL, NULL, KeyBoardHook, NULL, NULL, NULL);
		if (hTread) {
			CloseHandle(hTread);
		}
#endif
		break;
	}
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		printf("卸载DLL!\n");
        break;
    }
    return TRUE;
}

