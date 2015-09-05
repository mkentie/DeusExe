#pragma once

#include <unordered_set>
#include "FancyTreeView.h"


class CDataDirDialog
{
public:
    explicit CDataDirDialog();
    virtual ~CDataDirDialog();
    bool Show(const HWND hWndParent) const;

private:
    void ProcessDirFiles(const wchar_t* const pszDir, const wchar_t* const pszRelDir);
    void SearchDirs(const wchar_t* pszTargetDir, const wchar_t* const pszRootDir);
    
    /**
    Finds the items from the default.ini file
    */
    void FindDefaultItems();

    void AddItemsFromConfig();
    void PopulateList();
    void PopulateConfig() const;

    /**
    Strips a path into its component parts and adds those to the ini file.
    We do this instead of just tracking the components as we iterate the directories, so it's easy to add existing items, even if they don't actually exist on disk.
    */
    HTREEITEM AddItemToList(const wchar_t* const pszPath);

    static INT_PTR CALLBACK DataDirDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

    static const wchar_t* const sm_pszPaths;

    HWND m_hWnd;
    CFancyTreeView m_TreeView;

    // This is used so the top-level dirs (Shifter, HDTP) are first added to the list. However, their contents are added in the order they're on disk.
    // This prevents the order of sub-items from shifting depending on in which order they were added.
    bool m_bAddTopLevelDirsOnly;

    class CCaseInsensitiveHash
    {
    public:
        size_t operator()(const std::wstring& str) const
        {
            return appStrihash(str.c_str());
        }
    };

    class CCaseInsensitiveEquals
    {
    public:
        bool operator()(const std::wstring& str1, const std::wstring& str2) const
        {
            return _wcsicmp(str1.c_str(), str2.c_str()) == 0;
        }
    };
    template<class T> using CaseInsensitiveMap = std::unordered_set<T, CCaseInsensitiveHash, CCaseInsensitiveEquals>;


    CaseInsensitiveMap<std::wstring> m_DefaultDataDirs; //!< Data dirs the game has standard, don't allow the user to change these

    static const std::array<const wchar_t*, 7> sm_SupportedExtensions; // Extensions that make sense to show
};