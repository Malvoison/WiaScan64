// TwainSurrogate.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
//#include <gdiplus.h>
//#include <gdiplusflat.h>
#include "Utility.h"
#include "TwainSurrogate.h"
#include "TwainWrapper.h"
#include "TwainMsgWin.h"
#include "EnumSources.h"
#include "EnumPixelTypes.h"
#include "EnumBitDepths.h"
#include "EnumCapabilities.h"
#include "EnumPageSizes.h"
#include "EnumResolutions.h"
#include "GetResolutionRange.h"
#include "GetDefaultSource.h"
#include "SetResolution.h"
#include "EnableFeeder.h"
#include "EnableDuplex.h"
#include "SetPageSize.h"
#include "SetBitDepth.h"
#include "SetPixelType.h"
#include "SetSelectSrc.h"
#include "ShowUI.h"

#define MAX_LOADSTRING 100

using namespace Gdiplus;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
Image* m_DisplayImage = NULL;
LPIMGLIST img = NULL;
ULONG_PTR gdiplusToken;							//For Gdiplus Startup & Shutdown

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				DisplayImage(LPIMGLIST, HWND);

//  GdiPlus Stuff
Gdiplus::Status StartupStatus;
ULONG_PTR Token;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	//  There can be only one...
	HANDLE hMutex = CreateMutex(NULL, FALSE, "MtxTwainSurrogate");
	DWORD dwResult = WaitForSingleObject(hMutex, 0);
	if (dwResult == WAIT_TIMEOUT)
	{
		OutputDebugString("TwainSurrogate - another instance was already running\n");
		return -1;
	}

	Gdiplus::GdiplusStartupInput StartupInput(NULL, FALSE, FALSE);
	StartupStatus = Gdiplus::GdiplusStartup(&Token, &StartupInput, NULL);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TWAINSURROGATE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TWAINSURROGATE));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (StartupStatus ==  Gdiplus::Ok)
	{
		Gdiplus::GdiplusShutdown(Token);
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TWAINSURROGATE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TWAINSURROGATE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

			//  TWAIN Stuff
		case IDM_INIT:
			{				
				__Init(hWnd);
			}
			break;
		case IDM_RELEASE:
			{
				__Finish();
			}
			break;
		case IDM_SELECTSRC:
			{
				__SelectSrc();
			}
			break;
		case IDM_ENUMSRCS:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_ENUMSRCS_DLG), hWnd, EnumSrcsDlgProc);
			}
			break;
		case IDM_ACQUIRE:
			{
				__Acquire();
			}
			break;
		case IDM_TRANSFERPICTURES:
			{
				LPIMGLIST img = __TransferPictures();
				if(img != NULL)
				{
					DisplayImage(img, hWnd);
				}
			}
			break;
		case IDM_OPENSRC:
			{
				__OpenSrc();
			}
			break;
		case IDM_CLOSESRC:
			{
				__CloseSrc();
			}
			break;
		case IDM_FINISH:
			{
				__Finish();
			}
			break;
		case IDM_GETDEFAULTSOURCE:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_DEFAULTSRC_DLG), hWnd, DefaultSrcDlgProc);
			}
			break;
		case IDM_SETSELECTSOURCE:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_SETSELCTSRC_DLG), hWnd, SetSelectSrcDlgProc);
			}
			break;
		case IDM_SETPIXELTYPE:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_SETPXLTYPE_DLG), hWnd, SetPixelTypeDlgProc);
			}
			break;
		case IDM_ENUMPIXELTYPES:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_ENUMPXLTYPES_DLG), hWnd, EnumPxlTypesDlgProc);
			}
			break;
		case IDM_SETBITDEPTH:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_SETBITDEPTH_DLG), hWnd, SetBitDepthDlgProc);
			}
			break;
		case IDM_ENUMBITDEPTHS:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_ENUMBITDEPTHS_DLG), hWnd, EnumBitDepthsDlgProc);
			}
			break;
		case IDM_SETRESOLUTION:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_SETRESOLUTION_DLG), hWnd, SetResolutionDlgProc);
			}
			break;
		case IDM_GETRESOLUTIONRANGE:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_GETRESRANGE_DLG), hWnd, GetResRangeDlgProc);
			}
			break;
		case IDM_ENUMRESOLUTIONS:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_ENUMRES_DLG), hWnd, EnumResolutionDlgProc);
			}
			break;
		case IDM_SHOWUI:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_SHOWUI_DLG), hWnd, ShowUIDlgProc);
			}
			break;
		case IDM_ENUMCAPS:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_ENUMCAPS_DLG), hWnd, EnumCapsDlgProc);
			}
			break;
		case IDM_ENUMPAGESIZES:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_ENUMPAGESIZE_DLG), hWnd, EnumPgSizesDlgProc);
			}
			break;
		case IDM_SETPAGESIZE:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_SETPGSZ_DLG), hWnd, SetPageSizeDlgProc);
			}
			break;
		case IDM_SETFEEDERENABLED:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_FEEDER_DLG), hWnd, EnableFeederDlgProc);
			}
			break;
		case IDM_SETDUPLEX:
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_DUPLEX_DLG), hWnd, EnableDuplexDlgProc);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		//If images transferred, paint the m_DisplayImage
		if(m_DisplayImage != NULL)
		{	
			if(hdc != NULL )
			{
				Graphics graphics(hdc);

				//Get the size of the window
				RECT r; 
				GetClientRect(hWnd, &r);
				UINT nWindowWidth = r.right;
				UINT nWindowHeight = r.bottom;

				//Get size of image
				UINT nBitmapWidth = m_DisplayImage->GetWidth();
				UINT nBitmapHeight = m_DisplayImage->GetHeight();

				if (nBitmapWidth != 0 && nBitmapHeight != 0)
				{
					// Calculate the coordinates and the size of the image
					Rect rDest;

					if (nBitmapWidth <= nWindowWidth && nBitmapHeight <= nWindowHeight)
					{
						// If the image is smaller than the window, center the image
						rDest.X      = (nWindowWidth - nBitmapWidth) / 2;
						rDest.Y      = (nWindowHeight - nBitmapHeight) / 2;
						rDest.Width  = nBitmapWidth;
						rDest.Height = nBitmapHeight;
					}
					else
					{
						// If the image is larger than the window, resize and center 
						// the image while keeping the aspect ratio
						UINT nStretchedWidth  = nWindowWidth;
						UINT nStretchedHeight = MulDiv(nBitmapHeight, nWindowWidth, nBitmapWidth);

						if (nStretchedHeight > nWindowHeight)
						{
							nStretchedWidth  = MulDiv(nBitmapWidth, nWindowHeight, nBitmapHeight);
							nStretchedHeight = nWindowHeight;
						}
						rDest.X      = (nWindowWidth - nStretchedWidth) / 2;
						rDest.Y      = (nWindowHeight - nStretchedHeight) / 2;
						rDest.Width  = nStretchedWidth;
						rDest.Height = nStretchedHeight;
					}

					// Paint the image with a white background
					graphics.DrawImage(m_DisplayImage, rDest);
					graphics.ExcludeClip(rDest);
				}
				graphics.Clear((DWORD)Color::White);
			}
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		delete m_DisplayImage;
		GdiplusShutdown(gdiplusToken);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

//Displays first transferred image to screen
void DisplayImage(LPIMGLIST img, HWND hWnd)
{
	DWORD dwResult;
	char szTrace[512];
	char szTempPath[MAX_PATH];
	char szTempFile[MAX_PATH];
	WCHAR wszTempFile[MAX_PATH];

	//Set up Gdiplus
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);	

	//Create file for image
	dwResult = GetTempPath(MAX_PATH, szTempPath);
	dwResult = GetTempFileName(szTempPath, "SCN", 0, szTempFile);
	MultiByteToWideChar(CP_ACP, 0, szTempFile,
		strlen(szTempFile)+1, wszTempFile,
		sizeof(wszTempFile)/sizeof(wszTempFile[0]));

	HANDLE hFile = CreateFile(szTempFile, (GENERIC_READ | GENERIC_WRITE), 
		(FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if((hFile == INVALID_HANDLE_VALUE) || (hFile == NULL))
	{
		sprintf(szTrace, "CreateFile failed for file: %s\n", szTempFile);
		OutputDebugString(szTrace);
		SystemError();
		goto ErrorExit;
	}

	//Save image to file & close handle
	DWORD dwSize = sizeof(img->img);
	LPDWORD lpNumBytesWritten = (LPDWORD) GlobalAlloc(GPTR, img->nImageSize);;
	WriteFile(hFile, img->img, img->nImageSize, lpNumBytesWritten, 0);
	CloseHandle(hFile);

	//Load image for display
	if(m_DisplayImage != NULL)
	{
		delete m_DisplayImage;
	}
	m_DisplayImage = new Image(wszTempFile, FALSE);
	
ErrorExit:

	//Free img list
	LPIMGLIST lpCurrImg = img;
	if (img != NULL)
	{
		do
		{
			lpCurrImg = img;
			img = img->Next;
			GlobalFree(lpCurrImg);
		} while (img != NULL);
	}
	GlobalFree(lpNumBytesWritten);

	InvalidateRect(hWnd, NULL, TRUE);
}