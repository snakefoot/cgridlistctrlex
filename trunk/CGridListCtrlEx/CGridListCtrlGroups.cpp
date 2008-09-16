#include "stdafx.h"
#include "CGridListCtrlGroups.h"

#include <shlwapi.h>	// IsCommonControlsEnabled

#include "CGridColumnTrait.h"

#if _WIN32_WINNT >= 0x0501

BEGIN_MESSAGE_MAP(CGridListCtrlGroups, CGridListCtrlEx)
#if _WIN32_WINNT >= 0x0600
	ON_NOTIFY_REFLECT_EX(LVN_LINKCLICK, OnGroupTaskClick)	// Column Click
#endif
END_MESSAGE_MAP()

LRESULT CGridListCtrlGroups::InsertGroupHeader(int nIndex, int nGroupID, const CString& strHeader, DWORD dwState /* = LVGS_NORMAL */, DWORD dwAlign /*= LVGA_HEADER_LEFT*/)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.iGroupId = nGroupID;
	lg.state = dwState;
	lg.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
	lg.uAlign = dwAlign;

	// Header-title must be unicode (Convert if necessary)
#ifdef UNICODE
	lg.pszHeader = strHeader.GetBuffer();
	lg.cchHeader = strHeader.GetLength();
#else
	CComBSTR header = strHeader;
	lg.pszHeader = header;
	lg.cchHeader = header.Length();
#endif

	return InsertGroup(nIndex, (PLVGROUP)&lg );
}

BOOL CGridListCtrlGroups::SetRowGroupId(int nRow, int nGroupID)
{
	//OBS! Rows not assigned to a group will not show in group-view
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_GROUPID;
	lvItem.iItem = nRow;
	lvItem.iSubItem = 0;
	lvItem.iGroupId = nGroupID;
	return SetItem( &lvItem );
}

int CGridListCtrlGroups::GetRowGroupId(int nRow)
{
	LVITEM lvi = {0};
    lvi.mask = LVIF_GROUPID;
    lvi.iItem = nRow;
	VERIFY( GetItem(&lvi) );
    return lvi.iGroupId;
}

CString CGridListCtrlGroups::GetGroupHeader(int nGroupID)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.iGroupId = nGroupID;
	lg.mask = LVGF_HEADER | LVGF_GROUPID;
	VERIFY( GetGroupInfo(nGroupID, (PLVGROUP)&lg) != -1 );

#ifdef UNICODE
	return lg.pszHeader;
#else
	CComBSTR header( lg.pszHeader );
	return (LPCTSTR)COLE2T(header);
#endif
}

BOOL CGridListCtrlGroups::IsGroupStateEnabled()
{
	if (!IsGroupViewEnabled())
		return FALSE;

	OSVERSIONINFO osver = {0};
	osver.dwOSVersionInfoSize = sizeof(osver);
	GetVersionEx(&osver);
	WORD fullver = MAKEWORD(osver.dwMinorVersion, osver.dwMajorVersion);
	if (fullver < 0x0600)
		return FALSE;

	return TRUE;
}

// Vista SDK - ListView_GetGroupState / LVM_GETGROUPSTATE
BOOL CGridListCtrlGroups::HasGroupState(int nGroupID, DWORD dwState)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_STATE;
	lg.stateMask = dwState;
	if ( GetGroupInfo(nGroupID, (PLVGROUP)&lg) == -1)
		return FALSE;

	return lg.state==dwState;
}

// Vista SDK - ListView_SetGroupState / LVM_SETGROUPINFO
BOOL CGridListCtrlGroups::SetGroupState(int nGroupID, DWORD dwState)
{
	if (!IsGroupStateEnabled())
		return FALSE;

	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_STATE;
	lg.state = dwState;
	lg.stateMask = dwState;

#ifdef LVGS_COLLAPSIBLE
	if (HasGroupState(nGroupID, LVGS_COLLAPSIBLE))
		lg.state |= LVGS_COLLAPSIBLE;
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
}

