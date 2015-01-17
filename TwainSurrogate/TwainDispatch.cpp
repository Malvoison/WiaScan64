//  Filename:  TwainDispatch.cpp

#include "stdafx.h"
#include "TwainWrapper.h"
#include "TwainClient.h"
#include "TwainThread.h"
#include "Utility.h"
#include "TwainDispatch.h"


//  Globals
HANDLE hWaits[NUM_EVENTS];

LPIMGLIST lpCurrentImage = NULL;
LPBYTE lpImageData = NULL;
int nBytesRemaining = 0;
BOOL bTransferComplete = FALSE;
#define MAX_IMAGE_CHUNK 1592


static DWORD dwTwainThreadId = 0;
static HANDLE hTwainThreadHandle = NULL;

//  GLOBALS FOR SCAN_DIRECT
char szGuid[37];
WCHAR wszGuid[37];
char szWeb[257];
WCHAR wszWeb[257];

class CQuickStringWrap
{
    LPWSTR _szAlloc;

public:
    CQuickStringWrap()
    {
        _szAlloc = NULL;
    }

    ~CQuickStringWrap()
    {
        if (_szAlloc != NULL)
            delete [] _szAlloc;
    }

    operator LPCWSTR() const { return _szAlloc;}

    BOOL Set(LPCWSTR szIn, DWORD dwLen)
    {
        LPWSTR szNew;
        
        szNew = new WCHAR[dwLen+1];

        if (szNew == NULL)
        {
            SetLastError( ERROR_OUTOFMEMORY);
            return FALSE;
        }

        memcpy(szNew, szIn, dwLen*sizeof(WCHAR));
        szNew[dwLen] = L'\0';

        if (_szAlloc != NULL)
            delete [] _szAlloc;

        _szAlloc = szNew;

        return TRUE;
    }
};


DWORD WINAPI CloseSrcProc(LPVOID)
{
	CloseSrc();
	return 0;
}

DWORD WINAPI OpenSrcProc(LPVOID)
{
	OpenSrc();
	return 0;
}

DWORD WINAPI FinishProc(LPVOID)
{
	Finish();
	return 0;
}

DWORD WINAPI InitTwainProc(LPVOID)
{
	InitTwain();
	return 0;
}

DWORD WINAPI ReleaseTwainProc(LPVOID)
{
	ReleaseTwain();
	return 0;
}

DWORD WINAPI GetDefaultSrcProc(LPVOID)
{
	GetDefaultSrc();
	SetEvent(hWaits[GETDEFAULTSRCEVENT]);
	return 0;
}

DWORD WINAPI SelectSrcProc(LPVOID)
{
	SelectSrc();
	SetEvent(hWaits[FNCOMPLETEEVENT]);
	return 0;
}

DWORD WINAPI EnumSrcsProc(LPVOID)
{
	EnumSrcs();
	SetEvent(hWaits[ENUMSRCSEVENT]);
	return 0;
}

DWORD WINAPI SetSelectedSourceProc(LPVOID lpParameter)
{
	char* szSrc = (char*) lpParameter;
	SetSelectedSource((const char*)szSrc);
	GlobalFree(lpParameter);
	SetEvent(hWaits[FNCOMPLETEEVENT]);
	return 0;
}

DWORD WINAPI SetPixelTypeProc(LPVOID lpParameter)
{
	int nPixelType = *((int*)lpParameter);
	SetPixelType(nPixelType);
	GlobalFree(lpParameter);
	SetEvent(hWaits[FNCOMPLETEEVENT]);
	return 0;
}

DWORD WINAPI SetPageSizeProc(LPVOID lpParameter)
{
	int nPageSize = *((int*)lpParameter);
	SetPageSize(nPageSize);
	GlobalFree(lpParameter);
	SetEvent(hWaits[FNCOMPLETEEVENT]);
	return 0;
}

DWORD WINAPI SetFeederEnabledProc(LPVOID lpParameter)
{
	BOOL bFeederEnabled = *((int*)lpParameter);
	SetFeederEnabled(bFeederEnabled);
	GlobalFree(lpParameter);
	SetEvent(hWaits[FNCOMPLETEEVENT]);
	return 0;
}

DWORD WINAPI SetDuplexProc(LPVOID lpParameter)
{
	BOOL bDuplex = *((int*)lpParameter);
	SetDuplex(bDuplex);
	GlobalFree(lpParameter);
	SetEvent(hWaits[FNCOMPLETEEVENT]);
	return 0;
}

DWORD WINAPI EnumPixelTypesProc(LPVOID)
{
	EnumPixelTypes();
	SetEvent(hWaits[ENUMPIXELTYPESEVENT]);
	return 0;
}

DWORD WINAPI SetBitDepthProc(LPVOID lpParameter)
{
	int nBitDepth = *((int*)lpParameter);
	SetBitDepth(nBitDepth);
	GlobalFree(lpParameter);
	SetEvent(hWaits[FNCOMPLETEEVENT]);
	return 0;
}

DWORD WINAPI EnumBitDepthsProc(LPVOID lpParameter)
{
	int nPixelType = *((int*)lpParameter);
	EnumBitDepths(nPixelType);
	GlobalFree(lpParameter);
	SetEvent(hWaits[ENUMBITDEPTHSEVENT]);
	return 0;
}

DWORD WINAPI SetResolutionProc(LPVOID lpParameter)
{
	float fResolution = *((float*)lpParameter);
	SetResolution(fResolution);
	GlobalFree(lpParameter);
	SetEvent(hWaits[FNCOMPLETEEVENT]);
	return 0;
}

DWORD WINAPI GetResolutionRangeProc(LPVOID)
{
	GetResolutionRange();
	SetEvent(hWaits[GETRESOLUTIONRANGEEVENT]);
	return 0;
}

DWORD WINAPI EnumResolutionsProc(LPVOID)
{
	EnumResolutions();
	SetEvent(hWaits[ENUMRESOLUTIONSEVENT]);
	return 0;
}

DWORD WINAPI EnumCapabilitiesProc(LPVOID)
{
	EnumCapabilities();
	SetEvent(hWaits[ENUMCAPSEVENT]);
	return 0;
}

DWORD WINAPI EnumPageSizesProc(LPVOID)
{
	EnumPageSizes();
	SetEvent(hWaits[ENUMPAGESIZESEVENT]);
	return 0;
}

DWORD WINAPI AcquireProc(LPVOID)
{
	Acquire();

	if (bAcquireCancelled)
	{
		bAcquireCancelled = FALSE;
		SetEvent(hWaits[TRANSFERREADYEVENT]);
	}
	return 0;
}

