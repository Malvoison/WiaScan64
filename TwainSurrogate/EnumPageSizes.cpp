//Filename: EnumPageSizes.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "EnumPageSizes.h"

//Global variables
LPPAGESIZELIST lpEnumPgSzsList = NULL;
LPPAGESIZELIST lpCurrPgSzsPtr = NULL;

INT_PTR CALLBACK EnumPgSizesDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		TCHAR szPageSizes[32];
		HWND hListBox;

		hListBox = GetDlgItem(hDlg, IDC_PAGESIZE_LIST);
		lpEnumPgSzsList = __EnumPageSizes();
		lpCurrPgSzsPtr = lpEnumPgSzsList;
		while(lpCurrPgSzsPtr)
		{
			_itoa(lpCurrPgSzsPtr->nPageSize, szPageSizes,10);
			SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM) szPageSizes);
			lpCurrPgSzsPtr = lpCurrPgSzsPtr->Next;
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
		if (lpEnumPgSzsList != NULL)
		{
			do
			{
				lpCurrPgSzsPtr = lpEnumPgSzsList;
				lpEnumPgSzsList = lpEnumPgSzsList->Next;
				GlobalFree(lpCurrPgSzsPtr);
			} while (lpEnumPgSzsList != NULL);
		}
		return (INT_PTR) TRUE;
		break;
	}

	return (INT_PTR) FALSE;
}