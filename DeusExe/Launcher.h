#pragma once


class CLauncher : private FExecHook
{
public:
    explicit CLauncher();
    CLauncher(const CLauncher&) = delete;
    CLauncher& operator=(const CLauncher&) = delete;

private:
    void ApplyAutoFOV(const size_t iSizeX, const size_t iSizeY);
    void MainLoop(UEngine * const pEngine);
    void LoadSettings();

    HWND m_hWnd = NULL;

    LARGE_INTEGER m_iPerfCounterFreq = {};
    size_t m_iSizeX = 0;
    size_t m_iSizeY = 0;
    UViewport* m_pViewPort = nullptr; //If user closes window, viewport disappears before we get WM_QUIT
    bool m_bPrevInMenu = false;
    bool m_bInBorderlessFullscreenWindow = false;

    //Settings
    float m_fFPSLimit; //Because GetMaxTickRate() is float
    UBOOL m_bRawInput;
    UBOOL m_bAutoFov;
    UBOOL m_bBorderlessFullscreenWindow;
    UBOOL m_bUseSingleCPU;

//From FExec
private:
    UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar);

};
