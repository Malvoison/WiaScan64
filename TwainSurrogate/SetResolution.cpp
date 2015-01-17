//Filename; SetResolution.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "SetResolution.h"

//Global variables
LPRESOLUTIONLIST lpResolutions = NULL;
LPRESOLUTIONLIST lpCurrResolutionPtr = NULL;
HWND hResCombo;

INT_PTR CALLBACK SetResolutionDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		TCHAR szResWhole[32];
		TCHAR szResFrac[32];

		hResCombo = GetDlgItem(hDlg, IDC_SETRESOLUTION_COMBO);
		lpResolutions = __EnumResolutions();
		lpCurrResolutionPtr = lpResolutions;

		while(lpCurrResolutionPtr)
		{
			_itoa(lpCurrResolutionPtr->Whole, szResWhole, 10);
			_itoa(lpCurrResolutionPtr->Fraction, szResFrac, 10);
			strcat(szResWhole, ".");
			strcat(szResWhole, szResFrac);

			SendMessage(hResCombo, CB_ADDSTRING, 0, (LPARAM) szResWhole);
			lpCurrResolutionPtr = lpCurrResolutionPtr->Next;
		}
		return (INT_PTR) TRUE;
		break;
	case WM_COMMAND:
		if(LOWORD (wParam) == IDOK)
		{
			int nResIndex = SendMessage(hResCombo, CB_GETCURSEL, 0, 0);

			if(nResIndex != CB_ERR)
			{
				TCHAR szIndexItemText[32];
				(TCHAR) SendMessage(hResCombo, CB_GETLBTEXT, nResIndex, (LPARAM) szIndexItemText);
				double nItemInt = atof(szIndexItemText);
				__SetResolution((float)nItemInt);
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
		if(lpResolutions != NULL)
		{
			do
			{
				lpCurrResolutionPtr = lpResolutions;
				lpResolutions = lpResolutions->Next;
				GlobalFree(lpCurrResolutionPtr);
			} while (lpResolutions != NULL);
		}
	}

	return (INT_PTR) FALSE;
}