//Filename: GetResolutionRange.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "GetResolutionRange.h"

//Global variable
LPRESOLUTIONRANGE lpResRange = NULL;

void SetTextBox(int nResWholeValue, int nResFracValue, HWND hDlgProc, int nTextBoxId)
{
	TCHAR szResWhole[32];
	TCHAR szResFrac[32];
	HWND ResTextBox;

	ResTextBox = GetDlgItem(hDlgProc, nTextBoxId);

	_itoa(nResWholeValue, szResWhole, 10);
	_itoa(nResFracValue, szResFrac, 10);
	strcat(szResWhole, ".");
	strcat(szResWhole, szResFrac);

	SetWindowText(ResTextBox, szResWhole);
	return;
}

INT_PTR CALLBACK GetResRangeDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		lpResRange = __GetResolutionRange();

		//Set each value for corresponding resolution range item
		if(lpResRange)
		{
			SetTextBox(lpResRange->CurrentValueWhole, lpResRange->CurrentValueFrac, hDlg, IDC_CURR_RES_VALUE);
			SetTextBox(lpResRange->DefaultValueWhole, lpResRange->DefaultValueFrac, hDlg, IDC_DEFAULT_RES_VALUE);
			SetTextBox(lpResRange->MinValueWhole, lpResRange->MinValueFrac, hDlg, IDC_MIN_RES_VALUE);
			SetTextBox(lpResRange->MaxValueWhole, lpResRange->MaxValueFrac, hDlg, IDC_MAX_RES_VALUE);
			SetTextBox(lpResRange->StepSizeWhole, lpResRange->StepSizeFrac, hDlg, IDC_STEPSZ_RES_VALUE);
		}
		GlobalFree(lpResRange);
		return (INT_PTR) TRUE;
		break;
	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR) TRUE;
		}
		break;
	}

	return (INT_PTR) FALSE;
}