#pragma once
#include "NativeHooks.h"


class CGUIScalingFix : public CNativeHooks::CFixBaseT<CGUIScalingFix, APlayerPawnExt, EXTENSION_PreRenderWindows>
{
public:
    static const wchar_t* const sm_pszConfigString;

    static void Factory(const wchar_t* const pszIniSection)
    {
        INT i = 0;
        GConfig->GetInt(pszIniSection, sm_pszConfigString, i);
        if (i)
        {
            new CGUIScalingFix(i);
        }
    }

    void ReplacementFunc(const APlayerPawnExt& PlayerPawnThis, CGUIScalingFix& Context, FFrame& Stack, RESULT_DECL);

private:
    explicit CGUIScalingFix(const decltype(XRootWindow::hMultiplier) iScaleAmount);

    const decltype(XRootWindow::hMultiplier) m_iScaleAmount;
    decltype (UCanvas::X) m_iSizeX = 0;
    decltype (UCanvas::Y) m_iSizeY = 0;
};
