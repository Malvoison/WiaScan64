
#include "stdafx.h"
#include "VirtualChannelImpl.h"
#include "WiaImpl.h"

//  Globals
HANDLE hWaits[NUM_EVENTS];
HANDLE hMtxImageQueue;
std::queue<LPIMGLIST> imageQueue;
LPIMGLIST lpCurrentImage = NULL;
LPBYTE lpImageData = NULL;
int nBytesRemaining = 0;
BOOL bTransferComplete = FALSE;
#define MAX_IMAGE_CHUNK 1592


static DWORD dwWiaThreadId = 0;
static HANDLE hWiaThreadHandle = NULL;

LPWSTR TranslateCommand(int nCmd)
{
	LPWSTR szCommand = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 512);
	
	switch (nCmd)
	{
	case PM_CLOSESRC:
		lstrcpy(szCommand, L"PM_CLOSESRC");
		break;
	case PM_OPENSRC:
		lstrcpy(szCommand, L"PM_OPENSRC");
		break;
	case PM_FINISH:
		lstrcpy(szCommand, L"PM_FINISH");
		break;
	case PM_INIT:
		lstrcpy(szCommand, L"PM_INIT");
		break;
	case PM_GETDEFAULTSOURCE:
		lstrcpy(szCommand, L"PM_GETDEFAULTSOURCE");
		break;
	case PM_SELECTSRC:
		lstrcpy(szCommand, L"PM_SELECTSRC");
		break;
	case PM_ENUMSRCS:
		lstrcpy(szCommand, L"PM_ENUMSRCS");
		break;
	case PM_SETSELECTEDSOURCE:
		lstrcpy(szCommand, L"PM_SETSELECTEDSOURCE");
		break;
	case PM_SETPIXELTYPE:
		lstrcpy(szCommand, L"PM_SETPIXELTYPE");
		break;
	case PM_ENUMPIXELTYPES:
		lstrcpy(szCommand, L"PM_ENUMPIXELTYPES");
		break;
	case PM_SETBITDEPTH:
		lstrcpy(szCommand, L"PM_SETBITDEPTH");
		break;
	case PM_ENUMBITDEPTHS:
		lstrcpy(szCommand, L"PM_ENUMBITDEPTHS");
		break;
	case PM_SETRESOLUTION:
		lstrcpy(szCommand, L"PM_SETRESOLUTION");
		break;
	case PM_GETRESOLUTIONRANGE:
		lstrcpy(szCommand, L"PM_GETRESOLUTIONRANGE");
		break;
	case PM_ENUMRESOLUTIONS:
		lstrcpy(szCommand, L"PM_ENUMRESOLUTIONS");
		break;
	case PM_ACQUIRE:
		lstrcpy(szCommand, L"PM_ACQUIRE");
		break;
	case PM_TRANSFERPICTURES:
		lstrcpy(szCommand, L"PM_TRANSFERPICTURES");
		break;
	case PM_RELEASE:	
		lstrcpy(szCommand, L"PM_RELEASE");
		break;
	case PM_PING:
		lstrcpy(szCommand, L"PM_PING");
		break;
	case PM_PICSTART:
		lstrcpy(szCommand, L"PM_PICSTART");
		break;
	case PM_PICDATA:
		lstrcpy(szCommand, L"PM_PICDATA");
		break;
	case PM_PICDONE:
		lstrcpy(szCommand, L"PM_PICDONE");
		break;
	case PM_TRANSFERDONE:
		lstrcpy(szCommand, L"PM_TRANSFERDONE");
		break;
	case PM_TRANSFERREADY:
		lstrcpy(szCommand, L"PM_TRANSFERREADY");
		break;
	default:
		wsprintf(szCommand, L"UNKNOWN COMMAND: 0x%x", (nCmd - WM_APP));
		break;
	}

	return szCommand;	
}

LPBYTE Poll()
{
	DWORD dwRetVal = WaitForMultipleObjects(NUM_EVENTS, hWaits, FALSE, INFINITE);
	
	if (dwRetVal == WAIT_FAILED)
	{
		ShowError();
		return NULL;
	}

	if (dwRetVal == WAIT_TIMEOUT)
		return NULL;

	switch (dwRetVal - WAIT_OBJECT_0)
	{
	case VERCHECKEVENT:
		{
			LPBYTE lpRet = (LPBYTE) GlobalAlloc(GPTR, (sizeof(int) * 2) + sizeof(int));
			int nCmd = PM_VERCHECK;
			int nSize = sizeof(int);
			int nVersion = SCAN_VERSION;
			memcpy(lpRet, &nCmd, sizeof(int));
			memcpy(lpRet + sizeof(int), &nSize, sizeof(int));
			memcpy(lpRet + (2 * sizeof(int)), &nVersion, sizeof(int));
			ResetEvent(hWaits[VERCHECKEVENT]);
			return lpRet;
			break;
		}

	default:
		break;
	}

	return NULL;
}

