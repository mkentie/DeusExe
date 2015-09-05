#include "stdafx.h"
#include "FancyTreeView.h"

CFancyTreeView::CFancyTreeView()
:m_hWnd(NULL)
,m_hWndButtonUp(NULL)
,m_hWndButtonDown(NULL)
,m_bStateChangeByCode(false)
{

}

CFancyTreeView::~CFancyTreeView()
{

}

void CFancyTreeView::Init(const HWND hwndTreeView, const HWND hWndButtonUp, const HWND hWndButtonDown)
{
    assert(hwndTreeView);
    assert(hWndButtonUp);
    assert(hWndButtonDown);

    m_hWnd = hwndTreeView;
    m_hWndButtonUp = hWndButtonUp;
    m_hWndButtonDown = hWndButtonDown;
    
    SetWindowTheme(m_hWnd, L"explorer", nullptr); //Exporer-style arrows instead of pluses to expand list items
    TreeView_SetExtendedStyle(m_hWnd, TVS_EX_PARTIALCHECKBOXES | TVS_EX_AUTOHSCROLL, TVS_EX_PARTIALCHECKBOXES | TVS_EX_AUTOHSCROLL);
}

HTREEITEM CFancyTreeView::InsertItem(const wchar_t* const pszText, const HTREEITEM hParent)
{
    m_Items.emplace_back(GetCount());

    TVINSERTSTRUCT Item;
    Item.hParent = hParent;
    Item.hInsertAfter = TVI_LAST;
    Item.itemex.mask = TVIF_TEXT | TVIF_PARAM;
    Item.itemex.pszText = const_cast<wchar_t*>(pszText);
    Item.itemex.lParam = reinterpret_cast<LPARAM>(&m_Items.back());

    const HTREEITEM hItem = TreeView_InsertItem(m_hWnd, &Item);

    if(hParent!=TVI_ROOT)
    {
        CTreeItem& ParentItem = TreeItemFromHandle(hParent);
        ParentItem.m_iNumChildren++;

        //An unchecked item was added, so set parent to half checked if necessary
        if(GetItemState(hParent) == EItemState::CHECKED)
        {
            SetItemState(hParent, EItemState::HALF);
        }
    }

    return hItem;
}

HTREEITEM CFancyTreeView::InsertItemUnique(const wchar_t* const pszText, const HTREEITEM hParent)
{
    //Unfortunately there doesn't seem to be a better way to do this without keeping our own tiered string-to-HTREEITEM-map.
    for(HTREEITEM hChild = TreeView_GetChild(m_hWnd, hParent); hChild; hChild = TreeView_GetNextSibling(m_hWnd, hChild))
    {
        wchar_t szBuf[MAX_PATH];
        GetItemText(hChild, szBuf, _countof(szBuf));
        if(_wcsicmp(szBuf, pszText) == 0)
        {
            return hChild;
        }
    }
    return InsertItem(pszText, hParent);
}

void CFancyTreeView::SelectFirstItem()
{
    const HTREEITEM hRootItem = TreeView_GetRoot(m_hWnd);
    if(hRootItem)
    {
        TreeView_SelectItem(m_hWnd, hRootItem);
    }
}

void CFancyTreeView::GetItemText(const HTREEITEM hItem, wchar_t* const pszBuf, const size_t iBufCount) const
{
    assert(hItem);

    TVITEMEX Item;
    Item.hItem = hItem;
    Item.mask = TVIF_TEXT;
    Item.cchTextMax = iBufCount;
    Item.pszText = pszBuf;
    TreeView_GetItem(m_hWnd, &Item);
}

HTREEITEM CFancyTreeView::FindLeaf(const HTREEITEM hItem) const
{
    assert(hItem);
    HTREEITEM hLeaf = hItem;
    for(HTREEITEM hChild = TreeView_GetChild(m_hWnd, hItem); hChild; hChild = TreeView_GetChild(m_hWnd, hChild))
    {
        hLeaf = hChild;
    }
    return hLeaf;
}

HTREEITEM CFancyTreeView::FindNextLeaf(const HTREEITEM hItem) const
{
    assert(hItem);
    
    //Try children
    const HTREEITEM hLeaf = FindLeaf(hItem);
    if(hLeaf == TVI_ROOT) //No child found for root (i.e. empty list)
    {
        return NULL;
    }
    if(hLeaf!=hItem)
    {
        return hLeaf;
    }

    //Try children of siblings
    const HTREEITEM hSibling = TreeView_GetNextSibling(m_hWnd, hItem);
    if(hSibling)
    {
        return FindLeaf(hSibling);
    }
    
    //Try siblings of parent
    for(HTREEITEM hParent = TreeView_GetParent(m_hWnd, hItem); hParent; hParent = TreeView_GetParent(m_hWnd, hParent))
    {
        const HTREEITEM hParentSibling = TreeView_GetNextSibling(m_hWnd, hParent);
        if(hParentSibling)
        {
            return FindLeaf(hParentSibling);
        }
    }

    return NULL;
}

