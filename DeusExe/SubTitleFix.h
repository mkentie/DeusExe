#pragma once
#include "NativeHooks.h"

class CSubtitleFix : public CNativeHooks::CFixBaseT<CSubtitleFix, XWindow, EXTENSION_NewChild>
{
public:
    static const wchar_t* const sm_pszConfigString;

    static void Factory(const wchar_t* const pszIniSection)
    {
        UBOOL b = FALSE;
        GConfig->GetBool(pszIniSection, sm_pszConfigString, b);
        if (b)
        {
            new CSubtitleFix;
        }
    }

    void ReplacementFunc(XWindow& XWinThis, CSubtitleFix& /*Context*/, FFrame& Stack, RESULT_DECL);

private:
    explicit CSubtitleFix();
};

