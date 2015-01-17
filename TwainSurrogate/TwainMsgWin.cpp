//  Filename: TwainMsgWin.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "TwainThread.h"
#include "TwainMsgWin.h"
#include "Utility.h"


HWND CreateTwainMsgWindow(void)
{
	DebugTraceMessage("CreateTwainMsgWindow\n");
	HANDLE hModInst = GetModuleHandle(NULL);

	if (InitTWMain(hModInst))
	{
		HWND hWndRet = CreateWindow("TW_App_MainWnd", NULL, WS_OVERLAPPEDWINDOW,
			0, 0, 100, 100, HWND_MESSAGE, NULL, (HINSTANCE) hModInst, NULL);
		if (hWndRet != NULL)
		{
			ShowWindow(hWndRet, SW_SHOWNORMAL);
			return hWndRet;
		}
		else
		{
			SystemError();
		}
	}
	else
	{
		SystemError();
	}

	return NULL;
}

BOOL InitTWMain(HANDLE hInstance)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));

	//  setup the TWAIN message window
	wc.style = NULL;
	wc.lpfnWndProc = (WNDPROC) TW_MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = (HINSTANCE) hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "TW_App_MainWnd";

	RegisterClass(&wc);

	return TRUE;
}

LRESULT FAR PASCAL TW_MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	case WM_ENDSESSION:
		DebugTraceMessage("Destroying TW_App_MainWnd\n");
		DestroyWindow(hwnd);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
		break;
	}

	return 0L;
}