int CGridListCtrlGroups::GroupHitTest(const CPoint& point)
{
	if (!IsGroupViewEnabled())
		return -1;

	if (HitTest(point)!=-1)
		return -1;

#ifdef ListView_HitTestEx
#ifdef LVHT_EX_GROUP_HEADER
	LVHITTESTINFO lvhitinfo = {0};
	lvhitinfo.pt = point;
	ListView_HitTestEx(m_hWnd, &lvhitinfo);
	if ((lvhitinfo.flags & LVHT_EX_GROUP)==0)
		return -1;
#endif
#endif

#ifdef ListView_GetGroupCount
#ifdef ListView_GetGroupRect
#ifdef ListView_GetGroupInfoByIndex
	bool foundGroup = false;
	LRESULT groupCount = ListView_GetGroupCount(m_hWnd);
	if (groupCount <= 0)
		return -1;
	for(int i = 0 ; i < groupCount; ++i)
	{
		LVGROUP lg = {0};
		lg.cbSize = sizeof(lg);
		lg.mask = LVGF_GROUPID;
		VERIFY( ListView_GetGroupInfoByIndex(m_hWnd, i, &lg) );

		CRect rect(0,0,0,0);
		VERIFY( ListView_GetGroupRect(m_hWnd, lg.iGroupId, 0, &rect) );

		if (rect.PtInRect(point))
			return lg.iGroupId;
	}
	// Don't try other ways to find the group
	if (groupCount > 0)
		return -1;
#endif
#endif
#endif

	// We require that each group contains atleast one item
	if (GetItemCount()==0)
		return -1;

	CRect gridRect(0,0,0,0);
	GetClientRect(&gridRect);
	CRect headerRect(0,0,0,0);
	GetHeaderCtrl()->GetClientRect(headerRect);

	int nRowAbove = -1, nRowBelow = 0;
	for(nRowBelow = GetTopIndex(); nRowBelow < GetItemCount(); nRowBelow++)
	{
		GetRowGroupId(nRowBelow);

		CRect rectRowBelow;
		if (GetItemRect(nRowBelow, rectRowBelow, LVIR_BOUNDS)==FALSE)
			continue;	// Found invisible row

		rectRowBelow.right = gridRect.right;
		if (rectRowBelow.PtInRect(point))
			return -1;	// Hit a row
		if (rectRowBelow.top > point.y)
			break;		// Found row just below the point

		nRowAbove = nRowBelow;
	}

	// Check for hidden / collapsed groups between the two rows
	CRect groupRect = gridRect;
	groupRect.top = headerRect.bottom;
	groupRect.bottom = headerRect.bottom + headerRect.Height();

	if (nRowAbove==-1)
	{
		nRowAbove = 0;
	}
	else
	{
		CRect rectRowAbove;
		VERIFY( GetItemRect(nRowAbove, rectRowAbove, LVIR_BOUNDS) );
		groupRect.top = rectRowAbove.bottom;
		groupRect.bottom = groupRect.top + headerRect.Height();
		nRowAbove += 1;
	}

	// Seek down to check for hidden/collapsed groups
	int nPrevGroupId = -1;
	for(int nRow = nRowAbove; nRow < nRowBelow; ++nRow)
	{
		CRect rectRow;
		if (GetItemRect(nRow, rectRow, LVIR_BOUNDS))
			return -1;	// There should be no visible rows in between

		// Row inside hidden / collapsed group
		int nGroupId = GetRowGroupId(nRow);
		if (nGroupId!=nPrevGroupId)
		{
			// Found new collapsed group
			if (groupRect.PtInRect(point))
			{
				return nGroupId;	// Hit a collapsed group
			}
			else
			{
				// Not inside this collapsed group, maybe the next
				groupRect.top += headerRect.Height();
				groupRect.bottom += headerRect.Height();
				nPrevGroupId = nGroupId;
			}
		}
	}

	if (nRowBelow < GetItemCount())
	{
		// Probably hit the group just above this row
		//	- Check this is true by validating the rect
		CRect rectRow;
		if (GetItemRect(nRow, rectRow, LVIR_BOUNDS))
		{
			groupRect.top = rectRow.top - headerRect.Height();
			groupRect.bottom = rectRow.top;
			if (groupRect.PtInRect(point))
				return GetRowGroupId(nRowBelow);			
		}
	}

	return -1;
}

void CGridListCtrlGroups::CheckEntireGroup(int nGroupId, bool bChecked)
{
	for (int nRow=0; nRow<GetItemCount(); ++nRow)
	{
		if (GetRowGroupId(nRow) == nGroupId)
		{
			SetCheck(nRow, bChecked ? TRUE : FALSE);
		}
	}
}

