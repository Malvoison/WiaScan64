//  Filename:  Update.cpp

#include "stdafx.h"

static DWORD dwUpdateThreadId = 0;
static HANDLE hUpdateThreadHandle = NULL;

DWORD WINAPI GetFileVersionProc(LPVOID lpvParam)
{
    LPWSTR lpVersion = NULL;
    CheckFileVersionInfo(&lpVersion);

    gpEntryPoints->pVirtualChannelWrite(gdwOpenChannelUpdate, lpVersion, (lstrlen(lpVersion) *  sizeof(TCHAR)), lpVersion);

    return 0;
}

DWORD WINAPI UpdateVersionProc(LPVOID lpvParam)
{
    //  Perform useful tasks related to updating

    return 0;
}

void DataArrivalUpdate(LPBYTE pBuf, USHORT usLength)
{
    LPTHREAD_START_ROUTINE fnUpdate = NULL;
    LPVOID lpvParam = NULL;

    int* pCmd = (int*) pBuf;

    switch (*pCmd)
	{
	case PM_GETVERSION:
        fnUpdate = GetFileVersionProc;
        break;

	case PM_UPDATEVERSION:
        fnUpdate = UpdateVersionProc;
        lpvParam = pBuf;
        break;
	default:
        break;
	}

    if (fnUpdate != NULL)
	{
        hUpdateThreadHandle = CreateThread(NULL, 0, fnUpdate, lpvParam, 0, 
            &dwUpdateThreadId);
	}
}