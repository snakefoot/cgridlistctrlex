#include "stdafx.h"
#include "CGridListCtrlGroups.h"

#include <shlwapi.h>	// IsCommonControlsEnabled

#include "CGridColumnTrait.h"

// MFC headers with group-support is only availabe from VS.NET 
#if _MSC_VER >= 1300
// WIN32 defines for group-support is only available from 2003 PSDK
#if _WIN32_WINNT >= 0x0501

BEGIN_MESSAGE_MAP(CGridListCtrlGroups, CGridListCtrlEx)
#if _WIN32_WINNT >= 0x0600
	ON_NOTIFY_REFLECT_EX(LVN_LINKCLICK, OnGroupTaskClick)	// Task-Link Click
	ON_NOTIFY_REFLECT_EX(LVN_GETEMPTYMARKUP, OnGetEmptyMarkup)	// Request text to display when empty
#endif
END_MESSAGE_MAP()

CGridListCtrlGroups::CGridListCtrlGroups()
	:m_GroupHeight(-1)
{}

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
	lg.pszHeader = (LPWSTR)(LPCTSTR)strHeader;
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

//------------------------------------------------------------------------
//! Windows Vista or higher is required to modify the state of a group
//------------------------------------------------------------------------
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
	// Maintain LVGS_COLLAPSIBLE state
	if (HasGroupState(nGroupID, LVGS_COLLAPSIBLE))
		lg.state |= LVGS_COLLAPSIBLE;
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------
//! Find the group-id below the given point
//------------------------------------------------------------------------
int CGridListCtrlGroups::GroupHitTest(const CPoint& point)
{
	if (!IsGroupViewEnabled())
		return -1;

	if (HitTest(point)!=-1)
		return -1;

	if (IsGroupStateEnabled())
	{
#if _WIN32_WINNT >= 0x0600
#ifdef ListView_HitTestEx
#ifdef LVHT_EX_GROUP
		LVHITTESTINFO lvhitinfo = {0};
		lvhitinfo.pt = point;
		ListView_HitTestEx(m_hWnd, &lvhitinfo);
		if ((lvhitinfo.flags & LVHT_EX_GROUP)==0)
			return -1;
#endif
#endif
#endif

#if _WIN32_WINNT >= 0x0600
#ifdef ListView_GetGroupCount
#ifdef ListView_GetGroupRect
#ifdef ListView_GetGroupInfoByIndex
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
#endif
	}	// IsGroupStateEnabled()

	CRect headerRect;
	GetHeaderCtrl()->GetClientRect(&headerRect);
	if (headerRect.PtInRect(point))
		return -1;

	// We require that each group contains atleast one item
	if (GetItemCount()==0)
		return -1;

	CRect gridRect(0,0,0,0);
	GetClientRect(&gridRect);

	// Loop through all rows until a visible row below the point is found
	int nPrevGroupId = -1, nCollapsedGroups = 0;
	int nRowAbove = -1, nRowBelow = 0;
	for(nRowBelow = 0; nRowBelow < GetItemCount(); nRowBelow++)
	{
		int nGroupId = GetRowGroupId(nRowBelow);

		CRect rectRowBelow;
		if (GetItemRect(nRowBelow, rectRowBelow, LVIR_BOUNDS)==FALSE)
		{
			// Found invisible row (row within collapsed group)
			if (nGroupId != -1 && nGroupId != nPrevGroupId)
			{
				//	Test if the point is within a collapsed group
				nCollapsedGroups++;
				nPrevGroupId = nGroupId;
				CRect groupRect = gridRect;
				if (nRowAbove!=-1)
				{
					GetItemRect(nRowAbove, groupRect, LVIR_BOUNDS);
					groupRect.right = gridRect.right;
				}
				else
				{
					groupRect.bottom = headerRect.bottom;
				}
				groupRect.bottom += m_GroupHeight * nCollapsedGroups;
				if (groupRect.PtInRect(point))
					return nGroupId;	// Hit a collapsed group
			}
			continue;	
		}

		// Cheating by remembering the group-height from the first visible row
		if (m_GroupHeight==-1)
		{
			m_GroupHeight = rectRowBelow.top - headerRect.Height();
		}

		rectRowBelow.right = gridRect.right;
		if (rectRowBelow.PtInRect(point))
			return -1;	// Hit a row
		if (rectRowBelow.top > point.y)
			break;		// Found row just below the point

		nCollapsedGroups = 0;
		nRowAbove = nRowBelow;
	}

	if (nRowBelow < GetItemCount())
	{
		// Probably hit a group above this row
		return GetRowGroupId(nRowBelow);
	}

	return -1;
}

