//  Filename:  TwainWrapper.h

#pragma once

typedef struct tagImgList
{
	BYTE* img;
	int nImageSize;
	tagImgList* Next;
} IMGLIST, *LPIMGLIST;

typedef struct tagBitDepthList
{
	int nDepth;
	tagBitDepthList* Next;
} BITDEPTHLIST, *LPBITDEPTHLIST;

typedef struct tagPixelTypeList
{
	int nType;
	tagPixelTypeList* Next;
} PIXELTYPELIST, *LPPIXELTYPELIST;

typedef struct tagResolutionList
{
	int Whole;
	int Fraction;
	tagResolutionList* Next;
} RESOLUTIONLIST, *LPRESOLUTIONLIST;

typedef struct tagResolutionRange
{
	int MinValueWhole;
	int MinValueFrac;
	int MaxValueWhole;
	int MaxValueFrac;
	int StepSizeWhole;
	int StepSizeFrac;
	int DefaultValueWhole;
	int DefaultValueFrac;
	int CurrentValueWhole;
	int CurrentValueFrac;
} RESOLUTIONRANGE, *LPRESOLUTIONRANGE;

typedef struct tagSrcList
{
	TW_STR32 src;
	tagSrcList* Next;
} SRCLIST, *LPSRCLIST;

typedef struct tagCapList
{
	int nCap;
	tagCapList* Next;
} CAPLIST, *LPCAPLIST;

typedef struct tagPageSizeList
{
	int nPageSize;
	tagPageSizeList* Next;
} PAGESIZELIST, *LPPAGESIZELIST;

void __CloseSrc(void);
void __OpenSrc(void);
void __Finish(void);
void __Init(HWND hwndp);
LPCSTR __GetDefaultSrc();
void __SelectSrc(void);
LPSRCLIST __EnumSrcs();
void __SetSelectedSource(LPCSTR src);
void __SetPixelType(int nPixelType);
LPPIXELTYPELIST __EnumPixelTypes(void);
void __SetBitDepth(int nBitDepth);
LPBITDEPTHLIST __EnumBitDepths(int nPixelType);
void __SetResolution(float fResolution);
LPRESOLUTIONRANGE __GetResolutionRange();
LPRESOLUTIONLIST __EnumResolutions();
void __Acquire(void);
LPIMGLIST __TransferPictures();
int __PassMessage(LPMSG msg);
LPCAPLIST __EnumCapabilities();
LPPAGESIZELIST __EnumPageSizes();
void __SetPageSize(int nPageSize);
void __SetFeederEnabled(BOOL bFeededEnabled);
void __SetDuplex(BOOL bDuplex);
