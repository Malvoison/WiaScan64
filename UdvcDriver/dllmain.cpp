// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "UdvcDriver_i.h"
#include "dllmain.h"
#include "xdlldata.h"

CUdvcDriverModule _AtlModule;

class CUdvcDriverApp : public CWinApp
{
public:

// Overrides
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CUdvcDriverApp, CWinApp)
END_MESSAGE_MAP()

CUdvcDriverApp theApp;

BOOL CUdvcDriverApp::InitInstance()
{
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(m_hInstance, DLL_PROCESS_ATTACH, NULL))
		return FALSE;
#endif
	return CWinApp::InitInstance();
}

int CUdvcDriverApp::ExitInstance()
{
	return CWinApp::ExitInstance();
}
