// TwainWrapper.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
//#include <gdiplus.h>
//#include <gdiplusflat.h>
#include "TwainWrapper.h"
#include "TwainMsgWin.h"
#include "Utility.h"
#include "TwainDispatch.h"

using namespace Gdiplus;



HMODULE hModTwain = NULL;
DSMENTRYPROC DSMEntry = NULL;

//  APPLICATION GLOBALS
HWND hwnd;
TW_IDENTITY appid = { 0 };
TW_IDENTITY srcds = { 0 };
TW_EVENT evtmsg;
MSG winmsg;
int nShowUI = 1;
BOOL bAcquireCancelled = FALSE;


//  DS globals
BOOL dsmOpen = FALSE;
BOOL dsOpen = FALSE;
BOOL dsEnabled = FALSE;
TW_USERINTERFACE guif = { 0 };


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  dwReason, 
                       LPVOID lpReserved
					 )
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		//  Create the necessary events
		//  GetDefaultSrcEvent
		hWaits[GETDEFAULTSRCEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  EnumSrcsEvent
		hWaits[ENUMSRCSEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  EnumPixelTypesEvent
		hWaits[ENUMPIXELTYPESEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  EnumBitDepthsEvent
		hWaits[ENUMBITDEPTHSEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  GetResolutionRangeEvent
		hWaits[GETRESOLUTIONRANGEEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  EnumResolutionsEvent
		hWaits[ENUMRESOLUTIONSEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  TransferPicturesEvent
		hWaits[TRANSFERPICTURESEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  TransferReadyEvent
		hWaits[TRANSFERREADYEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  FnCompleteEvent
		hWaits[FNCOMPLETEEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  TransferDirectDoneEvent
		hWaits[TRANSFERDIRECTDONEEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  EnumCapsEvent
		hWaits[ENUMCAPSEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  EnumPageSizesEvent
		hWaits[ENUMPAGESIZESEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
		//  VerCheckEvent
		hWaits[VERCHECKEVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);

		break;
	case DLL_PROCESS_DETACH:
		for (int i = 0; i < NUM_EVENTS; i++)
		{
			CloseHandle(hWaits[i]);
		}
		break;
	default:
		break;
	}

    return TRUE;
}

BOOL InitializeTwain()
{
	DebugTraceMessage("InitializeTwain (TwainWrapper.cpp)\n");

	hModTwain = LoadLibrary("twain_32.dll");
	if (hModTwain == NULL)
		return FALSE;
	DSMEntry = (DSMENTRYPROC) GetProcAddress(hModTwain, "DSM_Entry");
	if (DSMEntry == NULL)
		return FALSE;

	appid.Id = NULL;
	appid.Version.MajorNum = 1;
	appid.Version.MinorNum = 1;
	appid.Version.Language = TWLG_USA;
	appid.Version.Country = TWCY_USA;
	lstrcpyn(appid.Version.Info, "POC 5.0g", 32);
	appid.ProtocolMajor = TWON_PROTOCOLMAJOR;
	appid.ProtocolMinor = TWON_PROTOCOLMINOR;
	appid.SupportedGroups = DG_IMAGE | DG_CONTROL;
	lstrcpyn(appid.Manufacturer, "EHS", 32);
	lstrcpyn(appid.ProductFamily, "CareRevolution", 32);
	lstrcpyn(appid.ProductName, "EHS Remote Scanning", 32);

	srcds.Id = NULL;

	return TRUE;
}

void ResetTwain(BOOL bReload)
{

	DebugTraceMessage("ResetTwain (TwainWrapper.cpp)\n");

	FreeLibrary(hModTwain);
	DSMEntry = NULL;
	hModTwain = NULL;
	ZeroMemory(&srcds, sizeof(TW_IDENTITY));
	ZeroMemory(&appid, sizeof(TW_IDENTITY));
	ZeroMemory(&guif, sizeof(TW_USERINTERFACE));

	dsmOpen = FALSE;
	dsOpen = FALSE;
	dsEnabled = FALSE;

	if (bReload)
	{
		__Init(hwnd);
	}
}

TW_FIX32 FloatToFIX32(float floater)
{
	TW_FIX32 Fix32_value;
	TW_BOOL sign = (floater < 0)?TRUE:FALSE;
	TW_INT32 value = (TW_INT32) (floater * 65536.0 + (sign?(-0.5):0.5));

	memset(&Fix32_value, 0, sizeof(TW_FIX32));

	Fix32_value.Whole = LOWORD(value >> 16);
	Fix32_value.Frac = LOWORD(value & 0x0000ffffL);

	return (Fix32_value);
}

float FIX32ToFloat (TW_FIX32 fix32)
{
	float   floater = 0;

	floater = (float) fix32.Whole + (float) (fix32.Frac / 65536.0);
	return(floater);
}

int TwainError(TW_UINT16 rc, long lLine, BOOL bSrcError)
{
	try
	{
		char szMessage[300];
		BOOL bGetStatus = FALSE;

		switch (rc)
		{
		case TWRC_SUCCESS:
			break;
		case TWRC_FAILURE:
			sprintf(szMessage, "Line: %d rc = TWRC_FAILURE\n", lLine);
			DebugTraceMessage(szMessage);
			bGetStatus = TRUE;
			break;
		case TWRC_CHECKSTATUS:
			sprintf(szMessage, "Line: %d rc = TWRC_CHECKSTATUS\n", lLine);
			DebugTraceMessage(szMessage);
			bGetStatus = TRUE;
			break;
		case TWRC_CANCEL:
			sprintf(szMessage, "Line: %d rc = TWRC_CANCEL\n", lLine);
			DebugTraceMessage(szMessage);
			break;
		case TWRC_DSEVENT:
			break;
		case TWRC_NOTDSEVENT:
			break;
		case TWRC_XFERDONE:
			break;
		case TWRC_ENDOFLIST:
			break;
		case TWRC_INFONOTSUPPORTED:
			sprintf(szMessage, "Line: %d rc = TWRC_INFONOTSUPPORTED\n", lLine);
			DebugTraceMessage(szMessage);
			bGetStatus = TRUE;
			break;
		case TWRC_DATANOTAVAILABLE:
			sprintf(szMessage, "Line: %d rc = TWRC_DATANOTAVAILABLE\n", lLine);
			DebugTraceMessage(szMessage);
			bGetStatus = TRUE;
			break;
		default:
			break;
		}

		if (bGetStatus)
		{
			TW_STATUS status;

			if (bSrcError)
			{
				DSMEntry(&appid, &srcds, DG_CONTROL, DAT_STATUS, MSG_GET, &status);
			}
			else
			{
				DSMEntry(&appid, NULL, DG_CONTROL, DAT_STATUS, MSG_GET, &status);
			}

			switch (status.ConditionCode)
			{
			case TWCC_SUCCESS:
				DebugTraceMessage("TWCC:  It worked!\n");
				break;
			case TWCC_BUMMER:
				DebugTraceMessage("TWCC:  Failure due to unknown causes\n");
				break;
			case TWCC_LOWMEMORY:
				DebugTraceMessage("TWCC:  Not enough memory to perform operation\n");
				break;
			case TWCC_NODS:
				DebugTraceMessage("TWCC:  No data source\n");
				break;
			case TWCC_MAXCONNECTIONS:
				DebugTraceMessage("TWCC:  DS is connected to max possible applications\n");
				break;
			case TWCC_OPERATIONERROR:
				DebugTraceMessage("TWCC:  DS or DSM reported error, application shouldn't\n");
				break;
			case TWCC_BADCAP:
				DebugTraceMessage("TWCC:  Unknown capability\n");
				break;
			case TWCC_BADPROTOCOL:
				DebugTraceMessage("TWCC:  Unrecognized DG DAT MSG combination\n");
				break;
			case TWCC_BADVALUE:
				DebugTraceMessage("TWCC:  Data parameter out of range\n");
				break;
			case TWCC_SEQERROR:
				DebugTraceMessage("TWCC:  DG DAT MSG out of expected sequence\n");
				break;
			case TWCC_BADDEST:
				DebugTraceMessage("TWCC:  Unknown destination Application/Source in DSM_Entry\n");
				break;
			case TWCC_CAPUNSUPPORTED:
				DebugTraceMessage("TWCC:  Capability not supported by source\n");
				break;
			case TWCC_CAPBADOPERATION:
				DebugTraceMessage("TWCC:  Operation not supported by capability\n");
				break;
			case TWCC_CAPSEQERROR:
				DebugTraceMessage("TWCC:  Capability has dependency on other capability\n");
				break;
			case TWCC_DENIED:
				DebugTraceMessage("TWCC:  File System operation is denied\n");
				break;
			case TWCC_FILEEXISTS:
				DebugTraceMessage("TWCC:  File already exists\n");
				break;
			case TWCC_FILENOTFOUND:
				DebugTraceMessage("TWCC:  File not found\n");
				break;
			case TWCC_NOTEMPTY:
				DebugTraceMessage("TWCC:  Directory not empty\n");
				break;
			case TWCC_PAPERJAM:
				DebugTraceMessage("TWCC:  The feeder is jammed\n");
				break;
			case TWCC_PAPERDOUBLEFEED:
				DebugTraceMessage("TWCC:  The feeder detected multiple pages\n");
				break;
			case TWCC_FILEWRITEERROR:
				DebugTraceMessage("TWCC:  Error writing file\n");
				break;
			case TWCC_CHECKDEVICEONLINE:
				DebugTraceMessage("TWCC:  The device went offline prior to or during this operation\n");
				break;
			default:
				break;
			}

			return status.ConditionCode;
		}
		else
		{
			return 0;
		}
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (TwainError)\n");
		ResetTwain(TRUE);
	}
	return 0xFF;
}

#define CHECKDSERROR(rc) if (rc != TWRC_SUCCESS) TwainError(rc, __LINE__, TRUE)
#define CHECKDSMERROR(rc) if (rc != TWRC_SUCCESS) TwainError(rc, __LINE__, FALSE)

BOOL QueryDeviceOnline(TW_IDENTITY src)
{
	try
	{
		return TRUE;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (QueryDeviceOnline)\n");
	}
}

VOID* GetPixelInfo(const BITMAPINFO* bmpptr)
{
	BITMAPINFOHEADER bmi = bmpptr->bmiHeader;

	if (bmi.biSizeImage == 0)
	{
		bmi.biSizeImage = 
			((((bmi.biWidth * bmi.biBitCount) + 31) & ~31) >> 3) * bmi.biHeight;
	}

	int p = bmi.biClrUsed;
	if ((p == NULL) && (bmi.biBitCount <= 8))
	{
		p = 1 << bmi.biBitCount;
	}

	p = (p * 4) + bmi.biSize + (int) bmpptr;

	return (VOID*) p;
}

//
//  Transition to State 3
//
void __CloseSrc(void)
{
	DebugTraceMessage("TwainWrapper: CloseSrc\n");
	try
	{
		TW_UINT16 rc;

		if (dsEnabled)
		{
			rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_USERINTERFACE, MSG_DISABLEDS, &guif);
			CHECKDSERROR(rc);
			dsEnabled = FALSE;
		}
		if (dsOpen)
		{
			rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_IDENTITY, MSG_CLOSEDS, &srcds);
			CHECKDSMERROR(rc);
			dsOpen = FALSE;
		}

	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (CloseSrc)\n");
		ResetTwain(TRUE);
		dsOpen = FALSE;
	}
}

void __OpenSrc(void)
{
	DebugTraceMessage("TwainWrapper: OpenSrc\n");
	try
	{
		if (dsOpen)
		{
			return;
		}

		TW_UINT16 rc;
		if (srcds.Id == NULL)
		{
			rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, &srcds);
			CHECKDSMERROR(rc);
		}

		rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &srcds);
		CHECKDSMERROR(rc);
		if (rc == TWRC_SUCCESS)
		{
			dsOpen = TRUE;
		}
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (OpenSrc)\n");
		ResetTwain(TRUE);
	}
}

//
//  Transition to State 2
//
void __Finish(void)
{
	DebugTraceMessage("TwainWrapper: Finish\n");
	try
	{
		__CloseSrc();

		TW_UINT16 rc;
		if (appid.Id != NULL)
		{
			if (dsmOpen)
			{
				rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, &hwnd);
				CHECKDSMERROR(rc);
			}
			dsmOpen = FALSE;
			appid.Id = NULL;
		}

		ResetTwain(FALSE);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (Finish)\n");
		ResetTwain(FALSE);
	}
}

//  Transition to State 3
void __Init(HWND hwndp)
{
	DebugTraceMessage("TwainWrapper: Init\n");

	try
	{
		//  clear out any lingering nastiness
		__Finish();

		//  Initialize
		if (!InitializeTwain())
		{
			dsmOpen = FALSE;
			return;
		}

		//  open the data source manager
		TW_UINT16 rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_PARENT, MSG_OPENDSM, &hwndp);
		CHECKDSMERROR(rc);
		if (rc == TWRC_SUCCESS)
		{
			//  set the default data source
			rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, &srcds);
			CHECKDSMERROR(rc);
			if (rc == TWRC_SUCCESS)
			{
				hwnd = hwndp;
				dsmOpen = TRUE;
			}
			else
			{
				rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, &hwndp);
				CHECKDSMERROR(rc);
				dsmOpen = FALSE;
			}
		}
		else
		{
			dsmOpen = FALSE;
		}
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (Init)\n");
	}
}

LPCSTR __GetDefaultSrc()
{
	DebugTraceMessage("TwainWrapper: GetDefaultSrc\n");
	try
	{
		TW_UINT16 rc;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return NULL;
		}

		TW_IDENTITY ds = { 0 };
		rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, &ds);
		CHECKDSMERROR(rc);
		if (rc == TWRC_SUCCESS)
		{
			char* szRetVal = (char*) GlobalAlloc(GPTR, lstrlen(ds.ProductName) + 1);
			lstrcpyn(szRetVal, ds.ProductName, sizeof(TW_STR32));
			return szRetVal;
		}
		else
		{
			return NULL;
		}
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (GetDefaultSource)\n");
		ResetTwain(TRUE);
	}

	return NULL;
}