DWORD WINAPI AcquireDirectProc(LPVOID lpParameter)
{
	memset(szGuid, 0, sizeof(szGuid));
	memset(szWeb, 0, sizeof(szWeb));

	char* szParams = reinterpret_cast<char*>(lpParameter);

	memcpy(szGuid, szParams, 36);
	memcpy(szWeb, szParams + 36, 256);

	MultiByteToWideChar(CP_ACP, 0, szGuid,
		strlen(szGuid)+1, wszGuid,
		sizeof(wszGuid)/sizeof(wszGuid[0]));
	MultiByteToWideChar(CP_ACP, 0, szWeb,
		strlen(szWeb)+1, wszWeb,
		sizeof(wszWeb)/sizeof(wszWeb[0]));

	GlobalFree(lpParameter);

	Acquire();

	if (bAcquireCancelled)
	{
		bAcquireCancelled = FALSE;
		SetEvent(hWaits[TRANSFERREADYEVENT]);
	}

	return 0;
}

DWORD WINAPI TransferPicturesProc(LPVOID)
{
	TransferPictures();
	SetEvent(hWaits[TRANSFERPICTURESEVENT]);
	return 0;
}

void CALLBACK AsyncCallback(
	HINTERNET hInternet,
	DWORD_PTR dwContext,
	DWORD dwInternetStatus,
	LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength )
{
	WCHAR wszBuffer[256];

	switch (dwInternetStatus)
	{
	case WINHTTP_CALLBACK_STATUS_RESOLVING_NAME:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_RESOLVING_NAME:  ");
		swprintf(wszBuffer, L"%s\n", (LPWSTR)lpvStatusInformation);
		OutputDebugStringW(wszBuffer);
		break;
	case WINHTTP_CALLBACK_STATUS_NAME_RESOLVED:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_NAME_RESOLVED:  ");
		swprintf(wszBuffer, L"%s\n", (LPWSTR)lpvStatusInformation);
		OutputDebugStringW(wszBuffer);
		break;
	case WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER:  ");
		swprintf(wszBuffer, L"%s\n", (LPWSTR)lpvStatusInformation);
		OutputDebugStringW(wszBuffer);
		break;
	case WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER:  ");
		swprintf(wszBuffer, L"%s\n", (LPWSTR)lpvStatusInformation);
		OutputDebugStringW(wszBuffer);
		break;
	case WINHTTP_CALLBACK_STATUS_SENDING_REQUEST:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_SENDING_REQUEST\n");
		break;
	case WINHTTP_CALLBACK_STATUS_REQUEST_SENT:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_REQUEST_SENT:  ");
		swprintf(wszBuffer, L"%d bytes\n", *((DWORD*)lpvStatusInformation));
		OutputDebugStringW(wszBuffer);
		break;
	case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE\n");
		break;
	case WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED:  ");
		swprintf(wszBuffer, L"%d bytes\n", *((DWORD*)lpvStatusInformation));
		OutputDebugStringW(wszBuffer);
		break;
	case WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION\n");
		break;
	case WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED\n");
		break;
	case WINHTTP_CALLBACK_STATUS_HANDLE_CREATED:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_HANDLE_CREATED\n");
		break;
	case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING\n");
		break;
	case WINHTTP_CALLBACK_STATUS_DETECTING_PROXY:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_DETECTING_PROXY\n");
		break;
	case WINHTTP_CALLBACK_STATUS_REDIRECT:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_REDIRECT:  ");
		swprintf(wszBuffer, L"%s\n", (LPWSTR)lpvStatusInformation);
		OutputDebugStringW(wszBuffer);
		break;
	case WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE:  ");
		swprintf(wszBuffer, L"Status Code = %d\n", *((DWORD*)lpvStatusInformation));
		OutputDebugStringW(wszBuffer);
		break;
	case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
		{
			OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:  ");
			DWORD* dwSSLStat = reinterpret_cast<DWORD*>(lpvStatusInformation);
			switch (*dwSSLStat)
			{
			case WINHTTP_CALLBACK_STATUS_FLAG_CERT_REV_FAILED:
				OutputDebugString("Certificate revocation check failed.\n");
				break;
			case WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CERT:
				OutputDebugString("SSL certificate is invalid.\n");
				break;
			case WINHTTP_CALLBACK_STATUS_FLAG_CERT_REVOKED:
				OutputDebugString("SSL certificate has been revoked.\n");
				break;
			case WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CA:
				OutputDebugString("Invalid certificate authority\n");
				break;
			case WINHTTP_CALLBACK_STATUS_FLAG_CERT_CN_INVALID:
				OutputDebugString("Certificate common name is invalid\n");
				break;
			case WINHTTP_CALLBACK_STATUS_FLAG_CERT_DATE_INVALID:
				OutputDebugString("Certificate date is invalid\n");
				break;
			case WINHTTP_CALLBACK_STATUS_FLAG_CERT_WRONG_USAGE:
				OutputDebugString("WINHTTP_CALLBACK_STATUS_FLAG_CERT_WRONG_USAGE\n");
				break;
			case WINHTTP_CALLBACK_STATUS_FLAG_SECURITY_CHANNEL_ERROR:
				OutputDebugString("WINHTTP_CALLBACK_STATUS_FLAG_SECURITY_CHANNEL_ERROR\n");
				break;
			default:
				OutputDebugString("Unknown SSL Error\n");
				break;
			}
			break;
		}
	case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE\n");
		break;
	case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE\n");
		break;
	case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_READ_COMPLETE:  ");
		swprintf(wszBuffer, L"%d bytes\n", *((DWORD*)lpvStatusInformation));
		OutputDebugStringW(wszBuffer);
		break;
	case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:  ");
		swprintf(wszBuffer, L"%d bytes\n", *((DWORD*)lpvStatusInformation));
		OutputDebugStringW(wszBuffer);
		break;
	case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
		{
			OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:  ");
			WINHTTP_ASYNC_RESULT* war = reinterpret_cast<WINHTTP_ASYNC_RESULT*>(lpvStatusInformation);
			switch (war->dwResult)
			{
			case API_RECEIVE_RESPONSE:
				swprintf(wszBuffer, L"Error in WinHttpReceiveResponse.  Error Code = %\n", war->dwError);
				OutputDebugStringW(wszBuffer);
				break;
			case API_QUERY_DATA_AVAILABLE:
				swprintf(wszBuffer, L"Error in WinHttpQueryDataAvailable.  Error Code = %\n", war->dwError);
				OutputDebugStringW(wszBuffer);
				break;
			case API_READ_DATA:
				swprintf(wszBuffer, L"Error in WinHttpReadData.  Error Code = %\n", war->dwError);
				OutputDebugStringW(wszBuffer);
				break;
			case API_WRITE_DATA:
				swprintf(wszBuffer, L"Error in WinHttpWriteData.  Error Code = %\n", war->dwError);
				OutputDebugStringW(wszBuffer);
				break;
			case API_SEND_REQUEST:
				swprintf(wszBuffer, L"Error in WinHttpSendRequest.  Error Code = %\n", war->dwError);
				OutputDebugStringW(wszBuffer);
				break;
			default:
				OutputDebugString("Unknown Request Error\n");
				break;
			}
			break;
		}
	case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
		OutputDebugString("Internet Status: WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE\n");
		break;
	default:
		OutputDebugString("Internet Status: UNKNOWN\n");
		break;
	}

	return;
}

