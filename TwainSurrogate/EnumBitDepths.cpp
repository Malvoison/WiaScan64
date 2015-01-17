//Filename: EnumBitDepths.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "EnumBitDepths.h"
#include "TwainMsgWin.h"
#include "Utility.h"
#include "TwainDispatch.h"

//Global variables
LPBITDEPTHLIST lpEnumBitList = NULL;
LPBITDEPTHLIST lpCurrBitListPtr = NULL;
extern TW_IDENTITY appid;
extern TW_IDENTITY srcds;
extern DSMENTRYPROC DSMEntry;
extern BOOL dsmOpen;
extern BOOL dsOpen;

INT_PTR CALLBACK EnumBitDepthsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		
	case WM_INITDIALOG:
		TCHAR szBitDepths[32];
		HWND hBitListBox;
		TW_CAPABILITY cap;
		TW_UINT16 rc;
		TW_UINT16 CurrentPxlValue;
		TW_ONEVALUE* twOne;

		if (!dsmOpen)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR) FALSE;
		}

		if(!dsOpen)
		{
			__OpenSrc();
		}

		hBitListBox = GetDlgItem(hDlg, IDC_ENUM_BIT_LIST);

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
		lpEnumBitList = __EnumBitDepths(CurrentPxlValue);
		lpCurrBitListPtr = lpEnumBitList;
		while(lpCurrBitListPtr)
		{
			_itoa(lpCurrBitListPtr->nDepth, szBitDepths, 10);
			SendMessage(hBitListBox, LB_ADDSTRING, 0, (LPARAM) szBitDepths);
			lpCurrBitListPtr = lpCurrBitListPtr->Next;
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
		if (lpEnumBitList != NULL)
		{
			do
			{
				lpCurrBitListPtr = lpEnumBitList;
				lpEnumBitList = lpEnumBitList->Next;
				GlobalFree(lpCurrBitListPtr);
			} while (lpEnumBitList != NULL);
		}
		return (INT_PTR) TRUE;
		break;
	}

	return (INT_PTR) FALSE;
}