//Filename: ShowUI.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "ShowUI.h"

//Global variables
HWND hShowUICheck;
extern BOOL dsmOpen;
extern BOOL dsOpen;
extern BOOL dsEnabled;
extern TW_IDENTITY appid;
extern TW_IDENTITY srcds;
extern TW_USERINTERFACE guif;
extern DSMENTRYPROC DSMEntry;

INT_PTR CALLBACK ShowUIDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK)
		{
			TW_UINT16 rc;

			if (!dsmOpen)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR) FALSE;
			}
			
			if(!dsOpen)
			{
				__OpenSrc();
			}

			hShowUICheck = GetDlgItem(hDlg, IDC_SHOWUI_CHECK);

			if(Button_GetState(hShowUICheck))
			{
				memset(&guif, 0, sizeof(TW_USERINTERFACE));
				guif.ShowUI = 1;
				guif.hParent = hDlg;
				rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDS, (TW_MEMREF)&guif);
				dsEnabled = TRUE;
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