#pragma once

#include "NativeHooks.h"

class CFreeSpaceFix : public CNativeHooks::CFixBaseT<CFreeSpaceFix, UObject, DEUSEX_GetSaveFreeSpace>
{
public:
    static void Factory(const wchar_t* const /*pszIniSection*/)
    {
        new CFreeSpaceFix;
    }

    void ReplacementFunc(UObject& UObjectThis, CFreeSpaceFix& FixObjectThis, FFrame& Stack, RESULT_DECL);

private:
    explicit CFreeSpaceFix();
};
