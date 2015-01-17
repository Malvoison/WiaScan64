//  Filename:  EnumSources.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "EnumSources.h"

//Global variables
LPSRCLIST lpEnumSrcList = NULL;
LPSRCLIST lpCurrSrcListPtr = NULL;

INT_PTR CALLBACK EnumSrcsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		HWND hListBox;
		hListBox = GetDlgItem(hDlg, IDC_SOURCES_LIST);

		lpEnumSrcList = __EnumSrcs();		
		lpCurrSrcListPtr = lpEnumSrcList;				
		while(lpCurrSrcListPtr)
		{
			SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM) lpCurrSrcListPtr->src);
			lpCurrSrcListPtr = lpCurrSrcListPtr->Next;
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
		if (lpEnumSrcList != NULL)
		{
			do
			{
				lpCurrSrcListPtr = lpEnumSrcList;
				lpEnumSrcList = lpEnumSrcList->Next;
				GlobalFree(lpCurrSrcListPtr);
			} while (lpEnumSrcList != NULL);
		}
		return (INT_PTR) TRUE;
		break;
	}

	return (INT_PTR) FALSE;
}