void CFancyTreeView::SwapItems(const HTREEITEM hItem1, const HTREEITEM hItem2)
{
    assert(hItem1);
    assert(hItem2);
    //Swap the items' SortKey members and then re-sort the list using those
    CTreeItem& Item1 = TreeItemFromHandle(hItem1);
    CTreeItem& Item2 = TreeItemFromHandle(hItem2);
    std::swap(Item1.m_iSortKey, Item2.m_iSortKey);
    
    TVSORTCB SortCB;
    SortCB.hParent = TVI_ROOT;

    // Compare function so we can sort items by their SortKey members
    SortCB.lpfnCompare = [](const LPARAM lParam1, const LPARAM lParam2, const LPARAM /*lParamSort*/)
    {
        const CTreeItem* const pItem1 = reinterpret_cast<CTreeItem*>(lParam1);
        assert(pItem1);
        const CTreeItem* const pItem2 = reinterpret_cast<CTreeItem*>(lParam2);
        assert(pItem2);
        return pItem1->m_iSortKey < pItem2->m_iSortKey ? -1 : pItem1->m_iSortKey == pItem2->m_iSortKey ? 0 : 1;
    };

    TreeView_SortChildrenCB(m_hWnd, &SortCB, 0);
}

void CFancyTreeView::MoveItemUp()
{
    const HTREEITEM hItem = TreeView_GetSelection(m_hWnd);
    assert(hItem);

    const HTREEITEM hPrevSibling = TreeView_GetPrevSibling(m_hWnd, hItem);
    assert(hPrevSibling != NULL);
    SwapItems(hItem, hPrevSibling);

    EnableWindow(m_hWndButtonUp, TreeView_GetPrevSibling(m_hWnd, hItem) != NULL);
    EnableWindow(m_hWndButtonDown, TRUE);
}

void CFancyTreeView::MoveItemDown()
{ 
    const HTREEITEM hItem = TreeView_GetSelection(m_hWnd);
    assert(hItem);

    const HTREEITEM hNextSibling = TreeView_GetNextSibling(m_hWnd, hItem);
    assert(hNextSibling != NULL);
    SwapItems(hItem, hNextSibling);

    EnableWindow(m_hWndButtonUp, TRUE);
    EnableWindow(m_hWndButtonDown, TreeView_GetNextSibling(m_hWnd, hItem) != NULL);
}

void CFancyTreeView::ApplylItemStateToChildren(const HTREEITEM hItem, const EItemState State)
{
    assert(hItem);
    //The function applies non - recursively; however the children will apply the state to their own children in the event handler
    for(HTREEITEM hSibling = TreeView_GetChild(m_hWnd, hItem); hSibling; hSibling = TreeView_GetNextSibling(m_hWnd, hSibling))
    {
        SetItemState(hSibling, State);
    }
}

