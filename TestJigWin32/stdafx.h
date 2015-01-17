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

#pragma comment(lib, "wtsapi32.lib")

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#include "LocalDBWin32.h"
#include "WiaScanServerHelper.h"

//  Macros
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


