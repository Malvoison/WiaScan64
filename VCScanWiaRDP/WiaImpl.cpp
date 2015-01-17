//  Filename:  WiaImpl.cpp
//  Author:  Kenneth E. Watts, BSEE, JD

#include "stdafx.h"
#include "WiaImpl.h"
#include "VirtualChannelImpl.h"
#include "WiaWrap.h"
#include "BitmapWnd.h"

DWORD dwWiaThreadId = 0;
HANDLE hWiaThreadHandle = NULL;
HANDLE hWiaThreadReadyEvent = NULL;
DWORD dwImageTransferThreadId = 0;
HANDLE hImageTransferThreadHandle = NULL;
HANDLE hWiaThreadExitEvent = NULL;
HANDLE hMtxGlobals = NULL;
HANDLE hImageXferReadyEvent = NULL;
//  Semaphores for the transfer channels
HANDLE hSemImageXfer[NUM_XFER_CHANNELS];

/////////////////////////////////////////////////////////////////////////////////////////////
//  Image Converter Thread Proc
////////////////////////////////////////////////////////////////////////////////////////////


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

   Gdiplus::GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

DWORD WINAPI ImageConverterThreadProc(PVOID param)
{
	LPIMGLIST lpImg = (LPIMGLIST) param;

	//  Save bitmap to file
	WCHAR szTempFileNameIn[MAX_PATH];
	WCHAR szTempPathBuffer[MAX_PATH];

	DWORD dwRetVal = GetTempPath(MAX_PATH, szTempPathBuffer);
	if (dwRetVal > MAX_PATH || (dwRetVal == 0))
	{
		DebugTraceMessage(L"ImageConverterThreadProc::GetTempPath failed\n");
		ShowError();
		return -1;
	}

	UINT uRetVal = GetTempFileName(szTempPathBuffer, L"VCSCAN", 0, szTempFileNameIn);
	if (uRetVal == 0)
	{
		DebugTraceMessage(L"ImageConverterThreadProc::GetTempFilename failed\n");
		ShowError();
		return -1;
	}

	HANDLE hTempBmp = CreateFile(szTempFileNameIn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hTempBmp == INVALID_HANDLE_VALUE)
	{
		DebugTraceMessage(L"ImageConverterThreadProc::CreateFile failed for BMP file\n");
		ShowError();
		return -1;
	}

	DWORD dwBytesWritten;
	BOOL bResult = WriteFile(hTempBmp, lpImg->img, lpImg->nImageSize, &dwBytesWritten, NULL);
	if (!bResult)
	{
		DebugTraceMessage(L"ImageConverterThreadProc::WriteFile failed for BMP file\n");
		ShowError();
		CloseHandle(hTempBmp);
		return -1;
	}

	CloseHandle(hTempBmp);
	HeapFree(hHeap, 0, lpImg->img);

	//  Load into GDI+ image class and save as JPEG file
	WCHAR szTempFileNameOut[MAX_PATH];
	uRetVal = GetTempFileName(szTempPathBuffer, L"VCSCAN", 0, szTempFileNameOut);
	if (uRetVal == 0)
	{
		DebugTraceMessage(L"ImageConverterThreadProc::GetTempFilename failed \n");
		ShowError();
		return -1;
	}

	Gdiplus::Image image(szTempFileNameIn);

	CLSID jpegClsid;
	GetEncoderClsid(L"image/jpeg", &jpegClsid);

	image.Save(szTempFileNameOut, &jpegClsid, NULL);

	//  Read file into buffer
	HANDLE hTempJpeg = CreateFile(szTempFileNameOut, GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hTempJpeg == INVALID_HANDLE_VALUE)
	{
		DebugTraceMessage(L"ImageConverterThreadProc::CreateFile failed for hTempJpeg\n");
		ShowError();
		return -1;
	}

	DWORD dwFileSize = GetFileSize(hTempJpeg, NULL);
	LPBYTE lpJpegBuffer = (LPBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwFileSize);
	DWORD dwBytesRead;
	bResult = ReadFile(hTempJpeg, lpJpegBuffer, dwFileSize, &dwBytesRead, NULL);
	if (!bResult)
	{
		DebugTraceMessage(L"ImageConverterThreadProc::ReadFile failed for hTempJpeg\n");
		ShowError();
		CloseHandle(hTempJpeg);
		return -1;
	}

	CloseHandle(hTempJpeg);

	//  Post message to transfer thread	
	lpImg->img = lpJpegBuffer;
	lpImg->nImageSize = dwFileSize;
	PostThreadMessage(dwImageTransferThreadId, PM_TRANSFERREADY, 0, (LPARAM)lpImg);

	//  Cleanup temp files
	DeleteFile(szTempFileNameOut);
	DeleteFile(szTempFileNameIn);

	return 0;
}