BOOL CFancyTreeView::HandleNotify(const NMHDR* const pNMH)
{
    assert(pNMH);
    switch(pNMH->code)
    {
    case TVN_ITEMCHANGING:
    {
        const NMTVITEMCHANGE* const pItemInfo = reinterpret_cast<const NMTVITEMCHANGE *>(pNMH);
        assert(pItemInfo->uChanged == TVIF_STATE);

        if(GetCount() == 0) //For some reason we get notifications during init, and calling GetParent() etc. on those is an access violation
        {
            return FALSE;
        }

        if((pItemInfo->uStateNew ^ pItemInfo->uStateOld) & TVIS_STATEIMAGEMASK) //Checkbox state changed
        {
            const EItemState NewState = StateImageMaskToItemState(pItemInfo->uStateNew);
            if(!m_bStateChangeByCode)
            {
                TreeView_SelectItem(m_hWnd, pItemInfo->hItem); //If box clicked, select item
                if(NewState == EItemState::HALF) //Don't allow half-state by user input, only programmatically)
                {
                    SetItemState(pItemInfo->hItem, EItemState::UNCHECKED);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;

    case TVN_ITEMCHANGED:
    {
        const NMTVITEMCHANGE* const pItemInfo = reinterpret_cast<const NMTVITEMCHANGE *>(pNMH);
        assert(pItemInfo->uChanged == TVIF_STATE);
        if(GetCount() == 0) //For some reason we get notifications during init, and calling GetParent() etc. on those is an access violation
        {
            return FALSE;
        }

        const EItemState NewState = StateImageMaskToItemState(pItemInfo->uStateNew);
        const EItemState OldState = StateImageMaskToItemState(pItemInfo->uStateOld);
        if(NewState != OldState) //Checkbox state changed
        {
            CTreeItem& Item = TreeItemFromHandle(pItemInfo->hItem);
            
            //Change parent item status
            const HTREEITEM hParent = TreeView_GetParent(m_hWnd,pItemInfo->hItem);
            if(hParent)
            {
                CTreeItem& ParentItem = TreeItemFromHandle(hParent);
                ParentItem.m_iChildrenChecked += OldState == EItemState::CHECKED ? -1 : NewState == EItemState::CHECKED ? +1 : 0;
                ParentItem.m_iChildrenHalfChecked += OldState == EItemState::HALF ? -1 : NewState == EItemState::HALF ? +1 : 0;

                //Set parent state, but not if it's in the process of changing its children thanks to its own state being changed
                if(!ParentItem.m_bModifyingChildren)
                {
                    switch(NewState)
                    {
                    case EItemState::UNCHECKED:
                        SetItemState(hParent, ParentItem.m_iChildrenChecked == 0 && ParentItem.m_iChildrenHalfChecked == 0 ? EItemState::UNCHECKED : EItemState::HALF);
                        break;
                    case EItemState::CHECKED:
                        SetItemState(hParent, ParentItem.m_iChildrenChecked == ParentItem.m_iNumChildren ? EItemState::CHECKED : EItemState::HALF);
                        break;
                    case EItemState::HALF:
                        SetItemState(hParent, EItemState::HALF);
                        break;
                    }
                }
                
                assert(ParentItem.m_iChildrenChecked + ParentItem.m_iChildrenHalfChecked <= ParentItem.m_iNumChildren);
            }

            //Don't change children if we got here because a child changed our own state; that means the children already match
            if((NewState == EItemState::CHECKED && Item.m_iChildrenChecked < Item.m_iNumChildren) || NewState == EItemState::UNCHECKED && (Item.m_iChildrenChecked>0 || Item.m_iChildrenHalfChecked > 0))
            {
                Item.m_bModifyingChildren = true;
                ApplylItemStateToChildren(pItemInfo->hItem, NewState);
                Item.m_bModifyingChildren = false;
            }
            assert(Item.m_iChildrenChecked + Item.m_iChildrenHalfChecked <= Item.m_iNumChildren);
            assert(NewState == EItemState::CHECKED && Item.m_iChildrenChecked == Item.m_iNumChildren
                || NewState == EItemState::UNCHECKED && Item.m_iChildrenChecked == 0 && Item.m_iChildrenHalfChecked == 0
                || NewState == EItemState::HALF);
        }
    }
    return FALSE;

    case TVN_SELCHANGING:
    {
        const NMTREEVIEW * const pItemInfo = reinterpret_cast<const NMTREEVIEW*>(pNMH);

        if(m_hWndButtonUp && m_hWndButtonDown)
        {
            const bool bIsTopLevel = TreeView_GetParent(m_hWnd, pItemInfo->itemNew.hItem) == NULL;
            const bool bIsTopItem = TreeView_GetPrevSibling(m_hWnd, pItemInfo->itemNew.hItem) == NULL;
            const bool bIsBottomItem = TreeView_GetNextSibling(m_hWnd, pItemInfo->itemNew.hItem) == NULL;

            EnableWindow(m_hWndButtonUp, bIsTopLevel && !bIsTopItem);
            EnableWindow(m_hWndButtonDown, bIsTopLevel && !bIsBottomItem);
        }
    }
    return FALSE;

    }
    return FALSE;
}

CFancyTreeView::CTreeItem& CFancyTreeView::TreeItemFromHandle(const HTREEITEM hItem)
{
    assert(hItem);
    TVITEMEX Item;
    Item.hItem = hItem;
    Item.mask = TVIF_HANDLE | TVIF_PARAM;
    TreeView_GetItem(m_hWnd, &Item);
    
    CTreeItem* const pTreeItem = reinterpret_cast<CTreeItem*>(Item.lParam);
    assert(pTreeItem);
    return *pTreeItem;
}
