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
    constexpr float fDeg2Rad = static_cast<float>(M_PI) / 180.0f;
    constexpr float fDefaultAspect = 4.0f / 3.0f;
    const float fAspect = static_cast<float>(iResX) / iResY;

    const float fFov = atanf(tanf(0.5f*GetDefaultFOV()*fDeg2Rad)*(fAspect / fDefaultAspect)) / fDeg2Rad*2.0f;
    return fFov;
}

void Misc::CenterWindowOnMonitor(const HWND hWnd, const HMONITOR hMonitor)
{
    assert(hWnd);
    assert(hMonitor);

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);
    RECT r;
    GetWindowRect(hWnd, &r);

    const int iW = r.right - r.left;
    const int iH = r.bottom - r.top;

    //Center window on monitor
    const int iX = (mi.rcMonitor.left + mi.rcMonitor.right - iW) / 2;
    const int iY = (mi.rcMonitor.top + mi.rcMonitor.bottom - iH) / 2;
    MoveWindow(hWnd, iX, iY, iW, iH, FALSE);
#ifdef _DEBUG
    //Check window is still same size
    RECT r2;
    GetWindowRect(hWnd, &r2);
    assert(r.right - r.left == r2.right - r2.left);
    assert(r.bottom - r.top == r2.bottom - r2.top);
#endif
}

void Misc::SetBorderlessFullscreen(const HWND hWnd, const BorderlessFullscreenMode Mode)
{
    assert(hWnd);

    LONG_PTR Style = GetWindowLongPtr(hWnd, GWL_STYLE);

    if (Mode != BorderlessFullscreenMode::NONE)
    {
        Style &= ~(WS_CAPTION | WS_THICKFRAME);
        SetWindowLongPtr(hWnd, GWL_STYLE, Style);

        int iX;
        int iY;
        int iW;
        int iH;

        if (Mode == BorderlessFullscreenMode::CURRENT_MONITOR)
        {
            const HMONITOR hM = MonitorFromWindow(hWnd, 0);

            MONITORINFO mi;
            mi.cbSize = sizeof(mi);
            GetMonitorInfo(hM, &mi);

            iX = mi.rcMonitor.left;
            iY = mi.rcMonitor.top;
            iW = mi.rcMonitor.right - mi.rcMonitor.left;
            iH = mi.rcMonitor.bottom - mi.rcMonitor.top;
        }
        else
        {
            iX = GetSystemMetrics(SM_XVIRTUALSCREEN);
            iY = GetSystemMetrics(SM_YVIRTUALSCREEN);
            iW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            iH = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        }

        SetWindowPos(hWnd, NULL, iX, iY, iW, iH, SWP_FRAMECHANGED);
    }
    else
    {
        Style |= (WS_CAPTION | WS_THICKFRAME);
        SetWindowLongPtr(hWnd, GWL_STYLE, Style);
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
    }
}