//------------------------------------------------------------------------
//! Update the checkbox of the label column (first column)
//------------------------------------------------------------------------
void CGridListCtrlGroups::CheckEntireGroup(int nGroupId, bool bChecked)
{
	if (!(GetExtendedStyle() & LVS_EX_CHECKBOXES))
		return;

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

//------------------------------------------------------------------------
//! Create a group for each unique values within a column
//------------------------------------------------------------------------
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
			if (IsGroupStateEnabled())
				dwState = LVGS_COLLAPSIBLE;
#endif
			VERIFY( InsertGroupHeader(nGroupId, nGroupId, groups.GetKeyAt(nGroupId), dwState) != -1);

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
		if (point.x!=-1 && point.y!=-1)
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
		const CString& columnTitle = GetColumnHeading(i);

		// +1 as zero is a reserved value in TrackPopupMenu() 
		menu.InsertMenu(0, uFlags, i+1, static_cast<LPCTSTR>(columnTitle));
	}

	UINT uFlags = MF_BYPOSITION | MF_STRING;
	if (IsGroupViewEnabled())
	{
		menu.InsertMenu(0, uFlags | MF_SEPARATOR, 0, _T(""));
		menu.InsertMenu(0, uFlags, GetColumnTraitSize()+1, _T("Disable grouping"));
	}

	if (nCol!=-1)
	{
		if (!IsGroupViewEnabled())
			menu.InsertMenu(0, uFlags | MF_SEPARATOR, 0, _T(""));

		// Retrieve column-title
		const CString& columnTitle = GetColumnHeading(nCol);

		menu.InsertMenu(0, uFlags, GetColumnTraitSize()+2, CString(_T("Group by: ")) + columnTitle);
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

BOOL CGridListCtrlGroups::SetGroupFooter(int nGroupID, const CString& footer, DWORD dwAlign)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_FOOTER | LVGF_ALIGN;
	lg.uAlign = dwAlign;
#ifdef UNICODE
	lg.pszFooter = (LPWSTR)(LPCTSTR)footer;
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
	lg.pszTask = (LPWSTR)(LPCTSTR)task;
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
	lg.pszSubtitle = (LPWSTR)(LPCTSTR)subtitle;
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
		lg.pszDescriptionTop = (LPWSTR)(LPCTSTR)topDesc;
		lg.cchDescriptionTop = topDesc.GetLength();
	}
	if (!bottomDesc.IsEmpty())
	{
		// Bottom description is drawn under the top description text when there is
		// a title image, no extended image, and uAlign==LVGA_HEADER_CENTER.
		lg.mask |= LVGF_DESCRIPTIONBOTTOM;
		lg.pszDescriptionBottom = (LPWSTR)(LPCTSTR)bottomDesc;
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

BOOL CGridListCtrlGroups::OnGetEmptyMarkup(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_EmptyMarkupText.IsEmpty())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	NMLVEMPTYMARKUP* pEmptyMarkup = (NMLVEMPTYMARKUP*)pNMHDR;
	pEmptyMarkup->dwFlags = EMF_CENTERED;

#ifdef UNICODE
	lstrcpyn(pEmptyMarkup->szMarkup, (LPCTSTR)m_EmptyMarkupText, sizeof(pEmptyMarkup->szMarkup));
#else
	_mbstowcsz(pEmptyMarkup->szMarkup, (LPCTSTR)m_EmptyMarkupText, sizeof(pEmptyMarkup->szMarkup));
#endif
	*pResult = TRUE;

#endif
	return TRUE;
}

BOOL CGridListCtrlGroups::OnGroupTaskClick(NMHDR* pNMHDR, LRESULT* pResult)
{
#if _WIN32_WINNT >= 0x0600
	NMLVLINK* pLinkInfo = (NMLVLINK*)pNMHDR;
	int nGroupId = pLinkInfo->iSubItem;
	nGroupId;	// Avoid unreferenced variable warning
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
		// Request group-id of the column (Virtual-list/LVS_OWNERDATA)
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

		const CString& LookupGroupName(int nGroupId)
		{
			int groupIdx = m_GroupNames.FindKey(nGroupId);
			if (groupIdx==-1)
			{
				static const CString emptyStr;
				return emptyStr;
			}			
			return m_GroupNames.GetValueAt(groupIdx);
		}
	};

	int CALLBACK SortFuncGroup(int leftId, int rightId, void* lParamSort)
	{
		PARAMSORT& ps = *(PARAMSORT*)lParamSort;

		const CString& left = ps.LookupGroupName(leftId);
		const CString& right = ps.LookupGroupName(rightId);

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
			if (nGroupId!=-1 && paramsort.m_GroupNames.FindKey(nGroupId)==-1)
				paramsort.m_GroupNames.Add(nGroupId, GetGroupHeader(nGroupId));
		}

		// Avoid bug in CListCtrl::SortGroups() which differs from ListView_SortGroups
		ListView_SortGroups(m_hWnd, SortFuncGroup, &paramsort);
	}

	// Always sort the rows, so the handicapped GroupHitTest() will work
	//	- Must ensure that the rows are reordered along with the groups.
	return CGridListCtrlEx::SortColumn(columnIndex, ascending);
}

void CGridListCtrlGroups::OnPaint()
{
#if _WIN32_WINNT >= 0x0600
	if (UsingVisualStyle())
	{
		// Use LVN_GETEMPTYMARKUP if available
		CListCtrl::OnPaint();	// default
		return;
	}
#endif

    CGridListCtrlEx::OnPaint();
}

#endif // _WIN32_WINNT >= 0x0501
#endif // _MSC_VER >= 1300