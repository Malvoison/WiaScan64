//Filename: SetSelectSrc.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "SetSelectSrc.h"

//Global variables
LPSRCLIST lpSetSrcList = NULL;
LPSRCLIST lpCurrSetSrcListPtr = NULL;
HWND hSetSrcCombo;

INT_PTR CALLBACK SetSelectSrcDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		hSetSrcCombo = GetDlgItem(hDlg, IDC_SETSRC_COMBO);

		lpSetSrcList = __EnumSrcs();
		lpCurrSetSrcListPtr = lpSetSrcList;

		while (lpCurrSetSrcListPtr)
		{
			SendMessage(hSetSrcCombo, CB_ADDSTRING, 0, (LPARAM) lpCurrSetSrcListPtr->src);
			lpCurrSetSrcListPtr = lpCurrSetSrcListPtr->Next;
		}
		return (INT_PTR) TRUE;
		break;
	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK)
		{
			int nSetSrcIndex = SendMessage(hSetSrcCombo, CB_GETCURSEL, 0, 0);

			if(nSetSrcIndex != CB_ERR)
			{
				TCHAR szIndexItemTex[32];
				(TCHAR) SendMessage(hSetSrcCombo, CB_GETLBTEXT, nSetSrcIndex, (LPARAM) szIndexItemTex);
				__SetSelectedSource(szIndexItemTex);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR) TRUE;
		}
		else if(LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR) TRUE;
		}
		break;
	case WM_DESTROY:
		if(lpSetSrcList != NULL)
		{
			do
			{
				lpCurrSetSrcListPtr = lpSetSrcList;
				lpSetSrcList = lpSetSrcList->Next;
				GlobalFree(lpCurrSetSrcListPtr);
			} while (lpSetSrcList != NULL);
			GlobalFree(lpSetSrcList);
		}
	}

	return (INT_PTR) FALSE;
}