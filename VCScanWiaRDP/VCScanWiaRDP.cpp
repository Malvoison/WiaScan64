// VCScanWiaRDP.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "VirtualChannelImpl.h"

//  channel stuff
LPVOID lpInitHandle;
DWORD gdwOpenChannelServerCommand = 0;
DWORD gdwOpenChannelClientInfo = 0;
DWORD gdwOpenChannelImageXfer1 = 0;
DWORD gdwOpenChannelImageXfer2 = 0;
DWORD gdwOpenChannelImageXfer3 = 0;
DWORD gdwOpenChannelImageXfer4 = 0;
DWORD gdwOpenChannelUpdate = 0;

PCHANNEL_ENTRY_POINTS gpEntryPoints = NULL;
DWORD gdwControlCode;

BOOL bPollQuit = FALSE;

HANDLE hPollThreadHandle = NULL;
DWORD dwPollThreadId = 0;

HANDLE hThreadServerCommand = NULL;
HANDLE hThreadClientInfo = NULL;
HANDLE hThreadImageXfer1 = NULL;
HANDLE hThreadImageXfer2 = NULL;
HANDLE hThreadImageXfer3 = NULL;
HANDLE hThreadImageXfer4 = NULL;

HANDLE hHeap = NULL;
HINSTANCE g_hInstance = 0;

Gdiplus::Status StartupStatus;
ULONG_PTR       Token;

//  incoming channel data
LPBYTE lpServerCommandData = NULL;
LPBYTE lpClientInfoData = NULL;
LPBYTE lpImageXfer1Data = NULL;
LPBYTE lpImageXfer2Data = NULL;
LPBYTE lpImageXfer3Data = NULL;
LPBYTE lpImageXfer4Data = NULL;
LPBYTE lpImageXferData[4];
LPBYTE lpImageXferLastWrite[4];
LPBYTE lpUpdateData = NULL;

WiaWrap::CProgressDlg* cpd;
HWND hWndDlg;

INT_PTR  CALLBACK DialogProc(  
    HWND   hDlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg) 
    {
		case WM_INITDIALOG: 
		{
			SendDlgItemMessage(hDlg, IDC_PROGRESS_BAR, PBM_SETRANGE32, (WPARAM) 0, (LPARAM) 100);

			return TRUE;
		}

		case WM_COMMAND: 
		{
			switch (LOWORD(wParam)) 
			{
				case IDCANCEL:
				{
                    // If the user presses the cancel button, set the m_bCancelled flag.

                    // The cancel operation will probably take some time, so 
                    // change the cancel button text to "wait...", so that 
                    // the user will see the cancel command is received and 
                    // is being processed

					TCHAR szWait[DEFAULT_STRING_SIZE] = _T("...");

					LoadString(g_hInstance, IDS_WAIT, szWait, COUNTOF(szWait));

				    SetDlgItemText(hDlg, IDCANCEL, szWait);

					return TRUE;
				}
			}

			break;
		}
    }

    return FALSE;
}

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
				DebugTraceMessage(L"CHANNEL_EVENT_DATA_RECEIVED\n");

				gdwControlCode = *pdwControlCode;

				DataArrival((LPBYTE)pdata, (UINT32)dataLength);
				nCmd = *((int*)pdata);
				wszCommand = TranslateCommand(nCmd);
				DebugTraceMessage(L"***COMMAND RECEIVED: %s\n", wszCommand);
				HeapFree(hHeap, 0, wszCommand);
				break;
			case CHANNEL_EVENT_WRITE_COMPLETE:
				//
				//  Need to free the buffer being written but only if it's completely written
				//
				break;
			case CHANNEL_EVENT_WRITE_CANCELLED:
				//
				//  Need to free the buffer being written but only if it's completely written
				//
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

