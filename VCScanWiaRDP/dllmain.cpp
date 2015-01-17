// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "VirtualChannelImpl.h"

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
			OutputDebugString(L"VCScanWiaRDP: DLL_PROCESS_ATTACH\n");

			CoInitialize(NULL);

			//  Create the necessary synchronization
			InitializeCriticalSectionEx(&csLogFile, 4000, 0);
            //  Image Queue mutex
            hMtxImageQueue = CreateMutex(NULL, FALSE, NULL);
            //  Image Transfer Ready Event
            hImageXferReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  GetDefaultSrcEvent
			hWaits[GETDEFAULTSRCEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  EnumSrcsEvent
			hWaits[ENUMSRCSEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  EnumPixelTypesEvent
			hWaits[ENUMPIXELTYPESEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  EnumBitDepthsEvent
			hWaits[ENUMBITDEPTHSEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  GetResolutionRangeEvent
			hWaits[GETRESOLUTIONRANGEEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  EnumResolutionsEvent
			hWaits[ENUMRESOLUTIONSEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  TransferPicturesEvent
			hWaits[TRANSFERPICTURESEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  TransferReadyEvent
			hWaits[TRANSFERREADYEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  FnCompleteEvent
			hWaits[FNCOMPLETEEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  TransferDirectDoneEvent
			hWaits[TRANSFERDIRECTDONEEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  EnumCapsEvent
			hWaits[ENUMCAPSEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  EnumPageSizesEvent
			hWaits[ENUMPAGESIZESEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
			//  VerCheckEvent
			hWaits[VERCHECKEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		OutputDebugString(L"VCScanWiaRDP: DLL_PROCESS_DETACH\n");
	
		CoUninitialize();

		for (int i = 0; i < NUM_EVENTS; i++)
		{
			CloseHandle(hWaits[i]);
		}

        CloseHandle(hMtxImageQueue);

		DeleteCriticalSection(&csLogFile);

		break;
	}
	return TRUE;
}

