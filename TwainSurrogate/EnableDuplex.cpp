//Filename: EnableDuplex.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "EnableDuplex.h"

INT_PTR CALLBACK EnableDuplexDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK)
		{
			HWND hEnableDuplexCheck;
			hEnableDuplexCheck = GetDlgItem(hDlg, IDC_ENABLEDUPLEX_CHECK);
			if(Button_GetState(hEnableDuplexCheck))
			{
				__SetDuplex(TRUE);
			}
			else
			{
				__SetDuplex(FALSE);
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