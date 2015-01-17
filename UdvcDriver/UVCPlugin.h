// UVCPlugin.h : Declaration of the CUVCPlugin

#pragma once
#include "resource.h"       // main symbols



#include "UdvcDriver_i.h"



using namespace ATL;


// CUVCPlugin

class ATL_NO_VTABLE CUVCPlugin :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CUVCPlugin, &CLSID_UVCPlugin>,	
	public IWTSPlugin
{
public:
	CUVCPlugin()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_UVCPLUGIN)

DECLARE_NOT_AGGREGATABLE(CUVCPlugin)

BEGIN_COM_MAP(CUVCPlugin)
	COM_INTERFACE_ENTRY(IWTSPlugin)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	//   IWTSPlugin
	//
	HRESULT STDMETHODCALLTYPE Initialize(IWTSVirtualChannelManager *pChannelMgr) { return S_OK; }

	HRESULT STDMETHODCALLTYPE Connected()
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Disconnected(DWORD dwDisconnectCode)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Terminated()
	{
		return S_OK;
	}

};

OBJECT_ENTRY_AUTO(__uuidof(UVCPlugin), CUVCPlugin)
