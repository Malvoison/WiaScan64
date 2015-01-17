// VCScanWiaRDP.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

//  channel stuff
LPVOID lpInitHandle;

PCHANNEL_ENTRY_POINTS gpEntryPoints = NULL;
DWORD gdwControlCode;
DWORD gdwOpenChannel;
DWORD gdwOpenChannelData;
DWORD gdwOpenChannelOutput;
LPHANDLE gphChannel;

BOOL bPollQuit = FALSE;

HANDLE hPollThreadHandle = NULL;
DWORD dwPollThreadId = 0;

HANDLE hThreadServerCommand = NULL;
HANDLE hThreadClientInfo = NULL;
HANDLE hThreadData = NULL;

HINSTANCE g_hInstance = 0;

ULONG_PTR       Token;

//  incoming channel data
LPBYTE lpServerCommandData = NULL;
LPBYTE lpXmlData = NULL;
LPBYTE lpImageXfer1Data = NULL;
LPBYTE lpImageXfer2Data = NULL;
LPBYTE lpImageXfer3Data = NULL;
LPBYTE lpImageXfer4Data = NULL;
LPBYTE lpImageXferData[4];
LPBYTE lpImageXferLastWrite[4];

HANDLE hHeap;

#define MAXDEBUGSTRING	1024

HANDLE hFileLogInfo = NULL;
BOOL bLoggingInitialized = FALSE;
extern CRITICAL_SECTION csLogFile;
HWND g_hWndMstsc;

//
//  Utility Functions
//
void DebugTraceMessage(LPCWSTR lpszFormat, ...)
{
	// duplicate printf functionality for DebugTraceMessage
	wchar_t *pszDebugString = (wchar_t*) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAXDEBUGSTRING);
	va_list args;
	va_start(args, lpszFormat);

	if(pszDebugString)
	{
		/*
		* format and output the debug string
		*/
		// prepend the current thread ID
		wchar_t szThreadFormat[MAXDEBUGSTRING];
		wsprintf(szThreadFormat, L"(Thrd ID: 0x%x) ", GetCurrentThreadId());
		lstrcat(szThreadFormat, lpszFormat);
		lstrcat(szThreadFormat, L"\r\n");
		wvsprintf(pszDebugString, szThreadFormat, args);	
		OutputDebugString(pszDebugString);

		//  Write to the log file
		EnterCriticalSection(&csLogFile);

		DWORD dwNumWritten;
		WriteFile(hFileLogInfo, pszDebugString, lstrlen(pszDebugString) * sizeof(TCHAR), &dwNumWritten, NULL);

		LeaveCriticalSection(&csLogFile);

		//  Ensure we've been initialized
		//if (gpEntryPoints != NULL && gdwOpenChannelOutput != 0)
		//{
		//	gpEntryPoints->pVirtualChannelWrite(gdwOpenChannelOutput, pszDebugString, MAXDEBUGSTRING, pszDebugString);
		//}
		//else
		//{
		//	HeapFree(hHeap, 0, pszDebugString);
		//}
		HeapFree(hHeap, 0, pszDebugString);

		pszDebugString = NULL;
	}
	va_end(args);

	return;

}

//
//  Virtual Channel Functions
//