//  Shows the standard TWAIN source selection dialog
void __SelectSrc(void)
{
	DebugTraceMessage("TwainWrapper: SelectSrc\n");
	try
	{
		TW_UINT16 rc;

		__CloseSrc();

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return;
		}

		rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_IDENTITY, MSG_USERSELECT, &srcds);
		CHECKDSMERROR(rc);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (Select)\n");
		ResetTwain(TRUE);
	}
}

LPSRCLIST __EnumSrcs()
{
	DebugTraceMessage("TwainWrapper: EnumSrcs\n");
	try
	{
		LPSRCLIST ppszSrcs = NULL;
		TW_UINT16 rc;
		TW_IDENTITY ds;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
			{
				return NULL;
			}
		}

		rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &ds);
		CHECKDSMERROR(rc);
		while (rc != TWRC_ENDOFLIST)
		{
			LPSRCLIST lpNew = (LPSRCLIST) GlobalAlloc(GPTR, sizeof(SRCLIST));
			lstrcpyn(lpNew->src, ds.ProductName, sizeof(TW_STR32));
			if (ppszSrcs == NULL)
			{
				ppszSrcs = lpNew;
			}
			else
			{
				LPSRCLIST lpTemp = ppszSrcs;
				while (lpTemp->Next != NULL)
				{
					lpTemp = lpTemp->Next;
				}
				lpTemp->Next = lpNew;
			}
		
			rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &ds);
			CHECKDSMERROR(rc);
		}

		return ppszSrcs;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (EnumSources)\n");
		ResetTwain(TRUE);
	}
	return NULL;
}

