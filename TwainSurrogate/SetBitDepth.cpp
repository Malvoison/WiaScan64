//Filename: SetBitDepth.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "SetBitDepth.h"

//Global variables
LPBITDEPTHLIST lpBitList = NULL;
LPBITDEPTHLIST lpCurrentBitListPtr = NULL;
HWND hBitCombo;
extern TW_IDENTITY appid;
extern TW_IDENTITY srcds;
extern DSMENTRYPROC DSMEntry;
extern BOOL dsmOpen;
extern BOOL dsOpen;

INT_PTR CALLBACK SetBitDepthDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		TCHAR szBitDepths[32];
		TW_CAPABILITY cap;
		TW_UINT16 rc;
		TW_ONEVALUE* twOne;
		TW_UINT16 CurrentPxlValue;

		if (!dsmOpen)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR) FALSE;
		}
			
		if(!dsOpen)
		{
			__OpenSrc();
		}

		hBitCombo = GetDlgItem(hDlg, IDC_BITDEPTH_COMBO);
		
		//Get current pixel type
		cap.Cap = ICAP_PIXELTYPE;
		cap.ConType = TWON_DONTCARE16;
		cap.hContainer = NULL;
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_GETCURRENT, &cap);
		twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
		if(twOne)
		{
			CurrentPxlValue = (TW_UINT16) twOne->Item;
		}
		//If no pixel type chosen, use 0 as default
		else
		{
			CurrentPxlValue = 0;
		}
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);

		//Get bit depths available for current pixel type
		lpBitList = __EnumBitDepths(CurrentPxlValue);
		lpCurrentBitListPtr = lpBitList;

		while(lpCurrentBitListPtr)
		{
			_itoa(lpCurrentBitListPtr->nDepth, szBitDepths, 10);
			SendMessage(hBitCombo, CB_ADDSTRING, 0, (LPARAM) szBitDepths);
			lpCurrentBitListPtr = lpCurrentBitListPtr->Next;
		}
		return (INT_PTR) TRUE;
		break;
	case WM_COMMAND:
		if(LOWORD (wParam) == IDOK)
		{
			int nBitIndex = SendMessage(hBitCombo, CB_GETCURSEL, 0, 0);

			if(nBitIndex != CB_ERR)
			{
				TCHAR szIndexItemText[32];
				(TCHAR) SendMessage(hBitCombo, CB_GETLBTEXT, nBitIndex, (LPARAM) szIndexItemText);
				int nItemInt = atoi(szIndexItemText);
				__SetBitDepth(nItemInt);
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
		if (lpBitList != NULL)
		{
			do
			{
				lpCurrentBitListPtr = lpBitList;
				lpBitList = lpBitList->Next;
				GlobalFree(lpCurrentBitListPtr);
			} while (lpBitList != NULL);
		}
		return (INT_PTR) TRUE;
		break;
	}

	return (INT_PTR) FALSE;
}