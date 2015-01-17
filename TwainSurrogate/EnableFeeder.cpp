//Filename: EnableFeeder.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "EnableFeeder.h"

INT_PTR CALLBACK EnableFeederDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK)
		{
			HWND hEnableFeederCheck;
			hEnableFeederCheck = GetDlgItem(hDlg, IDC_ENABLEFEEDER_CHECK);
			if(Button_GetState(hEnableFeederCheck))
			{
				__SetFeederEnabled(TRUE);
			}
			else
			{
				__SetFeederEnabled(FALSE);
			}
			EndDialog(hDlg, LOWORD(wParam));
		}
		else if(LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
		}
		return (INT_PTR) TRUE;
	}
	return (INT_PTR) FALSE;
}