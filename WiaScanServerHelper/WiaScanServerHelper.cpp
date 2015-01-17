// WiaScanServerHelper.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

HWND hWndCallback = NULL;
HANDLE hChannelHandleServerCommand = NULL;
HANDLE hChannelHandleClientInfo = NULL;
HANDLE hChannelHandleImageXfer1 = NULL;
HANDLE hChannelHandleImageXfer2 = NULL;
HANDLE hChannelHandleImageXfer3 = NULL;
HANDLE hChannelHandleImageXfer4 = NULL;
HANDLE hClientInfoThread = NULL;
HANDLE hImageXfer1Thread = NULL;
HANDLE hImageXfer2Thread = NULL;
HANDLE hImageXfer3Thread = NULL;
HANDLE hImageXfer4Thread = NULL;

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

	OutputDebugString((LPTSTR)lpMsgBuf);
	OutputDebugString(L"\n");

	LocalFree(lpMsgBuf);
}

//extern "C" HANDLE __declspec(dllexport) OpenDynamicChannel(const char* szChannel)
//{
//	HANDLE hWTSHandle = NULL;
//	HANDLE hWTSFileHandle;
//	HANDLE hFile;
//	PVOID vcFileHandlePtr = NULL;
//	DWORD len;
//	DWORD rc = ERROR_SUCCESS;
//
//	hWTSHandle = WTSVirtualChannelOpenEx(
//		WTS_CURRENT_SESSION,
//		(LPSTR)szChannel,
//		WTS_CHANNEL_OPTION_DYNAMIC | WTS_CHANNEL_OPTION_DYNAMIC_PRI_HIGH);
//	if (NULL == hWTSHandle)
//	{
//		goto ErrorExit;
//	}
//
//	BOOL fSucc = WTSVirtualChannelQuery(
//		hWTSHandle,
//		WTSVirtualFileHandle,
//		&vcFileHandlePtr,
//		&len);
//	if (!fSucc)
//	{
//		goto ErrorExit;
//	}
//	if (len != sizeof(HANDLE))
//	{
//		goto ErrorExit;
//	}
//
//	hWTSFileHandle = *(HANDLE*)vcFileHandlePtr;
//	fSucc = DuplicateHandle(
//		GetCurrentProcess(),
//		hWTSFileHandle,
//		GetCurrentProcess(),
//		&hFile,
//		0,
//		FALSE,
//		DUPLICATE_SAME_ACCESS);
//	if (!fSucc)
//	{
//		goto ErrorExit;
//	}
//
//	return hFile;
//
//ErrorExit:
//	if (vcFileHandlePtr)
//	{
//		WTSFreeMemory(vcFileHandlePtr);
//	}
//	if (hWTSHandle)
//	{
//		WTSVirtualChannelClose(hWTSHandle);
//	}
//
//	return NULL;
//}
//
//extern "C" LPBYTE __declspec(dllexport) ReadChannelHelper(PVOID param)
//{
//	HANDLE hFile;
//	BYTE ReadBuffer[CHANNEL_PDU_LENGTH];
//	DWORD dwRead;
//	BYTE b = 0;
//	CHANNEL_PDU_HEADER *pHdr;
//	BOOL fSucc;
//	HANDLE hEvent;
//
//	hFile = (HANDLE)param;
//	pHdr = (CHANNEL_PDU_HEADER*)ReadBuffer;
//
//	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//
//	for (ULONG msgSize = START_MSG_SIZE; 
//		msgSize < MAX_MSG_SIZE;
//		msgSize += STEP_MSG_SIZE)
//	{
//		OVERLAPPED Overlapped = {0};
//		DWORD TotalRead = 0;
//		do
//		{
//			Overlapped.hEvent = hEvent;  INFINITE;
//
//			//  Read the entire message
//			fSucc = ReadFile(
//				hFile,
//				ReadBuffer,
//				sizeof(ReadBuffer),
//				&dwRead,
//				&Overlapped);
//			if (!fSucc)
//			{
//				if (GetLastError() == ERROR_IO_PENDING)
//				{
//					DWORD dw = WaitForSingleObject(Overlapped.hEvent, INFINITE);
//					_ASSERT(WAIT_OBJECT_0 == dw);
//					fSucc = GetOverlappedResult(
//						hFile,
//						&Overlapped,
//						&dwRead,
//						FALSE);
//				}
//			}
//
//			if (!fSucc)
//			{
//				DWORD error = GetLastError();
//				//  do something useful here
//				return NULL;
//			}
//
//			ULONG packetSize = dwRead - sizeof(*pHdr);
//			TotalRead += packetSize;
//			PBYTE pData = (PBYTE)(pHdr + 1);
//			for (ULONG i = 0; i < packetSize; pData++, i++, b++)
//			{
//				_ASSERT(*pData == b);
//			}
//
//			_ASSERT(msgSize == pHdr->length);
//
//		} while (0 == (pHdr->flags & CHANNEL_FLAG_LAST));
//
//		_ASSERT(TotalRead == msgSize);
//	}
//
//	return NULL;
//}
//
//extern "C" void __declspec(dllexport) WriteChannelHelper(PVOID param)
//{
//	HANDLE hFile;
//	BYTE WriteBuffer[MAX_MSG_SIZE];
//	DWORD dwWritten;
//	BOOL fSucc;
//	BYTE b = 0;
//	HANDLE hEvent;
//
//	hFile = (HANDLE)param;
//
//	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//
//	for (ULONG msgSize = START_MSG_SIZE;
//		msgSize < MAX_MSG_SIZE;
//		msgSize += STEP_MSG_SIZE)
//	{
//		OVERLAPPED Overlapped = {0};
//		Overlapped.hEvent = hEvent;
//
//		for (ULONG i = 0; i < msgSize; i++, b++)
//		{
//			WriteBuffer[i] = b;
//		}
//
//		fSucc = WriteFile(
//			hFile,
//			WriteBuffer,
//			msgSize,
//			&dwWritten,
//			&Overlapped);
//		if (!fSucc)
//		{
//			if (GetLastError() == ERROR_IO_PENDING)
//			{
//				DWORD dw = WaitForSingleObject(Overlapped.hEvent, _MAX_WAIT);
//				_ASSERT(WAIT_OBJECT_0 == dw);
//				fSucc = GetOverlappedResult(
//					hFile,
//					&Overlapped,
//					&dwWritten,
//					FALSE);
//			}
//		}
//
//		if (!fSucc)
//		{
//			DWORD error = GetLastError();
//		}
//
//		_ASSERT(dwWritten == msgSize);
//	}
//
//}

