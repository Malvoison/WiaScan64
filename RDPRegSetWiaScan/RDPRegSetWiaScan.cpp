// RDPRegSetWiaScan.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "RDPRegSetWiaScan.h"

int APIENTRY _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//  Add the terminal services addin key
	//  for dictation if it does not already exist

	HKEY hkScanKey = NULL;
	const wchar_t* szBaseScanKey = 
		L"SOFTWARE\\Microsoft\\Terminal Server Client\\Default\\AddIns\\VCScanWiaRDP";
	LONG lResult = RegOpenKey(HKEY_CURRENT_USER, szBaseScanKey, &hkScanKey);
	if (lResult != ERROR_SUCCESS)  // if not there, add it
	{
		//  create the key
		lResult = RegCreateKey(HKEY_CURRENT_USER, szBaseScanKey, &hkScanKey);
		if (lResult != ERROR_SUCCESS)
		{
			MessageBox(NULL, L"Error configuring EHS Remote Scanning.  Please contact your system administrator.",
				L"Configuration Error", MB_ICONSTOP | MB_OK);
			goto ErrorExit;
		}

		//  create/set the string value
		wchar_t* szValue = L"C:\\Program Files\\SuccessEHS\\Scanning\\vcscanwiardp.dll";
		DWORD dwType = REG_SZ;
		DWORD dwSize = lstrlen(szValue) * sizeof(TCHAR);
		lResult = RegSetValueEx(hkScanKey, L"Name", NULL, dwType,
			reinterpret_cast<LPBYTE>(szValue), dwSize);
		if (lResult != ERROR_SUCCESS)
		{
			MessageBox(NULL, L"Error configuring SuccessEHS Remote Scanning.  Please contact your system administrator.",
				L"Configuration Error", MB_ICONSTOP | MB_OK);
			goto ErrorExit;
		}

		DWORD dwValue = 4;
		dwSize = sizeof(DWORD);
		lResult = RegSetValueEx(hkScanKey, L"LogLevel", NULL, REG_DWORD, (LPBYTE)&dwValue, dwSize);
		if (lResult != ERROR_SUCCESS)
		{
			MessageBox(NULL, L"Error configuring SuccessEHS Remote Scanning.  Please contact your system administrator.",
				L"Configuration Error", MB_ICONSTOP | MB_OK);
			goto ErrorExit;
		}
	}

ErrorExit:
	if (hkScanKey != NULL)
	{
		RegCloseKey(hkScanKey);
	}

	return lResult;
}
