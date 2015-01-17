//  Filename:  TwainMsgWin.h

#pragma once

HWND CreateTwainMsgWindow(void);
BOOL InitTWMain(HANDLE hInstance);
LRESULT FAR PASCAL TW_MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);