DWORD WINAPI TransferDirectPicturesProc(LPVOID)
{
	//  do the standard lpImageList thingy
	TransferPictures();

	LPIMGLIST lpList = lpImageList;
	int nCount = 0;
	DWORD dwBytesWritten = 0;
	BOOL bResults = FALSE;
	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
	char szStatusCode[256];
	while (lpList != NULL)
	{
		//  TASK #1 - CREATE THE URL
		WCHAR szServerUrl[512];
		swprintf(szServerUrl, L"https://%s/ScanDirect/ScanDirect.aspx?key=%s&action=PUT&display=%d",
			wszWeb, wszGuid, nCount);
		//  LOCAL VARIABLES
		URL_COMPONENTS urlServerComponents;
		CQuickStringWrap strTargetServer;
		CQuickStringWrap strTargetPath;
		CQuickStringWrap strTargetUsername;
		CQuickStringWrap strTargetPassword;
		
		//  crack the URL
		memset(&urlServerComponents, 0, sizeof(urlServerComponents));
		urlServerComponents.dwStructSize = sizeof(urlServerComponents);
		urlServerComponents.dwHostNameLength = 1;
		urlServerComponents.dwUrlPathLength = 1;
		urlServerComponents.dwUserNameLength = 1;
		urlServerComponents.dwPasswordLength = 1;

		if (!WinHttpCrackUrl(szServerUrl, 0, 0, &urlServerComponents))
		{
			OutputDebugString("--->WinHttpCrackUrl failed\n");
			goto ErrorExit;
		}

		if (urlServerComponents.lpszUserName == NULL)
			urlServerComponents.dwUserNameLength = 0;
		if (urlServerComponents.lpszPassword == NULL)
			urlServerComponents.dwPasswordLength = 0;

		if (!strTargetServer.Set(urlServerComponents.lpszHostName, urlServerComponents.dwHostNameLength) ||
			!strTargetPath.Set(urlServerComponents.lpszUrlPath, urlServerComponents.dwUrlPathLength))
		{
			OutputDebugString("strTarget*.Set failed for HostName or UrlPath!\n");
			goto ErrorExit;
		}

		// for the username and password, if they are empty, leave the string pointers as NULL.
		//  This allows for the current process's default credentials to be used
		if (urlServerComponents.dwUserNameLength != 0
			&& !strTargetUsername.Set(urlServerComponents.lpszUserName, 
									urlServerComponents.dwUserNameLength))
		{
			OutputDebugString("strTarget*.Set failed for UserName\n");
			goto ErrorExit;
		}

		if (urlServerComponents.dwPasswordLength != 0
			&& !strTargetPassword.Set(urlServerComponents.lpszPassword, 
									urlServerComponents.dwPasswordLength))
		{
			OutputDebugString("strTarget*.Set failed for Password\n");
			goto ErrorExit;
		}

		
		//  Open a WinHttp session
		hSession = WinHttpOpen(L"EHS Remote Scanning Client/1.1",
					WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
					WINHTTP_NO_PROXY_NAME,
					WINHTTP_NO_PROXY_BYPASS, 0);
		if (hSession == NULL)
		{
			OutputDebugString("WinHttpOpen failed\n");
			SystemError();
			goto ErrorExit;
		}

				//// Install the status callback function.
        WINHTTP_STATUS_CALLBACK isCallback = WinHttpSetStatusCallback( hSession,
			(WINHTTP_STATUS_CALLBACK)AsyncCallback,
			WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS |
			WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, NULL);

		if (isCallback == WINHTTP_INVALID_STATUS_CALLBACK)
            OutputDebugString("******************* Callback NOT Installed\n");
		else
			OutputDebugString("******************* Callback Installed\n");

		//  Open a connection to the target server

		hConnect = WinHttpConnect(hSession, strTargetServer,
					urlServerComponents.nPort, 0);
		if (hConnect == NULL)
		{
			OutputDebugString("WinHttpConnect failed\n");
			SystemError();
			goto ErrorExit;
		}

		//  Open the request
		hRequest = WinHttpOpenRequest(hConnect, L"POST", strTargetPath,
					NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 
					urlServerComponents.nScheme == INTERNET_SCHEME_HTTPS ?
					WINHTTP_FLAG_SECURE : 0);
		if (hRequest == NULL)
		{
			OutputDebugString("WinHttpOpenRequest failed\n");
			SystemError();
			goto ErrorExit;
		}

		//  send the request
		char szSizeMessage[256];
		sprintf(szSizeMessage, "Trying to send %d bytes\n", lpList->nImageSize);
		OutputDebugString(szSizeMessage);

		if (!WinHttpSendRequest(hRequest,
					WINHTTP_NO_ADDITIONAL_HEADERS, 0,
					WINHTTP_NO_REQUEST_DATA, 0,
					lpList->nImageSize, 0))
		{
			OutputDebugString("WinHttpSendRequest failed\n");
			SystemError();
			goto ErrorExit;
		}
		
		//  write data to the server in 4k chunks
		DWORD dwImageLeft = lpList->nImageSize;
		LPBYTE lpTempImage = lpList->img;
		while (dwImageLeft > 0)
		{
			BYTE buffer[4096];
			if (dwImageLeft > sizeof(buffer))
			{
				memcpy(buffer, lpTempImage, sizeof(buffer));
				lpTempImage += sizeof(buffer);
				dwImageLeft -= sizeof(buffer);
				dwBytesWritten = sizeof(buffer);
			}
			else
			{
				memcpy(buffer, lpTempImage, dwImageLeft);
				dwBytesWritten = dwImageLeft;
				dwImageLeft = 0;
			}

			if (!WinHttpWriteData(hRequest, buffer, dwBytesWritten,
								&dwBytesWritten))
			{
				OutputDebugString("WinHttpWriteData failed: ");
				DWORD dwError = GetLastError();
				switch (dwError)
				{
				case ERROR_WINHTTP_CONNECTION_ERROR:
					OutputDebugString("ERROR_WINHTTP_CONNECTION_ERROR\n");
					break;
				case ERROR_WINHTTP_INCORRECT_HANDLE_STATE:
					OutputDebugString("ERROR_WINHTTP_INCORRECT_HANDLE_STATE\n");
					break;
				case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE:
					OutputDebugString("ERROR_WINHTTP_INCORRECT_HANDLE_TYPE\n");
					break;
				case ERROR_WINHTTP_INTERNAL_ERROR:
					OutputDebugString("ERROR_WINHTTP_INTERNAL_ERROR\n");
					break;
				case ERROR_WINHTTP_OPERATION_CANCELLED:
					OutputDebugString("ERROR_WINHTTP_OPERATION_CANCELLED\n");
					break;
				case ERROR_WINHTTP_TIMEOUT:
					OutputDebugString("ERROR_WINHTTP_TIMEOUT\n");
					break;
				case ERROR_NOT_ENOUGH_MEMORY:
					OutputDebugString("ERROR_NOT_ENOUGH_MEMORY\n");
					break;
				default:
					OutputDebugString("UNKNOWN ERROR\n");
					break;
				}
				goto ErrorExit;
			}
		}
		//  end the request
		if (!WinHttpReceiveResponse(hRequest, NULL))
		{
			SystemError();
			goto ErrorExit;
		}
		
		//  check the status code
		DWORD dwTemp, dwStatusCode;
		dwTemp = sizeof(dwStatusCode);
		if (!WinHttpQueryHeaders(hRequest,
				WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				NULL, &dwStatusCode, &dwTemp, NULL))
		{
			OutputDebugString("WinHttpQueryHeaders failed\n");
			SystemError();
			goto ErrorExit;
		}

		sprintf(szStatusCode, "Server returned status code: %d\n", dwStatusCode);
		OutputDebugString(szStatusCode);

		//  close any open handles
		if (hRequest) WinHttpCloseHandle(hRequest);
		if (hConnect) WinHttpCloseHandle(hConnect);
		if (hSession) WinHttpCloseHandle(hSession);

		nCount++;
		lpList = lpList->Next;
	}

ErrorExit:
	//  close any open handles
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	memset(szGuid, 0, sizeof(szGuid));
	memset(szWeb, 0, sizeof(szWeb));
	memset(wszGuid, 0, sizeof(wszGuid)/sizeof(wszGuid[0]));
	memset(wszWeb, 0, sizeof(szWeb)/sizeof(wszGuid[0]));

	SetEvent(hWaits[TRANSFERDIRECTDONEEVENT]);
	return 0;
}

