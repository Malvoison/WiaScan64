//Filename: EnumResolutions.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "EnumResolutions.h"

//Global variables
LPRESOLUTIONLIST lpEnumResList = NULL;
LPRESOLUTIONLIST lpCurrResListPtr = NULL;

INT_PTR CALLBACK EnumResolutionDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		TCHAR szResWhole[32];
		TCHAR szResFrac[32];
		HWND hListBox;

		hListBox = GetDlgItem(hDlg, IDC_ENUMRES_LIST);
		lpEnumResList = __EnumResolutions();
		lpCurrResListPtr = lpEnumResList;
		while(lpCurrResListPtr)
		{
			_itoa(lpCurrResListPtr->Whole, szResWhole, 10);
			_itoa(lpCurrResListPtr->Fraction, szResFrac, 10);

			strcat(szResWhole, ".");
			strcat(szResWhole, szResFrac);

			SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM) szResWhole);
			lpCurrResListPtr = lpCurrResListPtr->Next;
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
		if (lpEnumResList != NULL)
		{
			do
			{
				lpCurrResListPtr = lpEnumResList;
				lpEnumResList = lpEnumResList->Next;
				GlobalFree(lpCurrResListPtr);
			} while (lpEnumResList != NULL);
		}
		return (INT_PTR) TRUE;
		break;
	}

	return (INT_PTR) FALSE;
}