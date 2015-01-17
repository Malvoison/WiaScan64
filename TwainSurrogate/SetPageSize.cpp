//Filename: SetPageSize.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "SetPageSize.h"

//Global variables
LPPAGESIZELIST lpPgSzsList = NULL;
LPPAGESIZELIST lpCurrentPgSzsPtr = NULL;
HWND hPgSzCombo;

INT_PTR CALLBACK SetPageSizeDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		TCHAR szPageSizes[32];
		hPgSzCombo = GetDlgItem(hDlg, IDC_PAGESIZE_COMBO);
		lpPgSzsList = __EnumPageSizes();
		lpCurrentPgSzsPtr = lpPgSzsList;

		while(lpCurrentPgSzsPtr)
		{
			_itoa(lpCurrentPgSzsPtr->nPageSize,szPageSizes,10);
			SendMessage(hPgSzCombo, CB_ADDSTRING, 0, (LPARAM) szPageSizes);
			lpCurrentPgSzsPtr = lpCurrentPgSzsPtr->Next;
		}
		return (INT_PTR) TRUE;
		break;
	case WM_COMMAND:
		if(LOWORD (wParam) == IDOK)
		{
			int nPgSzIndex = SendMessage(hPgSzCombo, CB_GETCURSEL, 0, 0);

			if(nPgSzIndex != CB_ERR)
			{
				TCHAR szIndexItemText[32];
				(TCHAR) SendMessage(hPgSzCombo, CB_GETLBTEXT, nPgSzIndex, (LPARAM) szIndexItemText);
				int nItemInt = atoi(szIndexItemText);
				__SetPageSize(nItemInt);
			}
			EndDialog(hDlg, LOWORD (wParam));
		}
		else if(LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD (wParam));
		}
		return (INT_PTR) TRUE;
		break;
	case WM_DESTROY:
		if (lpPgSzsList != NULL)
		{
			do
			{
				lpCurrentPgSzsPtr = lpPgSzsList;
				lpPgSzsList = lpPgSzsList->Next;
				GlobalFree(lpCurrentPgSzsPtr);
			} while (lpPgSzsList != NULL);
		}
	}

	return (INT_PTR) FALSE;
}