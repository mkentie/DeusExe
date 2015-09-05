#pragma once

class CLauncherDialog
{
public:
    explicit CLauncherDialog();
    virtual ~CLauncherDialog();
    bool Show(const HWND hWndParent) const;
private:
    void FillLinkControl(const HWND hWndLinkControl, const wchar_t* const pszIniFilePath);

    static INT_PTR CALLBACK LauncherDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

    HWND m_hWndIniFile1; //!< System.ini
    HWND m_hWndIniFile2; //!< User.ini
    HWND m_hWndWebsite;
};