void __SetSelectedSource(LPCSTR src)
{
	DebugTraceMessage("TwainWrapper: SetSelectedSource\n");
	try
	{
		TW_UINT16 rc;

		//  Close whatever source, if any, is open...
		__CloseSrc();

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return;
		}

		TW_IDENTITY ds = { 0 };
		rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &ds);
		CHECKDSMERROR(rc);
		while (rc != TWRC_ENDOFLIST)
		{
			if (lstrcmpi(src, ds.ProductName) == 0)
			{
				CopyMemory(&srcds, &ds, sizeof(TW_IDENTITY));
				break;
			}

			rc = DSMEntry(&appid, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &ds);
			CHECKDSMERROR(rc);
		}
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (SetSelectedSource)\n");
		ResetTwain(TRUE);
	}
}

void __SetPixelType(int nPixelType)
{
	DebugTraceMessage("TwainWrapper: SetPixelType\n");
	try
	{
		TW_UINT16 rc;
		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return;
		}

		TW_CAPABILITY cap;
		cap.Cap = ICAP_PIXELTYPE;
		cap.ConType = TWON_ONEVALUE;
		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
		TW_ONEVALUE* twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
		twOne->ItemType = TWTY_UINT16;
		twOne->Item = (short) nPixelType;

		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		CHECKDSERROR(rc);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (SetPixelType)\n");
		ResetTwain(TRUE);
	}
}

void __SetPageSize(int nPageSize)
{
	DebugTraceMessage("TwainWrapper:  __SetPageSize\n");
	try
	{
		TW_UINT16 rc;
		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return;
		}

		TW_CAPABILITY cap;
		cap.Cap = ICAP_SUPPORTEDSIZES;
		cap.ConType = TWON_ONEVALUE;
		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
		TW_ONEVALUE* twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
		twOne->ItemType = TWTY_UINT16;
		twOne->Item = (short) nPageSize;

		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		CHECKDSERROR(rc);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (__SetPageSize)\n");
		ResetTwain(TRUE);
	}
}

void __SetFeederEnabled(BOOL bFeederEnabled)
{
	DebugTraceMessage("TwainWrapper:  __SetFeederEnabled\n");
	try
	{
		TW_UINT16 rc;
		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return;
		}

		TW_CAPABILITY cap;
		cap.Cap = CAP_FEEDERENABLED;
		cap.ConType = TWON_ONEVALUE;
		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
		TW_ONEVALUE* twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
		twOne->ItemType = TWTY_BOOL;
		twOne->Item = (short) bFeederEnabled;

		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		CHECKDSERROR(rc);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (__SetFeederEnabled)\n");
		ResetTwain(TRUE);
	}
}

