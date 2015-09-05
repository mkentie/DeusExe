#pragma once

class CFixApp
{
public:
    explicit CFixApp();
    virtual ~CFixApp();
    bool Show(const HWND hWndParent) const;

    static const char sm_iDefaultFPSLimit = 120;

private:
    void ReadSettings();
    void PopulateDialog();
    void ApplySettings() const;

    static INT_PTR CALLBACK FixAppDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

    //Constants
    static const unsigned char  sm_iBPP_16 = 16;
    static const unsigned char  sm_iBPP_32 = 32;

    struct Resolution //These are all ints to match what Unreal loads from the ini file
    {
        size_t iX;
        size_t iY;
    };

    //Members
    HWND m_hWnd;
    HWND m_hWndCBGUIScales;
    HWND m_hWndCBRenderers;
    HWND m_hWndCBResolutions;
    HWND m_hWndTxtResX;
    HWND m_hWndTxtResY;
    HWND m_hWndTxtFOV;
    std::vector<std::wstring> m_Renderers;
    std::deque<Resolution> m_Resolutions; //Don't want push_back to change address
};
