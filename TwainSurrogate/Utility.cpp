//  Filename:  Utility.cpp

#include "stdafx.h"
#include "Utility.h"

#define MAXDEBUGSTRING	512

void DebugTraceMessage(LPCSTR lpszFormat, ...)
{
	/*
	* duplicate printf functionality for DebugTraceMessage
	*/
//#ifdef _DEBUG
	char *pszDebugString = (char*) malloc(MAXDEBUGSTRING);
	va_list args;
	va_start(args, lpszFormat);
	
	if(pszDebugString)
	{
		/*
		* format and output the debug string
		*/
		//  prepend the current thread ID
		char szThreadFormat[MAXDEBUGSTRING];
		sprintf(szThreadFormat, "(Thrd ID: 0x%x) ", GetCurrentThreadId());
		lstrcat(szThreadFormat, lpszFormat);
		vsprintf(pszDebugString, szThreadFormat, args);	
		OutputDebugString(pszDebugString);

		/*
		* Cleanup local string
		*/
		free(pszDebugString);
		pszDebugString = NULL;
	}
	va_end(args);
//#endif
	return;
}

void SystemError(void)
{
	DWORD dwError = GetLastError();

	char szTemp[50];
	sprintf(szTemp, "SystemError:  <%d>\n", dwError);
	OutputDebugString(szTemp);

	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		NULL);

	OutputDebugString((const char*)lpMsgBuf);
	OutputDebugString("\n");

	LocalFree(lpMsgBuf);
}