void CGridListCtrlGroups::DeleteEntireGroup(int nGroupId)
{
	for (int nRow=0; nRow<GetItemCount(); ++nRow)
	{
		if (GetRowGroupId(nRow) == nGroupId)
		{
			DeleteItem(nRow);
			nRow--;
		}
	}
	RemoveGroup(nGroupId);
}

BOOL CGridListCtrlGroups::GroupByColumn(int nCol)
{
	SetSortArrow(-1, false);

	RemoveAllGroups();

	EnableGroupView( GetItemCount() > 0 );

	if (IsGroupViewEnabled())
	{
		CSimpleMap<CString,CSimpleArray<int> > groups;

		// Loop through all rows and find possible groups
		for(int nRow=0; nRow<GetItemCount(); ++nRow)
		{
			const CString& cellText = GetItemText(nRow, nCol);

			int nGroupId = groups.FindKey(cellText);
			if (nGroupId==-1)
			{
				CSimpleArray<int> rows;
				rows.Add(nRow);
				groups.Add(cellText, rows);
			}
			else
				groups.GetValueAt(nGroupId).Add(nRow);
		}

		// Look through all groups and assign rows to group
		for(int nGroupId = 0; nGroupId < groups.GetSize(); ++nGroupId)
		{
			const CSimpleArray<int>& groupRows = groups.GetValueAt(nGroupId);
			DWORD dwState = LVGS_NORMAL;
#ifdef LVGS_COLLAPSIBLE
			dwState = LVGS_COLLAPSIBLE;
#endif
			VERIFY( InsertGroupHeader(nGroupId, nGroupId, groups.GetKeyAt(nGroupId), dwState) != -1);
			SetGroupTask(nGroupId, _T("Task: Check Group"));
			CString subtitle;
			subtitle.Format(_T("Subtitle: %i rows"), groupRows.GetSize());
			SetGroupSubtitle(nGroupId, subtitle );
			SetGroupFooter(nGroupId, _T("Group Footer"));
			
			for(int groupRow = 0; groupRow < groupRows.GetSize(); ++groupRow)
			{
				VERIFY( SetRowGroupId(groupRows[groupRow], nGroupId) );
			}
		}
		return TRUE;
	}

	return FALSE;
}

void CGridListCtrlGroups::CollapseAllGroups()
{
	// Loop through all rows and find possible groups
	for(int nRow=0; nRow<GetItemCount(); ++nRow)
	{
		int nGroupId = GetRowGroupId(nRow);
		if (nGroupId!=-1)
		{
			if (!HasGroupState(nGroupId,LVGS_COLLAPSED))
			{
				SetGroupState(nGroupId,LVGS_COLLAPSED);
			}
		}
	}
}

void CGridListCtrlGroups::ExpandAllGroups()
{
	// Loop through all rows and find possible groups
	for(int nRow=0; nRow<GetItemCount(); ++nRow)
	{
		int nGroupId = GetRowGroupId(nRow);
		if (nGroupId!=-1)
		{
			if (HasGroupState(nGroupId,LVGS_COLLAPSED))
			{
				SetGroupState(nGroupId,LVGS_NORMAL);
			}
		}
	}
}

void CGridListCtrlGroups::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if ( IsGroupViewEnabled() )
	{
		CPoint pt = point;
		ScreenToClient(&pt);

		int nGroupId = GroupHitTest(pt);
		if (nGroupId!=-1)
		{
			OnContextMenuGroup(pWnd, point, nGroupId);
			return;
		}
	}
	CGridListCtrlEx::OnContextMenu(pWnd, point);
}

namespace {
	bool IsCommonControlsEnabled()
	{
		// Test if application has access to common controls (Required for grouping)
		HMODULE hinstDll = ::LoadLibrary(_T("comctl32.dll"));
		if (hinstDll)
		{
			DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hinstDll, "DllGetVersion");
			::FreeLibrary(hinstDll);
			if (pDllGetVersion != NULL)
			{
				DLLVERSIONINFO dvi = {0};
				dvi.cbSize = sizeof(dvi);
				HRESULT hRes = pDllGetVersion ((DLLVERSIONINFO *) &dvi);
				if (SUCCEEDED(hRes))
					return dvi.dwMajorVersion >= 6;
			}
		}
		return false;
	}
}

