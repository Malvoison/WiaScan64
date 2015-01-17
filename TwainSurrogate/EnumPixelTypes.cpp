//Filename: EnumPixelTypes.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "EnumPixelTypes.h"

//Global variables
LPPIXELTYPELIST lpEnumPxlList = NULL;
LPPIXELTYPELIST lpCurrPxlListPtr = NULL;

INT_PTR CALLBACK EnumPxlTypesDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		TCHAR szPxlType[32];
		HWND hListBox;

		hListBox = GetDlgItem(hDlg, IDC_ENUM_PXLS_LIST);
		lpEnumPxlList = __EnumPixelTypes();
		lpCurrPxlListPtr = lpEnumPxlList;
		while(lpCurrPxlListPtr)
		{
			_itoa(lpCurrPxlListPtr->nType, szPxlType, 10);
			SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM) szPxlType);
			lpCurrPxlListPtr = lpCurrPxlListPtr->Next;
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
		if (lpEnumPxlList != NULL)
		{
			do
			{
				lpCurrPxlListPtr = lpEnumPxlList;
				lpEnumPxlList = lpEnumPxlList->Next;
				GlobalFree(lpCurrPxlListPtr);
			} while (lpEnumPxlList != NULL);
		}
		return (INT_PTR) TRUE;
		break;
	}

	return (INT_PTR) FALSE;
}