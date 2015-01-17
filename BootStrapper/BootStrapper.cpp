// BootStrapper.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

using namespace ATL;

#define CHECK_QUIT_HT(_x_)		if (FAILED(hr)) { return hr; }

class ATL_NO_VTABLE CWIAPlugin :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IWTSPlugin
{
public:

	BEGIN_COM_MAP(CWIAPlugin)
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

	//   IWTSPlugin
	//
	HRESULT STDMETHODCALLTYPE Initialize(IWTSVirtualChannelManager *pChannelMgr);

	HRESULT STDMETHODCALLTYPE Connected()
	{
		Connected();
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

class ATL_NO_VTABLE CWiaChannelCallback :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IWTSVirtualChannelCallback
{
	CComPtr<IWTSVirtualChannel> m_ptrChannel;

public:

	BEGIN_COM_MAP(CWiaChannelCallback)
		COM_INTERFACE_ENTRY(IWTSVirtualChannelCallback)
	END_COM_MAP()

	VOID SetChannel(IWTSVirtualChannel *pChannel)
	{
		m_ptrChannel = pChannel;
	}

	//  IWTSVirtualChannelCallback
	//
	HRESULT STDMETHODCALLTYPE OnDataReceived(ULONG cbSize, __in_bcount(cbSize) BYTE *pBuffer)
	{
		return m_ptrChannel->Write(cbSize, pBuffer, NULL);
	}

	HRESULT STDMETHODCALLTYPE OnClose()
	{
		return m_ptrChannel->Close();
	}
};

class ATL_NO_VTABLE CWiaListenerCallback :
	public  CComObjectRootEx<CComMultiThreadModel>,
	public IWTSListenerCallback
{
public:

	BEGIN_COM_MAP(CWiaListenerCallback)
		COM_INTERFACE_ENTRY(IWTSListenerCallback)
	END_COM_MAP()

	HRESULT STDMETHODCALLTYPE  OnNewChannelConnection(
		__in IWTSVirtualChannel *pChannel,
		__in_opt BSTR data,
		__out  BOOL *pbAccept,
		__out IWTSVirtualChannelCallback **ppCallback);
};

HRESULT VCAPITYPE VirtualChannelGetInstance(
	REFIID refiid,
	ULONG *pNumObjs,
	VOID **ppObjArray)
{



	return S_OK;
}