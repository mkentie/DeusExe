#include "stdafx.h"
#include "SubTitleFix.h"

const wchar_t* const CSubtitleFix::sm_pszConfigString = L"SubtitleFix";

CSubtitleFix::CSubtitleFix()
:CFixBaseT(L"Widescreen subtitle fix")
{

}

void CSubtitleFix::ReplacementFunc(XWindow& XWinThis, CSubtitleFix& /*Context*/, FFrame& Stack, RESULT_DECL)
{
    P_GET_OBJECT(UClass, pNewClass);
    P_GET_UBOOL_OPTX(bShow, 1);
    P_FINISH;

    //Note: we don't replace CinematicWindow, which has the same quirk. Advantage: cutscenes with no subtitles won't be cropped. Disadvantage: as soon as subtitles do show up, black bar spawns/grows.
    //Constructor function comparison is for early-out for most windows
    if (pNewClass->ClassConstructor == &XModalWindow::InternalConstructor && wcscmp(pNewClass->GetFullName(), L"Class DeusEx.ConWindowActive") == 0)
    {
        UClass* const pReplacementClass = LoadClass<UObject>(nullptr, L"DeusExe.ConWindowActive2", L"DeusExe", 0, nullptr);
        if (pReplacementClass)
        {
            pNewClass = pReplacementClass;
        }
    }

    XWindow* const pNewWin = XWinThis.CreateNewWindow(pNewClass, &XWinThis, bShow);

    *(static_cast<XWindow** const>(Result)) = pNewWin;
}
