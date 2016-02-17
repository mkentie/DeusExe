#include "stdafx.h"
#include "DataDirDialog.h"
#include "Misc.h"
#include "FileManagerDeusExe.h"
#include "resource.h"

const wchar_t* const CDataDirDialog::sm_pszPaths = L"Paths";

decltype(CDataDirDialog::sm_SupportedExtensions) CDataDirDialog::sm_SupportedExtensions =
{
    L"*.int", L"*.u", L"*.utx", L"*.umx", L"*.dx", L"*.unr", L"*.uax",
};

CDataDirDialog::CDataDirDialog()
{

}

CDataDirDialog::~CDataDirDialog()
{

};

bool CDataDirDialog::Show(const HWND hWndParent) const
{
    return DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DATADIRS), hWndParent, DataDirDialogProc, reinterpret_cast<LPARAM>(this)) == 1;
}

void CDataDirDialog::ProcessDirFiles(const wchar_t* const pszDir, const wchar_t* const pszRelDir)
{
    assert(pszDir);
    assert(pszRelDir);

    wchar_t szRelativePath[MAX_PATH];
    wcscpy_s(szRelativePath, pszRelDir);
    wchar_t* const pszRelFileName = szRelativePath + wcslen(szRelativePath);
    const size_t iCharsLeft = _countof(szRelativePath) - (pszRelFileName - szRelativePath);

    //For each extension, check if a file exists
    for(const wchar_t* const pszExt : sm_SupportedExtensions)
    {
        wchar_t szFileSpec[MAX_PATH];
        PathCombine(szFileSpec, pszDir, pszExt);
        WIN32_FIND_DATA FindData;
        const HANDLE hDir = FindFirstFile(szFileSpec, &FindData);
        if(hDir != INVALID_HANDLE_VALUE)
        {
            do
            {
                if(!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    //Add extension
                    wcscpy_s(pszRelFileName, iCharsLeft, pszExt);
                    AddItemToList(szRelativePath);
                    break;
                }

            } while(FindNextFile(hDir, &FindData) != FALSE);
            FindClose(hDir);
        }
    }
}

void CDataDirDialog::SearchDirs(const wchar_t* const pszTargetDir, const wchar_t* const pszRootDir)
{
    assert(pszTargetDir);
    assert(pszRootDir);

    //Build search string
    wchar_t szTargetPath[MAX_PATH];
    PathCombine(szTargetPath, pszTargetDir, L"*.");
    WIN32_FIND_DATA FindData;
    const HANDLE hDir = FindFirstFile(szTargetPath, &FindData);

    //Find subdirectories
    if(hDir != INVALID_HANDLE_VALUE)
    {
        do
        {
            if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && wcscmp(FindData.cFileName, L"..") != 0 && wcscmp(FindData.cFileName, L".") != 0)
            {
                wchar_t szChildPath[MAX_PATH];
                PathCombine(szChildPath, pszTargetDir, FindData.cFileName);

                //Convert to relative path
                wchar_t szRelativePath[MAX_PATH];
                PathRelativePathTo(szRelativePath, pszRootDir, FILE_ATTRIBUTE_DIRECTORY, szChildPath, FILE_ATTRIBUTE_DIRECTORY);

                //Extra check for "." as we're relative to System, and DefaultDataDirs is not
                if(wcscmp(szRelativePath, L".") == 0) 
                {
                    continue;
                }
                //Check if directory is one of the standard ones, if so don't process it any further
                if(m_DefaultDataDirs.find(szRelativePath) != m_DefaultDataDirs.cend())
                {
                    continue;
                }

                SearchDirs(szChildPath, pszRootDir);

                PathAddBackslash(szRelativePath);
                ProcessDirFiles(szChildPath, szRelativePath);
                
            }
        } while(FindNextFile(hDir, &FindData) != FALSE);

        FindClose(hDir);
    }
}

