#pragma once

//  FN call command defines
#define PM_INIT					WM_APP + 0x0001
#define PM_SENDDATA				WM_APP + 0x0002

#define PM_PING					WM_APP + 0x0100
#define PM_DATASENT				WM_APP + 0x0200

#define PM_VERCHECK				WM_APP + 0x0300

#define PM_LOCALLAUNCH			WM_APP = 0x0401

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

#define SCAN_VERSION			4

LPWSTR TranslateCommand(int nCmd);
LPBYTE Poll();

void DataArrivalServerCommand(LPBYTE pBuf, USHORT usLength);
