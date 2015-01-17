//  Filename:  ClientDBWinChannel.h

#pragma once

class CClientDBWinChannelCallback : public IWTSVirtualChannelCallback
{
	CComPtr<IWTSVirtualChannel> m_ptrChannel;
	LONG m_cRef;
public:
	CClientDBWinChannelCallback()
	{
		m_cRef = 0;
	}

    // IUnknown interface
	//
    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

	//  IWTSVirtualChannelCallback
	//
	HRESULT STDMETHODCALLTYPE OnDataReceived(ULONG cbSize, __in_bcount(cbSize) BYTE *pBuffer)
	{
		//  This is the echo functionality
		OutputDebugString(L"IWTSVirtualChannelCallback::IWTSVirtualChannelCallback\n");
		return m_ptrChannel->Write(cbSize, pBuffer, NULL);

		//  perform useful work here
		/*return S_OK;*/
	}

	HRESULT STDMETHODCALLTYPE OnClose()
	{
		return m_ptrChannel->Close();
	}

	VOID SetChannel(IWTSVirtualChannel *pChannel)
	{
		m_ptrChannel = pChannel;
	}

};

class CClientDBWinListenerCallback :	public IWTSListenerCallback
{
	LONG m_cRef;

public:
	CClientDBWinListenerCallback()
	{
		m_cRef = 0;
	}

    // IUnknown interface
	//
    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

	//  IWTSListenerCallback
	//
	HRESULT STDMETHODCALLTYPE  OnNewChannelConnection(
		__in IWTSVirtualChannel *pChannel,
		__in_opt BSTR data,
		__out  BOOL *pbAccept,
		__out IWTSVirtualChannelCallback **ppCallback);
};

