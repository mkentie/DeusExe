#include "stdafx.h"
#include "FreeSpaceFix.h"
#include "FileManagerDeusExe.h"

CFreeSpaceFix::CFreeSpaceFix()
:CFixBaseT(L"Free save space fix")
{

}

void CFreeSpaceFix::ReplacementFunc(UObject& /*UObjectThis*/, CFreeSpaceFix& /*FixObjectThis*/, FFrame& Stack, RESULT_DECL)
{
    P_FINISH;

    const wchar_t* const pszSave = L"..\\Save";
    wchar_t szSaveDirNew[MAX_PATH];
    static_cast<FFileManagerDeusExe*>(GFileManager)->ToModernFileName(szSaveDirNew, pszSave);

    ULARGE_INTEGER BytesAvailable;
    const BOOL retval = GetDiskFreeSpaceEx(szSaveDirNew, &BytesAvailable, nullptr, nullptr);

    const int iResult = static_cast<int>(std::min<ULONGLONG>(BytesAvailable.QuadPart / 1024, std::numeric_limits<int>::max()));
    *static_cast<int*>(Result) = iResult;
}