void CDataDirDialog::FindDefaultItems()
{
    //Store directories from default.ini in map so we ignore then

    TMultiMap<FString, FString>* pSection = GConfig->GetSectionPrivate(L"Core.System", FALSE, TRUE, L"default.ini");
    assert(pSection);

    TArray<FString> Defaults;
    pSection->MultiFind(sm_pszPaths, Defaults);
    for(int i = 0; i < Defaults.Num(); i++)
    {
        //Convert format like "..\Music\*.umx" to "..\Music"
        wchar_t szBuf[MAX_PATH];
        wcscpy_s(szBuf, *Defaults(i));
        PathRemoveFileSpec(szBuf);

        m_DefaultDataDirs.insert(szBuf);
    }

    const wchar_t* const pszCDPath = GConfig->GetStr(L"Engine.Engine", L"CdPath");
    assert(pszCDPath);
    if(pszCDPath[0]!='\0')
    {
        m_DefaultDataDirs.insert(pszCDPath);
    }
}

void CDataDirDialog::AddItemsFromConfig()
{
    //Find current items
    assert(GSys);
    for(int i = 0; i < GSys->Paths.Num(); i++)
    {
        //Convert format like "..\Music\*.umx" to "..\Music"
        wchar_t szBuf[MAX_PATH];
        wcscpy_s(szBuf, *GSys->Paths(i));
        PathRemoveFileSpec(szBuf);

        if(m_DefaultDataDirs.find(szBuf) == m_DefaultDataDirs.cend()) //Don't show the default entries in the list
        {
            const HTREEITEM hItem = AddItemToList(*GSys->Paths(i));
            m_TreeView.SetItemState(hItem, CFancyTreeView::EItemState::CHECKED);
        }
    }

    //Add int overrides
    TMultiMap<FString, FString>* const pSectionInt = GConfig->GetSectionPrivate(PROJECTNAME, FALSE, TRUE); //Int files have their own section as we use our own override mechanism
    if(pSectionInt)
    {
        TArray<FString> IntPaths;
        pSectionInt->MultiFind(FFileManagerDeusExe::sm_pszIntPaths, IntPaths); //Returns in reverse order
        for(int i = IntPaths.Num() - 1; i >= 0; i--)
        {
            //Never in a default data directory as we don't give users a way to add .int files there
            const HTREEITEM hItem = AddItemToList(*IntPaths(i));
            m_TreeView.SetItemState(hItem, CFancyTreeView::EItemState::CHECKED);
        }
    }
}

void CDataDirDialog::PopulateList()
{   
    wchar_t szCurrentDir[MAX_PATH];

    m_bAddTopLevelDirsOnly = true; //Add top level dirs using .ini file priority
    AddItemsFromConfig();
    m_bAddTopLevelDirsOnly = false;
    
    //Search directories
    GetCurrentDirectory(_countof(szCurrentDir),szCurrentDir);
    wchar_t szDirUp[MAX_PATH];
    PathCombine(szDirUp,szCurrentDir,L"..");
    SearchDirs(szDirUp,szCurrentDir);

    AddItemsFromConfig(); //Add subdirs using disk priority
}

void CDataDirDialog::PopulateConfig() const
{
    assert(GSys);

    TMultiMap<FString, FString>* const pSection = GConfig->GetSectionPrivate(L"Core.System", TRUE, FALSE);
    assert(pSection);
    TMultiMap<FString, FString>* const pSectionInt = GConfig->GetSectionPrivate(PROJECTNAME, TRUE, FALSE); //Int files have their own section as we use our own override mechanism
    assert(pSectionInt);

    //Clear list
    pSection->Remove(sm_pszPaths);
    pSectionInt->Remove(FFileManagerDeusExe::sm_pszIntPaths);

    for(HTREEITEM hTreeItem = m_TreeView.FindNextLeaf(m_TreeView.GetRoot()); hTreeItem; hTreeItem = m_TreeView.FindNextLeaf(hTreeItem))
    {
        if(m_TreeView.GetItemState(hTreeItem) == CFancyTreeView::EItemState::CHECKED)
        {
            wchar_t szTreeText[MAX_PATH];
            m_TreeView.GetItemText(hTreeItem, szTreeText, _countof(szTreeText));

            if(wcscmp(PathFindExtension(szTreeText), L".int") == 0)
            {
                pSectionInt->Add(FFileManagerDeusExe::sm_pszIntPaths, szTreeText);
            }
            else
            {
                pSection->Add(sm_pszPaths, szTreeText);
            }
        }
        
    }
    
    //Re-add default items
    TMultiMap<FString, FString>* const pDefSection = GConfig->GetSectionPrivate(L"Core.System", FALSE, FALSE, L"default.ini");
    assert(pDefSection);

    TArray<FString> Defaults;
    pDefSection->MultiFind(sm_pszPaths, Defaults);
    for(int i = 0; i < Defaults.Num(); i++)
    {
        pSection->Add(sm_pszPaths, *Defaults(i));
    }

    GSys->LoadConfig();
}


