#pragma once

namespace Misc
{
    bool SetDEP(const DWORD dwFlags);

    void GetUserDocsDir(wchar_t(&pszBuf)[MAX_PATH]);

    void GetGameSystemDir(wchar_t(&pszBuf)[MAX_PATH]);

    const wchar_t* GetVersion();

    float GetDefaultFOV();

    float CalcFOV(const size_t iResX, const size_t iResY);

    void CenterWindowOnMonitor(const HWND hWnd, const HMONITOR hMonitor);

    void SetBorderlessFullscreen(const HWND hWnd, const bool bEnable);
};