void DataArrival(LPBYTE pBuf, USHORT usLength)
{
	LPTHREAD_START_ROUTINE fnWia = NULL;
	LPVOID lpvParam = NULL;

	int* pCmd = (int*) pBuf;
	switch (*pCmd)
	{
	case PM_PING:
		OutputDebugString(L"PM_PING\n");
		break;

	case PM_VERCHECK:
		SetEvent(hWaits[VERCHECKEVENT]);
		break;

	default:
		break;
	}

	if (fnWia != NULL)
	{
		hWiaThreadHandle = CreateThread(NULL, 0, fnWia, lpvParam, 0, 
			&dwWiaThreadId);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  SERVER COMMAND Channel
///////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI  InitWiaProc(LPVOID)
{
	InitWia();
	return 0;
}

DWORD WINAPI ReleaseWiaProc(LPVOID)
{
	ReleaseWia();
	return 0;
}

DWORD WINAPI AcquireProc(LPVOID)
{
	AcquireWia();
	return 0;
}

DWORD WINAPI SetPreferencesProc(LPVOID)
{
	SetWiaPreferences();
	return 0;
}

void DataArrivalServerCommand(LPBYTE pBuf, USHORT usLength)
{
	LPTHREAD_START_ROUTINE fnWia = NULL;
	LPVOID lpvParam = NULL;

	int* pCmd = (int*) pBuf;
	switch (*pCmd)
	{
	case PM_PING:
		DebugTraceMessage(L"PM_PING\n");
		break;

	case PM_INIT:
		DebugTraceMessage(L"DataArrivalServerCommand::PM_INIT\n");
		fnWia = InitWiaProc;
		break;

	case PM_RELEASE:
		DebugTraceMessage(L"DataArrivalServerCommand::PM_RELEASE\n");
		fnWia = ReleaseWiaProc;
		break;

	case PM_ACQUIRE:
		DebugTraceMessage(L"DataArrivalServerCommand::PM_ACQUIRE\n");
		fnWia = AcquireProc;
		break;

	case PM_SETPREFERENCES:
		DebugTraceMessage(L"DataArrivalServerCommand::PM_SETPREFERENCES\n");
		fnWia = SetPreferencesProc;
		break;

	default:
		break;
	}

	if (fnWia != NULL)
	{
		hWiaThreadHandle = CreateThread(NULL, 0, fnWia, lpvParam, 0, 
			&dwWiaThreadId);
	}
}

//
//  DUMMY IMPLEMENTATION FOR TESTING PURPOSES
//
extern "C" HANDLE __declspec(dllexport) WTSVirtualChannelOpenQ(
	HANDLE hServer, DWORD SessionId, LPSTR pVirtualName)
{
	return (HANDLE) 666;
}

extern "C" BOOL __declspec(dllexport) WTSVirtualChannelWriteQ(
	HANDLE hChannelHandle, PCHAR Buffer, ULONG Length, PULONG pBytesWritten)
{
	DataArrival((LPBYTE)Buffer, (USHORT)Length);
	*pBytesWritten = Length;

	return TRUE;
}

extern "C" BOOL __declspec(dllexport) WTSVirtualChannelReadQ(
	HANDLE hChannelHandle, ULONG TimeOut, PCHAR Buffer,
	ULONG BufferSize, PULONG pBytesRead)
{
	LPBYTE lpBuff = NULL;
	ULONG ulExpiration = 0;

	if (TimeOut == 0xFFFF)
	{
		while (!(lpBuff = Poll()))
		{
			Sleep(100);
		}
	}
	else
	{
		while(!(lpBuff = Poll()))
		{
			Sleep(100);
			ulExpiration += 100;
			if (ulExpiration == TimeOut)
			{
				*pBytesRead = 0;
				return FALSE;
			}
		}
	}

	int nCmd = *((int*)lpBuff);
	int nLength = 0;
	switch (nCmd)
	{
	case PM_PICSTART:
	case PM_TRANSFERDONE:
		nLength = (2 * sizeof(int));
		memcpy(Buffer, &nCmd, sizeof(int));
		*pBytesRead = nLength;
		break;
	default:
		nLength = (2 * sizeof(int)) + *((int*)(lpBuff + sizeof(int)));
		*pBytesRead = nLength;
		memcpy(Buffer, lpBuff, nLength);
		break;
	}

	GlobalFree(lpBuff);

	return TRUE;
}

extern "C" BOOL __declspec(dllexport) WTSVirtualChannelCloseQ(
	HANDLE hChannelHandle)
{
	return TRUE;
}