void WINAPI VirtualChannelOpenEvent(DWORD openHandle, UINT event, LPVOID pdata, 
									UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{
	LPDWORD pdwControlCode = (LPDWORD)pdata;
	int nCmd;
	LPWSTR wszCommand;
	try
	{
		if (dataLength == totalLength)
		{
			switch (event)
			{
			case CHANNEL_EVENT_DATA_RECEIVED:
				DebugTraceMessage(L"Channel Event Data Recieved");
				gdwControlCode = *pdwControlCode;

				nCmd = *((int*)pdata);
				wszCommand = TranslateCommand(nCmd);
				DebugTraceMessage(L"***COMMAND RECEIVED: %s\n", wszCommand);

				if (lstrcmp(wszCommand, L"PM_INIT") == 0) 
				{
					DebugTraceMessage(L"Init: %s", wszCommand);
				} 
				else if (lstrcmp(wszCommand, L"PM_SENDDATA") == 0)
				{
					DebugTraceMessage(L"SendData: %s", wszCommand);
				}
				else if (lstrcmp(wszCommand, L"PM_DATASENT") == 0) 
				{
					DebugTraceMessage(L"Data Sent: %s", wszCommand);
				}

				HeapFree(hHeap, 0, wszCommand);
				break;
			case CHANNEL_EVENT_WRITE_COMPLETE:
				DebugTraceMessage(L"Channel Event Data Write Completed");
				break;
			case CHANNEL_EVENT_WRITE_CANCELLED:
				DebugTraceMessage(L"Channel Event Data Write Cancelled");
				break;
			default:
				DebugTraceMessage(L"Unrecognized open event\n");
				break;
			}
		}
		else
		{
			DebugTraceMessage(L"VCScanRDP: data is broken up\n");
		}
	}
	catch (...)
	{
		DebugTraceMessage(L"Exception in VirtualChannelOpenEvent\n");
	}
}

void WINAPI VirtualChannelOpenEventData(DWORD openHandle, UINT event, LPVOID pdata, 
										UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{	
	static LPBYTE lpLastWrite = NULL;

	try
	{
		if (dataLength == totalLength)
		{
			switch (event)
			{
			case CHANNEL_EVENT_DATA_RECEIVED:
				{
					if ((dataFlags & CHANNEL_FLAG_FIRST) || (dataFlags & CHANNEL_FLAG_ONLY))
					{
						lpXmlData = (LPBYTE) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, totalLength);
						lpLastWrite = lpXmlData;
						RtlCopyMemory(lpLastWrite, pdata, dataLength);
						lpLastWrite += dataLength;

						if (dataFlags & CHANNEL_FLAG_ONLY)
						{
							DebugTraceMessage((WCHAR *)lpXmlData);		
							ProcessData(lpXmlData);						
							break;
						}
					}
					else if (!(dataFlags & CHANNEL_FLAG_FIRST)&& !(dataFlags & CHANNEL_FLAG_LAST))  // CHANNEL_FLAG_MIDDLE
					{
						RtlCopyMemory(lpLastWrite, pdata, dataLength);
						lpLastWrite += dataLength;
					}
					else //  CHANNEL_FLAG_LAST
					{
						RtlCopyMemory(lpLastWrite, pdata, dataLength);
						lpLastWrite = NULL;
						DebugTraceMessage((WCHAR *)lpXmlData);		
						ProcessData(lpXmlData);						
					}
				}
				break;
			case CHANNEL_EVENT_WRITE_COMPLETE:
				DebugTraceMessage(L"Channel Event Data Write Completed");
				HeapFree(hHeap, 0, pdata);
				break;
			case CHANNEL_EVENT_WRITE_CANCELLED:
				DebugTraceMessage(L"Channel Event Data Write Cancelled");
				HeapFree(hHeap, 0, pdata);
				break;
			default:
				DebugTraceMessage(L"Unrecognized open event\n");
				break;
			}
		}
		else
		{
			DebugTraceMessage(L"VCScanRDP: data is broken up\n");
		}
	}
	catch (...)
	{
		DebugTraceMessage(L"Exception in VirtualChannelOpenEvent\n");
	}
}

void WINAPI VirtualChannelOpenEventOutput(DWORD openHandle, UINT event, LPVOID pdata,
										  UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{

}


VOID VCAPITYPE VirtualChannelInitEventProc(LPVOID pInitHandle, UINT event, LPVOID pData,
										   UINT dataLength)
{
	UINT ui;
	//DebugTraceMessage(L"VirtualChannelInitEventProc\n");
	DWORD dwExitCode = 0;
	try
	{
		switch (event)
		{
		case CHANNEL_EVENT_INITIALIZED:
			DebugTraceMessage(L"CHANNEL_EVENT_INITIALIZED (VCScanWiaRDP)\n");
			break;
		case CHANNEL_EVENT_CONNECTED:
			ui = gpEntryPoints->pVirtualChannelOpen(pInitHandle, &gdwOpenChannel, CHANNELNAME_SERVERCOMMAND,
				(PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEvent);
			if (ui != CHANNEL_RC_OK)
				DebugTraceMessage(L"Open of RDP virtual channel failed");

			ui = gpEntryPoints->pVirtualChannelOpen(pInitHandle, &gdwOpenChannelData, CHANNELNAME_SERVERDATA,
				(PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEventData);
			if (ui != CHANNEL_RC_OK)
				DebugTraceMessage(L"Open of RDP virtual channel Data failed");

			ui = gpEntryPoints->pVirtualChannelOpen(pInitHandle, &gdwOpenChannelOutput, CHANNELNAME_SERVEROUTPUT,
				(PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEventOutput);
			if (ui != CHANNEL_RC_OK)
				DebugTraceMessage(L"Open of RDP virtual channel output failed");

			break;
		case CHANNEL_EVENT_V1_CONNECTED:
			DebugTraceMessage(L"Connecting to a non windows 2000 Terminal Server");
			break;
		case CHANNEL_EVENT_DISCONNECTED:
			DebugTraceMessage(L"CHANNEL_EVENT_DISCONNECTED (VCScanWiaRDP)\n");
			break;
		case CHANNEL_EVENT_TERMINATED:
			HeapFree(hHeap, 0, gpEntryPoints);

			HeapDestroy(hHeap);
			break;
		default:
			DebugTraceMessage(L"Unknown CHANNEL_EVENT (VCScanWiaRDP)\n");
			break;
		}
	}
	catch (...)
	{
		DebugTraceMessage(L"Exception in VirtualChannelInitEventProc\n");
	}
}

BOOL VCAPITYPE VirtualChannelEntry(PCHANNEL_ENTRY_POINTS pEntryPoints)
{
	CHANNEL_DEF cd[3];
	UINT        uRet;

	//  Create a 10MB heap
	hHeap = HeapCreate(0, 10240000, 0);
	if (hHeap == NULL)
	{
		DebugTraceMessage(L"VirtualChannelEntry:  HeapCreate failed\n");
		return FALSE;
	}

	gpEntryPoints = (PCHANNEL_ENTRY_POINTS) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, pEntryPoints->cbSize);
	if (gpEntryPoints == NULL)
	{
		DebugTraceMessage(L"Error allocating entry points");
		return FALSE;
	}

	CopyMemory(gpEntryPoints, pEntryPoints, pEntryPoints->cbSize);
	RtlZeroMemory(&cd[0], sizeof(CHANNEL_DEF) * 3);

	for (int i = 0; i < CHANNEL_COUNT; i++)
	{
		cd[i].options = 
			CHANNEL_OPTION_ENCRYPT_RDP | 
			CHANNEL_OPTION_PRI_HIGH |
			CHANNEL_OPTION_COMPRESS | 
			CHANNEL_OPTION_SHOW_PROTOCOL;
	}

	DebugTraceMessage(L"cd.options = %i", cd[0].options);

	lstrcpyA(cd[0].name, CHANNELNAME_SERVERCOMMAND);
	lstrcpyA(cd[1].name, CHANNELNAME_SERVERDATA);
	lstrcpyA(cd[2].name, CHANNELNAME_SERVEROUTPUT);

	uRet = gpEntryPoints->pVirtualChannelInit((LPVOID *)&gphChannel,
		(PCHANNEL_DEF)&cd[0], 3, VIRTUAL_CHANNEL_VERSION_WIN2000,
		(PCHANNEL_INIT_EVENT_FN)VirtualChannelInitEventProc);
	if (uRet != CHANNEL_RC_OK)
	{
		MessageBox(NULL,TEXT("RDP Virtual channel Init Failed"),
			TEXT("VC Inventory"),MB_OK);
		return FALSE;
	}

	//  make sure each channel was initialized
	//  if any aren't, fail the lot
	for (int i = 0; i < CHANNEL_COUNT; i++)
	{
		if (!(cd[i].options & CHANNEL_OPTION_INITIALIZED))
			return false;
	}

	return TRUE;
}