void __SetDuplex(BOOL bDuplex)
{
	DebugTraceMessage("TwainWrapper:  __SetDuplex\n");
	try
	{
		TW_UINT16 rc;
		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return;
		}

		TW_CAPABILITY cap;
		cap.Cap = CAP_DUPLEXENABLED;
		cap.ConType = TWON_ONEVALUE;
		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
		TW_ONEVALUE* twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
		twOne->ItemType = TWTY_BOOL;
		twOne->Item = (short) bDuplex;

		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		CHECKDSERROR(rc);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (__SetDuplex)\n");
		ResetTwain(TRUE);
	}
}

LPPIXELTYPELIST __EnumPixelTypes(void)
{
	DebugTraceMessage("TwainWrapper: EnumPixelTypes\n");
	try
	{
		LPPIXELTYPELIST lpPixelList = NULL;
		TW_UINT16 rc;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return NULL;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return NULL;
		}

		TW_CAPABILITY cap;
		cap.Cap = ICAP_PIXELTYPE;
		cap.ConType = TWON_DONTCARE16;
		cap.hContainer = NULL;
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_GET, &cap);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			__CloseSrc();
			return NULL;
		}

		TW_ENUMERATION* twEnum = (TW_ENUMERATION*) GlobalLock(cap.hContainer);
		TW_UINT16* pPixel = reinterpret_cast<TW_UINT16*>(&twEnum->ItemList[0]);
		for (unsigned int i = 0; i < twEnum->NumItems; i++)
		{
			LPPIXELTYPELIST lpNew = (LPPIXELTYPELIST) GlobalAlloc(GPTR, sizeof(PIXELTYPELIST));
			lpNew->nType = *pPixel;
			if (lpPixelList == NULL)
			{
				lpPixelList = lpNew;
			}
			else
			{
				LPPIXELTYPELIST lpTemp = lpPixelList;
				while (lpTemp->Next != NULL)
				{
					lpTemp = lpTemp->Next;
				}
				lpTemp->Next = lpNew;
			}
			pPixel++;
		}

		__CloseSrc();

		return lpPixelList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (EnumPixelTypes)\n");
		ResetTwain(TRUE);
	}
	return NULL;
}

void __SetBitDepth(int nBitDepth)
{
	DebugTraceMessage("TwainWrapper: SetBitDepth\n");
	try
	{
		TW_UINT16 rc;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return;
		}

		TW_CAPABILITY cap;
		cap.Cap = ICAP_BITDEPTH;
		cap.ConType = TWON_ONEVALUE;
		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
		TW_ONEVALUE* twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
		twOne->ItemType = TWTY_UINT16;
		twOne->Item = (short) nBitDepth;
		
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		CHECKDSERROR(rc);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (SetBitDepth)\n");
		ResetTwain(TRUE);
	}
}

LPBITDEPTHLIST __EnumBitDepths(int nPixelType)
{
	DebugTraceMessage("TwainWrapper: EnumBitDepths\n");
	try
	{
		LPBITDEPTHLIST lpBitDepthList = NULL;
		TW_UINT16 rc;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return NULL;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return NULL;
		}
		//  firstly, get the current bit depth so we can put it back where it was
		TW_CAPABILITY cap;
		cap.Cap = ICAP_PIXELTYPE;
		cap.ConType = TWON_DONTCARE16;
		cap.hContainer = NULL;
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_GETCURRENT, &cap);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			__CloseSrc();
			return NULL;
		}
		TW_ONEVALUE* twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
		TW_UINT16 CurrentValue = (TW_UINT16) twOne->Item;
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		//  set the source to the desired pixel type
		cap.Cap = ICAP_PIXELTYPE;
		cap.ConType = TWON_ONEVALUE;
		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
		twOne = (TW_ONEVALUE*)GlobalLock(cap.hContainer);
		twOne->Item = nPixelType;
		twOne->ItemType = TWTY_UINT16;
		GlobalUnlock(cap.hContainer);
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
		GlobalFree(cap.hContainer);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			__CloseSrc();
			return NULL;
		}
		//  get the list of available bit depths for this pixel type
		cap.Cap = ICAP_BITDEPTH;
		cap.ConType = TWON_DONTCARE16;
		cap.hContainer = NULL;
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_GET, &cap);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			__CloseSrc();
			return NULL;
		}

		if (cap.ConType == TWON_ENUMERATION)
		{
			TW_ENUMERATION* twEnum = (TW_ENUMERATION*) GlobalLock(cap.hContainer);
			TW_UINT16* pBitDepth = reinterpret_cast<TW_UINT16*>(&twEnum->ItemList[0]);
			for (unsigned int i = 0; i < twEnum->NumItems; i++)
			{
				LPBITDEPTHLIST lpNew = (LPBITDEPTHLIST) GlobalAlloc(GPTR, sizeof(BITDEPTHLIST));
				lpNew->nDepth = *pBitDepth;
				if (lpBitDepthList == NULL)
				{
					lpBitDepthList = lpNew;
				}
				else
				{
					LPBITDEPTHLIST lpTemp = lpBitDepthList;
					while (lpTemp->Next != NULL)
					{
						lpTemp = lpTemp->Next;
					}
					lpTemp->Next = lpNew;
				}
				pBitDepth++;
			}
		}
		else
		{
			TW_ONEVALUE* twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
			TW_UINT16* pBitDepth = reinterpret_cast<TW_UINT16*>(&twOne->Item);
			LPBITDEPTHLIST lpNew = (LPBITDEPTHLIST) GlobalAlloc(GPTR, sizeof(BITDEPTHLIST));
			lpNew->nDepth = *pBitDepth;
			lpBitDepthList = lpNew;
		}

		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		//  put the pixel type back where it was
		cap.Cap = ICAP_PIXELTYPE;
		cap.ConType = TWON_ONEVALUE;
		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
		twOne = (TW_ONEVALUE*)GlobalLock(cap.hContainer);
		twOne->Item = CurrentValue;
		twOne->ItemType = TWTY_UINT16;
		GlobalUnlock(cap.hContainer);
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
		GlobalFree(cap.hContainer);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			DebugTraceMessage("Error resetting ICAP_PIXELTYPE in EnumBitDepths\n");
		}

		__CloseSrc();
		return lpBitDepthList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (EnumBitDepths)\n");
		ResetTwain(TRUE);
	}
	return NULL;
}