extern "C" LPSTR __declspec(dllexport) TranslateCommand(int nCmd)
{
	LPSTR szCommand = (LPSTR) GlobalAlloc(GPTR, 256);
	ZeroMemory(szCommand, 256);

	switch (nCmd)
	{
	case PM_CLOSESRC:
		lstrcpy(szCommand, "PM_CLOSESRC");
		break;
	case PM_OPENSRC:
		lstrcpy(szCommand, "PM_OPENSRC");
		break;
	case PM_FINISH:
		lstrcpy(szCommand, "PM_FINISH");
		break;
	case PM_INIT:
		lstrcpy(szCommand, "PM_INIT");
		break;
	case PM_GETDEFAULTSOURCE:
		lstrcpy(szCommand, "PM_GETDEFAULTSOURCE");
		break;
	case PM_SELECTSRC:
		lstrcpy(szCommand, "PM_SELECTSRC");
		break;
	case PM_ENUMSRCS:
		lstrcpy(szCommand, "PM_ENUMSRCS");
		break;
	case PM_SETSELECTEDSOURCE:
		lstrcpy(szCommand, "PM_SETSELECTEDSOURCE");
		break;
	case PM_SETPIXELTYPE:
		lstrcpy(szCommand, "PM_SETPIXELTYPE");
		break;
	case PM_ENUMPIXELTYPES:
		lstrcpy(szCommand, "PM_ENUMPIXELTYPES");
		break;
	case PM_SETBITDEPTH:
		lstrcpy(szCommand, "PM_SETBITDEPTH");
		break;
	case PM_ENUMBITDEPTHS:
		lstrcpy(szCommand, "PM_ENUMBITDEPTHS");
		break;
	case PM_SETRESOLUTION:
		lstrcpy(szCommand, "PM_SETRESOLUTION");
		break;
	case PM_GETRESOLUTIONRANGE:
		lstrcpy(szCommand, "PM_GETRESOLUTIONRANGE");
		break;
	case PM_ENUMRESOLUTIONS:
		lstrcpy(szCommand, "PM_ENUMRESOLUTIONS");
		break;
	case PM_ACQUIRE:
		lstrcpy(szCommand, "PM_ACQUIRE");
		break;
	case PM_TRANSFERPICTURES:
		lstrcpy(szCommand, "PM_TRANSFERPICTURES");
		break;
	case PM_RELEASE:	
		lstrcpy(szCommand, "PM_RELEASE");
		break;
	case PM_PING:
		lstrcpy(szCommand, "PM_PING");
		break;
	case PM_PICSTART:
		lstrcpy(szCommand, "PM_PICSTART");
		break;
	case PM_PICDATA:
		lstrcpy(szCommand, "PM_PICDATA");
		break;
	case PM_PICDONE:
		lstrcpy(szCommand, "PM_PICDONE");
		break;
	case PM_TRANSFERDONE:
		lstrcpy(szCommand, "PM_TRANSFERDONE");
		break;
	case PM_TRANSFERREADY:
		lstrcpy(szCommand, "PM_TRANSFERREADY");
		break;
	default:
		sprintf(szCommand, "UNKNOWN COMMAND: 0x%x", (nCmd - WM_APP));
		break;
	}

	return szCommand;
}

