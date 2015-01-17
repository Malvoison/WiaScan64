//Filename: EnumCapabilities.cpp
#include "stdafx.h"
#include "TwainWrapper.h"
#include "EnumCapabilities.h"

//Global variables
LPCAPLIST lpEnumCapsList = NULL;
LPCAPLIST lpCurrCapsListPtr = NULL;

INT_PTR CALLBACK EnumCapsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		TCHAR szCapability[32];
		HWND hListBox;

		hListBox = GetDlgItem(hDlg, IDC_ENUM_CAPS_LIST);
		lpEnumCapsList = __EnumCapabilities();
		lpCurrCapsListPtr = lpEnumCapsList;
		while(lpCurrCapsListPtr)
		{
			_itoa(lpCurrCapsListPtr->nCap, szCapability, 10);
			SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM) szCapability);
			lpCurrCapsListPtr = lpCurrCapsListPtr->Next;
		}
		return (INT_PTR) TRUE;
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR) TRUE;
		}
		break;
	case WM_DESTROY:
		if (lpEnumCapsList != NULL)
		{
			do
			{
				lpCurrCapsListPtr = lpEnumCapsList;
				lpEnumCapsList = lpEnumCapsList->Next;
				GlobalFree(lpCurrCapsListPtr);
			} while (lpEnumCapsList != NULL);
		}
		return (INT_PTR) TRUE;
		break;
	}

	return (INT_PTR) FALSE;
}