//  Filename:  WiaScanServerHelper.cpp

#include "stdafx.h"

typedef BOOL (__cdecl * FNTESTCHANNELAVAILABLE)(LPCSTR);
typedef void (__cdecl * FNINITIALIZEWIACHANNELHANDLERS)(HWND);
typedef BOOL (__cdecl * FNSENDSERVERCOMMAND)(int);
typedef void (__cdecl * FNRELEASEWIACHANNELHANDLERS)();

static HMODULE hModWiaScanServerHelper = NULL;
FNTESTCHANNELAVAILABLE TestChannelAvailable;
FNINITIALIZEWIACHANNELHANDLERS InitializeWiaChannelHandlers;
FNSENDSERVERCOMMAND SendServerCommand;
//extern "C" void __declspec(dllexport) ReleaseWiaChannelHandlers()
FNRELEASEWIACHANNELHANDLERS ReleaseWiaChannelHandlers;

BOOL InitializeWiaScanServerHelper()
{
	BOOL bResult = FALSE;
	hModWiaScanServerHelper = LoadLibrary(L"WiaScanServerHelper.dll");
	if (hModWiaScanServerHelper != NULL)
	{
		TestChannelAvailable = (FNTESTCHANNELAVAILABLE)GetProcAddress(hModWiaScanServerHelper, "TestChannelAvailable");
		InitializeWiaChannelHandlers = (FNINITIALIZEWIACHANNELHANDLERS)GetProcAddress(hModWiaScanServerHelper, "InitializeWiaChannelHandlers");
		SendServerCommand = (FNSENDSERVERCOMMAND)GetProcAddress(hModWiaScanServerHelper, "SendServerCommand");
		ReleaseWiaChannelHandlers = (FNRELEASEWIACHANNELHANDLERS)GetProcAddress(hModWiaScanServerHelper, "ReleaseWiaChannelHandlers");
	}

	return bResult;
}

BOOL TestChannelAvailableWrapper(LPCSTR szChannel)
{
	if (hModWiaScanServerHelper == NULL)
	{
		InitializeWiaScanServerHelper();
	}

	return TestChannelAvailable(szChannel);
}

BOOL SendServerCommandWrapper(int nCmd)
{
	if (hModWiaScanServerHelper == NULL)
	{
		InitializeWiaScanServerHelper();
	}

	return SendServerCommand(nCmd);
}

void InitializeWiaChannelHandlersWrapper(HWND hwnd)
{
	if (hModWiaScanServerHelper == NULL)
	{
		InitializeWiaScanServerHelper();
	}

	InitializeWiaChannelHandlers(hwnd);
}

void ReleaseWiaChannelHandlersWrapper()
{
	if (hModWiaScanServerHelper == NULL)
	{
		InitializeWiaScanServerHelper();
	}

	ReleaseWiaChannelHandlers();
}

