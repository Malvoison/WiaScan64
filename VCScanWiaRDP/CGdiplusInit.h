//////////////////////////////////////////////////////////////////////////
//
// CGdiplusInit
//

/*++

    CGdiplusInit is a wrapper class that automatically initializes and 
    shuts down the GDI+ library. GDI+ initialization is necessary for 
    any program that uses GDI+ functions.

Methods

    CGdiplusInit
        Initializes the GDI+ library.

    ~CGdiplusInit
        Shuts down the GDI+ library.

--*/

#pragma once

class CGdiplusInit : public Gdiplus::GdiplusStartupOutput
{
public:
    CGdiplusInit(
        Gdiplus::DebugEventProc debugEventCallback       = 0,
        BOOL                    suppressBackgroundThread = FALSE,
        BOOL                    suppressExternalCodecs   = FALSE
    )
    {
        Gdiplus::GdiplusStartupInput StartupInput(
            debugEventCallback,
            suppressBackgroundThread,
            suppressExternalCodecs
        );

        StartupStatus = Gdiplus::GdiplusStartup(
            &Token, 
            &StartupInput, 
            this
        );
    }

    ~CGdiplusInit()
    {
        if (StartupStatus == Gdiplus::Ok)
        {
            Gdiplus::GdiplusShutdown(Token);
        }
    }

private:
    Gdiplus::Status StartupStatus;
    ULONG_PTR       Token;
};