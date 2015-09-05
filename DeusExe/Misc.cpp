#include "stdafx.h"
#include "Misc.h"

bool Misc::SetDEP(const DWORD dwFlags = PROCESS_DEP_ENABLE)
{
        const HMODULE hMod = GetModuleHandleW(L"Kernel32.dll");
        if(!hMod)
        {
            return false;
        }

        const auto procSet = reinterpret_cast<BOOL(WINAPI * const)(DWORD)>(GetProcAddress(hMod, "SetProcessDEPPolicy"));
        if(!procSet)
        {
            return false;
        }

        return procSet(dwFlags)!=FALSE;
}

/**
Returns game directory in user documents directory
*/
void Misc::GetUserDocsDir(wchar_t(&pszBuf)[MAX_PATH])
{
    SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,NULL,pszBuf);
    PathAppend(pszBuf, FRIENDLYGAMENAME);
}

/**
Returns System directory in game directory
*/
void Misc::GetGameSystemDir(wchar_t(&pszBuf)[MAX_PATH])
{
    GetModuleFileName(NULL,pszBuf,MAX_PATH);
    PathRemoveFileSpec(pszBuf);
}

static std::unique_ptr<wchar_t[]> pszVersion;
const wchar_t* Misc::GetVersion()
{
    if(!pszVersion)
    {
        //Get version from resource
        wchar_t szFileName[MAX_PATH];
        GetModuleFileName(0, szFileName, _countof(szFileName));
        DWORD dwHandle; //Doesn't do anything but still needed
        const DWORD dwSize = GetFileVersionInfoSize(szFileName, &dwHandle);
        std::unique_ptr<char[]> DataPtr(new char[dwSize]);
        GetFileVersionInfo(szFileName, 0, dwSize, DataPtr.get());
        void* pVersion = nullptr;

        UINT iLen;
        VerQueryValue(DataPtr.get(), L"\\StringFileInfo\\041304b0\\ProductVersion", &pVersion, &iLen);
        assert(pVersion);
        pszVersion.reset(new wchar_t[iLen]);
        wcscpy_s(pszVersion.get(), iLen, static_cast<wchar_t*>(pVersion));
    }

    return pszVersion.get();
}

float Misc::GetDefaultFOV()
{
    float fFOV = 75.0;
    GConfig->GetFloat(L"Engine.PlayerPawn", L"DesiredFOV", fFOV, L"DefUser.ini");
    return fFOV;
}

float Misc::CalcFOV(const size_t iResX, const size_t iResY)
{
    static const float fDeg2Rad = static_cast<float>(M_PI) / 180.0f;
    static const float fDefaultAspect = 4.0f / 3.0f;
    const float fAspect = static_cast<float>(iResX) / iResY;

    const float fFov = atanf(tanf(0.5f*GetDefaultFOV()*fDeg2Rad)*(fAspect / fDefaultAspect)) / fDeg2Rad*2.0f;
    return fFov;
}