HTREEITEM CDataDirDialog::AddItemToList(const wchar_t* const pszPath)
{
    assert(pszPath);

    const wchar_t* const pszFileName = PathFindFileName(pszPath);
    assert(pszFileName);

    HTREEITEM hTreeItem = m_TreeView.GetRoot();

    const wchar_t* pszPathComponent = PathFindNextComponent(pszPath); //Skip ".."
    for(const wchar_t* pszNextComponent = PathFindNextComponent(pszPathComponent); pszNextComponent; pszPathComponent = pszNextComponent, pszNextComponent = PathFindNextComponent(pszPathComponent))
    {
        if(pszPathComponent == pszFileName) //Leaf item should be the whole path, so it matches the complete .ini file entry (easier for both users and me)
        {
            hTreeItem = m_TreeView.InsertItemUnique(pszPath, hTreeItem); //Always unique
        }
        else
        {
            wchar_t szBuf[MAX_PATH];
            wcsncpy_s(szBuf, pszPathComponent, pszNextComponent - pszPathComponent - 1);// -1 to skip backslash
            hTreeItem = m_TreeView.InsertItemUnique(szBuf, hTreeItem);
        }

        if(m_bAddTopLevelDirsOnly)
        {
            break;
        }

    }
    return hTreeItem;
}

INT_PTR CALLBACK CDataDirDialog::DataDirDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    CDataDirDialog* pThis = reinterpret_cast<CDataDirDialog*>(GetProp(hwndDlg,L"this"));

    switch (uMsg)
    {

    case WM_INITDIALOG:
        {
            SetProp(hwndDlg,L"this",reinterpret_cast<HANDLE>(lParam));
            pThis =  reinterpret_cast<CDataDirDialog*>(lParam);
            pThis->m_hWnd = hwndDlg;
            SendMessage(hwndDlg, WM_SETICON, ICON_BIG,reinterpret_cast<LPARAM>(LoadIcon(reinterpret_cast<HINSTANCE>(GetWindowLong(hwndDlg,GWL_HINSTANCE)), MAKEINTRESOURCE(IDI_ICON))));

            pThis->m_TreeView.Init(GetDlgItem(hwndDlg, IDC_DIRTREE), GetDlgItem(pThis->m_hWnd, IDC_UP), GetDlgItem(pThis->m_hWnd, IDC_DOWN));

            pThis->FindDefaultItems();
            pThis->PopulateList();
            
            pThis->m_TreeView.SelectFirstItem();
        }
        return TRUE;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {

            case IDOK:
                EndDialog(hwndDlg, 1);
                pThis->PopulateConfig();
                return TRUE;

            case IDCANCEL:
                EndDialog(hwndDlg, 0);
                return TRUE;

            case IDC_UP:
            {
                pThis->m_TreeView.MoveItemUp();
            }
            return TRUE;

            case IDC_DOWN:
            {
                pThis->m_TreeView.MoveItemDown();
            }
            return TRUE;

            }
            break;
        }
        break;

    case WM_NOTIFY:
    {
        const NMHDR* const pNMH = reinterpret_cast<NMHDR*>(lParam);
        assert(pNMH);
        if(pThis && pNMH->hwndFrom == pThis->m_TreeView.GetHWnd())
        {
            const BOOL bResult = pThis->m_TreeView.HandleNotify(pNMH);
            SetWindowLong(hwndDlg, DWL_MSGRESULT, bResult);
            return TRUE;
        }
        break;
    }

    case WM_CLOSE:
        EndDialog(hwndDlg,0);
        return TRUE;
    }

    return FALSE;
}
