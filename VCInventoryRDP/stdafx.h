// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//#define _WIN32_WINNT 0x0600
//#include <WinSDKVer.h>
//
//#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS

#pragma warning(disable:4005)

// Windows Header Files:
#include <crtdbg.h>
#include <errno.h>
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <commctrl.h>
#include <shellapi.h>
#include <objbase.h>
#include <atlbase.h>
#include <wia.h>
#include <sti.h>
#include <gdiplus.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <queue>

#include <wtsapi32.h>
#include <lmcons.h>
#include <pchannel.h>
#include <CChannel.h>

#include "VirtualChannelImpl.h"

#include <unordered_map>

#define COUNTOF(x) ( sizeof(x) / sizeof(*x) )

#define DEFAULT_STRING_SIZE 256
extern HINSTANCE g_hInstance;
extern PCHANNEL_ENTRY_POINTS gpEntryPoints;
extern DWORD gdwOpenChannelServerCommand;
extern DWORD gdwOpenChannelServerData;
extern DWORD gdwOpenChannelOutput;
extern HANDLE mtxDBProcess;
extern HANDLE hHeap;
extern WCHAR moduleName[MAX_PATH];

//
//  definitions
//
#define CHANNELNAME "VCScan"
#define TSVERSIONINFO 0
#define TSMEMORYINFO 1
#define QUERY_INTERVAL 2000  //  milliseconds

#define CHANNEL_COUNT				3
#define CHANNELNAME_SERVERCOMMAND	"SRVCMD"
#define CHANNELNAME_SERVERDATA		"SRVDAT"
#define CHANNELNAME_SERVEROUTPUT	"SRVOUT"

typedef struct
{
	char szClientName[MAX_COMPUTERNAME_LENGTH + 1];
	TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	TCHAR szUserName[UNLEN + 1];
	OSVERSIONINFO osvi;
	SYSTEM_INFO si;
} TS_VERSION_INFO;

typedef struct tagImgList
{
	LPBYTE img;
	int nImageSize;
	int nImageSequence;
} IMGLIST, *LPIMGLIST;

//
//  declarations
//
//void ShowError(void);

void WINAPI VirtualChannelOpenEvent(DWORD openHandle, UINT event, LPVOID pdata, 
									UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags);
VOID VCAPITYPE VirtualChannelInitEventProc(LPVOID pInitHandle, UINT event, LPVOID pData,
										   UINT dataLength);
BOOL VCAPITYPE VirtualChannelEntry(PCHANNEL_ENTRY_POINTS pEntryPoints);
DWORD WINAPI ImageConverterThreadProc(PVOID param);
void DebugTraceMessage(LPCWSTR lpszFormat, ...);
void ProcessData(LPBYTE data);

//extern HANDLE hHeap;
//extern HANDLE hFileLogInfo;
//extern CRITICAL_SECTION csLogFile;
//extern HWND g_hWndMstsc;
//extern std::queue<LPIMGLIST> imageQueue;
//extern HANDLE hMtxImageQueue;
//extern HANDLE hHeap;
//extern HANDLE hImageXferReadyEvent;
//extern HANDLE hSemImageXfer[NUM_XFER_CHANNELS];
//extern DWORD dwImageTransferThreadId;
//extern HANDLE hImageTransferThreadHandle;

//
//  Macros
//
#define WHEREAMI DebugTraceMessage(L"%s  %s\n", __FUNCTION__, __FILE__)
#define WHEREDOESITHURT DebugTraceMessage(L"Exception in %s (%s)", __FUNCTION__, __FILE__)



