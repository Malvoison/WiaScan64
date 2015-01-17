//  Filename:  TwainThread.cpp

#include "stdafx.h"
#include "TwainMsgWin.h"
#include "TwainWrapper.h"
#include "TwainThread.h"
#include "Utility.h"

//  GLOBALS
HANDLE hTwainThreadReadyEvent = NULL;
HANDLE hTwainThreadExitEvent = NULL;
HANDLE hMtxGlobals = NULL;
HANDLE hXferReady = NULL;

//  used with GetDefaultSrc
TW_STR32 szDefaultSrc;
//  used with EnumSrcs
LPSRCLIST lpSrcList = NULL;
//  used with SetSelectedSrc
TW_STR32 szSelectedSrc;
//  used with SetPixelType & EnumBitDepths
int nPixelType;
//  used with EnumPixelTypes
LPPIXELTYPELIST lpPixelTypeList = NULL;
//  used with SetBitDepth
int nBitDepth;
//  used with EnumBitDepths
LPBITDEPTHLIST lpBitDepthList = NULL;
//  used with SetResolution
float fResolution;
//  used with GetResolutionRange
LPRESOLUTIONRANGE lpResolutionRange = NULL;
//  used with EnumResolutions
LPRESOLUTIONLIST lpResolutionList = NULL;
//  used with TransferPictures
LPIMGLIST lpImageList = NULL;
//  used with EnumCapabilities
LPCAPLIST lpCapList = NULL;
//  used with EnumPageSizes
LPPAGESIZELIST lpPageSizeList = NULL;
//  used with SetPageSize
int nPageSize;
//  used with SetFeederEnabled
BOOL bFeederEnabled;
//  used with SetDuplex
BOOL bDuplex;


BOOL bProcessTwainMessage = TRUE;

DWORD WINAPI TwainThreadProc(PVOID pvParam)
{
	UNREFERENCED_PARAMETER(pvParam);
	
	DebugTraceMessage("Twain thread started...\n");

	HWND hWndTwain = NULL;
	//  start the thread's message pump
	MSG msg;
	//  force the system to create the msg queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	//  signal that the TWAIN thread is ready
	SetEvent(hTwainThreadReadyEvent);

	//  Enter the message loop
	DebugTraceMessage("Entering thread message loop\n");
	BOOL bRet;
	//  keep going until we get a WM_QUIT
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		//  if this is a TWAIN message, move on
		if (bProcessTwainMessage)
		{
			int nRet;
			switch (nRet = __PassMessage(&msg))
			{
			case -1:	//  TWRC_NOTDSEVENT
				break;
			case 1:		//  MSG_XFERREADY
				SetEvent(hXferReady);
				continue;
				break;
			default:
				break;
			}
		}

		if (bRet == -1)
		{
			DebugTraceMessage("Twain thread abnormal termination: ");
			SystemError();
		}

		if (msg.message < WM_USER)
		{
			DispatchMessage(&msg);
		}

		if (msg.message > WM_APP && msg.message <= 0xBFFF)
		{
			switch (msg.message)
			{
			case PM_CLOSESRC:
				__CloseSrc();
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_OPENSRC:
				__OpenSrc();
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_FINISH:
				__Finish();
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_INIT:
				hWndTwain = CreateTwainMsgWindow();
				__Init(hWndTwain);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_GETDEFAULTSOURCE:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				lstrcpyn(szDefaultSrc, __GetDefaultSrc(), sizeof(TW_STR32));
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_SELECTSRC:
				__SelectSrc();
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_ENUMSRCS:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				lpSrcList = __EnumSrcs();
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_SETSELECTEDSOURCE:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				__SetSelectedSource(szSelectedSrc);
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_SETPIXELTYPE:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				__SetPixelType(nPixelType);
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_SETPAGESIZE:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				__SetPageSize(nPageSize);
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_SETFEEDERENABLED:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				__SetFeederEnabled(bFeederEnabled);
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_SETDUPLEX:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				__SetDuplex(bDuplex);
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_ENUMPIXELTYPES:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				lpPixelTypeList = __EnumPixelTypes();
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_SETBITDEPTH:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				__SetBitDepth(nBitDepth);
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_ENUMBITDEPTHS:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				lpBitDepthList = __EnumBitDepths(nPixelType);
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_SETRESOLUTION:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				__SetResolution(fResolution);
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_GETRESOLUTIONRANGE:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				lpResolutionRange = __GetResolutionRange();
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_ENUMRESOLUTIONS:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				lpResolutionList = __EnumResolutions();
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_ENUMCAPS:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				lpCapList = __EnumCapabilities();
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_ENUMPAGESIZES:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				lpPageSizeList = __EnumPageSizes();
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_ACQUIRE:
				__Acquire();
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_TRANSFERPICTURES:
				WaitForSingleObject(hMtxGlobals, INFINITE);
				lpImageList = __TransferPictures();
				ReleaseMutex(hMtxGlobals);
				SetEvent((HANDLE)msg.wParam);
				break;
			case PM_RELEASE:
				__Finish();
				break;
			default:
				break;
			}			
		}
	}

	DebugTraceMessage("Sending WM_CLOSE to Twain window\n");
	SendMessage(hWndTwain, WM_CLOSE, 0, 0);

	DebugTraceMessage("Twain thread exiting\n");
	SetEvent(hTwainThreadExitEvent);
	return 0;
}