extern "C" BOOL __declspec(dllexport) TestChannelAvailable(LPCSTR szChannel)
{
	BOOL bResult = FALSE;
	HANDLE hChannelHandle = NULL;

	hChannelHandle = WTSVirtualChannelOpen(WTS_CURRENT_SERVER, WTS_CURRENT_SESSION, const_cast<LPSTR>(szChannel));

	if (hChannelHandle == NULL)
	{
		ShowError();
		goto ErrorExit;
	}

	DWORD dwBytesWritten = 0;
	int nCmd = PM_PING;
	if (WTSVirtualChannelWrite(hChannelHandle, reinterpret_cast<PCHAR>(&nCmd), sizeof(int), &dwBytesWritten))
	{
		bResult = TRUE;
	}

ErrorExit:
	if (hChannelHandle)
	{
		WTSVirtualChannelClose(hChannelHandle);
	}

	return bResult;
}

//  WiaScan channel functions
extern "C" BOOL __declspec(dllexport) SendServerCommand(int nCmd)
{
	BOOL bResult = FALSE;
	int nCmdLocal = nCmd;
	DWORD dwWritten;

	if (hChannelHandleServerCommand == NULL)
	{
		hChannelHandleServerCommand = WTSVirtualChannelOpen(WTS_CURRENT_SERVER, WTS_CURRENT_SESSION, CHANNELNAME_SERVERCOMMAND);
	}
	if (WTSVirtualChannelWrite(hChannelHandleServerCommand, reinterpret_cast<PCHAR>(&nCmdLocal), sizeof(int), &dwWritten))
	{
		bResult = TRUE;
	}
	else
	{
		OutputDebugString(L"WTSVirtualChannelWrite failed in SendServerCommand\n");
		bResult = FALSE;
	}

	
	return bResult;
}

