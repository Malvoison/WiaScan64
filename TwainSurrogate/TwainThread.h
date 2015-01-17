//  Filename:  TwainThread.h

#pragma once

//  FN call command defines
#define PM_CLOSESRC				WM_APP + 0x0001
#define PM_OPENSRC				WM_APP + 0x0002
#define PM_FINISH				WM_APP + 0x0003
#define PM_INIT					WM_APP + 0x0004
#define PM_GETDEFAULTSOURCE		WM_APP + 0x0005
#define PM_SELECTSRC			WM_APP + 0x0006
#define PM_ENUMSRCS				WM_APP + 0x0007
#define PM_SETSELECTEDSOURCE	WM_APP + 0x0008
#define PM_SETPIXELTYPE			WM_APP + 0x0009
#define PM_ENUMPIXELTYPES		WM_APP + 0x000A
#define PM_SETBITDEPTH			WM_APP + 0x000B
#define PM_ENUMBITDEPTHS		WM_APP + 0x000C
#define PM_SETRESOLUTION		WM_APP + 0x000D
#define PM_GETRESOLUTIONRANGE	WM_APP + 0x000E
#define PM_ENUMRESOLUTIONS		WM_APP + 0x000F
#define PM_ACQUIRE				WM_APP + 0x0010
#define PM_TRANSFERPICTURES		WM_APP + 0x0011
#define PM_RELEASE				WM_APP + 0x0012
#define PM_SHOWUI				WM_APP + 0x0013
#define PM_ENUMCAPS				WM_APP + 0x0014
#define PM_ENUMPAGESIZES		WM_APP + 0x0015
#define PM_SETPAGESIZE			WM_APP + 0x0016
#define PM_SETFEEDERENABLED		WM_APP + 0x0017
#define PM_SETDUPLEX			WM_APP + 0x0018

#define PM_PING					WM_APP + 0x0100
#define PM_PICSTART				WM_APP + 0x0200
#define PM_PICDATA				WM_APP + 0x0201
#define PM_PICDONE				WM_APP + 0x0202
#define PM_TRANSFERDONE			WM_APP + 0x0203
#define PM_TRANSFERREADY		WM_APP + 0x0204
#define PM_FNCOMPLETE			WM_APP + 0x0205


#define PM_ACQUIREDIRECT			WM_APP + 0x0206
#define PM_TRANSFERDIRECTPICTURES	WM_APP + 0x0207
#define PM_TRANSFERDIRECTREADY		WM_APP + 0x0208
#define PM_TRANSFERDIRECTDONE		WM_APP + 0x0209

#define PM_ACKPICDATA			WM_APP + 0x020A

#define PM_VERCHECK				WM_APP + 0x0300



//  global objects
extern HANDLE hTwainThreadReadyEvent;
extern HANDLE hTwainThreadExitEvent;
extern HANDLE hMtxGlobals;
extern HANDLE hXferReady;

extern int nShowUI;
extern BOOL bAcquireCancelled;

//  used with GetDefaultSrc
extern TW_STR32 szDefaultSrc;
//  used with EnumSrcs
extern LPSRCLIST lpSrcList;
//  used with SetSelectedSrc
extern TW_STR32 szSelectedSrc;
//  used with SetPixelType
extern int nPixelType;
//  used with EnumPixelTypes
extern LPPIXELTYPELIST lpPixelTypeList;
//  used with SetBitDepth
extern int nBitDepth;
//  used with EnumBitDepths
extern LPBITDEPTHLIST lpBitDepthList;
//  used with SetResolution
extern float fResolution;
//  used with GetResolutionRange
extern LPRESOLUTIONRANGE lpResolutionRange;
//  used with EnumResolutions
extern LPRESOLUTIONLIST lpResolutionList;
//  used with TransferPictures
extern LPIMGLIST lpImageList;
//  used with EnumCapabilities
extern LPCAPLIST lpCapList;
//  used with EnumPageSizes
extern LPPAGESIZELIST lpPageSizeList;
//  used with SetPageSize
extern int nPageSize;
//  used with SetFeederEnabled
extern BOOL bFeederEnabled;
//  used with SetDuplex
extern BOOL bDuplex;





//  Function signatures
DWORD WINAPI TwainThreadProc(PVOID pvParam);