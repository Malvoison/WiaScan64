//  Filename:  TwainDispatch.h

#pragma once

//  Globals

DWORD WINAPI CloseSrcProc(LPVOID lpParameter);
DWORD WINAPI OpenSrcProc(LPVOID lpParameter);
DWORD WINAPI FinishProc(LPVOID lpParameter);
DWORD WINAPI InitTwainProc(LPVOID lpParameter);
DWORD WINAPI ReleaseTwainProc(LPVOID lpParameter);
DWORD WINAPI GetDefaultSrcProc(LPVOID lpParameter);
DWORD WINAPI SelectSrcProc(LPVOID lpParameter);
DWORD WINAPI EnumSrcsProc(LPVOID lpParameter);
DWORD WINAPI SetSelectedSourceProc(LPVOID lpParameter);
DWORD WINAPI SetPixelTypeProc(LPVOID lpParameter);
DWORD WINAPI EnumPixelTypesProc(LPVOID lpParameter);
DWORD WINAPI SetBitDepthProc(LPVOID lpParameter);
DWORD WINAPI EnumBitDepthsProc(LPVOID lpParameter);
DWORD WINAPI SetResolutionProc(LPVOID lpParameter);
DWORD WINAPI GetResolutionRangeProc(LPVOID lpParameter);
DWORD WINAPI EnumResolutionsProc(LPVOID lpParameter);
DWORD WINAPI AcquireProc(LPVOID lpParameter);
DWORD WINAPI TransferPicturesProc(LPVOID lpParameter);

extern "C" void __declspec(dllexport) DataArrival(LPBYTE pBuf, USHORT usLength);
extern "C" LPBYTE __declspec(dllexport) Poll();

#define GETDEFAULTSRCEVENT		0
#define ENUMSRCSEVENT			1
#define ENUMPIXELTYPESEVENT		2
#define ENUMBITDEPTHSEVENT		3
#define	GETRESOLUTIONRANGEEVENT 4
#define	ENUMRESOLUTIONSEVENT	5
#define	TRANSFERPICTURESEVENT	6
#define TRANSFERREADYEVENT		7
#define FNCOMPLETEEVENT			8
#define TRANSFERDIRECTDONEEVENT 9
#define ENUMCAPSEVENT			10
#define ENUMPAGESIZESEVENT		11
#define VERCHECKEVENT			12

#define NUM_EVENTS 13

extern HANDLE hWaits[NUM_EVENTS];

#define SCAN_VERSION			3