void CGridListCtrlGroups::OnContextMenuHeader(CWnd* pWnd, CPoint point, int nCol)
{
	if (!IsCommonControlsEnabled())
	{
		CGridListCtrlEx::OnContextMenuHeader(pWnd, point, nCol);
		return;
	}

	// Show context-menu with the option to show hide columns
	CMenu menu;
	VERIFY( menu.CreatePopupMenu() );

	for( int i = GetColumnTraitSize()-1 ; i >= 0; --i)
	{
		CGridColumnTrait* pTrait = GetColumnTrait(i);
		CGridColumnTrait::ColumnState& columnState = pTrait->GetColumnState();

		if (columnState.m_AlwaysHidden)
			continue;	// Cannot be shown

		UINT uFlags = MF_BYPOSITION | MF_STRING;

		// Put check-box on context-menu
		if (IsColumnVisible(i))
			uFlags |= MF_CHECKED;
		else
			uFlags |= MF_UNCHECKED;

		// Retrieve column-title
		LVCOLUMN lvc = {0};
		lvc.mask = LVCF_TEXT;
		TCHAR sColText[256];
		lvc.pszText = sColText;
		lvc.cchTextMax = sizeof(sColText)-1;
		VERIFY( GetColumn(i, &lvc) );

		// +1 as zero is a reserved value in TrackPopupMenu() 
		menu.InsertMenu(0, uFlags, i+1, lvc.pszText);
	}

	UINT uFlags = MF_BYPOSITION | MF_STRING;
	if (IsGroupViewEnabled())
	{
		menu.InsertMenu(0, uFlags | MF_SEPARATOR, 0, _T(""));
		menu.InsertMenu(0, uFlags, GetColumnTraitSize()+1, _T("Disable grouping"));
	}
	else
	if (nCol!=-1)
	{
		menu.InsertMenu(0, uFlags | MF_SEPARATOR, 0, _T(""));

		// Retrieve column-title
		LVCOLUMN lvc = {0};
		lvc.mask = LVCF_TEXT;
		TCHAR sColText[256];
		lvc.pszText = sColText;
		lvc.cchTextMax = sizeof(sColText)-1;
		VERIFY( GetColumn(nCol, &lvc) );

		menu.InsertMenu(0, uFlags, GetColumnTraitSize()+2, CString(_T("Group by: ")) + lvc.pszText);
	}

	// Will return zero if no selection was made (TPM_RETURNCMD)
	int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this, 0);
	if (nResult!=0)
	{
		if (nResult <= GetColumnTraitSize())
		{
			int nCol = nResult-1;
			ShowColumn(nCol, !IsColumnVisible(nCol));
		}
		else
		{
			int nCmd = nResult - GetColumnTraitSize();
			switch(nCmd)
			{
				case 1: RemoveAllGroups(); EnableGroupView(FALSE); break;
				case 2: GroupByColumn(nCol); break;
			}
		}
	}
}

void CGridListCtrlGroups::OnContextMenuGroup(CWnd* pWnd, CPoint point, int nGroupId)
{
	CMenu menu;
	UINT uFlags = MF_BYPOSITION | MF_STRING;
	VERIFY( menu.CreatePopupMenu() );
	
	const CString& groupHeader = GetGroupHeader(nGroupId);

#ifndef LVGS_COLLAPSIBLE
	if (IsGroupStateEnabled())
	{
		if (HasGroupState(nGroupId,LVGS_COLLAPSED))
		{
			CString menuText = CString(_T("Expand group: ")) + groupHeader;
			menu.InsertMenu(0, uFlags, 1, menuText);
		}
		else
		{
			CString menuText = CString(_T("Collapse group: ")) + groupHeader;
			menu.InsertMenu(0, uFlags, 2, menuText);
		}
	}
#endif
	if (GetExtendedStyle() & LVS_EX_CHECKBOXES)
	{
		CString menuText = CString(_T("Check group: ")) + groupHeader;
		menu.InsertMenu(1, uFlags, 3, menuText);
		menuText = CString(_T("Uncheck group: ")) + groupHeader;
		menu.InsertMenu(2, uFlags, 4, menuText);
	}

	int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this, 0);
	switch(nResult)
	{
		case 1: SetGroupState(nGroupId,LVGS_NORMAL); break;
		case 2: SetGroupState(nGroupId,LVGS_COLLAPSED); break;
		case 3: CheckEntireGroup(nGroupId, true); break;
		case 4: CheckEntireGroup(nGroupId, false); break;
	}
}

