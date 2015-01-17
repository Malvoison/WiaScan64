// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "VirtualChannelImpl.h"

extern HANDLE hMtxImageQueue;
HANDLE hImageXferReadyEvent;
CRITICAL_SECTION csLogFile;
WCHAR moduleName[MAX_PATH];

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hInstance = hModule;

			DisableThreadLibraryCalls(hModule);
			OutputDebugString(L"VCInventoryRDP: DLL_PROCESS_ATTACH\n");

			CoInitialize(NULL);

			//  Create the necessary synchronization
			InitializeCriticalSectionEx(&csLogFile, 4000, 0);
			//  Image Queue mutex
			hMtxImageQueue = CreateMutex(NULL, FALSE, NULL);
			//  Image Transfer Ready Event
			hImageXferReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  GetDefaultSrcEvent

			GetModuleFileName(hModule, moduleName, sizeof(moduleName));
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		OutputDebugString(L"VCInventoryRDP: DLL_PROCESS_DETACH\n");

		CoUninitialize();

		CloseHandle(hMtxImageQueue);
		CloseHandle(hImageXferReadyEvent);

		DeleteCriticalSection(&csLogFile);

		break;
	}
	return TRUE;
}