DWORD WINAPI ClientInfoChannelMonitor(LPVOID lpvParam)
{
	OutputDebugString(L"Starting ClientInfoChannelMonitor\n");

	hChannelHandleClientInfo = WTSVirtualChannelOpen(WTS_CURRENT_SERVER, WTS_CURRENT_SESSION, CHANNELNAME_CLIENTINFO);
	if (hChannelHandleClientInfo == NULL)
	{
		OutputDebugString(L"WTSVirtualChannelOpenEx failed for CHANNELNAME_CLIENTINFO\n");
		ShowError();
		return -1;
	}

	LPBYTE lpPduBuffer = (LPBYTE)GlobalAlloc(GPTR, CHANNEL_PDU_LENGTH);
	RtlZeroMemory(lpPduBuffer, CHANNEL_CHUNK_LENGTH);
	LPBYTE lpMsgBuffer = NULL;
	LPBYTE lpLastWrite = NULL;
	DWORD dwBytesRead = 0;
	PCHANNEL_PDU_HEADER pdu = NULL;
	
	while (WTSVirtualChannelRead(hChannelHandleClientInfo, INFINITE, reinterpret_cast<PCHAR>(lpPduBuffer), CHANNEL_PDU_LENGTH, &dwBytesRead))
	{
		pdu = (PCHANNEL_PDU_HEADER)lpPduBuffer;
		if (lpMsgBuffer == NULL)
		{
			lpMsgBuffer = (LPBYTE)GlobalAlloc(GPTR, pdu->length);
			lpLastWrite = lpMsgBuffer;
		}
		
		RtlCopyMemory(lpLastWrite, lpPduBuffer + sizeof(CHANNEL_PDU_HEADER), dwBytesRead - sizeof(CHANNEL_PDU_HEADER));
		lpLastWrite += (dwBytesRead - sizeof(CHANNEL_PDU_HEADER));

		if ((pdu->flags & CHANNEL_FLAG_LAST) != 0)
		{
			OutputDebugString((LPCWSTR)lpMsgBuffer);
			GlobalFree(lpMsgBuffer);
			lpMsgBuffer = NULL;
			lpLastWrite = NULL;
		}
	}

	GlobalFree(lpPduBuffer);
	lpPduBuffer = NULL;

	OutputDebugString(L"Exiting ClientInfoChannelMonitor\n");

	return 0;
}

void ImageTransferHelper(HANDLE hChannelHandle)
{
	LPBYTE lpPduBuffer = (LPBYTE)GlobalAlloc(GPTR, CHANNEL_PDU_LENGTH);
	RtlZeroMemory(lpPduBuffer, CHANNEL_CHUNK_LENGTH);
	LPBYTE lpMsgBuffer = NULL;
	LPBYTE lpLastWrite = NULL;
	DWORD dwBytesRead = 0;
	PCHANNEL_PDU_HEADER pdu = NULL;
	wchar_t szTrace[1024];
	
	while (WTSVirtualChannelRead(hChannelHandle, INFINITE, reinterpret_cast<PCHAR>(lpPduBuffer), CHANNEL_PDU_LENGTH, &dwBytesRead))
	{
		pdu = (PCHANNEL_PDU_HEADER)lpPduBuffer;
		if (lpMsgBuffer == NULL)
		{
			lpMsgBuffer = (LPBYTE)GlobalAlloc(GPTR, pdu->length);
			lpLastWrite = lpMsgBuffer;
		}
		
		RtlCopyMemory(lpLastWrite, lpPduBuffer + sizeof(CHANNEL_PDU_HEADER), dwBytesRead - sizeof(CHANNEL_PDU_HEADER));
		lpLastWrite += (dwBytesRead - sizeof(CHANNEL_PDU_HEADER));

		if ((pdu->flags & CHANNEL_FLAG_LAST) != 0)
		{
			wsprintf(szTrace, L"Image sequence number: %d\n", *((int*)lpMsgBuffer));
			OutputDebugString(szTrace);

			if (hWndCallback != NULL)
			{
				PostMessage(hWndCallback, PM_WIATRANSFERDONE, (WPARAM)(pdu->length - 4), (LPARAM)(lpMsgBuffer + 4));
			}
			else
			{
				GlobalFree(lpMsgBuffer);
			}
			
			lpMsgBuffer = NULL;
			lpLastWrite = NULL;
		}
	}

	GlobalFree(lpPduBuffer);
	lpPduBuffer = NULL;
}

DWORD WINAPI ImageTransfer1ChannelMonitor(LPVOID lpvParam)
{
	OutputDebugString(L"Starting Image Transfer Channel Monitor for #1\n");

	hChannelHandleImageXfer1 = WTSVirtualChannelOpen(WTS_CURRENT_SERVER, WTS_CURRENT_SESSION, CHANNELNAME_IMAGEXFER1);
	if (hChannelHandleImageXfer1 == NULL)
	{
		OutputDebugString(L"WTSVirtualChannelOpenEx failed for CHANNELNAME_IMAGEXFER1\n");
		ShowError();
		return -1;
	}

	ImageTransferHelper(hChannelHandleImageXfer1);

	OutputDebugString(L"Exiting ImageTransfer1ChannelMonitor\n");

	return 0;
}

