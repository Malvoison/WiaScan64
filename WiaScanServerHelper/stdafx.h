// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <WtsApi32.h>
#include <pchannel.h>
#include <crtdbg.h>

#pragma comment(lib, "wtsapi32.lib")

#define _MAX_WAIT       60000
#define MAX_MSG_SIZE    0x20000
#define START_MSG_SIZE  4
#define STEP_MSG_SIZE   113

#define PM_PING					WM_APP + 0x0100
#define PM_WIATRANSFERDONE		WM_APP + 0x0700
#define PM_INIT					WM_APP + 0x0004
#define PM_ACQUIRE				WM_APP + 0x0010
#define PM_RELEASE				WM_APP + 0x0012
#define PM_SETPREFERENCES       WM_APP + 0x0402 
#define PM_TRANSFERDONE			WM_APP + 0x0203


#define CHANNELNAME_SERVERCOMMAND	"SRVCMD"
#define CHANNELNAME_CLIENTINFO		"CLNTNOT"
#define CHANNELNAME_IMAGEXFER1		"XFER1"
#define CHANNELNAME_IMAGEXFER2		"XFER2"
#define CHANNELNAME_IMAGEXFER3		"XFER3"
#define CHANNELNAME_IMAGEXFER4		"XFER4"