extern "C" void __declspec(dllexport) DataArrival(LPBYTE pBuf, USHORT usLength)
{

	LPTHREAD_START_ROUTINE fnTwain = NULL;
	LPVOID lpvParam = NULL;

	int* pCmd = (int*) pBuf;
	switch (*pCmd)
	{
	case PM_CLOSESRC:
		OutputDebugString("PM_CLOSESRC\n");
		fnTwain = CloseSrcProc;
		break;
	case PM_OPENSRC:
		OutputDebugString("PM_OPENSRC\n");
		fnTwain = OpenSrcProc;
		break;
	case PM_FINISH:
		OutputDebugString("PM_FINISH\n");
		fnTwain = FinishProc;
		break;
	case PM_INIT:
		OutputDebugString("PM_INIT\n");
		fnTwain = InitTwainProc;
		break;
	case PM_GETDEFAULTSOURCE:
		OutputDebugString("PM_GETDEFAULTSOURCE\n");
		fnTwain = GetDefaultSrcProc;
		break;
	case PM_SELECTSRC:
		OutputDebugString("PM_SELECTSRC\n");
		fnTwain = SelectSrcProc;
		break;
	case PM_ENUMSRCS:
		OutputDebugString("PM_ENUMSRCS\n");
		fnTwain = EnumSrcsProc;
		break;
	case PM_SETSELECTEDSOURCE:
		OutputDebugString("PM_SETSELECTEDSOURCE\n");
		fnTwain = SetSelectedSourceProc;
		lpvParam = GlobalAlloc(GPTR, sizeof(TW_STR32));
		memcpy(lpvParam, pBuf + sizeof(int), sizeof(TW_STR32));
		break;
	case PM_SETPIXELTYPE:
		OutputDebugString("PM_SETPIXELTYPE\n");
		fnTwain = SetPixelTypeProc;
		lpvParam = GlobalAlloc(GPTR, sizeof(int));
		memcpy(lpvParam, pBuf + sizeof(int), sizeof(int));
		break;
	case PM_SETPAGESIZE:
		OutputDebugString("PM_SETPAGESIZE\n");
		fnTwain = SetPageSizeProc;
		lpvParam = GlobalAlloc(GPTR, sizeof(int));
		memcpy(lpvParam, pBuf + sizeof(int), sizeof(int));
		break;
	case PM_SETFEEDERENABLED:
		OutputDebugString("PM_SETFEEDERENABLED\n");
		fnTwain = SetFeederEnabledProc;
		lpvParam = GlobalAlloc(GPTR, sizeof(int));
		memcpy(lpvParam, pBuf + sizeof(int), sizeof(int));
		break;
	case PM_SETDUPLEX:
		OutputDebugString("PM_SETDUPLEX\n");
		fnTwain = SetDuplexProc;
		lpvParam = GlobalAlloc(GPTR, sizeof(int));
		memcpy(lpvParam, pBuf + sizeof(int), sizeof(int));
		break;
	case PM_ENUMPIXELTYPES:
		OutputDebugString("PM_ENUMPIXELTYPES\n");
		fnTwain = EnumPixelTypesProc;
		break;
	case PM_SETBITDEPTH:
		OutputDebugString("PM_SETBITDEPTH\n");
		fnTwain = SetBitDepthProc;
		lpvParam = GlobalAlloc(GPTR, sizeof(int));
		memcpy(lpvParam, pBuf + sizeof(int), sizeof(int));
		break;
	case PM_ENUMBITDEPTHS:
		OutputDebugString("PM_ENUMBITDEPTHS\n");
		fnTwain = EnumBitDepthsProc;
		lpvParam = GlobalAlloc(GPTR, sizeof(int));
		memcpy(lpvParam, pBuf + sizeof(int), sizeof(int));
		break;
	case PM_SETRESOLUTION:
		OutputDebugString("PM_SETRESOLUTION\n");
		fnTwain = SetResolutionProc;
		lpvParam = GlobalAlloc(GPTR, sizeof(float));
		memcpy(lpvParam, pBuf + sizeof(int), sizeof(float));
		break;
	case PM_GETRESOLUTIONRANGE:
		OutputDebugString("PM_GETRESOLUTIONRANGE\n");
		fnTwain = GetResolutionRangeProc;
		break;
	case PM_ENUMRESOLUTIONS:
		OutputDebugString("PM_ENUMRESOLUTIONS\n");
		fnTwain = EnumResolutionsProc;
		break;
	case PM_ENUMCAPS:
		OutputDebugString("PM_ENUMCAPS\n");
		fnTwain = EnumCapabilitiesProc;
		break;
	case PM_ENUMPAGESIZES:
		OutputDebugString("PM_ENUMPAGESIZES\n");
		fnTwain = EnumPageSizesProc;
		break;
	case PM_ACQUIRE:
		OutputDebugString("PM_ACQUIRE\n");
		fnTwain = AcquireProc;
		break;
	case PM_TRANSFERPICTURES:
		OutputDebugString("PM_TRANSFERPICTURES\n");
		fnTwain = TransferPicturesProc;
		break;
	case PM_RELEASE:
		OutputDebugString("PM_RELEASE\n");
		fnTwain = ReleaseTwainProc;
		break;
	case PM_PING:
		OutputDebugString("PM_PING\n");
		break;
	case PM_ACQUIREDIRECT:
		OutputDebugString("PM_ACQUIREDIRECT\n");
		lpvParam = GlobalAlloc(GPTR, *(pCmd + 1));
		memcpy(lpvParam, pBuf + 8, *(pCmd + 1));
		fnTwain = AcquireDirectProc;
		break;
	case PM_TRANSFERDIRECTPICTURES:
		OutputDebugString("PM_TRANSFERDIRECTPICTURES\n");
		fnTwain = TransferDirectPicturesProc;
		break;
	case PM_SHOWUI:
		OutputDebugString("PM_SHOWUI\n");
		SetShowUI();
		break;
	case PM_ACKPICDATA:
		if (lpImageList != NULL)
		{
			SetEvent(hWaits[TRANSFERPICTURESEVENT]);
		}
		break;
	case PM_VERCHECK:
		SetEvent(hWaits[VERCHECKEVENT]);
		break;
	default:
		break;
	}

	if (fnTwain != NULL)
	{
		hTwainThreadHandle = CreateThread(NULL, 0, fnTwain, lpvParam, 0, 
			&dwTwainThreadId);
	}

}

