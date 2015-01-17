//  Filename:  WiaScanServerHelper.h

#pragma once

BOOL TestChannelAvailableWrapper(LPCSTR szChannel);
BOOL SendServerCommandWrapper(int nCmd);
void InitializeWiaChannelHandlersWrapper(HWND hwnd);
void ReleaseWiaChannelHandlersWrapper();