void __SetResolution(float fResolution)
{
	DebugTraceMessage("TwainWrapper: SetResolution\n");
	try
	{
		TW_UINT16 rc;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return;
		}
		//  Set the x and y resolutions for this device
		TW_CAPABILITY cap;
		cap.Cap = ICAP_XRESOLUTION;
		cap.ConType = TWON_ONEVALUE;
		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE) + sizeof(TW_FIX32));
		TW_ONEVALUE* twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
		twOne->ItemType = TWTY_FIX32;
		TW_FIX32 twFix = FloatToFIX32(fResolution);
		TW_FIX32* pFix = reinterpret_cast<TW_FIX32*>(&(twOne->Item));
		pFix->Whole = twFix.Whole;
		pFix->Frac = twFix.Frac;

		GlobalUnlock(cap.hContainer);
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
		CHECKDSERROR(rc);
		GlobalFree(cap.hContainer);
		//  now, set the y
		cap.Cap = ICAP_YRESOLUTION;
		cap.ConType = TWON_ONEVALUE;
		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE) + sizeof(TW_FIX32));
		twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
		twOne->ItemType = TWTY_FIX32;
		pFix = reinterpret_cast<TW_FIX32*>(&(twOne->Item));
		pFix->Whole = twFix.Whole;
		pFix->Frac = twFix.Frac;
		
		GlobalUnlock(cap.hContainer);
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
		CHECKDSERROR(rc);
		GlobalFree(cap.hContainer);
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (SetResolution)\n");
		ResetTwain(TRUE);
	}
}

LPRESOLUTIONRANGE __GetResolutionRange()
{
	DebugTraceMessage("TwainWrapper: GetResolutionRange\n");
	try
	{
		LPRESOLUTIONRANGE lpResolutionRange = NULL;
		TW_UINT16 rc;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return NULL;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return NULL;
		}
		//  get the range of X resolutions for this device
		TW_CAPABILITY cap;
		cap.Cap = ICAP_XRESOLUTION;
		cap.ConType = TWON_DONTCARE16;
		cap.hContainer = NULL;
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_GET, &cap);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			__CloseSrc();
			return NULL;
		}
		//  make sure we got a TW_RANGE back
		if (cap.ConType != TWON_RANGE)
		{
			GlobalFree(cap.hContainer);
			__CloseSrc();
			return NULL;
		}
		TW_RANGE* twRange = (TW_RANGE*) GlobalLock(cap.hContainer);
		LPRESOLUTIONRANGE pRange = (LPRESOLUTIONRANGE) GlobalAlloc(GPTR, sizeof(RESOLUTIONRANGE));
		TW_FIX32* pFix;

		pFix = reinterpret_cast<TW_FIX32*>(&(twRange->CurrentValue));
		pRange->CurrentValueWhole = pFix->Whole;
		pRange->CurrentValueFrac = pFix->Frac;
		pFix = reinterpret_cast<TW_FIX32*>(&(twRange->DefaultValue));
		pRange->DefaultValueWhole = pFix->Whole;
		pRange->DefaultValueFrac = pFix->Frac;
		pFix = reinterpret_cast<TW_FIX32*>(&(twRange->MaxValue));
		pRange->MaxValueWhole = pFix->Whole;
		pRange->MaxValueFrac = pFix->Frac;
		pFix = reinterpret_cast<TW_FIX32*>(&(twRange->MinValue));
		pRange->MinValueWhole = pFix->Whole;
		pRange->MaxValueFrac = pFix->Frac;
		pFix = reinterpret_cast<TW_FIX32*>(&(twRange->StepSize));
		pRange->StepSizeWhole = pFix->Whole;
		pRange->StepSizeFrac = pFix->Frac;

		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);

		return pRange;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (GetResolutionRange)\n");
		ResetTwain(TRUE);
	}
	return NULL;
}

LPRESOLUTIONLIST __EnumResolutions()
{
	DebugTraceMessage("TwainWrapper: EnumResolutions\n");
	try
	{
		LPRESOLUTIONLIST lpResolutionList = NULL;
		TW_UINT16 rc;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return NULL;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return NULL;
		}
		//  get the list of available X resolutions for this device
		TW_CAPABILITY cap;
		cap.Cap = ICAP_XRESOLUTION;
		cap.ConType = TWON_DONTCARE16;
		cap.hContainer = NULL;
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_GET, &cap);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			__CloseSrc();
			return NULL;
		}
		//  make sure we got a TW_ENUMERATION back
		if (cap.ConType != TWON_ENUMERATION)
		{
			GlobalFree(cap.hContainer);
			__CloseSrc();
			return NULL;
		}
		TW_ENUMERATION* twEnum = (TW_ENUMERATION*) GlobalLock(cap.hContainer);
		TW_FIX32* pRes = reinterpret_cast<TW_FIX32*>(&twEnum->ItemList[0]);
		for (unsigned int i = 0; i < twEnum->NumItems; i++)
		{
			LPRESOLUTIONLIST lpNew = (LPRESOLUTIONLIST) GlobalAlloc(GPTR, sizeof(RESOLUTIONLIST));
			lpNew->Whole = pRes->Whole;
			lpNew->Fraction = pRes->Frac;
			if (lpResolutionList == NULL)
			{
				lpResolutionList = lpNew;
			}
			else
			{
				LPRESOLUTIONLIST lpTemp = lpResolutionList;
				while (lpTemp->Next != NULL)
				{
					lpTemp = lpTemp->Next;
				}
				lpTemp->Next = lpNew;
			}
			pRes++;
		}
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		__CloseSrc();
		return lpResolutionList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (EnumResolutions)\n");
		ResetTwain(TRUE);
	}
	return NULL;
}

