//Filename: SetPixelType.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "SetPixelType.h"

//Global Variables
LPPIXELTYPELIST lpPxlList = NULL;
LPPIXELTYPELIST lpCurrentPxlListPtr = NULL;
HWND hPixelCombo;

INT_PTR CALLBACK SetPixelTypeDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		TCHAR szPxlType[32];

		hPixelCombo = GetDlgItem(hDlg, IDC_SETPXLTYPE_COMBO);
		lpPxlList = __EnumPixelTypes();
		lpCurrentPxlListPtr = lpPxlList;
		while(lpCurrentPxlListPtr)
		{
			_itoa(lpCurrentPxlListPtr->nType, szPxlType, 10);
			SendMessage(hPixelCombo, CB_ADDSTRING, 0, (LPARAM) szPxlType);
			lpCurrentPxlListPtr = lpCurrentPxlListPtr->Next;
		}	
		return (INT_PTR) TRUE;
		break;
	case WM_COMMAND:
		if(LOWORD (wParam) == IDOK)
		{
			int nPxlIndex = SendMessage(hPixelCombo, CB_GETCURSEL, 0, 0);

			if(nPxlIndex != CB_ERR)
			{
				TCHAR szIndexItemText[32];
				(TCHAR) SendMessage(hPixelCombo, CB_GETLBTEXT, nPxlIndex, (LPARAM) szIndexItemText);
				int nItemInt = atoi(szIndexItemText);
				__SetPixelType(nItemInt);
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
		if (lpPxlList != NULL)
		{
			do
			{
				lpCurrentPxlListPtr = lpPxlList;
				lpPxlList = lpPxlList->Next;
				GlobalFree(lpCurrentPxlListPtr);
			} while (lpPxlList != NULL);
		}
		return (INT_PTR) TRUE;
		break;
	}

	return (INT_PTR) FALSE;
}