void CGridListCtrlGroups::OnContextMenuGrid(CWnd* pWnd, CPoint point)
{
	if (IsGroupStateEnabled())
	{
		CMenu menu;
		UINT uFlags = MF_BYPOSITION | MF_STRING;
		VERIFY( menu.CreatePopupMenu() );

		menu.InsertMenu(0, uFlags, 1, _T("Expand all groups"));
		menu.InsertMenu(1, uFlags, 2, _T("Collapse all groups"));
		menu.InsertMenu(2, uFlags, 3, _T("Disable grouping"));

		int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this, 0);
		switch(nResult)
		{
			case 1: ExpandAllGroups(); break;
			case 2: CollapseAllGroups(); break;
			case 3: RemoveAllGroups(); EnableGroupView(FALSE); break;
		}
	}
}

BOOL CGridListCtrlGroups::SetGroupFooter(int nGroupID, const CString& footer)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_FOOTER;
#ifdef UNICODE
	lg.pszFooter = footer.GetBuffer();
	lg.cchFooter = footer.GetLength();
#else
	CComBSTR bstrFooter = footer;
	lg.pszFooter = bstrFooter;
	lg.cchFooter = bstrFooter.Length();
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

BOOL CGridListCtrlGroups::SetGroupTask(int nGroupID, const CString& task)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_TASK;
#ifdef UNICODE
	lg.pszTask = task.GetBuffer();
	lg.cchTask = task.GetLength();
#else
	CComBSTR bstrTask = task;
	lg.pszTask = bstrTask;
	lg.cchTask = bstrTask.Length();
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

BOOL CGridListCtrlGroups::SetGroupSubtitle(int nGroupID, const CString& subtitle)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_SUBTITLE;
#ifdef UNICODE
	lg.pszSubtitle = subtitle.GetBuffer();
	lg.cchSubtitle = subtitle.GetLength();
#else
	CComBSTR bstrSubtitle = subtitle;
	lg.pszSubtitle = bstrSubtitle;
	lg.cchSubtitle = bstrSubtitle.Length();
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

BOOL CGridListCtrlGroups::SetGroupTitleImage(int nGroupID, int nImage, const CString& topDesc, const CString& bottomDesc)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_TITLEIMAGE;
	lg.iTitleImage = nImage;	// Index of the title image in the control imagelist.

#ifdef UNICODE
	if (!topDesc.IsEmpty())
	{
		// Top description is drawn opposite the title image when there is
		// a title image, no extended image, and uAlign==LVGA_HEADER_CENTER.
		lg.mask |= LVGF_DESCRIPTIONTOP;
		lg.pszDescriptionTop = topDesc;
		lg.cchDescriptionTop = topDesc.GetLength();
	}
	if (!bottomDesc.IsEmpty())
	{
		// Bottom description is drawn under the top description text when there is
		// a title image, no extended image, and uAlign==LVGA_HEADER_CENTER.
		lg.mask |= LVGF_DESCRIPTIONBOTTOM;
		lg.pszDescriptionBottom = bottomDesc;
		lg.cchDescriptionBottom = bottomDesc.GetLength();
	}
#else
	CComBSTR bstrTopDesc = topDesc;
	CComBSTR bstrBottomDesc = bottomDesc;
	if (!topDesc.IsEmpty())
	{
		lg.mask |= LVGF_DESCRIPTIONTOP;
		lg.pszDescriptionTop = bstrTopDesc;
		lg.cchDescriptionTop = bstrTopDesc.Length();
	}
	if (!bottomDesc.IsEmpty())
	{
		lg.mask |= LVGF_DESCRIPTIONBOTTOM;
		lg.pszDescriptionBottom = bstrBottomDesc;
		lg.cchDescriptionBottom = bstrBottomDesc.Length();
	}
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