LPCAPLIST __EnumCapabilities(void)
{
	DebugTraceMessage("TwainWrapper: __EnumCapabilities\n");
	try
	{
		LPCAPLIST lpCapList = NULL;
		TW_UINT16 rc;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return NULL;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return NULL;
		}

		//  get list of available capabilities for this source
		TW_CAPABILITY cap;
		cap.Cap = CAP_SUPPORTEDCAPS;
		cap.ConType = TWON_DONTCARE16;
		cap.hContainer = NULL;
		
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_GET, &cap);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			__CloseSrc();
			return NULL;
		}

		//  make sure we got a TW_ARRAY back
		if ((cap.ConType != TWON_ARRAY) && (cap.ConType != TWON_ENUMERATION))
		{
			OutputDebugString("***Unknown container type in __EnumCapabilities\n");
			GlobalFree(cap.hContainer);
			__CloseSrc();
			return NULL;
		}

		unsigned int nItems = 0;
		TW_UINT16* pCap = NULL;
		if (cap.ConType == TWON_ARRAY)
		{
			TW_ARRAY* twArr = (TW_ARRAY*) GlobalLock(cap.hContainer);
			nItems = twArr->NumItems;
			pCap = reinterpret_cast<TW_UINT16*>(&twArr->ItemList[0]);
		}
		else
		{
			TW_ENUMERATION* twEnum = (TW_ENUMERATION*) GlobalLock(cap.hContainer);
			nItems = twEnum->NumItems;
			pCap = reinterpret_cast<TW_UINT16*>(&twEnum->ItemList[0]);
		}

		char szLoopMsg[256];
		for (unsigned int i = 0; i < nItems; i++)
		{
			sprintf(szLoopMsg, "%d.) Capability value = 0x%x\n", i + 1, *pCap);
			OutputDebugString(szLoopMsg);

			LPCAPLIST lpNew = (LPCAPLIST) GlobalAlloc(GPTR, sizeof(CAPLIST));

			//  check for sheet-fed scanner
//			if (*pCap == CAP_FEEDERENABLED)
//			{
//				TW_CAPABILITY capFeeder;
////				GlobalFree(cap.hContainer);
//				capFeeder.Cap = CAP_FEEDERENABLED;
//				capFeeder.ConType = TWON_ONEVALUE;
//				capFeeder.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE) + sizeof(TW_BOOL));
//				TW_ONEVALUE* twOne = (TW_ONEVALUE*) GlobalLock(capFeeder.hContainer);
//				twOne->ItemType = TWTY_BOOL;
//				twOne->Item = FALSE;
//				GlobalUnlock(capFeeder.hContainer);
//				rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
//				CHECKDSERROR(rc);
//				GlobalFree(capFeeder.hContainer);
//				
//				if (rc != TWRC_SUCCESS)
//				{
//					//  if we fail setting CAP_FEEDERENABLED to FALSE
//					//  then we must have a sheet-fed scanner, so exclude
//					//  this capability from the list
//					pCap++;
//					continue;
//				}
//			}

			lpNew->nCap = *pCap;
			if (lpCapList == NULL)
			{
				lpCapList = lpNew;
			}
			else
			{
				LPCAPLIST lpTemp = lpCapList;
				while (lpTemp->Next != NULL)
				{
					lpTemp = lpTemp->Next;
				}
				lpTemp->Next = lpNew;
			}
			pCap++;
		}
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		__CloseSrc();

		return lpCapList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (EnumCapabilities)\n");
		ResetTwain(TRUE);
	}
	return NULL;
}

LPPAGESIZELIST __EnumPageSizes()
{
	DebugTraceMessage("TwainWrapper: __EnumPageSizes\n");
	try
	{
		LPPAGESIZELIST lpPageSizeList = NULL;
		TW_UINT16 rc;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return NULL;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return NULL;
		}

		//  get list of available page sizes for this source
		TW_CAPABILITY cap;
		cap.Cap = ICAP_SUPPORTEDSIZES;
		cap.ConType = TWON_DONTCARE16;
		cap.hContainer = NULL;
		
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_GET, &cap);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			__CloseSrc();
			return NULL;
		}

		//  make sure we got a TW_ARRAY back
		if ((cap.ConType != TWON_ARRAY) && (cap.ConType != TWON_ENUMERATION))
		{
			OutputDebugString("***Unknown container type in __EnumPageSizes\n");
			GlobalFree(cap.hContainer);
			__CloseSrc();
			return NULL;
		}

		unsigned int nItems = 0;
		TW_UINT16* pSize = NULL;
		if (cap.ConType == TWON_ARRAY)
		{
			TW_ARRAY* twArr = (TW_ARRAY*) GlobalLock(cap.hContainer);
			nItems = twArr->NumItems;
			pSize = reinterpret_cast<TW_UINT16*>(&twArr->ItemList[0]);
		}
		else
		{
			TW_ENUMERATION* twEnum = (TW_ENUMERATION*) GlobalLock(cap.hContainer);
			nItems = twEnum->NumItems;
			pSize = reinterpret_cast<TW_UINT16*>(&twEnum->ItemList[0]);
		}

		for (unsigned int i = 0; i < nItems; i++)
		{
			LPPAGESIZELIST lpNew = (LPPAGESIZELIST) GlobalAlloc(GPTR, sizeof(PAGESIZELIST));
			lpNew->nPageSize = *pSize;
			if (lpPageSizeList == NULL)
			{
				lpPageSizeList = lpNew;
			}
			else
			{
				LPPAGESIZELIST lpTemp = lpPageSizeList;
				while (lpTemp->Next != NULL)
				{
					lpTemp = lpTemp->Next;
				}
				lpTemp->Next = lpNew;
			}
			pSize++;
		}
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		__CloseSrc();

		return lpPageSizeList;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (EnumPageSizes)\n");
		ResetTwain(TRUE);
	}
	return NULL;
}

