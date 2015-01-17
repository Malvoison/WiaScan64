//  Filename:  TwainClient.h

#pragma once

extern "C" void __declspec(dllexport) SetShowUI(void);
extern "C" void __declspec(dllexport) CloseSrc(void);
extern "C" void __declspec(dllexport) OpenSrc(void);
extern "C" void __declspec(dllexport) Finish(void);
extern "C" void __declspec(dllexport) Init(HWND hwndp);
extern "C" LPCSTR __declspec(dllexport) GetDefaultSrc();
extern "C" void __declspec(dllexport) SelectSrc(void);
extern "C" LPSRCLIST __declspec(dllexport) EnumSrcs();
extern "C" void __declspec(dllexport) SetSelectedSource(LPCSTR src);
extern "C" void __declspec(dllexport) SetPixelType(int nPixelType);
extern "C" LPPIXELTYPELIST __declspec(dllexport) EnumPixelTypes(void);
extern "C" void __declspec(dllexport) SetBitDepth(int nBitDepth);
extern "C" LPBITDEPTHLIST __declspec(dllexport) EnumBitDepths(int nPixelType);
extern "C" void __declspec(dllexport) SetResolution(float fResolution);
extern "C" LPRESOLUTIONRANGE __declspec(dllexport) GetResolutionRange();
extern "C" LPRESOLUTIONLIST __declspec(dllexport) EnumResolutions();
extern "C" LPCAPLIST __declspec(dllexport) EnumCapabilities();
extern "C" LPPAGESIZELIST __declspec(dllexport) EnumPageSizes();
extern "C" void __declspec(dllexport) Acquire(void);
extern "C" LPIMGLIST __declspec(dllexport) TransferPictures();
extern "C" void __declspec(dllexport) InitTwain(void);
extern "C" void __declspec(dllexport) ReleaseTwain(void);
extern "C" void __declspec(dllexport) SetPageSize(int);
extern "C" void __declspec(dllexport) SetFeederEnabled(BOOL);
extern "C" void __declspec(dllexport) SetDuplex(BOOL);