extern "C" LPBYTE __declspec(dllexport) Poll()
{
	
	DWORD dwRetVal = WaitForMultipleObjects(NUM_EVENTS, hWaits, FALSE, INFINITE);
	
	if (dwRetVal == WAIT_FAILED)
	{
		SystemError();
		return NULL;
	}

	if (dwRetVal == WAIT_TIMEOUT)
		return NULL;

	switch (dwRetVal - WAIT_OBJECT_0)
	{
	case VERCHECKEVENT:
		{
			char* szRet = (char*) GlobalAlloc(GPTR, (sizeof(int) * 2) + sizeof(int));

			int nCmd = PM_VERCHECK;
			int nSize = sizeof(int);
			int nVersion = SCAN_VERSION;
			memcpy(szRet, &nCmd, sizeof(int));
			memcpy(szRet + sizeof(int), &nSize, sizeof(int));
			memcpy(szRet + (2 * sizeof(int)), &nVersion, sizeof(int));
			ResetEvent(hWaits[VERCHECKEVENT]);
			return (LPBYTE)szRet;
			break;
		}
	case GETDEFAULTSRCEVENT:		//  GetDefaultSrcEvent
		{
			try
			{
				char* szRet = (char*) GlobalAlloc(GPTR, (sizeof(int) * 2) + sizeof(TW_STR32));
				if (szRet == NULL)
				{
					OutputDebugString("***GlobalAlloc failed***\n");
					SystemError();
					throw NULL;
				}
				int nCmd = PM_GETDEFAULTSOURCE;
				int nSize = sizeof(TW_STR32);
				
				memcpy(szRet, &nCmd, sizeof(int));
				memcpy(szRet + sizeof(int), &nSize, sizeof(int));
				memcpy(szRet + (2 * sizeof(int)), szDefaultSrc, sizeof(TW_STR32));
				ResetEvent(hWaits[GETDEFAULTSRCEVENT]);
				return (LPBYTE)szRet;
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: GETDEFAULTSRCEVENT\n");
			}
		}
	case ENUMSRCSEVENT:		//  EnumSrcsEvent
		{
			try
			{
				// count the number of sources in the list
				int nCount = 0;
				LPSRCLIST lpTemp = lpSrcList;
				while (lpTemp != NULL)
				{
					nCount++;
					lpTemp = lpTemp->Next;
				}
				char* szRet = (char*) GlobalAlloc(GPTR, (sizeof(int) * 2) + (sizeof(TW_STR32) * nCount));
				if (szRet == NULL)
				{
					OutputDebugString("***GlobalAlloc failed***\n");
					SystemError();
					throw NULL;
				}
				int nCmd = PM_ENUMSRCS;
				int nSize = sizeof(TW_STR32) * nCount;
				memcpy(szRet, &nCmd, sizeof(int));
				memcpy(szRet + sizeof(int), &nSize, sizeof(int));
				if (lpSrcList != NULL)
				{
					lpTemp = lpSrcList;
					char* szSrc = szRet + (2 * sizeof(int));
					for (int i = 0; i < nCount; i++)
					{
						memcpy(szSrc, lpTemp->src, sizeof(TW_STR32));
						szSrc += sizeof(TW_STR32);
						lpTemp = lpTemp->Next;
					}
				}
				ResetEvent(hWaits[ENUMSRCSEVENT]);
				//  free the src list
				lpTemp = lpSrcList;
				while (lpTemp != NULL)
				{
					LPSRCLIST lpDel = lpTemp;
					lpTemp = lpTemp->Next;
					GlobalFree(lpDel);
				}
				lpSrcList = NULL;

				return (LPBYTE)szRet;
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: ENUMSRCSEVENT\n");
			}
		}
	case ENUMPIXELTYPESEVENT:		//  EnumPixelTypesEvent
		{
			try
			{
				//  count the number of pixel types in the list
				int nCount = 0;
				LPPIXELTYPELIST lpTemp = lpPixelTypeList;
				while (lpTemp != NULL)
				{
					nCount++;
					lpTemp = lpTemp->Next;
				}
				int* pRet = (int*) GlobalAlloc(GPTR, (sizeof(int) * 2) + (sizeof(int) * nCount));
				if (pRet == NULL)
				{
					OutputDebugString("***GlobalAlloc failed***\n");
					SystemError();
					throw NULL;
				}
				int nCmd = PM_ENUMPIXELTYPES;
				int nSize = sizeof(int) * nCount;
				memcpy(pRet, &nCmd, sizeof(int));
				memcpy(pRet + 1, &nSize, sizeof(int));
				if (lpPixelTypeList != NULL)
				{
					lpTemp = lpPixelTypeList;
					int* pPix = pRet + 2;
					for (int i = 0; i < nCount; i++)
					{
						memcpy(pPix, &lpTemp->nType, sizeof(int));
						pPix++;
						lpTemp = lpTemp->Next;
					}
				}
				ResetEvent(hWaits[ENUMPIXELTYPESEVENT]);
				//  free the pixel list
				lpTemp = lpPixelTypeList;
				while (lpTemp != NULL)
				{
					LPPIXELTYPELIST lpDel = lpTemp;
					lpTemp = lpTemp->Next;
					GlobalFree(lpDel);
				}
				lpPixelTypeList = NULL;

				return (LPBYTE)pRet;
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: ENUMPIXELTYPESEVENT\n");
			}
		}
	case ENUMBITDEPTHSEVENT:		//  EnumBitDepthsEvent
		//  count the number of Bit Depths in the list
		{
			try
			{
				int nCount = 0;
				LPBITDEPTHLIST lpTemp = lpBitDepthList;
				while (lpTemp != NULL)
				{
					nCount++;
					lpTemp = lpTemp->Next;
				}
				int* pRet = (int*) GlobalAlloc(GPTR, (sizeof(int) * 2) + (sizeof(int) * nCount));
				if (pRet ==	NULL)
				{
					OutputDebugString("***GlobalAlloc failed***\n");
					SystemError();
					throw NULL;
				}
				int nCmd = PM_ENUMBITDEPTHS;
				int nSize = sizeof(int) * nCount;
				memcpy(pRet, &nCmd, sizeof(int));
				memcpy(pRet + 1, &nSize, sizeof(int));
				if (lpBitDepthList != NULL)
				{
					lpTemp = lpBitDepthList;
					int* pBits = pRet + 2;
					for (int i = 0; i < nCount; i++)
					{
						memcpy(pBits, &lpTemp->nDepth, sizeof(int));
						pBits++;
						lpTemp = lpTemp->Next;
					}
				}
				ResetEvent(hWaits[ENUMBITDEPTHSEVENT]);
				//  free the bit depth list
				lpTemp = lpBitDepthList;
				while (lpTemp != NULL)
				{
					LPBITDEPTHLIST lpDel = lpTemp;
					lpTemp = lpTemp->Next;
					GlobalFree(lpDel);
				}
				lpBitDepthList = NULL;

				return (LPBYTE)pRet;
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: ENUMBITDEPTHSEVENT\n");
			}
		}
	case GETRESOLUTIONRANGEEVENT:		//  GetResolutionRangeEvent
		{
			try
			{
				if (lpResolutionRange != NULL)
				{
					char* szRet = (char*) GlobalAlloc(GPTR, (sizeof(int) * 2) + (sizeof(RESOLUTIONRANGE)));
					if (szRet == NULL)
					{
						OutputDebugString("***GlobalAlloc failed***\n");
						SystemError();
						throw NULL;
					}
					int nCmd = PM_GETRESOLUTIONRANGE;
					int nSize = sizeof(RESOLUTIONRANGE);
					memcpy(szRet, &nCmd, sizeof(int));
					memcpy(szRet + sizeof(int), &nSize, sizeof(int));
					memcpy(szRet + (2 * sizeof(int)), lpResolutionRange, sizeof(RESOLUTIONRANGE));
					ResetEvent(hWaits[4]);
					
					GlobalFree(lpResolutionRange);
					lpResolutionRange = NULL;

					return (LPBYTE)szRet;
				}
				else
				{
					ResetEvent(hWaits[GETRESOLUTIONRANGEEVENT]);
					return NULL;
				}
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: GETRESOLUTIONRANGEEVENT\n");
			}
		}
	case ENUMRESOLUTIONSEVENT:		//  EnumResolutionsEvent
		{
			try
			{
				//  count the number of resolutions in the list
				int nCount = 0;
				LPRESOLUTIONLIST lpTemp = lpResolutionList;
				while (lpTemp != NULL)
				{
					nCount++;
					lpTemp = lpTemp->Next;
				}
				int* pRet = (int*) GlobalAlloc(GPTR, (sizeof(int) * 2) + (sizeof(int) * 2 * nCount));
				if (pRet == NULL)
				{
					OutputDebugString("***GlobalAlloc failed***\n");
					SystemError();
					throw NULL;
				}
				int nCmd = PM_ENUMRESOLUTIONS;
				int nSize = sizeof(int) * 2 * nCount;
				memcpy(pRet, &nCmd, sizeof(int));
				memcpy(pRet + 1, &nSize, sizeof(int));
				if (lpResolutionList != NULL)
				{
					lpTemp = lpResolutionList;
					int* pRes = pRet + 2;
					for (int i = 0; i < nCount; i++)
					{
						memcpy(pRes, &lpTemp->Whole, sizeof(int));
						pRes++;
						memcpy(pRes, &lpTemp->Fraction, sizeof(int));
						pRes++;
						lpTemp = lpTemp->Next;
					}
				}
				ResetEvent(hWaits[ENUMRESOLUTIONSEVENT]);
				//  free the resolution list
				lpTemp = lpResolutionList;
				while (lpTemp != NULL)
				{
					LPRESOLUTIONLIST lpDel = lpTemp;
					lpTemp = lpTemp->Next;
					GlobalFree(lpDel);
				}
				lpResolutionList = NULL;

				return (LPBYTE)pRet;
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: ENUMRESOLUTIONSEVENT\n");
			}
		}
	case ENUMCAPSEVENT:
		{
			try
			{
				//  count the number of caps in the list
				int nCount = 0;
				LPCAPLIST lpTemp = lpCapList;
				while (lpTemp != NULL)
				{
					nCount++;
					lpTemp = lpTemp->Next;
				}

				int* pRet = (int*) GlobalAlloc(GPTR, (sizeof(int) * 2) + sizeof(int) * nCount);
				if (pRet == NULL)
				{
					OutputDebugString("***GlobalAlloc failed***\n");
					SystemError();
					throw NULL;
				}
				int nCmd = PM_ENUMCAPS;
				int nSize = sizeof(int) * nCount;
				memcpy(pRet, &nCmd, sizeof(int));
				memcpy(pRet + 1, &nSize, sizeof(int));
				if (lpCapList != NULL)
				{
					lpTemp = lpCapList;
					int* pRes = pRet + 2;
					for (int i = 0; i < nCount; i++)
					{
						memcpy(pRes, &lpTemp->nCap, sizeof(int));
						pRes++;
						lpTemp = lpTemp->Next;
					}
				}

				ResetEvent(hWaits[ENUMCAPSEVENT]);
				//  free the capabilities list
				lpTemp = lpCapList;
				while (lpTemp != NULL)
				{
					LPCAPLIST lpDel = lpTemp;
					lpTemp = lpTemp->Next;
					GlobalFree(lpDel);
				}
				lpCapList = NULL;

				return (LPBYTE)pRet;
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: ENUMCAPSEVENT\n");
			}
		}
	case ENUMPAGESIZESEVENT:
		{
			try
			{
				//  count the number of caps in the list
				int nCount = 0;
				LPPAGESIZELIST lpTemp = lpPageSizeList;
				while (lpTemp != NULL)
				{
					nCount++;
					lpTemp = lpTemp->Next;
				}

				int* pRet = (int*) GlobalAlloc(GPTR, (sizeof(int) * 2) + sizeof(int) * nCount);
				if (pRet == NULL)
				{
					OutputDebugString("***GlobalAlloc failed***\n");
					SystemError();
					throw NULL;
				}
				int nCmd = PM_ENUMPAGESIZES;
				int nSize = sizeof(int) * nCount;
				memcpy(pRet, &nCmd, sizeof(int));
				memcpy(pRet + 1, &nSize, sizeof(int));
				if (lpCapList != NULL)
				{
					lpTemp = lpPageSizeList;
					int* pRes = pRet + 2;
					for (int i = 0; i < nCount; i++)
					{
						memcpy(pRes, &lpTemp->nPageSize, sizeof(int));
						pRes++;
						lpTemp = lpTemp->Next;
					}
				}

				ResetEvent(hWaits[ENUMPAGESIZESEVENT]);
				//  free the capabilities list
				lpTemp = lpPageSizeList;
				while (lpTemp != NULL)
				{
					LPPAGESIZELIST lpDel = lpTemp;
					lpTemp = lpTemp->Next;
					GlobalFree(lpDel);
				}
				lpPageSizeList = NULL;

				return (LPBYTE)pRet;
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: ENUMPAGESIZESEVENT\n");
			}
		}
	case TRANSFERPICTURESEVENT:		//  TransferPicturesEvent
		{
			try
			{
				//  Check to see if we're done
				if (bTransferComplete)
				{
					try
					{
		//				DebugTraceMessage("Sending PM_TRANSFERDONE\n");
						bTransferComplete = FALSE;
						lpCurrentImage = NULL;
						lpImageData = NULL;
						nBytesRemaining = NULL;
		//				ResetEvent(hWaits[TRANSFERPICTURESEVENT]);
						int nCmd = PM_TRANSFERDONE;
						int nSize = 0;
						int* pRet = (int*) GlobalAlloc(GPTR, (2 * sizeof(int)));
						if (pRet == NULL)
						{
							OutputDebugString("***GlobalAlloc failed***\n");
							SystemError();
							throw NULL;
						}
						memcpy(pRet, &nCmd, sizeof(int));
						memcpy(pRet + 1, &nSize, sizeof(int));
				
						//  FREE THE IMAGES
						LPIMGLIST lpFree = lpImageList;
						while (lpFree != NULL)
						{
							LPIMGLIST lpItem = lpFree;

							GlobalFree(lpFree->img);
							lpFree = lpFree->Next;

							GlobalFree(lpItem);
						}

						lpImageList = NULL;

						DebugTraceMessage("Sending PM_TRANSFERDONE\n");
						ResetEvent(hWaits[TRANSFERPICTURESEVENT]);
						return (LPBYTE)pRet;
					}
					catch (...)
					{
						OutputDebugString("TRANSFERPICTURESEVENT Exception: PM_TRANSFERDONE\n");
						throw NULL;
					}
	//				return NULL;
				}
				//  First Time thru
				if (lpCurrentImage == NULL)
				{
					try
					{
						lpCurrentImage = lpImageList;
						//  if, for some reason, we got no pictures back from the scanner,
						//  send the PM_TRANSFERDONE reply
						if (lpCurrentImage == NULL)
						{
							try
							{
								bTransferComplete = FALSE;
								lpCurrentImage = NULL;
								lpImageData = NULL;
								nBytesRemaining = NULL;

								int nCmd = PM_TRANSFERDONE;
								int nSize = 0;
								int* pRet = (int*) GlobalAlloc(GPTR, (2 * sizeof(int)));
								if (pRet == NULL)
								{
									OutputDebugString("***GlobalAlloc failed***\n");
									SystemError();
									throw NULL;
								}
								memcpy(pRet, &nCmd, sizeof(int));
								memcpy(pRet + 1, &nSize, sizeof(int));
								ResetEvent(hWaits[TRANSFERPICTURESEVENT]);
								OutputDebugString("No images...sending PM_TRANSFERDONE\n");
								return (LPBYTE)pRet;
							}
							catch (...)
							{
								OutputDebugString("TRANSFERPICTURESEVENT Exception: PM_TRANSFERDONE\n");
								throw NULL;
							}
						}
						lpImageData = lpCurrentImage->img;
						nBytesRemaining = lpCurrentImage->nImageSize;
						DebugTraceMessage("First image size: %d\n", nBytesRemaining);
						int* pRet = (int*) GlobalAlloc(GPTR, 2 * sizeof(int));
						if (pRet == NULL)
						{
							OutputDebugString("***GlobalAlloc failed***\n");
							SystemError();
							throw NULL;
						}
						int nCmd = PM_PICSTART;
						int nSize = 0;
						memcpy(pRet, &nCmd, sizeof(int));
						memcpy(pRet + 1, &nSize, sizeof(int));
						//  WAIT FOR ACK
						ResetEvent(hWaits[TRANSFERPICTURESEVENT]);
						DebugTraceMessage("Sending PM_PICSTART\n");
						return (LPBYTE)pRet;
					}
					catch (...)
					{
						OutputDebugString("TRANSFERPICTURESEVENT Exception: PM_PICSTART\n");
						throw NULL;
					}
				}
				//  Middle time thru
				if (nBytesRemaining > MAX_IMAGE_CHUNK)
				{
					try
					{
						int nCmd = PM_PICDATA;
						int nSize = MAX_IMAGE_CHUNK;
						char* szRet = (char*) GlobalAlloc(GPTR, (2 * sizeof(int)) + MAX_IMAGE_CHUNK);
						if (szRet == NULL)
						{
							OutputDebugString("***GlobalAlloc failed***\n");
							SystemError();
							throw NULL;
						}
						memcpy(szRet, &nCmd, sizeof(int));
						memcpy(szRet + sizeof(int), &nSize, sizeof(int));
						memcpy(szRet + (2 * sizeof(int)), lpImageData, MAX_IMAGE_CHUNK);
						//  update the pointer
						lpImageData += MAX_IMAGE_CHUNK;
						nBytesRemaining -= MAX_IMAGE_CHUNK;
		//				DebugTraceMessage("Sending PM_PICDATA.  Bytes remaining: %d\n", nBytesRemaining);
						//  WAIT FOR ACK
						ResetEvent(hWaits[TRANSFERPICTURESEVENT]);
						return (LPBYTE)szRet;
					}
					catch (...)
					{
						OutputDebugString("TRANSFERPICTURESEVENT Exception: PM_PICDATA\n");
					}
				}
				//  last time thru for a pic
				if (nBytesRemaining <= MAX_IMAGE_CHUNK)
				{
					try
					{
						DebugTraceMessage("Sending PM_PICDONE\n");
						int nCmd = PM_PICDONE;
						int nSize = nBytesRemaining;
						char* szRet = (char*) GlobalAlloc(GPTR, (2 * sizeof(int)) + nBytesRemaining);
						if (szRet == NULL)
						{
							OutputDebugString("***GlobalAlloc failed***\n");
							SystemError();
							throw NULL;
						}
						memcpy(szRet, &nCmd, sizeof(int));
						memcpy(szRet + sizeof(int), &nSize, sizeof(int));
						memcpy(szRet + (2 * sizeof(int)), lpImageData, nBytesRemaining);

						nBytesRemaining = 0;
						lpCurrentImage = lpCurrentImage->Next;
						if (lpCurrentImage == NULL)
						{
							bTransferComplete = TRUE;
						}
						else
						{
							lpImageData = lpCurrentImage->img;
							nBytesRemaining = lpCurrentImage->nImageSize;
						}

						//  WAIT FOR ACK
						ResetEvent(hWaits[TRANSFERPICTURESEVENT]);
						return (LPBYTE)szRet;
		//				return NULL;
					}
					catch (...)
					{
						OutputDebugString("TRANSFERPICTURESEVENT Exception: PM_PICDONE\n");
						throw NULL;
					}
				}

				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: TRANSFERPICTURESEVENT\n");
			}
		}
	case TRANSFERREADYEVENT:
		{
			try
			{
				int nCmd = (szGuid[0] == 0) ? PM_TRANSFERREADY : PM_TRANSFERDIRECTREADY;
				int nSize = 0;
				int* pRet = (int*) GlobalAlloc(GPTR, (2 * sizeof(int)));
				if (pRet == NULL)
				{
					OutputDebugString("***GlobalAlloc failed***\n");
					SystemError();
					throw NULL;
				}
				memcpy(pRet, &nCmd, sizeof(int));
				memcpy(pRet + 1, &nSize, sizeof(int));
				ResetEvent(hWaits[TRANSFERREADYEVENT]);
				return (LPBYTE)pRet;
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: TRANSFERREADYEVENT\n");
			}
		}
	case FNCOMPLETEEVENT:
		{
			try
			{
				OutputDebugString("FNCOMPLETEEVENT\n");
				int nCmd = PM_FNCOMPLETE;
				int nSize = 0;
				int* pRet = (int*) GlobalAlloc(GPTR, (2 * sizeof(int)));
				if (pRet == NULL)
				{
					OutputDebugString("***GlobalAlloc failed***\n");
					SystemError();
					throw NULL;
				}
				memcpy(pRet, &nCmd, sizeof(int));
				memcpy(pRet + 1, &nSize, sizeof(int));
				ResetEvent(hWaits[FNCOMPLETEEVENT]);
				return (LPBYTE)pRet;
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: FNCOMPLETEEVENT\n");
			}
		}
	case TRANSFERDIRECTDONEEVENT:
		{
			try
			{
				OutputDebugString("TRANSFERDIRECTDONEEVENT\n");
				int nCmd = PM_TRANSFERDIRECTDONE;
				int nSize = 0;
				int* pRet = (int*) GlobalAlloc(GPTR, (2 * sizeof(int)));
				if (pRet == NULL)
				{
					OutputDebugString("***GlobalAlloc failed***\n");
					SystemError();
					throw NULL;
				}
				memcpy(pRet, &nCmd, sizeof(int));
				memcpy(pRet + 1, & nSize, sizeof(int));
				ResetEvent(hWaits[TRANSFERDIRECTDONEEVENT]);
				return (LPBYTE)pRet;
				break;
			}
			catch (...)
			{
				OutputDebugString("Poll Exception: TRANSFERDIRECTDONEEVENT\n");
			}
		}
	default:
		break;
	}

	return NULL;
}