void __Acquire(void)
{
	DebugTraceMessage("TwainWrapper: __Acquire\n");
	try
	{
		TW_UINT16 rc;

		if (!dsmOpen)
		{
			__Init(hwnd);
			if (!dsmOpen)
				return;
		}

		if (!dsOpen)
		{
			__OpenSrc();
			if (!dsOpen)
				return;
		}

		TW_CAPABILITY cap;
		cap.Cap = CAP_XFERCOUNT;
		cap.ConType = TWON_ONEVALUE;
		cap.hContainer = GlobalAlloc(GHND, sizeof(TWON_ONEVALUE));
		TW_ONEVALUE* twOne = (TW_ONEVALUE*) GlobalLock(cap.hContainer);
		twOne->ItemType = TWTY_INT16;
		twOne->Item = -1;
		GlobalUnlock(cap.hContainer);
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &cap);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			__CloseSrc();
			return;
		}

		memset(&guif, 0, sizeof(TW_USERINTERFACE));
		guif.ShowUI = nShowUI;
		guif.ModalUI = nShowUI;
		guif.hParent = hwnd;
		rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDS, 
			(TW_MEMREF)&guif);
		CHECKDSERROR(rc);
		if (rc != TWRC_SUCCESS)
		{
			__CloseSrc();
			bAcquireCancelled = TRUE;
			return;
		}
		else
		{
			dsEnabled = TRUE;
		}
		
		nShowUI = 0;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (Acquire)\n");
		ResetTwain(TRUE);
	}
}

typedef GpStatus (WINGDIPAPI *FNGdipCreateBitmapFromGdiDib)(GDIPCONST BITMAPINFO* gdiBitmapInfo, 
		VOID* gdiBitmapData, GpBitmap** bitmap);

typedef GpStatus (WINGDIPAPI *FNGdipSaveImageToFile)(GpImage *image, GDIPCONST WCHAR* filename, 
		GDIPCONST CLSID* clsidEncoder, GDIPCONST EncoderParameters* encoderParams);

typedef GpStatus (WINGDIPAPI *FNGdipDisposeImage)(GpImage *image);

