//Filename: GetDefaultSource.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "GetDefaultSource.h"

//Global Variable
LPCSTR lpDefaultSource = NULL;

INT_PTR CALLBACK DefaultSrcDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		HWND hDefaultSrcBox;
		hDefaultSrcBox = GetDlgItem(hDlg, IDC_DEFAULTSRC_VALUE);
		lpDefaultSource = __GetDefaultSrc();
		SetWindowText(hDefaultSrcBox, lpDefaultSource);
		return (INT_PTR) TRUE;
		break;
	case WM_COMMAND:
		if(LOWORD (wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD (wParam));
			return (INT_PTR) TRUE;
		}
		break;
	}
	return (INT_PTR) FALSE;
}