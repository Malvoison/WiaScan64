//  Filename:  DynamicImpl.cpp
//  Author:  Kenneth E. Watts
//  Purpose:  yes

#include "stdafx.h"
#include "ClientDBWinChannel.h"

//  Definition of Univeral Plugin

class CUVCPlugin : public IWTSPlugin
{
	LONG               m_cRef;
public:
	CUVCPlugin()
	{
		m_cRef = 0;
	}
    // IUnknown interface
	//
    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

	//   IWTSPlugin
	//
	HRESULT STDMETHODCALLTYPE Initialize(IWTSVirtualChannelManager *pChannelMgr);

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

STDMETHODIMP CUVCPlugin::QueryInterface(REFIID iid, LPVOID *ppvObj)
{
	if (ppvObj == NULL)
	{
		return E_POINTER;
	}

	if (iid == IID_IUnknown)
	{
		*ppvObj = (IUnknown*) this;
	}
	else if (iid == IID_IWTSPlugin)
	{
		*ppvObj = (IWTSPlugin*) this;
	}
	else
	{
		*ppvObj = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CUVCPlugin::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CUVCPlugin::Release()
{
	LONG cRef = InterlockedDecrement(&m_cRef);

	if (cRef == 0)
	{
		delete this;
	}

	return cRef;
}


//  IWTSPlugin::Initialize implementation
//
HRESULT CUVCPlugin::Initialize(__in IWTSVirtualChannelManager *pChannelMgr)
{
	HRESULT hr = S_OK;

	//  Initialize the ClientDBWin channel
	OutputDebugString(L"CUCPlugin::Initialize\n");
	CClientDBWinListenerCallback* pListenerCallback = new CClientDBWinListenerCallback();	
	IWTSListener* pListener = NULL;
	hr = pListenerCallback->QueryInterface(IID_IWTSListenerCallback, reinterpret_cast<void**>(&pListenerCallback));
	if (SUCCEEDED(hr))
	{
		hr = pChannelMgr->CreateListener("ClientDBWin", 0, pListenerCallback, &pListener);
	}

	return hr;
}

HRESULT VCAPITYPE VirtualChannelGetInstance(
	REFIID refiid,
	ULONG *pNumObjs,
	VOID **ppObjArray)
{
	OutputDebugString(L"VirtualChannelGetInstance\n");
	HRESULT hr;

	if (ppObjArray == NULL)
	{
		*pNumObjs = 1;
		return S_OK;
	}

	CUVCPlugin* pUVCPlugin;
	IWTSPlugin* ptrUVCPlugin;

	pUVCPlugin = new CUVCPlugin();
	
	hr = pUVCPlugin->QueryInterface(IID_IWTSPlugin, reinterpret_cast<void**>(&ptrUVCPlugin));
	if (SUCCEEDED(hr))
	{
		ppObjArray[0] = ptrUVCPlugin;
	}

	return hr;
}