DWORD WINAPI TransferDoneThreadProc(PVOID param)
{
	PostThreadMessage(dwImageTransferThreadId, PM_TRANSFERDONE, 0, NULL);
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//  Image Transfer Thread Proc
////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ImageTransferThreadProc(PVOID param)
{
	//  Start the thread's message pump
	MSG msg;
	//  Force the system to create the message queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	//  Enter the message loop
	DebugTraceMessage(L"Entering Image Transfer thread message loop\n");
	BOOL bRet;
	//  Keep going until we get WM_QUIT
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			DebugTraceMessage(L"WIA thread abnormal termination: ");
			ShowError();
			return -1;
		}

		if (msg.message < WM_USER)
		{
			DispatchMessage(&msg);
		}

		LPBYTE lpTransferBuffer = NULL;
		DWORD dwBufferSize = 0;
		LPIMGLIST lpImg = NULL;

		if (msg.message == PM_TRANSFERDONE)
		{
			lpTransferBuffer = (LPBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(int));
			dwBufferSize = sizeof(int);
		}
		else if (msg.message = PM_TRANSFERREADY)
		{
			lpImg = (LPIMGLIST)msg.lParam;
			dwBufferSize = sizeof(int) + lpImg->nImageSize;
			lpTransferBuffer = (LPBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY,  dwBufferSize);
			RtlCopyMemory(lpTransferBuffer, &lpImg->nImageSequence, sizeof(int));
			RtlCopyMemory(lpTransferBuffer + sizeof(int), lpImg->img, lpImg->nImageSize);			
		}
		else
		{
			continue;
		}

		//  Write the image to the first available image transfer queue
		DWORD dwWaitResult = WaitForMultipleObjects(NUM_XFER_CHANNELS, hSemImageXfer, FALSE, INFINITE);

		if (dwWaitResult == WAIT_FAILED)
		{
			ShowError();
			return -1;
		}

		DWORD dwSelectedChannel;
		switch (dwWaitResult - WAIT_OBJECT_0)
		{
		case XFER1:
			DebugTraceMessage(L"Transferring image on transfer channel: %d\n", XFER1);
			dwSelectedChannel = gdwOpenChannelImageXfer1;			
			break;
		case XFER2:
			dwSelectedChannel = gdwOpenChannelImageXfer2;
			DebugTraceMessage(L"Transferring image on transfer channel: %d\n", XFER2);		
			break;
		case XFER3:
			dwSelectedChannel = gdwOpenChannelImageXfer3;
			DebugTraceMessage(L"Transferring image on transfer channel: %d\n", XFER3);			
			break;
		case XFER4:
			dwSelectedChannel = gdwOpenChannelImageXfer4;
			DebugTraceMessage(L"Transferring image on transfer channel: %d\n", XFER4);
			break;
		default:
			break;
		}

		UINT nWriteResult = gpEntryPoints->pVirtualChannelWrite(dwSelectedChannel, lpTransferBuffer, dwBufferSize, lpTransferBuffer);
		if (nWriteResult != CHANNEL_RC_OK)
		{
			switch (nWriteResult)
			{
			case CHANNEL_RC_BAD_CHANNEL_HANDLE:
				DebugTraceMessage(L"VirtualChannelWrite Error: %s\n", L"CHANNEL_RC_BAD_CHANNEL_HANDLE");
				break;
			case CHANNEL_RC_NO_MEMORY:
				DebugTraceMessage(L"VirtualChannelWrite Error: %s\n", L"CHANNEL_RC_NO_MEMORY");
				break;
			case CHANNEL_RC_NOT_CONNECTED:
				DebugTraceMessage(L"VirtualChannelWrite Error: %s\n", L"CHANNEL_RC_NOT_CONNECTED");
				break;
			case CHANNEL_RC_NULL_DATA:
				DebugTraceMessage(L"VirtualChannelWrite Error: %s\n", L"CHANNEL_RC_NULL_DATA");
				break;
			case CHANNEL_RC_ZERO_LENGTH:
				DebugTraceMessage(L"VirtualChannelWrite Error: %s\n", L"CHANNEL_RC_ZERO_LENGTH");
				break;
			default:
				break;
			}
		}
		
		if (lpImg != NULL)
		{
			//  Free the IMGLIST struct
			HeapFree(hHeap, 0, lpImg);
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//  WiaThread Functions
////////////////////////////////////////////////////////////////////////////////////////////

void OnAcquire()
{
	HRESULT hr;

	WiaWrap::CComPtrArray<IStream> ppStream;

	hr = WiaWrap::WiaGetImage(
		g_hWndMstsc,
		StiDeviceTypeDefault,
		0,
		WIA_INTENT_NONE,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&ppStream.Count(),
		&ppStream
		);

	if (FAILED(hr))
	{
		MessageBox(g_hWndMstsc, L"Error...error", L"Error", MB_ICONSTOP | MB_OK);

		return;
	}

}

void OnPreferences()
{
	HRESULT hr = WiaWrap::WiaSetPreferences(g_hWndMstsc);
	if (FAILED(hr))
	{
		MessageBox(g_hWndMstsc, L"Error...error", L"Error", MB_ICONSTOP | MB_OK);

		return;
	}
}

DWORD WINAPI WiaThreadProc(PVOID pvParam)
{	
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		DebugTraceMessage(L"WiaThreadProc::CoInitialize FAILED\n");
		return -1;
	}
	//  Start the thread's message pump
	MSG msg;
	//  Force the system to create the message queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	//  Signal that the thread is ready
	SetEvent(hWiaThreadReadyEvent);

	//  Enter the message loop
	DebugTraceMessage(L"Entering WIA thread message loop\n");
	BOOL bRet;
	//  Keep going until we get WM_QUIT
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			DebugTraceMessage(L"WIA thread abnormal termination: ");
			ShowError();
			return -1;
		}

		if (msg.message < WM_USER)
		{
			DispatchMessage(&msg);
		}

		if (msg.message > WM_APP && msg.message <= 0xBFFF)
		{
			switch (msg.message)
			{
			case PM_INIT:
				//  perform meaningful initialization here
				DebugTraceMessage(L"WiaThreadProc::PM_INIT\n");
				CBitmapWnd::Register();
				SetEvent((HANDLE)msg.wParam);
				break;

			case PM_RELEASE:
				//  perform useful uninitialization here
				DebugTraceMessage(L"WiaThreadProc::PM_RELEASE\n");
				SetEvent((HANDLE)msg.wParam);
				break;

			case PM_ACQUIRE:
				{
					WaitForSingleObject(hMtxGlobals, INFINITE);
					OnAcquire();
					ReleaseMutex(hMtxGlobals);
					SetEvent((HANDLE)msg.wParam);
				}
				break;

			case PM_SETPREFERENCES:
				{
					WaitForSingleObject(hMtxGlobals, INFINITE);
					OnPreferences();
					ReleaseMutex(hMtxGlobals);
					SetEvent((HANDLE)msg.wParam);
				}
				break;

			default:
				break;
			}
		}		
	}

	DebugTraceMessage(L"WIA thread exiting\n");
	SetEvent(hWiaThreadExitEvent);

	hWiaThreadHandle = NULL;

	CoUninitialize();

	return 0;
}