LPIMGLIST __TransferPictures()
{
	DebugTraceMessage("TwainWrapper: TransferPictures\n");
	try
	{
		char szTrace[512];
		LPIMGLIST pics = NULL;

		if (!dsEnabled)
		{
			OutputDebugString("DS not enabled!\n");
			return NULL;
		}

		TW_UINT16 rc;
		HBITMAP hbitmap = NULL;
		TW_PENDINGXFERS pxfr;

		//  Initialize GDI+
		GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		//  GET THE GDI+ FUNCTIONS
		HMODULE hMod = LoadLibrary("gdiplus.dll");
		if (hMod == NULL)
		{
			MessageBox(NULL, "GDIPLUS.DLL not found.", "GDIPLUS Error", MB_OK | MB_ICONSTOP);
			OutputDebugString("GDIPLUS.DLL not found...are we on Windows 2000?\n");
			goto ErrorExit;
		}

		FNGdipCreateBitmapFromGdiDib GdipCreateBitmapFromGdiDib = 
			(FNGdipCreateBitmapFromGdiDib) GetProcAddress(hMod, "GdipCreateBitmapFromGdiDib");
		FNGdipSaveImageToFile GdipSaveImageToFile = 
			(FNGdipSaveImageToFile) GetProcAddress(hMod, "GdipSaveImageToFile");
		FNGdipDisposeImage GdipDisposeImage = 
			(FNGdipDisposeImage) GetProcAddress(hMod, "GdipDisposeImage");

		//  everything is ducky up to this point
		pxfr.Count = 0;
		do
		{
			OutputDebugString("Looping for pictures...\n");
//			pxfr.Count = 0;
			hbitmap = NULL;

			TW_IMAGEINFO iinf;
			rc = DSMEntry(&appid, &srcds, DG_IMAGE, DAT_IMAGEINFO, MSG_GET, &iinf);
			CHECKDSERROR(rc);
			if (rc != TWRC_SUCCESS)
			{
				OutputDebugString("DAT_IMAGEINFO error\n");
				__CloseSrc();
				goto ErrorExit;
			}

			rc = DSMEntry(&appid, &srcds, DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, &hbitmap);
			if (hbitmap == NULL)
			{
				OutputDebugString("DAT_IMAGENATIVEXFER returned NULL hbitmap\n");
			}

			CHECKDSERROR(rc);
			if (rc != TWRC_XFERDONE)
			{
				OutputDebugString("DAT_IMAGENATIVEXFER error\n");
				__CloseSrc();
				goto ErrorExit;
			}

			rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, &pxfr);
			sprintf(szTrace, "Remaining transfers: %d\n", pxfr.Count);
			OutputDebugString(szTrace);

			CHECKDSERROR(rc);
			if (rc != TWRC_SUCCESS)
			{
				OutputDebugString("DAT_PENDINGXFERS error\n");
				__CloseSrc();
				goto ErrorExit;
			}
///////////////////////////////////////////////////////////////////////////////
			//  Create a GDI+ bitmap from the BITMAPINFO
			BITMAPINFO* bmpptr = (BITMAPINFO*) GlobalLock(hbitmap);
			VOID* pixptr = GetPixelInfo(bmpptr);
			GlobalUnlock(hbitmap);
				
			GpBitmap* bmp = NULL;
			GpStatus gpStat = GdipCreateBitmapFromGdiDib(bmpptr, pixptr, &bmp);
			if (gpStat != Ok)
			{
				sprintf(szTrace, "GdipCreateBitmapFromGdiDib failed with status: %d\n",
					(int) gpStat);
				OutputDebugString(szTrace);

			}

			
			//  get the JPEG encoder CLSID
			CLSID jpgClsid;
			ImageCodecInfo* pImageCodecInfo;
			UINT num;
			UINT size;
			GetImageEncodersSize(&num, &size);
			pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
			GetImageEncoders(num, size, pImageCodecInfo);
			BOOL bFound = FALSE;
			for (unsigned int i = 0; i < num; i++)
			{
				if (lstrcmpiW(L"image/jpeg", pImageCodecInfo[i].MimeType) == 0)
				{
					jpgClsid = pImageCodecInfo[i].Clsid;
					bFound = TRUE;
					break;
				}
			}
			if (!bFound)
			{
				OutputDebugString("Failed to find image/jpeg encoder!!!\n");
			}

			free(pImageCodecInfo);
			//  save the image to a temp file
			DWORD dwResult;
			char szTempPath[MAX_PATH];
			dwResult = GetTempPath(MAX_PATH, szTempPath);
			sprintf(szTrace, "TEMP PATH: %s\n", szTempPath);
			OutputDebugString(szTrace);

			char szTempFile[MAX_PATH];
			WCHAR wszTempFile[MAX_PATH];

			dwResult = GetTempFileName(szTempPath, "SCN", 0, szTempFile);
			sprintf(szTrace, "TEMP FILE: %s\n", szTempFile);
			OutputDebugString(szTrace);

			MultiByteToWideChar(CP_ACP, 0, szTempFile,
				strlen(szTempFile)+1, wszTempFile,
				sizeof(wszTempFile)/sizeof(wszTempFile[0]));

			EncoderParameters encoderParameters;
			ULONG quality;

			encoderParameters.Count = 1;
			encoderParameters.Parameter[0].Guid = EncoderQuality;
			encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
			encoderParameters.Parameter[0].NumberOfValues = 1;
			quality = 75L;
			encoderParameters.Parameter[0].Value = &quality;

			gpStat = GdipSaveImageToFile(bmp, wszTempFile, &jpgClsid, &encoderParameters);
			if (gpStat != Ok)
			{
				sprintf(szTrace, "GdipSaveImageToFile failed with status: %d\n",
					(int) gpStat);
				OutputDebugString(szTrace);
			}

			
			gpStat = GdipDisposeImage(bmp);
			if (gpStat != Ok)
			{
				sprintf(szTrace, "GdipDisposeImage failed with status: %d\n",
					(int) gpStat);
				OutputDebugString(szTrace);
			}

			GlobalFree(hbitmap);

			HANDLE hFile = CreateFile(szTempFile, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if ((hFile == INVALID_HANDLE_VALUE) || (hFile == NULL))
			{
				sprintf(szTrace, "CreateFile failed for file: %s\n", szTempFile);
				OutputDebugString(szTrace);
				SystemError();
				goto ErrorExit;
			}

			DWORD dwSize = GetFileSize(hFile, NULL);
			sprintf(szTrace, "File size is: %d for file: %s\n",
				dwSize, szTempFile);
			OutputDebugString(szTrace);
			BYTE* cbBuffer = (BYTE*) GlobalAlloc(GPTR, dwSize);
			if (cbBuffer == NULL)
			{
				OutputDebugString("GlobalAlloc failed for cbBuffer\n");


				SystemError();
				goto ErrorExit;
			}

			DWORD dwBytesRead = 0;
			if (!ReadFile(hFile, cbBuffer, dwSize, &dwBytesRead, NULL))
			{
				OutputDebugString("ReadFile failed\n");
				SystemError();
				goto ErrorExit;
			}
			sprintf(szTrace, "%d bytes read from file\n", dwBytesRead);
			OutputDebugString(szTrace);

			CloseHandle(hFile);

			//  COMMENT OUT DELETE FILE
			//if (!DeleteFile(szTempFile))
			//{
			//	SystemError();
			//}

///////////////////////////////////////////////////////////////////////////////
			LPIMGLIST lpNew = (LPIMGLIST) GlobalAlloc(GPTR, sizeof(IMGLIST));
			if (lpNew == NULL)
			{
				OutputDebugString("GlobalAlloc failed for IMGLIST\n");
			}

			lpNew->img = cbBuffer;
			lpNew->nImageSize = (int) dwSize;
			if (pics == NULL)
			{
				pics = lpNew;
			}
			else
			{
				LPIMGLIST lpTemp = pics;
				while (lpTemp->Next != NULL)
				{
					lpTemp = lpTemp->Next;
				}
				lpTemp->Next = lpNew;
			}

		} while (pxfr.Count != 0);

ErrorExit:
		//  Shutdown GDI+
		GdiplusShutdown(gdiplusToken);

		__Finish();

		OutputDebugString("Exiting _TransferPictures\n");
		if (pics == NULL)
		{
			OutputDebugString("_TransferPictures returning NULL\n");
		}

		return pics;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (TransferPictures)\n");
		ResetTwain(TRUE);
	}
	return NULL;
}

int __PassMessage(LPMSG msg)
{
//	DebugTraceMessage("TwainWrapper: PassMessage\n");
	try
	{
		if (!dsmOpen)
			return 0;

		if (!dsOpen)
			return 0;

		if (srcds.Id == NULL)
			return 0;

		//  Modify the MSG
		MSG newMsg;
		int pos = GetMessagePos();
		newMsg.hwnd = msg->hwnd;
		newMsg.message = msg->message;
		newMsg.wParam = msg->wParam;
		newMsg.lParam = msg->wParam;
		newMsg.time = GetMessageTime();
		newMsg.pt.x = pos;
		newMsg.pt.y = pos >> 16;

		memset(&evtmsg, 0, sizeof(TW_EVENT));
		evtmsg.pEvent = &newMsg;
		evtmsg.TWMessage = NULL;
		TW_UINT16 rc = DSMEntry(&appid, &srcds, DG_CONTROL, DAT_EVENT, MSG_PROCESSEVENT, 
			(TW_MEMREF)&evtmsg);
		CHECKDSERROR(rc);
		if (rc == TWRC_NOTDSEVENT)
			return -1;

		if (evtmsg.TWMessage == MSG_XFERREADY)
		{
			DebugTraceMessage("MSG_XFERREADY received\n");
			SetEvent(hWaits[TRANSFERREADYEVENT]);
			return 1;
		}
		if (evtmsg.TWMessage == MSG_CLOSEDSREQ)
			return 2;
		if (evtmsg.TWMessage == MSG_CLOSEDSOK)
			return 3;
		if (evtmsg.TWMessage == MSG_DEVICEEVENT)
			return 4;

		return 0;
	}
	catch (...)
	{
		DebugTraceMessage("Exception in TwainWrapper.dll (PassMessage)\n");
		ResetTwain(TRUE);
	}
	return 0;
}