//  Filename:  ClientDBWinChannel.cpp

#include "stdafx.h"
#include "ClientDBWinChannel.h"


STDMETHODIMP CClientDBWinChannelCallback::QueryInterface(REFIID iid, LPVOID *ppvObj)
{
	if (ppvObj == NULL)
	{
		return E_POINTER;
	}

	if (iid == IID_IUnknown)
	{
		*ppvObj = (IUnknown*) this;
	}
	else if (iid == IID_IWTSVirtualChannelCallback)
	{
		*ppvObj = (IWTSVirtualChannelCallback*) this;
	}
	else
	{
		*ppvObj = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CClientDBWinChannelCallback::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CClientDBWinChannelCallback::Release()
{
	LONG cRef = InterlockedDecrement(&m_cRef);

	if (cRef == 0)
	{
		delete this;
	}

	return cRef;
}

STDMETHODIMP CClientDBWinListenerCallback::QueryInterface(REFIID iid, LPVOID *ppvObj)
{
	if (ppvObj == NULL)
	{
		return E_POINTER;
	}

	if (iid == IID_IUnknown)
	{
		*ppvObj = (IUnknown*) this;
	}
	else if (iid == IID_IWTSListenerCallback)
	{
		*ppvObj = (IWTSListenerCallback*) this;
	}
	else
	{
		*ppvObj = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CClientDBWinListenerCallback::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CClientDBWinListenerCallback::Release()
{
	LONG cRef = InterlockedDecrement(&m_cRef);

	if (cRef == 0)
	{
		delete this;
	}

	return cRef;
}

//  IWTSListenerCallback::OnNewChannelConnection
//
HRESULT CClientDBWinListenerCallback::OnNewChannelConnection(
	__in IWTSVirtualChannel *pChannel,
	__in_opt BSTR data,
	__out BOOL *pbAccept,
	__out IWTSVirtualChannelCallback **ppCallback)
{
	HRESULT hr;
	CComObject<CClientDBWinChannelCallback> *pCallback;
	CComPtr<CClientDBWinChannelCallback> ptrCallback;

	CClientDBWinChannelCallback *callback = new CClientDBWinChannelCallback();
	callback->SetChannel(pChannel);
	IWTSVirtualChannelCallback* pChannelCallback = NULL;
	hr = callback->QueryInterface(IID_IWTSVirtualChannelCallback, reinterpret_cast<void**>(&pChannelCallback));
	*ppCallback = pChannelCallback;

	*pbAccept = TRUE;

	return hr;
}