//  DUMMY IMPLEMENTATION FOR TESTING PURPOSES

extern "C" HANDLE __declspec(dllexport) WTSVirtualChannelOpen(
	HANDLE hServer, DWORD SessionId, LPSTR pVirtualName)
{
	return (HANDLE) 666;
}

extern "C" BOOL __declspec(dllexport) WTSVirtualChannelWrite(
	HANDLE hChannelHandle, PCHAR Buffer, ULONG Length, PULONG pBytesWritten)
{
	DataArrival((LPBYTE)Buffer, (USHORT)Length);
	*pBytesWritten = Length;

	return TRUE;
}

extern "C" BOOL __declspec(dllexport) WTSVirtualChannelRead(
	HANDLE hChannelHandle, ULONG TimeOut, PCHAR Buffer,
	ULONG BufferSize, PULONG pBytesRead)
{
	LPBYTE lpBuff = NULL;
	ULONG ulExpiration = 0;

	if (TimeOut == 0xFFFF)
	{
		while (!(lpBuff = Poll()))
		{
			Sleep(100);
		}
	}
	else
	{
		while(!(lpBuff = Poll()))
		{
			Sleep(100);
			ulExpiration += 100;
			if (ulExpiration == TimeOut)
			{
				*pBytesRead = 0;
				return FALSE;
			}
		}
	}

	int nCmd = *((int*)lpBuff);
	int nLength = 0;
	switch (nCmd)
	{
	case PM_PICSTART:
	case PM_TRANSFERDONE:
		nLength = (2 * sizeof(int));
		memcpy(Buffer, &nCmd, sizeof(int));
		*pBytesRead = nLength;
		break;
	default:
		nLength = (2 * sizeof(int)) + *((int*)(lpBuff + sizeof(int)));
		*pBytesRead = nLength;
		memcpy(Buffer, lpBuff, nLength);
		break;
	}

	GlobalFree(lpBuff);

	return TRUE;
}

extern "C" BOOL __declspec(dllexport) WTSVirtualChannelClose(
	HANDLE hChannelHandle)
{
	return TRUE;
}
