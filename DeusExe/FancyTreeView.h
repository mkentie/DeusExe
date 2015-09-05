#pragma once

class CFancyTreeView
{
public:
    enum class EItemState { UNCHECKED, CHECKED, HALF };

    explicit CFancyTreeView();
    ~CFancyTreeView();

    void Init(const HWND hwndTreeView, const HWND hWndButtonUp, const HWND hWndButtonDown);
    HWND GetHWnd() const { return m_hWnd; }

    HTREEITEM GetRoot() const { return TVI_ROOT; }
    unsigned int GetCount() const { return TreeView_GetCount(m_hWnd); }
    HTREEITEM InsertItem(const wchar_t* const pszText, const HTREEITEM hParent);
    HTREEITEM InsertItemUnique(const wchar_t* const pszText, const HTREEITEM hParent);

    void SelectFirstItem();
    void SetItemState(const HTREEITEM hItem, const EItemState State) { m_bStateChangeByCode = true; TreeView_SetItemState(m_hWnd, hItem, ItemStateToStateImageMask(State), TVIS_STATEIMAGEMASK); m_bStateChangeByCode = false; }
    EItemState GetItemState(const HTREEITEM hItem) const { return StateImageMaskToItemState(TreeView_GetItemState(m_hWnd, hItem, TVIS_STATEIMAGEMASK)); }
    void GetItemText(const HTREEITEM hItem, wchar_t* const pszBuf, const size_t iBufCount) const;

    /**
    Finds an item's leaf, which can be the item itself
    */
    HTREEITEM FindLeaf(const HTREEITEM hItem) const;
    HTREEITEM FindNextLeaf(const HTREEITEM hItem) const;

    void SwapItems(const HTREEITEM hItem1, const HTREEITEM hItem2);
    void MoveItemUp();
    void MoveItemDown();

    BOOL HandleNotify(const NMHDR* const pNMH);

private:
    class CTreeItem
    {
        friend class CFancyTreeView;
    public:
        CTreeItem(int iSortKey)
        :m_iSortKey(iSortKey)
        ,m_iChildrenChecked(0)
        ,m_iChildrenHalfChecked(0)
        ,m_iNumChildren(0)
        ,m_bModifyingChildren(false)
        {
        }

    private:
        unsigned int m_iSortKey; //!< Used for move up/down
        unsigned int m_iChildrenChecked; //!< Used to check/uncheck recursively
        unsigned int m_iChildrenHalfChecked; //!< Used to check/uncheck recursively
        unsigned int m_iNumChildren; //!< Used to check/uncheck recursively
        bool m_bModifyingChildren; //!< This flag prevents children that are being modified from modifying their parent again
    };

    
    static EItemState StateImageMaskToItemState(const UINT iState) { return static_cast<EItemState>((iState >> 12) - 1); } //Unfortunately there's no reverse INDEXTOSTATEIMAGEMASK macro
    static UINT ItemStateToStateImageMask(const EItemState State) { return INDEXTOSTATEIMAGEMASK(static_cast<unsigned int>(State)+1); }
    
    CTreeItem& TreeItemFromHandle(const HTREEITEM hItem);

    void ApplylItemStateToChildren(const HTREEITEM hItem, const EItemState State);

    HWND m_hWnd;
    HWND m_hWndButtonUp;
    HWND m_hWndButtonDown;
    bool m_bStateChangeByCode; //!< Flag we use to allow half-checked state by code but not by user
    
    std::deque<CTreeItem> m_Items; //!< Deque so we can push_back items without invalidating pointers
};