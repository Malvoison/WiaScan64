//  Filename: TwainClient.cpp

#include "stdafx.h"

#include "TwainWrapper.h"
#include "TwainThread.h"
#include "TwainClient.h"
#include "Utility.h"

static DWORD dwTwainThreadId = 0;
static HANDLE hTwainThreadHandle = NULL;

extern "C" void __declspec(dllexport) SetShowUI(void)
{
	DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);
	nShowUI = 1;
}

extern "C" void __declspec(dllexport) CloseSrc(void)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		DWORD dwResult = PostThreadMessage(dwTwainThreadId, PM_CLOSESRC, (WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);

		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" void __declspec(dllexport) OpenSrc(void)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		DWORD dwResult = PostThreadMessage(dwTwainThreadId, PM_OPENSRC, (WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);

		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" void __declspec(dllexport) Finish(void)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		DWORD dwResult = PostThreadMessage(dwTwainThreadId, PM_FINISH, (WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);

		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" void __declspec(dllexport) InitTwain(void)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		hXferReady = CreateEvent(NULL, FALSE, FALSE, "TransferReady");
		hTwainThreadReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		hTwainThreadExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		hMtxGlobals = CreateMutex(NULL, FALSE, NULL);

		hTwainThreadHandle = CreateThread(NULL, 0, TwainThreadProc, NULL, 0, 
			&dwTwainThreadId);
		DWORD dwResult = WaitForSingleObject(hTwainThreadReadyEvent, 5000);
		if (dwResult != WAIT_OBJECT_0)
		{
			SystemError();
			return;
		}

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_INIT, (WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);

		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" void __declspec(dllexport) ReleaseTwain(void)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_RELEASE, (WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);

		CloseHandle(hFnComplete);

		dwResult = PostThreadMessage(dwTwainThreadId, WM_QUIT, 0, 0);

		WaitForSingleObject(hTwainThreadExitEvent, 30000);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" LPCSTR __declspec(dllexport) GetDefaultSrc()
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_GETDEFAULTSOURCE, 
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);

		return szDefaultSrc;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}

	return NULL;
}

extern "C" void __declspec(dllexport) SelectSrc(void)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_SELECTSRC, (WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);

		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" LPSRCLIST __declspec(dllexport) EnumSrcs()
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_ENUMSRCS, (WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);

		return lpSrcList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}

	return NULL;
}

extern "C" void __declspec(dllexport) SetSelectedSource(LPCSTR src)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		dwResult = WaitForSingleObject(hMtxGlobals, INFINITE);
		lstrcpyn(szSelectedSrc, src, sizeof(TW_STR32));
		ReleaseMutex(hMtxGlobals);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_SETSELECTEDSOURCE,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" void __declspec(dllexport) SetPixelType(int pixeltype)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		dwResult = WaitForSingleObject(hMtxGlobals, INFINITE);
		nPixelType = pixeltype;
		ReleaseMutex(hMtxGlobals);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_SETPIXELTYPE,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" void __declspec(dllexport) SetPageSize(int pagesize)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		dwResult = WaitForSingleObject(hMtxGlobals, INFINITE);
		nPageSize = pagesize;
		ReleaseMutex(hMtxGlobals);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_SETPAGESIZE,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)\n", __FUNCTION__, __FILE__);
	}
}

extern "C" void __declspec(dllexport) SetFeederEnabled(BOOL feederenabled)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		dwResult = WaitForSingleObject(hMtxGlobals, INFINITE);
		bFeederEnabled = feederenabled;
		ReleaseMutex(hMtxGlobals);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_SETFEEDERENABLED,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)\n", __FUNCTION__, __FILE__);
	}
}

extern "C" void __declspec(dllexport) SetDuplex(BOOL duplex)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		dwResult = WaitForSingleObject(hMtxGlobals, INFINITE);
		bDuplex = duplex;
		ReleaseMutex(hMtxGlobals);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_SETDUPLEX,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)\n", __FUNCTION__, __FILE__);
	}
}

extern "C" LPPIXELTYPELIST __declspec(dllexport) EnumPixelTypes(void)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_ENUMPIXELTYPES,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);

		return lpPixelTypeList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}

	return NULL;
}

extern "C" void __declspec(dllexport) SetBitDepth(int bitdepth)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		dwResult = WaitForSingleObject(hMtxGlobals, INFINITE);
		nBitDepth = bitdepth;
		ReleaseMutex(hMtxGlobals);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_SETBITDEPTH,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" LPBITDEPTHLIST __declspec(dllexport) EnumBitDepths(int pixeltype)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		dwResult = WaitForSingleObject(hMtxGlobals, INFINITE);
		nPixelType = pixeltype;
		ReleaseMutex(hMtxGlobals);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_ENUMBITDEPTHS,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);

		return lpBitDepthList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}

	return NULL;
}

extern "C" void __declspec(dllexport) SetResolution(float res)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		dwResult = WaitForSingleObject(hMtxGlobals, INFINITE);
		fResolution = res;
		ReleaseMutex(hMtxGlobals);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_SETRESOLUTION,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" LPRESOLUTIONRANGE __declspec(dllexport) GetResolutionRange()
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_GETRESOLUTIONRANGE,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);

		return lpResolutionRange;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}

	return NULL;
}

extern "C" LPRESOLUTIONLIST __declspec(dllexport) EnumResolutions()
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_ENUMRESOLUTIONS,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);

		return lpResolutionList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}

	return NULL;
}

extern "C" LPCAPLIST __declspec(dllexport) EnumCapabilities()
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_ENUMCAPS,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);

		return lpCapList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
	return NULL;
}

extern "C" LPPAGESIZELIST __declspec(dllexport) EnumPageSizes()
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_ENUMPAGESIZES,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);

		return lpPageSizeList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
	return NULL;
}

extern "C" void __declspec(dllexport) Acquire(void)
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		DWORD dwResult = PostThreadMessage(dwTwainThreadId, PM_ACQUIRE, 
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}
}

extern "C" LPIMGLIST __declspec(dllexport) TransferPictures()
{
	try
	{
		DebugTraceMessage("%s  %s\n", __FUNCTION__, __FILE__);

		DWORD dwResult;
		HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
		dwResult = PostThreadMessage(dwTwainThreadId, PM_TRANSFERPICTURES,
			(WPARAM)hFnComplete, 0);
		dwResult = WaitForSingleObject(hFnComplete, INFINITE);
		CloseHandle(hFnComplete);

		//HANDLE hEvent = OpenEvent(0x001F0000, 0, "TransferReady");
		//ResetEvent(hEvent);

		return lpImageList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in %s (%s)", __FUNCTION__, __FILE__);
	}

	return NULL;
}