void WINAPI VirtualChannelOpenEventServerCommand(DWORD openHandle, UINT event, LPVOID pdata, 
												 UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{
	static LPBYTE lpLastWrite = NULL;

	switch (event)
	{
	case CHANNEL_EVENT_DATA_RECEIVED:	
		DebugTraceMessage(L"%s: CHANNEL_EVENT_DATA_RECEIVED\n", L"SRVCMD");
		if ((dataFlags & CHANNEL_FLAG_FIRST) || (dataFlags & CHANNEL_FLAG_ONLY))
		{
			lpServerCommandData = (LPBYTE) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, totalLength);
			lpLastWrite = lpServerCommandData;
			RtlCopyMemory(lpLastWrite, pdata, dataLength);
			lpLastWrite += dataLength;
			if (dataFlags & CHANNEL_FLAG_ONLY)
			{
				//  perform useful tasks here
				//hWndDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROGRESS), 
				//	g_hWndMstsc, DialogProc);
				//if (hWndDlg != NULL)
				//{
				//	ShowWindow(hWndDlg, SW_SHOW);
				//	SetWindowPos(hWndDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
				//}
				DataArrivalServerCommand(lpServerCommandData, totalLength);
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
			DataArrivalServerCommand(lpServerCommandData, totalLength);
		}
		break;
	case CHANNEL_EVENT_WRITE_COMPLETE:
		HeapFree(hHeap, 0, pdata);
		break;
	case CHANNEL_EVENT_WRITE_CANCELLED:
		HeapFree(hHeap, 0, pdata);
		break;
	default:
		DebugTraceMessage(L"Unrecognized open event\n");
		break;
	}
}

void WINAPI VirtualChannelOpenEventClientInfo(DWORD openHandle, UINT event, LPVOID pdata, 
											  UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{
	static LPBYTE lpLastWrite = NULL;

	switch (event)
	{
	case CHANNEL_EVENT_DATA_RECEIVED:		
		if ((dataFlags & CHANNEL_FLAG_FIRST) || (dataFlags & CHANNEL_FLAG_ONLY))
		{
			lpClientInfoData = (LPBYTE) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, totalLength);
			lpLastWrite = lpClientInfoData;
			RtlCopyMemory(lpLastWrite, pdata, dataLength);
			lpLastWrite += dataLength;
			if (dataFlags & CHANNEL_FLAG_ONLY)
			{
				//  perform useful tasks here
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
		}
		break;
	case CHANNEL_EVENT_WRITE_COMPLETE:
		HeapFree(hHeap, 0, pdata);
		break;
	case CHANNEL_EVENT_WRITE_CANCELLED:
		HeapFree(hHeap, 0, pdata);
		break;
	default:
		DebugTraceMessage(L"Unrecognized open event\n");
		break;
	}
}

void WINAPI ImageXferHelper(DWORD openHandle, UINT event, LPVOID pdata, 
							UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags, int xferChannel)
{
	switch (event)
	{
	case CHANNEL_EVENT_DATA_RECEIVED:		
		if ((dataFlags & CHANNEL_FLAG_FIRST) || (dataFlags & CHANNEL_FLAG_ONLY))
		{
			lpImageXferData[xferChannel] = (LPBYTE) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, totalLength);
			lpImageXferLastWrite[xferChannel] = lpImageXferData[xferChannel];
			RtlCopyMemory(lpImageXferLastWrite[xferChannel], pdata, dataLength);
			lpImageXferLastWrite[xferChannel] += dataLength;
			if (dataFlags & CHANNEL_FLAG_ONLY)
			{
				//  perform useful tasks here
			}
		}
		else if (!(dataFlags & CHANNEL_FLAG_FIRST)&& !(dataFlags & CHANNEL_FLAG_LAST))  // CHANNEL_FLAG_MIDDLE
		{
			RtlCopyMemory(lpImageXferLastWrite[xferChannel], pdata, dataLength);
			lpImageXferLastWrite[xferChannel] += dataLength;
		}
		else //  CHANNEL_FLAG_LAST
		{
			RtlCopyMemory(lpImageXferLastWrite[xferChannel], pdata, dataLength);
			lpImageXferLastWrite[xferChannel] = NULL;
		}
		break;
	case CHANNEL_EVENT_WRITE_COMPLETE:				
	case CHANNEL_EVENT_WRITE_CANCELLED:
		HeapFree(hHeap, 0, pdata);
		ReleaseSemaphore(hSemImageXfer[xferChannel], 1, NULL);
		DebugTraceMessage(L"Image transfer complete on transfer channel: %d\n", xferChannel);
		break;
	default:
		DebugTraceMessage(L"Unrecognized open event\n");
		break;
	}
}

void WINAPI VirtualChannelOpenEventImageXfer1(DWORD openHandle, UINT event, LPVOID pdata, 
											  UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{
	ImageXferHelper(openHandle, event, pdata, dataLength, totalLength, dataFlags, XFER1);
}

void WINAPI VirtualChannelOpenEventImageXfer2(DWORD openHandle, UINT event, LPVOID pdata, 
											  UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{
	ImageXferHelper(openHandle, event, pdata, dataLength, totalLength, dataFlags, XFER2);
}

void WINAPI VirtualChannelOpenEventImageXfer3(DWORD openHandle, UINT event, LPVOID pdata, 
											  UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{
	ImageXferHelper(openHandle, event, pdata, dataLength, totalLength, dataFlags, XFER3);
}

void WINAPI VirtualChannelOpenEventImageXfer4(DWORD openHandle, UINT event, LPVOID pdata, 
											  UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{
	ImageXferHelper(openHandle, event, pdata, dataLength, totalLength, dataFlags, XFER4);
}

void WINAPI VirtualChannelOpenEventUpdate(DWORD openHandle, UINT event, LPVOID pdata, 
												 UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{
	static LPBYTE lpLastWrite = NULL;

	switch (event)
	{
	case CHANNEL_EVENT_DATA_RECEIVED:	
		DebugTraceMessage(L"%s: CHANNEL_EVENT_DATA_RECEIVED\n", L"UPDATE");
		if ((dataFlags & CHANNEL_FLAG_FIRST) || (dataFlags & CHANNEL_FLAG_ONLY))
		{
			lpUpdateData = (LPBYTE) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, totalLength);
			lpLastWrite = lpUpdateData;
			RtlCopyMemory(lpLastWrite, pdata, dataLength);
			lpLastWrite += dataLength;
			if (dataFlags & CHANNEL_FLAG_ONLY)
			{
				DataArrivalUpdate(lpUpdateData, totalLength);
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
			DataArrivalUpdate(lpUpdateData, totalLength);
		}
		break;
	case CHANNEL_EVENT_WRITE_COMPLETE:
		HeapFree(hHeap, 0, pdata);
		break;
	case CHANNEL_EVENT_WRITE_CANCELLED:
		HeapFree(hHeap, 0, pdata);
		break;
	default:
		DebugTraceMessage(L"Unrecognized open event\n");
		break;
	}
}

VOID VCAPITYPE VirtualChannelInitEventProc(LPVOID pInitHandle, UINT event, LPVOID pData,
										   UINT dataLength)
{
	DebugTraceMessage(L"VirtualChannelInitEventProc\n");
	DWORD dwExitCode = 0;
	UINT ui;
	try
	{
		switch (event)
		{
		case CHANNEL_EVENT_INITIALIZED:
			DebugTraceMessage(L"CHANNEL_EVENT_INITIALIZED (VCScanWiaRDP)\n");
			break;
		case CHANNEL_EVENT_CONNECTED:
			{
				DebugTraceMessage(L"CHANNEL_EVENT_CONNECTED (VCScanWiaRDP)\n");
				DebugTraceMessage(L"Connected to RD Session Host Server: %s\n", (LPCWSTR)pData);
				//  Open the Channel(s)
				ui = gpEntryPoints->pVirtualChannelOpen(pInitHandle, 
					&gdwOpenChannelServerCommand, CHANNELNAME_SERVERCOMMAND, 
					(PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEventServerCommand);
				if (ui != CHANNEL_RC_OK)
				{
					DebugTraceMessage(L"VirtualChannelOpen failed for %s\n", CHANNELNAME_SERVERCOMMAND);
					return;
				}

				ui = gpEntryPoints->pVirtualChannelOpen(pInitHandle, 
					&gdwOpenChannelClientInfo, CHANNELNAME_CLIENTINFO, 
					(PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEventClientInfo);
				if (ui != CHANNEL_RC_OK)
				{
					DebugTraceMessage(L"VirtualChannelOpen failed for %s\n", CHANNELNAME_CLIENTINFO);
					return;
				}

				ui = gpEntryPoints->pVirtualChannelOpen(pInitHandle, 
					&gdwOpenChannelImageXfer1, CHANNELNAME_IMAGEXFER1, 
					(PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEventImageXfer1);
				if (ui != CHANNEL_RC_OK)
				{
					DebugTraceMessage(L"VirtualChannelOpen failed for %s\n", CHANNELNAME_IMAGEXFER1);
					return;
				}

				ui = gpEntryPoints->pVirtualChannelOpen(pInitHandle, 
					&gdwOpenChannelImageXfer2, CHANNELNAME_IMAGEXFER2, 
					(PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEventImageXfer2);
				if (ui != CHANNEL_RC_OK)
				{
					DebugTraceMessage(L"VirtualChannelOpen failed for %s\n", CHANNELNAME_IMAGEXFER2);
					return;
				}

				ui = gpEntryPoints->pVirtualChannelOpen(pInitHandle, 
					&gdwOpenChannelImageXfer3, CHANNELNAME_IMAGEXFER3, 
					(PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEventImageXfer3);
				if (ui != CHANNEL_RC_OK)
				{
					DebugTraceMessage(L"VirtualChannelOpen failed for %s\n", CHANNELNAME_IMAGEXFER3);
					return;
				}

				ui = gpEntryPoints->pVirtualChannelOpen(pInitHandle, 
					&gdwOpenChannelImageXfer4, CHANNELNAME_IMAGEXFER4, 
					(PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEventImageXfer4);
				if (ui != CHANNEL_RC_OK)
				{
					DebugTraceMessage(L"VirtualChannelOpen failed for %s\n", CHANNELNAME_IMAGEXFER4);
					return;
				}

				//  Initialize GDI+
				Gdiplus::GdiplusStartupInput StartupInput(
					NULL, FALSE, FALSE);
				StartupStatus = Gdiplus::GdiplusStartup(
					&Token, &StartupInput, NULL);

				//  start the polling thread
				/*hPollThreadHandle = CreateThread(NULL, 0, PollThreadProcServerCommand, NULL, 0, &dwPollThreadId);*/
			}
			break;
		case CHANNEL_EVENT_V1_CONNECTED:
			DebugTraceMessage(L"CHANNEL_EVENT_V1_CONNECTED (VCScanWiaRDP)\n");
			break;
		case CHANNEL_EVENT_DISCONNECTED:
			DebugTraceMessage(L"CHANNEL_EVENT_DISCONNECTED (VCScanWiaRDP)\n");
			break;
		case CHANNEL_EVENT_TERMINATED:
			{
				DebugTraceMessage(L"CHANNEL_EVENT_TERMINATED (VCScanWiaRDP)\n");
				//
				//  free the entry points table and the heap
				//
				if (StartupStatus == Gdiplus::Ok)
				{
					Gdiplus::GdiplusShutdown(Token);
				}

				/*bPollQuit = TRUE;*/
			
				HeapFree(hHeap, 0, gpEntryPoints);

				HeapDestroy(hHeap);

				ShutdownLogging();
			
				/*TerminateThread(hPollThreadHandle, dwExitCode);*/
			}
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
	CHANNEL_DEF cd[7];
	UINT uRet;
	BOOL bRetVal = FALSE;
	
	try
	{
		//  Create a 10MB heap
		hHeap = HeapCreate(0, 10240000, 0);
		if (hHeap == NULL)
		{
			OutputDebugString(L"VirtualChannelEntry:  HeapCreate failed\n");
			return FALSE;
		}
        
		GetProcessAndWindowInfo();
		InitializeLogging();

		DebugTraceMessage(L"VirtualChannelEntry called\n");
		//  allocate memory for the entry points		
		gpEntryPoints = (PCHANNEL_ENTRY_POINTS) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, pEntryPoints->cbSize);
		if (gpEntryPoints == NULL)
		{
			ShowError();
			return FALSE;
		}
		RtlCopyMemory(gpEntryPoints, pEntryPoints, pEntryPoints->cbSize);

		//  initialize CHANNEL_DEF structure
		RtlZeroMemory(&cd[0], sizeof(CHANNEL_DEF) * 6);

		//  initialize common channel options
		for (int i = 0; i < CHANNEL_COUNT; i++)
		{
			cd[i].options = 
				CHANNEL_OPTION_ENCRYPT_RDP | 
				CHANNEL_OPTION_PRI_HIGH |
				CHANNEL_OPTION_COMPRESS | 
				CHANNEL_OPTION_SHOW_PROTOCOL;
		}

		lstrcpyA(cd[0].name, CHANNELNAME_SERVERCOMMAND);
		lstrcpyA(cd[1].name, CHANNELNAME_CLIENTINFO);
		lstrcpyA(cd[2].name, CHANNELNAME_IMAGEXFER1);
		lstrcpyA(cd[3].name, CHANNELNAME_IMAGEXFER2);
		lstrcpyA(cd[4].name, CHANNELNAME_IMAGEXFER3);
		lstrcpyA(cd[5].name, CHANNELNAME_IMAGEXFER4);
        lstrcpyA(cd[6].name, CHANNELNAME_UPDATE);

		//  register channel
		uRet = gpEntryPoints->pVirtualChannelInit((LPVOID*)&lpInitHandle,
			(PCHANNEL_DEF)&cd[0], CHANNEL_COUNT, VIRTUAL_CHANNEL_VERSION_WIN2000,
			(PCHANNEL_INIT_EVENT_FN)VirtualChannelInitEventProc);
		if (uRet != CHANNEL_RC_OK)
			return FALSE;

		//  make sure each channel was initialized
		//  if any aren't, fail the lot
		for (int i = 0; i < CHANNEL_COUNT; i++)
		{
			if (!(cd[0].options & CHANNEL_OPTION_INITIALIZED))
				return false;
		}
		
		return TRUE;
	}
	catch (...)
	{
		DebugTraceMessage(L"Exception in VirtualChannelEntry\n");
	}
	
	return FALSE;
}