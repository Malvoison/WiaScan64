// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS

#pragma warning(disable:4005)

// Windows Header Files:
#include <windows.h>
#include <atlbase.h>
#include <objbase.h>
#include <atlwin.h>
#include <atlcom.h>
#include <wtsapi32.h>
#include <lmcons.h>
#include <pchannel.h>
#include <cchannel.h>
#include <tsvirtualchannels.h>

//  Globals
extern HINSTANCE g_hInstance;

//  #defines and macros