BOOL CGridListCtrlGroups::OnGroupTaskClick(NMHDR* pNMHDR, LRESULT* pResult)
{
#if _WIN32_WINNT >= 0x0600
	NMLVLINK* pLinkInfo = (NMLVLINK*)pNMHDR;
	int nGroupId = pLinkInfo->iSubItem;
#endif
	return FALSE;
}

void CGridListCtrlGroups::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int nGroupId = GroupHitTest(point);
	if (nGroupId!=-1)
	{
		if (HasGroupState(nGroupId, LVGS_COLLAPSED))
			SetGroupState(nGroupId, LVGS_NORMAL);
		else
			SetGroupState(nGroupId, LVGS_COLLAPSED);
	}
}

//------------------------------------------------------------------------
//! Handles the LVN_GETDISPINFO message, which is sent when details are
//! needed for an item that specifies callback.
//!		- Cell-Group, when item is using I_GROUPIDCALLBACK
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pNMW = (NMLVDISPINFO*)pNMHDR;
	int nRow = pNMW->item.iItem;
	int nCol = pNMW->item.iSubItem;

	if (nRow< 0 || nRow >= GetItemCount())
		return FALSE;	// requesting invalid item

	if (nCol < 0 || nCol >= GetHeaderCtrl()->GetItemCount())
		return FALSE;	// requesting invalid item

	if (pNMW->item.mask & LVIF_GROUPID)
	{
		// Request group-id of the column (I_GROUPIDCALLBACK)
		int result = -1;
		if (CallbackCellGroup(nRow, nCol, result))
			pNMW->item.iGroupId = result;
		else
			pNMW->item.iGroupId = I_GROUPIDNONE;
	}
	return CGridListCtrlEx::OnGetDispInfo(pNMHDR, pResult);
}

namespace {
	struct PARAMSORT
	{
		PARAMSORT(HWND hWnd, int columnIndex, bool ascending)
			:m_hWnd(hWnd)
			,m_ColumnIndex(columnIndex)
			,m_Ascending(ascending)
		{}

		HWND m_hWnd;
		int  m_ColumnIndex;
		bool m_Ascending;
		CSimpleMap<int,CString> m_GroupNames;
	};

	// Comparison extracts values from the List-Control
	int CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		PARAMSORT& ps = *(PARAMSORT*)lParamSort;

		TCHAR left[256] = _T(""), right[256] = _T("");
		ListView_GetItemText(ps.m_hWnd, lParam1, ps.m_ColumnIndex, left, sizeof(left));
		ListView_GetItemText(ps.m_hWnd, lParam2, ps.m_ColumnIndex, right, sizeof(right));

		if (ps.m_Ascending)
			return _tcscmp( left, right );
		else
			return _tcscmp( right, left );			
	}

	int CALLBACK SortFuncGroup(int nGroupId1, int nGroupId2, void* lParamSort)
	{
		PARAMSORT& ps = *(PARAMSORT*)lParamSort;

		const CString& left = ps.m_GroupNames.Lookup(nGroupId1);
		const CString& right = ps.m_GroupNames.Lookup(nGroupId2);

		if (ps.m_Ascending)
			return _tcscmp( left, right );
		else
			return _tcscmp( right, left );	
	}
}

bool CGridListCtrlGroups::SortColumn(int columnIndex, bool ascending)
{
	if (IsGroupViewEnabled())
	{
		PARAMSORT paramsort(m_hWnd, columnIndex, ascending);

		GroupByColumn(columnIndex);

		// Cannot use GetGroupInfo during sort
		for(int nRow=0 ; nRow < GetItemCount() ; ++nRow)
		{
			int nGroupId = GetRowGroupId(nRow);
			if (nGroupId!=-1 && paramsort.m_GroupNames.Lookup(nGroupId).IsEmpty())
				paramsort.m_GroupNames.Add(nGroupId, GetGroupHeader(nGroupId));
		}

		// Avoid bug in CListCtrl::SortGroups() which differs from ListView_SortGroups
		ListView_SortGroups(m_hWnd, SortFuncGroup, &paramsort);
		return true;
	}
	else
	{
		return CGridListCtrlEx::SortColumn(columnIndex, ascending);
	}
}

#endif // _WIN32_WINNT >= 0x0501