void InitWia()
{
	if (hWiaThreadHandle != NULL)
		return;

	//  Create needful synchronization objects
	hWiaThreadReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hWiaThreadExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hMtxGlobals = CreateMutex(NULL, FALSE, NULL);
	for (int i = 0; i < NUM_XFER_CHANNELS; i++)
	{
		hSemImageXfer[i] = CreateSemaphore(NULL, 1, 1, NULL);
	}

	hImageTransferThreadHandle = CreateThread(NULL, 0, ImageTransferThreadProc, NULL, 0, &dwImageTransferThreadId);

	hWiaThreadHandle = CreateThread(NULL, 0, WiaThreadProc, NULL, 0, &dwWiaThreadId);
	DWORD dwResult = WaitForSingleObject(hWiaThreadReadyEvent, 5000);
	if (dwResult != WAIT_OBJECT_0)
	{
		ShowError();
		return;
	}

	HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
	dwResult = PostThreadMessage(dwWiaThreadId, PM_INIT, (WPARAM)hFnComplete, 0);
	dwResult = WaitForSingleObject(hFnComplete, INFINITE);

	CloseHandle(hFnComplete);
}

void ReleaseWia()
{	
	DWORD dwResult;
	HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
	dwResult = PostThreadMessage(dwWiaThreadId, PM_RELEASE, (WPARAM)hFnComplete, 0);
	dwResult = WaitForSingleObject(hFnComplete, INFINITE);

	CloseHandle(hFnComplete);

	dwResult = PostThreadMessage(dwWiaThreadId, WM_QUIT, 0, 0);

	WaitForSingleObject(hWiaThreadExitEvent, 30000);

	CloseHandle(hWiaThreadReadyEvent);
	CloseHandle(hWiaThreadExitEvent);
	CloseHandle(hMtxGlobals);
	for (int i = 0; i < NUM_XFER_CHANNELS; i++)
	{
		CloseHandle(hSemImageXfer[i]);
	}
}

void AcquireWia()
{
	DWORD dwResult;
	HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
	dwResult = PostThreadMessage(dwWiaThreadId, PM_ACQUIRE, (WPARAM)hFnComplete, 0);
	dwResult = WaitForSingleObject(hFnComplete, INFINITE);

	CloseHandle(hFnComplete);
}

void SetWiaPreferences()
{
	DWORD dwResult;
	HANDLE hFnComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
	dwResult = PostThreadMessage(dwWiaThreadId, PM_SETPREFERENCES, (WPARAM)hFnComplete, 0);
	dwResult = WaitForSingleObject(hFnComplete, INFINITE);

	CloseHandle(hFnComplete);
}