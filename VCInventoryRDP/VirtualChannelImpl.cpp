
#include "stdafx.h"
#include "VirtualChannelImpl.h"

//  Globals

HANDLE hWaits[NUM_EVENTS];
HANDLE hMtxImageQueue;
std::queue<LPIMGLIST> imageQueue;
LPIMGLIST lpCurrentImage = NULL;
LPBYTE lpImageData = NULL;
int nBytesRemaining = 0;
BOOL bTransferComplete = FALSE;
#define MAX_DATA_CHUNK 1592

static DWORD dwThreadDataId = 0;
static HANDLE hThreadDataHandle = NULL;
HANDLE mtxDBProcess = CreateMutex(NULL, FALSE, L"VCDBProcess");

void WriteToTempFileName(LPWSTR data, LPWSTR result);
void WINAPI CreateDBProcess(LPBYTE data);

LPWSTR TranslateCommand(int nCmd)
{
	LPWSTR szCommand = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 512);

	switch (nCmd)
	{
	case PM_INIT:
		lstrcpy(szCommand, L"PM_INIT");
		break;
	case PM_SENDDATA:	
		lstrcpy(szCommand, L"PM_SENDDATA");
		break;
	case PM_PING:
		lstrcpy(szCommand, L"PM_PING");
		break;
	case PM_DATASENT:
		lstrcpy(szCommand, L"PM_DATASENT");
		break;
	default:
		wsprintf(szCommand, L"UNKNOWN COMMAND: 0x%x", (nCmd - WM_APP));
		break;
	}

	return szCommand;
}

void ProcessData(LPBYTE data)
{
	hThreadDataHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CreateDBProcess, data, NULL, NULL);
	if (hThreadDataHandle == INVALID_HANDLE_VALUE)
	{
		//WaitForSingleObject(hThreadDataHandle, INFINITE);

		DebugTraceMessage(L"Failed to create thread");
	}
}

void WINAPI CreateDBProcess(LPBYTE data) 
{
	DebugTraceMessage((WCHAR *)data);

	WaitForSingleObject(mtxDBProcess, INFINITE);

	WCHAR *inFileName = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(WCHAR) * MAX_PATH);
	WriteToTempFileName((WCHAR *)data, inFileName);

	WCHAR pathString[MAX_PATH]; 
	GetTempPath(MAX_PATH, pathString);
	WCHAR outFileName[MAX_PATH];
	UINT fileResult = GetTempFileName(pathString, L"out", 0, outFileName);

	DebugTraceMessage(moduleName);

	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartupInfo; 
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);

	WCHAR directory[MAX_PATH];
	WCHAR drive[MAX_PATH];
	_wsplitpath(moduleName, drive, directory, NULL, NULL);

	WCHAR path[MAX_PATH];
	wsprintf(path, L"%s%s", drive, directory);

	WCHAR commands[MAX_PATH*2];
	wsprintf(commands, L"%sVCInventoryRDP.NET.exe /INFILE|%s /OUTFILE|%s", path, inFileName, outFileName);

	DebugTraceMessage(commands);

	if(CreateProcess(NULL, commands, NULL, NULL, FALSE, 0, NULL, NULL,&StartupInfo,&ProcessInfo))
	{ 
		WaitForSingleObject(ProcessInfo.hProcess, 30000);

		HANDLE hOutFileHandle = CreateFile(outFileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutFileHandle != INVALID_HANDLE_VALUE)
		{
			DWORD fileSize = 1592;
			DWORD bytesRead = 1;

			while (bytesRead > 0) 
			{
				LPBYTE buffer = (LPBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, fileSize);

				BOOL result = ReadFile(hOutFileHandle, buffer, fileSize, &bytesRead, NULL);
				if (result)
					result = gpEntryPoints->pVirtualChannelWrite(gdwOpenChannelOutput, buffer, fileSize, buffer);
				else 		
					DebugTraceMessage(L"Failed to read file");

				HeapFree(hHeap, NULL, buffer);
			}

			CloseHandle(hOutFileHandle);
		}
	}
	else
	{
		DebugTraceMessage(L"The process could not be started...");
	}

	ReleaseMutex(mtxDBProcess);
}

void WriteToTempFileName(LPWSTR data, LPWSTR result)
{
	try 
	{
		int dataSize = sizeof(WCHAR) * lstrlen(data);
		LPWSTR tempData = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dataSize);
		CopyMemory(tempData, data, dataSize);

		LPWSTR fileName = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(WCHAR) * MAX_PATH);

		WCHAR pathString[MAX_PATH]; 
		GetTempPath(MAX_PATH, pathString);
		GetTempFileName(pathString, L"in", 0, fileName);

		DebugTraceMessage((LPWSTR)fileName);
		DebugTraceMessage((LPWSTR)tempData);

		HANDLE hTempFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hTempFile == INVALID_HANDLE_VALUE)
		{
			DebugTraceMessage(L"Create File Failed: %i", GetLastError());
		}
		else 
		{
			if (lstrlen(data) > 0)
			{
				DWORD bytesWritten;
				BOOL hasWritten = WriteFile(hTempFile, tempData, dataSize, &bytesWritten, NULL); 
				if (hasWritten != TRUE)
				{
					DebugTraceMessage(L"Data Write Failed: %i", GetLastError());
				}
			}

			CloseHandle(hTempFile); 
		}
		CopyMemory(result, fileName, sizeof(WCHAR) * MAX_PATH);

		HeapFree(hHeap, NULL, tempData);
		HeapFree(hHeap, NULL, fileName);
	}
	catch(...)
	{
	}
}
