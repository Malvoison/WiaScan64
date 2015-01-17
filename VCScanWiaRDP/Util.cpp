
#include "stdafx.h"

#define MAXDEBUGSTRING	1024

HANDLE hFileLogInfo = NULL;
BOOL bLoggingInitialized = FALSE;
CRITICAL_SECTION csLogFile;
HWND g_hWndMstsc;

//
//  Utility Functions
//
void DebugTraceMessage(LPCWSTR lpszFormat, ...)
{
	/*
	* duplicate printf functionality for DebugTraceMessage
	*/
#ifdef _DEBUG
	wchar_t *pszDebugString = (wchar_t*) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAXDEBUGSTRING);
	va_list args;
	va_start(args, lpszFormat);
	
	if(pszDebugString)
	{
		/*
		* format and output the debug string
		*/
		//  prepend the current thread ID
		wchar_t szThreadFormat[MAXDEBUGSTRING];
		wsprintf(szThreadFormat, L"(Thrd ID: 0x%x) ", GetCurrentThreadId());
		lstrcat(szThreadFormat, lpszFormat);
		lstrcat(szThreadFormat, L"\r\n");
		wvsprintf(pszDebugString, szThreadFormat, args);	
		OutputDebugString(pszDebugString);

		//  Write to the log file
		EnterCriticalSection(&csLogFile);

		DWORD dwNumWritten;
		WriteFile(hFileLogInfo, pszDebugString, lstrlen(pszDebugString) * sizeof(TCHAR), &dwNumWritten, NULL);

		LeaveCriticalSection(&csLogFile);

		//  Ensure we've been initialized
		if (gpEntryPoints != NULL && gdwOpenChannelClientInfo != 0)
		{
			gpEntryPoints->pVirtualChannelWrite(gdwOpenChannelClientInfo, pszDebugString, MAXDEBUGSTRING, pszDebugString);
		}
		else
		{			
			HeapFree(hHeap, 0, pszDebugString);
		}
				
		pszDebugString = NULL;
	}
	va_end(args);
#endif
	return;
}

void ShowError(void)
{
	DWORD dwError = GetLastError();

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

	DebugTraceMessage(L"VCScanWiaRDP: %s\n", lpMsgBuf);

	LocalFree(lpMsgBuf);
}

void InitializeLogging(void)
{
	if (bLoggingInitialized)
		return;
	//  Load the registry path from the string table
	wchar_t* buf = new wchar_t[MAX_PATH];
	LoadString(g_hInstance, STR_HKEY, buf, MAX_PATH);
	//  Read LogLevel from the registry
	HKEY hKey = NULL;
	DWORD dwLogLevel;
	DWORD dwDataSize = sizeof(DWORD);
	RegCreateKeyEx(HKEY_CURRENT_USER, buf, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE, NULL, &hKey, NULL);
	RegQueryValueEx(hKey, L"LogLevel", NULL, NULL, (LPBYTE)&dwLogLevel, &dwDataSize);
	RegCloseKey(hKey);

	//  Open the files for logging
	wchar_t* wszInfoFile = GetDriverInstallPath();
	
	wcscat(wszInfoFile, L"InfoLog.log");

	hFileLogInfo = CreateFile(wszInfoFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	
    HeapFree(hHeap, 0, wszInfoFile);
}

LPWSTR GetDriverInstallPath()
{
    LPWSTR wszPath = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH);
    LPWSTR wszDir = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH);
    LPWSTR wszDrive = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH);

    GetModuleFileName(g_hInstance, wszPath, MAX_PATH);
    _wsplitpath(wszPath, wszDrive, wszDir, NULL, NULL);

    LPWSTR wszInstallPath = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH);
    wcscat(wszInstallPath, wszDrive);
    wcscat(wszInstallPath, wszDir);

    HeapFree(hHeap, 0, wszPath);
    HeapFree(hHeap, 0, wszDir);
    HeapFree(hHeap, 0, wszDrive);

    return wszInstallPath;
}

BOOL CheckFileVersionInfo(LPWSTR* ppwszVersion)
{
	TCHAR  szFileName[MAX_PATH + 1] = {0};
    GetModuleFileName(g_hInstance, szFileName, MAX_PATH);
    
    DWORD dwDummy;
    DWORD dwSize = GetFileVersionInfoSize(szFileName, &dwDummy);

    std::vector<BYTE> data(dwSize);
    GetFileVersionInfo(szFileName, NULL, dwSize, &data[0]);

    LPVOID pvProductVersion = NULL;
    UINT uiProductVersionLen = 0;
    VerQueryValue(&data[0], L"\\StringFileInfo\\040904b0\\FileVersion", &pvProductVersion, &uiProductVersionLen);
    *ppwszVersion = reinterpret_cast<LPWSTR>(pvProductVersion);

    return TRUE;
}

void ShutdownLogging(void)
{
	if (hFileLogInfo != NULL)
		CloseHandle(hFileLogInfo);
}

void GetProcessAndWindowInfo(void)
{
	DWORD dwProcessId = GetCurrentProcessId();
	g_hWndMstsc = FindWindowEx(NULL, NULL, L"TscShellContainerClass", NULL);
	DWORD dwOwnerProcess;
	GetWindowThreadProcessId(g_hWndMstsc, &dwOwnerProcess);
	if (dwProcessId == dwOwnerProcess)
	{
		DebugTraceMessage(L"Found MSTSC main window\n");
	}
	else
	{
		DebugTraceMessage(L"Found an MSTSC main window, but not mine\n");
	}

}