DWORD WINAPI ImageTransfer2ChannelMonitor(LPVOID lpvParam)
{
	OutputDebugString(L"Starting Image Transfer Channel Monitor for #2\n");

	hChannelHandleImageXfer2 = WTSVirtualChannelOpen(WTS_CURRENT_SERVER, WTS_CURRENT_SESSION, CHANNELNAME_IMAGEXFER2);
	if (hChannelHandleImageXfer2 == NULL)
	{
		OutputDebugString(L"WTSVirtualChannelOpenEx failed for CHANNELNAME_IMAGEXFER2\n");
		ShowError();
		return -1;
	}

	ImageTransferHelper(hChannelHandleImageXfer2);

	OutputDebugString(L"Exiting ImageTransfer2ChannelMonitor\n");

	return 0;
}

DWORD WINAPI ImageTransfer3ChannelMonitor(LPVOID lpvParam)
{
	OutputDebugString(L"Starting Image Transfer Channel Monitor for #3\n");

	hChannelHandleImageXfer3 = WTSVirtualChannelOpen(WTS_CURRENT_SERVER, WTS_CURRENT_SESSION, CHANNELNAME_IMAGEXFER3);
	if (hChannelHandleImageXfer3 == NULL)
	{
		OutputDebugString(L"WTSVirtualChannelOpenEx failed for CHANNELNAME_IMAGEXFER3\n");
		ShowError();
		return -1;
	}

	ImageTransferHelper(hChannelHandleImageXfer3);

	OutputDebugString(L"Exiting ImageTransfer3ChannelMonitor\n");

	return 0;
}

DWORD WINAPI ImageTransfer4ChannelMonitor(LPVOID lpvParam)
{
	OutputDebugString(L"Starting Image Transfer Channel Monitor for #4\n");

	hChannelHandleImageXfer4 = WTSVirtualChannelOpen(WTS_CURRENT_SERVER, WTS_CURRENT_SESSION, CHANNELNAME_IMAGEXFER4);
	if (hChannelHandleImageXfer4 == NULL)
	{
		OutputDebugString(L"WTSVirtualChannelOpenEx failed for CHANNELNAME_IMAGEXFER4\n");
		ShowError();
		return -1;
	}

	ImageTransferHelper(hChannelHandleImageXfer4);

	OutputDebugString(L"Exiting ImageTransfer4ChannelMonitor\n");

	return 0;
}


extern "C" void __declspec(dllexport) InitializeWiaChannelHandlers(HWND hWndImageCallback)
{
	hWndCallback = hWndImageCallback;

	hClientInfoThread = CreateThread(NULL, 0, ClientInfoChannelMonitor, NULL, 0, NULL);
	hImageXfer1Thread = CreateThread(NULL, 0, ImageTransfer1ChannelMonitor, NULL, 0, NULL);
	hImageXfer2Thread = CreateThread(NULL, 0, ImageTransfer2ChannelMonitor, NULL, 0, NULL);
	hImageXfer3Thread = CreateThread(NULL, 0, ImageTransfer3ChannelMonitor, NULL, 0, NULL);
	hImageXfer4Thread = CreateThread(NULL, 0, ImageTransfer4ChannelMonitor, NULL, 0, NULL);
}

extern "C" void __declspec(dllexport) ReleaseWiaChannelHandlers()
{
	WTSVirtualChannelClose(hChannelHandleServerCommand);
	hChannelHandleServerCommand = NULL;
	WTSVirtualChannelClose(hChannelHandleClientInfo);
	hChannelHandleClientInfo = NULL;
	WTSVirtualChannelClose(hChannelHandleImageXfer1);
	hChannelHandleImageXfer1 = NULL;
	WTSVirtualChannelClose(hChannelHandleImageXfer2);
	hChannelHandleImageXfer2 = NULL;
	WTSVirtualChannelClose(hChannelHandleImageXfer3);
	hChannelHandleImageXfer3 = NULL;
	WTSVirtualChannelClose(hChannelHandleImageXfer4);
	hChannelHandleImageXfer4 = NULL;

}

