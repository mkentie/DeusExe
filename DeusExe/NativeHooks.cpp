#include "stdafx.h"
#include "NativeHooks.h"

#include "FreeSpaceFix.h"
#include "SubTitleFix.h"
#include "GUIScalingFix.h"

CNativeHooks* CNativeHooks::sm_pSingleton;

CNativeHooks::CNativeHooks(const wchar_t* const pszIniSection)
{
    assert(!sm_pSingleton);
    sm_pSingleton = this;

    typedef void(*FactoryFunc) (const wchar_t* const);

    //Could create a fancy system to register factory functions, but this will suffice for now
    FactoryFunc FactoryFunctions[] =
    {
        &CFreeSpaceFix::Factory,
        &CSubtitleFix::Factory,
        &CGUIScalingFix::Factory,
    };

    for (const auto& Func : FactoryFunctions)
    {
        Func(pszIniSection);
    }
}
