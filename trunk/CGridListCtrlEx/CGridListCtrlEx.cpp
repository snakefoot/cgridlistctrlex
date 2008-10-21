#include "stdafx.h"
#include "CGridListCtrlEx.h"

#include <shlwapi.h>	// IsThemeEnabled

#include "CGridColumnTraitText.h"
#include "CGridRowTraitText.h"

BEGIN_MESSAGE_MAP(CGridListCtrlEx, CListCtrl)
	//{{AFX_MSG_MAP(CGridListCtrlEx)
	ON_NOTIFY_REFLECT_EX(LVN_BEGINLABELEDIT, OnBeginLabelEdit) 
	ON_NOTIFY_REFLECT_EX(LVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_NOTIFY_REFLECT_EX(LVN_GETDISPINFO, OnGetDispInfo)	// Text Callback
	ON_MESSAGE(LVM_DELETECOLUMN, OnDeleteColumn)
	ON_MESSAGE(LVM_INSERTCOLUMN, OnInsertColumn)
	ON_MESSAGE(LVM_SETCOLUMNWIDTH, OnSetColumnWidth)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_NOTIFY_EX(HDN_BEGINTRACKA, 0, OnHeaderBeginResize)
	ON_NOTIFY_EX(HDN_BEGINTRACKW, 0, OnHeaderBeginResize)
	ON_NOTIFY_EX(HDN_BEGINDRAG, 0, OnHeaderBeginDrag)
	ON_NOTIFY_EX(HDN_ENDDRAG, 0, OnHeaderEndDrag)
	ON_NOTIFY_EX(HDN_DIVIDERDBLCLICKA, 0, OnHeaderDividerDblClick)
	ON_NOTIFY_EX(HDN_DIVIDERDBLCLICKW, 0, OnHeaderDividerDblClick)
	ON_NOTIFY_EX(TTN_NEEDTEXTA, 0, OnToolNeedText)
	ON_NOTIFY_EX(TTN_NEEDTEXTW, 0, OnToolNeedText)
	ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnHeaderClick)	// Column Click
	ON_WM_CONTEXTMENU()	// OnContextMenu
	ON_WM_KEYDOWN()		// OnKeyDown
	ON_WM_LBUTTONDOWN()	// OnLButtonDown(UINT nFlags, CPoint point)
	ON_WM_RBUTTONDOWN()	// OnRButtonDown(UINT nFlags, CPoint point)
	ON_WM_LBUTTONDBLCLK()// OnLButtonDblClk(UINT nFlags, CPoint point)
	ON_WM_HSCROLL()		// OnHScroll
	ON_WM_VSCROLL()		// OnVScroll
	ON_WM_CHAR()		// OnChar
	ON_WM_PAINT()		// OnPaint
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CGridListCtrlEx::CGridListCtrlEx()
	:m_FocusCell(-1)
	,m_SortCol(-1)
	,m_Ascending(false)
	,m_UsingVisualStyle(false)
	,m_pEditor(NULL)
	,m_LastSearchCell(-1)
	,m_LastSearchRow(-1)
	,m_EmptyMarkupText(_T("There are no items to show in this view."))
	,m_Margin(1)		// Higher row-height (more room for edit-ctrl border)
	,m_pGridFont(NULL)
	,m_pCellFont(NULL)
	,m_pDefaultRowTrait(new CGridRowTraitText)
{}

CGridListCtrlEx::~CGridListCtrlEx()
{
	for(int nCol = GetColumnTraitSize()-1; nCol >= 0 ; --nCol)
		DeleteColumnTrait(nCol);

	delete m_pDefaultRowTrait;
	m_pDefaultRowTrait = NULL;

	delete m_pGridFont;
	m_pGridFont = NULL;
	delete m_pCellFont;
	m_pCellFont = NULL;
}

namespace {
	bool IsCommonControlsEnabled()
	{
		// Test if application has access to common controls
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

	bool IsThemeEnabled()
	{
		HMODULE hinstDll;
		bool XPStyle = false;
		bool (__stdcall *pIsAppThemed)();
		bool (__stdcall *pIsThemeActive)();

		// Test if operating system has themes enabled
		hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			(FARPROC&)pIsAppThemed = ::GetProcAddress(hinstDll, "IsAppThemed");
			(FARPROC&)pIsThemeActive = ::GetProcAddress(hinstDll,"IsThemeActive");
			::FreeLibrary(hinstDll);
			if (pIsAppThemed != NULL && pIsThemeActive != NULL)
			{
				if (pIsAppThemed() && pIsThemeActive())
				{
					// Test if application has themes enabled by loading the proper DLL
					return IsCommonControlsEnabled();
				}
			}
		}
		return XPStyle;
	}

	LRESULT EnableWindowTheme(HWND hwnd, LPCWSTR classList, LPCWSTR subApp, LPCWSTR idlist)
	{
		HMODULE hinstDll;
		HRESULT (__stdcall *pSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
		HANDLE (__stdcall *pOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
		HRESULT (__stdcall *pCloseThemeData)(HANDLE hTheme);

		hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			(FARPROC&)pOpenThemeData = ::GetProcAddress(hinstDll, "OpenThemeData");
			(FARPROC&)pCloseThemeData = ::GetProcAddress(hinstDll, "CloseThemeData");
			(FARPROC&)pSetWindowTheme = ::GetProcAddress(hinstDll, "SetWindowTheme");
			::FreeLibrary(hinstDll);
			if (pSetWindowTheme && pOpenThemeData && pCloseThemeData)
			{
				HANDLE theme = pOpenThemeData(hwnd,classList);
				if (theme!=NULL)
				{
					VERIFY(pCloseThemeData(theme)==S_OK);
					return pSetWindowTheme(hwnd, subApp, idlist);
				}
			}
		}
		return S_FALSE;
	}
}

LRESULT CGridListCtrlEx::EnableVisualStyles(bool bValue)
{
	if (!IsThemeEnabled())
	{
		m_UsingVisualStyle = false;
		return S_FALSE;
	}

	OSVERSIONINFO osver = {0};
	osver.dwOSVersionInfoSize = sizeof(osver);
	GetVersionEx(&osver);
	WORD fullver = MAKEWORD(osver.dwMinorVersion, osver.dwMajorVersion);
	if (fullver < 0x0600)
	{
		m_UsingVisualStyle = false;
		return S_FALSE;
	}

	LRESULT rc = S_FALSE;
	if (bValue)
		rc = EnableWindowTheme(GetSafeHwnd(), L"ListView", L"Explorer", NULL);
	else
		rc = EnableWindowTheme(GetSafeHwnd(), L"", L"", NULL);

	if (bValue && rc==S_OK)
	{
		// OBS! Focus retangle is not painted properly without double-buffering
		m_UsingVisualStyle = true;
#if (_WIN32_WINNT >= 0x501)
		SetExtendedStyle(LVS_EX_DOUBLEBUFFER | GetExtendedStyle());
#endif
	}
	else
	{
		m_UsingVisualStyle = false;
	}

	return rc;
}

void CGridListCtrlEx::OnCreateStyle()
{
	// Will be called twice when placed inside a CView

	// Not using VERIFY / ASSERT as MessageBox cannot be opened during subclassing/creating
	if (!(GetStyle() & LVS_REPORT))
		DebugBreak();	// CListCtrl must be created with style LVS_REPORT
	if (GetStyle() & LVS_OWNERDRAWFIXED)
		DebugBreak();	// CListCtrl must be created without style LVS_OWNERDRAWFIXED

	ModifyStyle(0, LVS_SHOWSELALWAYS);

	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_HEADERDRAGDROP);
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_GRIDLINES);
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_SUBITEMIMAGES);
#if (_WIN32_WINNT >= 0x501)
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_DOUBLEBUFFER);
#endif
	
	// Enable Vista-look if possible
	EnableVisualStyles(true);

	// Enable the standard tooltip
	EnableToolTips(TRUE);

	// Disable the builtin tooltip (if available)
	CToolTipCtrl* pToolTipCtrl = (CToolTipCtrl*)CWnd::FromHandle((HWND)::SendMessage(m_hWnd, LVM_GETTOOLTIPS, 0, 0L));
	if (pToolTipCtrl!=NULL && pToolTipCtrl->m_hWnd!=NULL)
        pToolTipCtrl->Activate(FALSE);
}

int CGridListCtrlEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Will not be called when placed inside a CDialog
	int rc = CListCtrl::OnCreate(lpCreateStruct);
	if (rc==0)
		OnCreateStyle();

	return rc;
}

void CGridListCtrlEx::PreSubclassWindow()
{
	// Changes made to style will not having any effect when placed in a CView
	CListCtrl::PreSubclassWindow();
	OnCreateStyle();
}

int CGridListCtrlEx::InsertColumnTrait(int nCol, const CString& columnHeading, int nFormat, int nWidth, int nSubItem, CGridColumnTrait* pTrait)
{
	if (pTrait!=NULL)
	{
		if (pTrait->GetColumnState().m_AlwaysHidden)
		{
			nWidth = 0;
			pTrait->GetColumnState().m_Visible = false;
		}

		// nCol specifies the position, if nCol is greater than count,
		// then insert as last
		if (nCol >= GetHeaderCtrl()->GetItemCount())
		{
			nCol = GetHeaderCtrl()->GetItemCount();
			InsertColumnTrait(nCol, pTrait);
		}
		else
		{
			InsertColumnTrait(nCol, pTrait);
		}
	}

	int index = InsertColumn(nCol, static_cast<LPCTSTR>(columnHeading), nFormat, nWidth, nSubItem);
	if (index != -1)
		VERIFY( index == nCol );
	else
		DeleteColumnTrait(nCol);
	return index;
}

//------------------------------------------------------------------------
//! Inserts the label column (first column) with invisible state
//! The label column behaves differently from the rest of the columns,
//! and to get the uniform look, then it should be hidden away.
//!	 - It has a special margin, which no other column has
//!  - When dragged to another position than the first, then it looses it special margin
//------------------------------------------------------------------------
int CGridListCtrlEx::InsertHiddenLabelColumn()
{
	// Must be the label column
	VERIFY(GetHeaderCtrl()->GetItemCount()==0);

	CGridColumnTrait* pColumTrait = new CGridColumnTrait;
	pColumTrait->GetColumnState().m_AlwaysHidden = true;
	return InsertColumnTrait(0, _T(""), LVCFMT_LEFT, -1, -1, pColumTrait);
}

const CHeaderCtrl* CGridListCtrlEx::GetHeaderCtrl() const
{
	ASSERT(::IsWindow(m_hWnd));

	HWND hWnd = (HWND) ::SendMessage(m_hWnd, LVM_GETHEADER, 0, 0);
	if (hWnd == NULL)
		return NULL;
	else
		return (const CHeaderCtrl*) CHeaderCtrl::FromHandle(hWnd);
}

CFont* CGridListCtrlEx::GetCellFont()
{
	if (m_pCellFont==NULL)
		return GetFont();
	return m_pCellFont;
}

//------------------------------------------------------------------------
//! Takes the current font and increases the font with the given margin
//! multiplier. Increases the row-height but keeps the cell font intact.
//! Gives more room for the grid-cell editors and their border.
//------------------------------------------------------------------------
void CGridListCtrlEx::SetCellMargin(double margin)
{
	m_Margin = margin;

	LOGFONT lf = {0};
	VERIFY(GetFont()->GetLogFont(&lf)!=0);

	delete m_pCellFont;
	m_pCellFont = new CFont;
	VERIFY( m_pCellFont->CreateFontIndirect(&lf) );

	lf.lfHeight = (int)(lf.lfHeight * m_Margin);
	lf.lfWidth = (int)(lf.lfWidth * m_Margin);
	delete m_pGridFont;
	m_pGridFont = NULL;
	m_pGridFont = new CFont();
	VERIFY( m_pGridFont->CreateFontIndirect(&lf) );

	CListCtrl::SetFont(m_pGridFont);
	GetHeaderCtrl()->SetFont(m_pCellFont);
	CToolTipCtrl* pToolTipCtrl = (CToolTipCtrl*)CWnd::FromHandle((HWND)::SendMessage(m_hWnd, LVM_GETTOOLTIPS, 0, 0L));
	if (pToolTipCtrl!=NULL && pToolTipCtrl->m_hWnd!=NULL)
		pToolTipCtrl->SetFont(m_pCellFont);
}

//------------------------------------------------------------------------
//! The column version of GetItemData(), one can specify an unique
//! identifier when using InsertColumn()
//------------------------------------------------------------------------
int CGridListCtrlEx::GetColumnData(int nCol) const
{
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_SUBITEM;
	VERIFY( GetColumn(nCol, &lvc) );
	return lvc.iSubItem;
}

//------------------------------------------------------------------------
//! Get column position in the CHeaderCtrl's display order array
//------------------------------------------------------------------------
int CGridListCtrlEx::GetColumnOrder(int nCol) const
{
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_ORDER;
	VERIFY( GetColumn(nCol, &lvc) );
	return lvc.iOrder;
}

int CGridListCtrlEx::GetFocusRow() const
{
	return GetNextItem(-1, LVNI_FOCUSED);
}

bool CGridListCtrlEx::IsRowSelected(int nRow) const
{
	return (GetItemState(nRow, LVIS_SELECTED) & LVIS_SELECTED) == LVIS_SELECTED;
}

BOOL CGridListCtrlEx::SelectRow(int nRow, bool bSelect)
{
	return SetItemState(nRow, bSelect ? LVIS_SELECTED : 0, LVIS_SELECTED);
}

//------------------------------------------------------------------------
//! Improved version of GetSubItemRect()
//!	- It doesn't return entire row-rect when using LVIR_BOUNDS for label-column (nCol==0)
//!	- It supports LVIR_LABEL for sub-items (nCol>0)
//! - It supports LVIR_BOUNDS when column width is less than subitem image width
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::GetCellRect(int nRow, int nCol, UINT nCode, CRect& rect)
{
	if (GetSubItemRect(nRow, nCol, nCode, rect)==FALSE)
		return FALSE;

	if (nCode == LVIR_BOUNDS)
	{
		// Find the left and right of the cell-rectangle using the CHeaderCtrl
		CRect colRect;
		if (GetHeaderCtrl()->GetItemRect(nCol, colRect)==FALSE)
			return FALSE;

		if (nCol==0)
		{
			// Fix bug where LVIR_BOUNDS gives the entire row for nCol==0
			CRect labelRect;
			if (GetSubItemRect(nRow, nCol, LVIR_LABEL, labelRect)==FALSE)
				return FALSE;

			rect.right = labelRect.right; 
			rect.left  = labelRect.right - colRect.Width();
		}
		else
		{
			// Fix bug when width is smaller than subitem image width
			rect.right = rect.left + colRect.Width();
		}
    }

	if (nCode == LVIR_LABEL && nCol>0)
	{
		if (!(GetExtendedStyle() & LVS_EX_SUBITEMIMAGES))
			return TRUE;	// no image in subitem

		int nImage = GetCellImage(nRow, nCol);
		if (nImage < 0)
			return TRUE;	// No image in subitem

		CRect iconRect;
		if (GetSubItemRect(nRow, nCol, LVIR_ICON, iconRect)==FALSE)
			return FALSE;

		rect.left += iconRect.Width();
	}

	return TRUE;
}

void CGridListCtrlEx::CellHitTest(const CPoint& pt, int& nRow, int& nCol) const
{
	nRow = -1;
	nCol = -1;

	LVHITTESTINFO lvhti = {0};
	lvhti.pt = pt;
	nRow = ListView_SubItemHitTest(m_hWnd, &lvhti);	// SubItemHitTest is non-const
	nCol = lvhti.iSubItem;
	if (!(lvhti.flags & LVHT_ONITEM))
		nRow = -1;
}

bool CGridListCtrlEx::IsCellCallback(int nRow, int nCol) const
{
	if (GetStyle() & LVS_OWNERDATA)
		return true;

	LV_ITEM lvi = {0};
	lvi.iItem = nRow;
	lvi.iSubItem = nCol;
	lvi.mask = LVIF_TEXT | LVIF_NORECOMPUTE;
	VERIFY( GetItem( &lvi ) );
	return lvi.pszText == LPSTR_TEXTCALLBACK;
}

int CGridListCtrlEx::GetCellImage(int nRow, int nCol) const
{
	LV_ITEM lvi = {0};
	lvi.iItem = nRow;
	lvi.iSubItem = nCol;
	lvi.mask = LVIF_IMAGE;
	VERIFY( GetItem( &lvi ) );
	return lvi.iImage;
}

BOOL CGridListCtrlEx::SetCellImage(int nRow, int nCol, int nImageId)
{
	LV_ITEM lvitem = {0};
	lvitem.mask = LVIF_IMAGE;
	lvitem.iItem = nRow;
	lvitem.iSubItem = nCol;
	lvitem.iImage = nImageId;	// I_IMAGENONE, I_IMAGECALLBACK
	return SetItem(&lvitem);
}

void CGridListCtrlEx::MoveFocusCell(bool right)
{
	if (GetItemCount()<=0)
	{
		m_FocusCell = -1;	// Entire row selected
		return;
	}

	if (m_FocusCell == -1)
	{
		// Entire row already selected
		if (right)
		{
			// Change to the first column in the current order
			m_FocusCell = GetFirstVisibleColumn();
		}
	}
	else
	{
		// Convert focus-cell to order index
		int nOrderIndex = -1;
		for(int i = 0; i < GetHeaderCtrl()->GetItemCount(); ++i)
		{
			int nCol = GetHeaderCtrl()->OrderToIndex(i);
			if (nCol == m_FocusCell)
			{
				nOrderIndex = i;
				break;
			}
		}

		// Move to the following column
		if (right)
			nOrderIndex++;
		else
			nOrderIndex--;

		// Convert order-index to focus cell
		if (nOrderIndex >= 0 && nOrderIndex < GetHeaderCtrl()->GetItemCount())
		{
			int nCol = GetHeaderCtrl()->OrderToIndex(nOrderIndex);
			if (IsColumnVisible(nCol))
				m_FocusCell = nCol;
			else
			if (!right)
				m_FocusCell = -1;	// Entire row selection
		}
		else if (!right)
			m_FocusCell = -1;	// Entire row selection
	}

	// Ensure the column is visible
	if (m_FocusCell >= 0)
	{
		VERIFY( EnsureColumnVisible(m_FocusCell, false) );
	}

	UpdateFocusCell(m_FocusCell);
}

// Force redraw of focus row, so the focus cell becomes visible
void CGridListCtrlEx::UpdateFocusCell(int nCol)
{
	m_FocusCell = nCol;	// Update focus cell before starting re-draw
	int nFocusRow = GetFocusRow();
	if (nFocusRow >= 0)
	{
		CRect itemRect;
		VERIFY( GetItemRect(nFocusRow, itemRect, LVIR_BOUNDS) );
		InvalidateRect(itemRect);
		UpdateWindow();
	}
}

//------------------------------------------------------------------------
//! Scrolls the view, so the column because visible
//!
//! http://www.codeguru.com/cpp/controls/listview/columns/article.php/c931/
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::EnsureColumnVisible(int nCol, bool bPartialOK)
{
	if (nCol < 0 || nCol >= GetHeaderCtrl()->GetItemCount())
		return FALSE;

	CRect rcHeader;
	if (GetHeaderCtrl()->GetItemRect(nCol, rcHeader)==FALSE)
		return FALSE;

	CRect rcClient;
	GetClientRect(&rcClient);

	int nOffset = GetScrollPos(SB_HORZ);

	if(bPartialOK)
	{
		if((rcHeader.left - nOffset < rcClient.right) && (rcHeader.right - nOffset > 0))
		{
			return TRUE;
		}
	}

	int nScrollX = 0;

	if((rcHeader.Width() > rcClient.Width()) || (rcHeader.left - nOffset < 0))
	{
		nScrollX = rcHeader.left - nOffset;
	}
	else if(rcHeader.right - nOffset > rcClient.right)
	{
		nScrollX = rcHeader.right - nOffset - rcClient.right;
	}

	if(nScrollX != 0)
	{
		CSize size(nScrollX, 0);
		if (Scroll(size)==FALSE)
			return FALSE;
	}

	return TRUE;
}

int CGridListCtrlEx::GetFirstVisibleColumn()
{
	int nColCount = GetHeaderCtrl()->GetItemCount();
	for(int i = 0; i < nColCount; ++i)
	{
		int nCol = GetHeaderCtrl()->OrderToIndex(i);
		if (IsColumnVisible(nCol))
		{
			return nCol;
		}
	}
	return -1;
}

BOOL CGridListCtrlEx::ShowColumn(int nCol, bool bShow)
{
	SetRedraw(FALSE);

	CGridColumnTrait* pTrait = GetColumnTrait(nCol);
	CGridColumnTrait::ColumnState& columnState = pTrait->GetColumnState();

	int nColCount = GetHeaderCtrl()->GetItemCount();
	int* pOrderArray = new int[nColCount];
	VERIFY( GetColumnOrderArray(pOrderArray, nColCount) );
	if (bShow)
	{
		// Restore the position of the column
		int nCurIndex = -1;
		for(int i = 0; i < nColCount ; ++i)
		{
			if (pOrderArray[i]==nCol)
				nCurIndex = i;
			else
			if (nCurIndex!=-1)
			{
				// We want to move it to the original position,
				// and after the last hidden column
				if ( (i <= columnState.m_OrgPosition)
				  || !IsColumnVisible(pOrderArray[i])
				   )
				{
					pOrderArray[nCurIndex] = pOrderArray[i];
					pOrderArray[i] = nCol;
					nCurIndex = i;
				}
			}
		}
	}
	else
	{
		// Move the column to the front of the display order list
		int nCurIndex(-1);
		for(int i = nColCount-1; i >=0 ; --i)
		{
			if (pOrderArray[i]==nCol)
			{
				// Backup the current position of the column
				columnState.m_OrgPosition = i;
				nCurIndex = i;
			}
			else
			if (nCurIndex!=-1)
			{
				pOrderArray[nCurIndex] = pOrderArray[i];
				pOrderArray[i] = nCol;
				nCurIndex = i;
			}
		}
	}

	VERIFY( SetColumnOrderArray(nColCount, pOrderArray) );
	delete [] pOrderArray;

	if (bShow)
	{
		// Restore the column width
		columnState.m_Visible = true;
		VERIFY( SetColumnWidth(nCol, columnState.m_OrgWidth) );
	}
	else
	{
		// Backup the column width
		int orgWidth = GetColumnWidth(nCol);
		VERIFY( SetColumnWidth(nCol, 0) );
		columnState.m_Visible = false;
		columnState.m_OrgWidth = orgWidth;
	}
	SetRedraw(TRUE);
	Invalidate(FALSE);
	return TRUE;
}

BOOL CGridListCtrlEx::SetColumnWidthAuto(int nCol, bool includeHeader)
{
	if (nCol == -1)
	{
		for(int i = 0; i < GetHeaderCtrl()->GetItemCount() ; ++i)
		{
			SetColumnWidthAuto(i, includeHeader);
		}
		return TRUE;
	}
	else
	{
		if (includeHeader)
			return SetColumnWidth(nCol, LVSCW_AUTOSIZE_USEHEADER);
		else
			return SetColumnWidth(nCol, LVSCW_AUTOSIZE);
	}
}

namespace {
	HBITMAP CreateSortBitmap(bool bAscending)
	{
		// Aquire the Display DC
		CDC* pDC = CDC::FromHandle(::GetDC(::GetDesktopWindow()));
		//create a memory dc
		CDC memDC;
		memDC.CreateCompatibleDC(pDC);

		//Create a memory bitmap
		CBitmap newbmp;
		CRect iconRect(0, 0, 16, 16);
		newbmp.CreateCompatibleBitmap(pDC, iconRect.Height(), iconRect.Width());

		//create a black brush
		CBrush brush;
		brush.CreateSolidBrush(RGB(0, 0, 0));

		//select the bitmap in the memory dc
		CBitmap *pOldBitmap = memDC.SelectObject(&newbmp);

		//make the bitmap white to begin with
		memDC.FillSolidRect(iconRect.top,iconRect.left,iconRect.bottom,iconRect.right,::GetSysColor(COLOR_3DFACE));

		//draw a rectangle using the brush
		CBrush *pOldBrush = memDC.SelectObject(&brush);
		if (bAscending)
		{
			// Arrow pointing down
			CPoint Pt[3];
			Pt[0] = CPoint(10, 6);	// Right
			Pt[1] = CPoint(4, 6);	// Left
			Pt[2] = CPoint(7, 9);	// Bottom
			memDC.Polygon(Pt, 3);
		}
		else
		{
			// Arrow pointing up
			CPoint Pt[3];
			Pt[0] = CPoint(7,  5);	// Top
			Pt[1] = CPoint(4,  8);	// Left
			Pt[2] = CPoint(10,  8);	// Right
			memDC.Polygon(Pt, 3);
		}
		memDC.SelectObject(pOldBrush);

		//select old bitmap back into the memory dc
		memDC.SelectObject(pOldBitmap);

		return (HBITMAP)newbmp.Detach();
	}
}

void CGridListCtrlEx::SetSortArrow(int colIndex, bool ascending)
{
	if (IsThemeEnabled())
	{
#if (_WIN32_WINNT >= 0x501)
		for(int i = 0; i < GetHeaderCtrl()->GetItemCount(); ++i)
		{
			HDITEM hditem = {0};
			hditem.mask = HDI_FORMAT;
			VERIFY( GetHeaderCtrl()->GetItem( i, &hditem ) );
			hditem.fmt &= ~(HDF_SORTDOWN|HDF_SORTUP);
			if (i == colIndex)
			{
				hditem.fmt |= ascending ? HDF_SORTDOWN : HDF_SORTUP;
			}
			VERIFY( CListCtrl::GetHeaderCtrl()->SetItem( i, &hditem ) );
		}
#endif
	}
	else
	{
		for(int i = 0; i < GetHeaderCtrl()->GetItemCount(); ++i)
		{
			HDITEM hditem = {0};
			hditem.mask = HDI_BITMAP | HDI_FORMAT;
			VERIFY( GetHeaderCtrl()->GetItem( i, &hditem ) );
			if (hditem.fmt & HDF_BITMAP && hditem.fmt & HDF_BITMAP_ON_RIGHT)
			{
				if (hditem.hbm)
				{
					VERIFY( DeleteObject(hditem.hbm) );
					hditem.hbm = NULL;
				}
				hditem.fmt &= ~(HDF_BITMAP|HDF_BITMAP_ON_RIGHT);
				VERIFY( CListCtrl::GetHeaderCtrl()->SetItem( i, &hditem ) );
			}
			if (i == colIndex)
			{
				hditem.fmt |= HDF_BITMAP|HDF_BITMAP_ON_RIGHT;
				//UINT bitmapID = m_Ascending ? IDB_DOWNARROW : IDB_UPARROW; 
				//hditem.hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmapID), IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS);
				hditem.hbm = CreateSortBitmap(ascending);
				VERIFY( hditem.hbm!=NULL );
				VERIFY( CListCtrl::GetHeaderCtrl()->SetItem( i, &hditem ) );
			}
		}
	}
}

void CGridListCtrlEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Catch event before the parent listctrl gets it to avoid extra scrolling
	//	- OBS! This can also prevent the key-events to reach LVN_KEYDOWN handlers
	switch(nChar)
	{
		case VK_RIGHT:	MoveFocusCell(true);	return;	// Do not allow scroll
		case VK_LEFT:	MoveFocusCell(false);	return;	// Do not allow scroll
		case 0x41:		// CTRL+A (Select all rows)
		{
			if (GetKeyState(VK_CONTROL) < 0)
			{
				if (!(GetStyle() & LVS_SINGLESEL))
					SelectRow(-1, true);
			}
			break;
		}
		case VK_ADD:	// CTRL + NumPlus (Auto size all columns)
		{
			if (GetKeyState(VK_CONTROL) < 0)
			{
				// Special handling to avoid showing "hidden" columns
				SetColumnWidthAuto(-1);
				return;
			}
		} break;
		case VK_F2:
		{
			EditCell(GetFocusRow(), m_FocusCell);
			break;
		}
	}
	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

// Keyboard search with subitems
void CGridListCtrlEx::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_FocusCell<=0)
	{
		CListCtrl::OnChar(nChar, nRepCnt, nFlags);
		return;
	}

	// No input within 2 seconds, resets the search
	if (m_LastSearchTime.GetCurrentTime() >= (m_LastSearchTime+2)
	 && m_LastSearchString.GetLength()>0)
		m_LastSearchString = _T("");

	// Changing cells, resets the search
	if (m_LastSearchCell!=m_FocusCell)
		m_LastSearchString = _T("");

	// Changing rows, resets the search
	if (m_LastSearchRow!=GetFocusRow())
		m_LastSearchString = _T("");

	m_LastSearchCell = m_FocusCell;
	m_LastSearchTime = m_LastSearchTime.GetCurrentTime();

	if ( m_LastSearchString.GetLength()==1
	  && m_LastSearchString.GetAt(0)==(TCHAR)nChar)
	{
		// When the same first character is entered again,
		// then just repeat the search
	}
	else
		m_LastSearchString.Insert(m_LastSearchString.GetLength()+1, (TCHAR)nChar);

	int nRow = GetFocusRow();
	if (nRow < 0)
		nRow = 0;
	int nCol = m_FocusCell;
	if (nCol < 0)
		nCol = GetFirstVisibleColumn();
	int nRowCount = GetItemCount();

	// Perform the search loop twice
	//	- First search from current position down to bottom
	//	- Then search from top to current position
	for(int j = 0; j < 2; ++j)
	{
		for(int i = nRow + 1; i < nRowCount; ++i)
		{
			CString cellText = GetItemText(i, nCol);
			if (cellText.GetLength()>=m_LastSearchString.GetLength())
			{
				cellText = cellText.Left(m_LastSearchString.GetLength());
				if (cellText.CompareNoCase(m_LastSearchString)==0)
				{
					// De-select all other rows
					SelectRow(-1, false);
					// Select row found
					SelectRow(i, true);
					// Focus row found
					SetItemState(i, LVIS_FOCUSED, LVIS_FOCUSED);	
					// Scroll to row found
					EnsureVisible(i, FALSE);			
					m_LastSearchRow = i;
					return;
				}
			}
		}
		nRowCount = nRow;
		nRow = -1;
	}
}

//------------------------------------------------------------------------
//! Handles the LVN_GETDISPINFO message, which is sent when details are
//! needed for an item that specifies callback.
//!		- Cell-Text, when item is using LPSTR_TEXTCALLBACK
//!		- Cell-Image, when item is using I_IMAGECALLBACK
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pNMW = (NMLVDISPINFO*)pNMHDR;
	int nRow = pNMW->item.iItem;
	int nCol = pNMW->item.iSubItem;

	if (nRow< 0 || nRow >= GetItemCount())
		return FALSE;	// requesting invalid item

	if (nCol < 0 || nCol >= GetHeaderCtrl()->GetItemCount())
		return FALSE;	// requesting invalid item

	int nColItemData = GetColumnData(nCol);
	int nRowItemData = (int)GetItemData(nRow);

	if(pNMW->item.mask & LVIF_TEXT)
	{
		// Request text
		CString result;
		if (CallbackCellText(nRow, nCol, result))
		{
			_tcsncpy(pNMW->item.pszText, static_cast<LPCTSTR>(result), pNMW->item.cchTextMax);
		}
	}

	if (pNMW->item.mask & LVIF_IMAGE)
	{
		// Request-Image
		int result = -1;
		if (CallbackCellImage(nRow, nCol, result))
            pNMW->item.iImage = result;
		else
		{
#if (_WIN32_IE >= 0x0501)
			pNMW->item.iImage = I_IMAGENONE;
#else
			pNMW->item.iImage = I_IMAGECALLBACK;
#endif
		}
	}

	if (pNMW->item.mask & LVIF_STATE)
	{
		// Request-selection/Focus state (Virtual-list/LVS_OWNERDATA)
		// Use LVM_SETITEMSTATE to set selection/focus state in LVS_OWNERDATA
	}

	// OBS! Append LVIF_DI_SETITEM to the mask if the item-text/image from now on should be cached in the list (SetItem)
	//	- Besides this bonus option, then don't touch the mask
	return FALSE;
}

bool CGridListCtrlEx::ShowToolTipText(const CPoint& pt) const
{
	return true;
}

bool CGridListCtrlEx::CallbackCellTooltip(int nRow, int nCol, CString& text)
{
	if (nRow!=-1 && nCol!=-1)
	{
		text = GetItemText(nRow, nCol);	// Cell-ToolTip
		return true;
	}
	return false;
}

#if defined(_WIN64)
INT_PTR CGridListCtrlEx::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
#else
int CGridListCtrlEx::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
#endif
{
	if (!ShowToolTipText(point))
		return -1;

	CPoint pt(GetMessagePos());
	ScreenToClient(&pt);

	int nRow, nCol;
	CellHitTest(pt, nRow, nCol);

	//Get the client (area occupied by this control
	RECT rcClient;
	GetClientRect( &rcClient );

	//Fill in the TOOLINFO structure
	pTI->hwnd = m_hWnd;
	pTI->uId = (UINT) (nRow * 1000 + nCol);
	pTI->lpszText = LPSTR_TEXTCALLBACK;	// Send TTN_NEEDTEXT when tooltip should be shown
	pTI->rect = rcClient;

	return pTI->uId; // Must return a unique value for each cell (Marks a new tooltip)
}

BOOL CGridListCtrlEx::OnToolNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt(GetMessagePos());
	ScreenToClient(&pt);

	int nRow, nCol;
	CellHitTest(pt, nRow, nCol);

	// Make const-reference to the returned anonymous CString-object,
	// will keep it alive until reaching scope end
	CString tooltip;
	if (!CallbackCellTooltip(nRow, nCol,tooltip) || tooltip.IsEmpty())
		return FALSE;

	// Non-unicode applications can receive requests for tooltip-text in unicode
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTA->szText));
	else
		_mbstowcsz(pTTTW->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTW->szText));
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTA->szText));
	else
		lstrcpyn(pTTTW->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTW->szText));
#endif
	return TRUE;
}

CWnd* CGridListCtrlEx::EditCell(int nRow, int nCol)
{
	if (nCol==-1 || nRow==-1)
		return NULL;

	CGridColumnTrait* pTrait = GetColumnTrait(nCol);
	CGridColumnTrait::ColumnState& columnState = pTrait->GetColumnState();
	if (!columnState.m_Editable)
		return NULL;

	m_pEditor = pTrait->OnEditBegin(*this, nRow, nCol);
	if (m_pEditor==NULL)
		return NULL;

	m_pEditor = OnTraitEditBegin(pTrait, m_pEditor, nRow, nCol);
	if (m_pEditor==NULL)
		return NULL;

	// Send Notification to parent of ListView ctrl
	LV_DISPINFO dispinfo = {0};
	dispinfo.hdr.hwndFrom = m_hWnd;
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = LVN_BEGINLABELEDIT;

	dispinfo.item.mask = LVIF_PARAM;
	dispinfo.item.iItem = nRow;
	dispinfo.item.iSubItem = nCol;
	dispinfo.item.lParam = GetItemData(nRow);
	if (GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dispinfo)==TRUE)
	{
		// Parent didn't want to start edit
		m_pEditor->PostMessage(WM_CLOSE);
		m_pEditor = NULL;
		return NULL;
	}

	// Show editor
	m_pEditor->ShowWindow(SW_SHOW);
	m_pEditor->SetFocus();
	return m_pEditor;
}

BOOL CGridListCtrlEx::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	// Parent Clistctrl might start a cell-edit on its own (Ignore this)
	if (m_pEditor == NULL)
	{
		*pResult = TRUE;// Block for edit
		return TRUE;	// Block message from reaching parent-dialog
	}

	int nRow = pDispInfo->item.iItem;
	int nCol = pDispInfo->item.iSubItem;

	CGridColumnTrait* pTrait = GetColumnTrait(nCol);
	CGridColumnTrait::ColumnState& columnState = pTrait->GetColumnState();
	if (!columnState.m_Editable)
	{
		*pResult = TRUE;// Block for edit
		return FALSE;	// Allow message to reach parent-dialog
	}

	*pResult = FALSE;// Accept editing
	return FALSE;	// Let parent-dialog get chance
}

BOOL CGridListCtrlEx::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	*pResult = FALSE;	// Reject edit by default

	int nRow = pDispInfo->item.iItem;
	int nCol = pDispInfo->item.iSubItem;

	if((pDispInfo->item.mask & LVIF_TEXT)&&
	   (pDispInfo->item.pszText != NULL)&&
       (nRow != -1) &&
	   (nCol != -1))
    {
		// Label edit completed by user
		CGridColumnTrait* pTrait = GetColumnTrait(nCol);
		if (OnTraitEditComplete(pTrait, m_pEditor, pDispInfo))
		{
			*pResult = TRUE;	// Accept edit

			// Handle situation where data is stored inside the CListCtrl
			if (!IsCellCallback(nRow,nCol))
				SetItemText(nRow, nCol, pDispInfo->item.pszText);
		}
		pTrait->OnEditEnd();
	}

	// Editor Control automatically kills themselves after posting this message
	m_pEditor = NULL;
	SetFocus();
	return FALSE;		// Parent dialog should get a chance
}

void CGridListCtrlEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing

	// Find out what subitem was clicked
	int nRow, nCol;
	CellHitTest(point, nRow, nCol);

	// If not left-clicking on an actual row, then don't update focus cell
	if (nRow==-1)
	{
		CListCtrl::OnLButtonDown(nFlags, point);
		return;
	}

	// Begin edit if the same cell is clicked twice
	bool startEdit = nRow!=-1 && nCol!=-1 && GetFocusRow()==nRow && m_FocusCell==nCol;

	// Update the focused cell before calling CListCtrl::OnLButtonDown()
	// as it might cause a row-repaint
	m_FocusCell = nCol;
	CListCtrl::OnLButtonDown(nFlags, point);

	// CListCtrl::OnLButtonDown() doesn't always cause a row-repaint
	// call our own method to ensure the row is repainted
	UpdateFocusCell(nCol);

	if (startEdit)
	{
		EditCell(nRow, nCol);
	}
}

void CGridListCtrlEx::OnRButtonDown(UINT nFlags, CPoint point)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing

	// Find out what subitem was clicked
	int nRow, nCol;
	CellHitTest(point, nRow, nCol);

	// If not right-clicking on an actual row, then don't update focus cell
	if (nRow!=-1)
	{
		// Update the focused cell before calling CListCtrl::OnRButtonDown()
		// as it might cause a row-repaint
		m_FocusCell = nCol;
	}

	CListCtrl::OnRButtonDown(nFlags, point);
}

void CGridListCtrlEx::OnLButtonDblClk(UINT nFlags, CPoint point)
{
}

//------------------------------------------------------------------------
//! Performs custom drawing of the CListCtrl
//!  - Ensures the CGridColumnTrait's can do their thing
//!  - Ensures that the focus rectangle is properly drawn
//------------------------------------------------------------------------
void CGridListCtrlEx::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*)(pNMHDR);
	int nRow = (int)pLVCD->nmcd.dwItemSpec;

	*pResult = CDRF_DODEFAULT;

	// Allow column-traits to perform their custom drawing
	if (pLVCD->nmcd.dwDrawStage & CDDS_SUBITEM)
	{
		CGridRowTrait* pRowTrait = GetRowTrait(nRow);
		pRowTrait->OnCustomDraw(*this, pLVCD, pResult);
		if (*pResult & CDRF_SKIPDEFAULT)
			return;	// Everything is handled by the row-trait

		int nCol = pLVCD->iSubItem;
		CGridColumnTrait* pTrait = GetColumnTrait(nCol);
		if (!pTrait->GetColumnState().m_Visible)
		{
			*pResult = CDRF_SKIPDEFAULT;
			return;
		}
		pTrait->OnCustomDraw(*this, pLVCD, pResult);
		OnTraitCustomDraw(pTrait, pLVCD, pResult);
		if (*pResult & CDRF_SKIPDEFAULT)
			return;	// Everything is handled by the column-trait
	}

	// Always perform drawing of cell-focus rectangle
	switch (pLVCD->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult |= CDRF_NOTIFYITEMDRAW;
			break;

		// Before painting a row
		case CDDS_ITEMPREPAINT:
		{
			*pResult |= CDRF_NOTIFYPOSTPAINT;	// Ensure row-traits gets called
			*pResult |= CDRF_NOTIFYSUBITEMDRAW;	// Ensure column-traits gets called
			CGridRowTrait* pTrait = GetRowTrait(nRow);
			pTrait->OnCustomDraw(*this, pLVCD, pResult);
		} break;

		// After painting the entire row
		case CDDS_ITEMPOSTPAINT:
		{
			CGridRowTrait* pTrait = GetRowTrait(nRow);
			pTrait->OnCustomDraw(*this, pLVCD, pResult);
		} break;
	}
}

CGridRowTrait* CGridListCtrlEx::GetRowTrait(int nRow)
{
	return m_pDefaultRowTrait;
}

void CGridListCtrlEx::SetDefaultRowTrait(CGridRowTrait* pRowTrait)
{
	ASSERT(pRowTrait!=NULL);
	delete m_pDefaultRowTrait;
	m_pDefaultRowTrait = pRowTrait;
}

bool CGridListCtrlEx::IsColumnVisible(int nCol)
{
	return GetColumnTrait(nCol)->GetColumnState().m_Visible;
}

CGridColumnTrait* CGridListCtrlEx::GetColumnTrait(int nCol)
{
	VERIFY( nCol >=0 && nCol < m_ColumnTraits.GetSize() );
	return m_ColumnTraits[nCol];
}

int CGridListCtrlEx::GetColumnTraitSize() const
{
	return m_ColumnTraits.GetSize();
}

void CGridListCtrlEx::InsertColumnTrait(int nCol, CGridColumnTrait* pTrait)
{
	VERIFY( nCol >=0 && nCol <= m_ColumnTraits.GetSize() );
	if (nCol == m_ColumnTraits.GetSize())
	{
		// Append column to the end of the array
		m_ColumnTraits.Add(pTrait);
	}
	else
	{
		// Insert column in the middle of the array
		CSimpleArray<CGridColumnTrait*> newArray;
		for(int i=0 ; i < m_ColumnTraits.GetSize(); ++i)
		{
			if (i == nCol)
				newArray.Add(pTrait);
			newArray.Add(m_ColumnTraits[i]);
		}
		m_ColumnTraits = newArray;
	}
}

void CGridListCtrlEx::DeleteColumnTrait(int nCol)
{
	delete GetColumnTrait(nCol);
	m_ColumnTraits.RemoveAt(nCol);
}

LRESULT CGridListCtrlEx::OnDeleteColumn(WPARAM wParam, LPARAM lParam)
{
	// Let CListCtrl handle the event
	LRESULT lRet = DefWindowProc(LVM_DELETECOLUMN, wParam, lParam);
	if (lRet == FALSE)
		return FALSE;

	// Book keeping of columns
	DeleteColumnTrait((int)wParam);
	return lRet;
}

LRESULT CGridListCtrlEx::OnInsertColumn(WPARAM wParam, LPARAM lParam)
{
	// Let CListCtrl handle the event
	LRESULT lRet = DefWindowProc(LVM_INSERTCOLUMN, wParam, lParam);
	if (lRet == -1)
		return -1;

	// Book keeping of columns
	if (GetColumnTraitSize() < GetHeaderCtrl()->GetItemCount())
	{
		// Inserted column without providing column traits
		InsertColumnTrait((int)lRet, new CGridColumnTraitText);
	}
	return lRet;
}

void CGridListCtrlEx::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing

	if (point.x==-1 && point.y==-1)
	{
		// OBS! point is initialized to (-1,-1) if using SHIFT+F10 or VK_APPS
		OnContextMenuKeyboard(pWnd, point);
	}
	else
	{
		// CListCtrl::OnRButtonDown() doesn't always cause a row-repaint
		// call our own method to ensure the row is repainted
		UpdateFocusCell(m_FocusCell);

		CPoint pt = point;
		ScreenToClient(&pt);

		CRect headerRect;
		GetHeaderCtrl()->GetClientRect(&headerRect);
		if (headerRect.PtInRect(pt))
		{
			HDHITTESTINFO hdhti = {0};
			hdhti.pt = pt;
			::SendMessage(GetHeaderCtrl()->GetSafeHwnd(), HDM_HITTEST, 0, (LPARAM) &hdhti);
			OnContextMenuHeader(pWnd, point, hdhti.iItem);
		}
		else
		{
			int nRow, nCol;
			CellHitTest(pt, nRow, nCol);
			if (nRow!=-1)
                OnContextMenuCell(pWnd, point, nRow, nCol);
			else
				OnContextMenuGrid(pWnd, point);
		}
	}
}

void CGridListCtrlEx::OnContextMenuKeyboard(CWnd* pWnd, CPoint point)
{
	int nCol = GetFocusCell();
	int nRow = GetFocusRow();

	if (nRow==-1)
	{
		// Place context-menu in the top-left corner of the grid
		CRect gridRect;
		GetClientRect(gridRect);
		ClientToScreen(gridRect);
		OnContextMenuGrid(pWnd, gridRect.TopLeft());
	}
	else
	{
		// Place context-menu over the selected row / cell
		CRect cellRect;
		if (nCol==-1)
			GetItemRect(nRow, cellRect, LVIR_BOUNDS);
		else
			GetCellRect(nRow, nCol, LVIR_BOUNDS, cellRect);
		ClientToScreen(cellRect);

		// Adjust point so context-menu doesn't cover row / cell
		point = cellRect.TopLeft();
		point.x += min(cellRect.Height() / 2, cellRect.Width() / 2);
		point.y += cellRect.Height() / 2;
		OnContextMenuCell(pWnd, point, nRow, nCol);
	}
}

void CGridListCtrlEx::OnContextMenuGrid(CWnd* pWnd, CPoint point)
{
}

void CGridListCtrlEx::OnContextMenuHeader(CWnd* pWnd, CPoint point, int nCol)
{
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

	// Will return zero if no selection was made (TPM_RETURNCMD)
	int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this, 0);
	if (nResult!=0)
	{
		int nCol = nResult-1;
		ShowColumn(nCol, !IsColumnVisible(nCol));
	}
}

void CGridListCtrlEx::OnContextMenuCell(CWnd* pWnd, CPoint point, int nRow, int nCol)
{
}

BOOL CGridListCtrlEx::OnHeaderDividerDblClick(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing

	NMHEADER* pNMH = (NMHEADER*)pNMHDR;
	SetColumnWidthAuto(pNMH->iItem);
	return TRUE;	// Don't let parent handle the event
}

BOOL CGridListCtrlEx::OnHeaderBeginResize(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing

	// Check that column is allowed to be resized
	NMHEADER* pNMH = (NMHEADER*)pNMHDR;
	int nCol = (int)pNMH->iItem;

	CGridColumnTrait* pTrait = GetColumnTrait(nCol);
	CGridColumnTrait::ColumnState& columnState = pTrait->GetColumnState();
	if (!columnState.m_Visible)
	{
		*pResult = TRUE;	// Block resize
		return TRUE;		// Block event
	}

	if (!columnState.m_Resizable)
	{
		*pResult = TRUE;	// Block resize
		return TRUE;		// Block event
	}

	return FALSE;
}

LRESULT CGridListCtrlEx::OnSetColumnWidth(WPARAM wParam, LPARAM lParam)
{
	// Check that column is allowed to be resized
	int nCol = (int)wParam;
	CGridColumnTrait* pTrait = GetColumnTrait(nCol);
	CGridColumnTrait::ColumnState& columnState = pTrait->GetColumnState();

	if (!columnState.m_Visible)
		return FALSE;

	if (!columnState.m_Resizable)
		return FALSE;

	// Let CListCtrl handle the event
	return DefWindowProc(LVM_SETCOLUMNWIDTH, wParam, lParam);
}

BOOL CGridListCtrlEx::OnHeaderBeginDrag(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing
	return FALSE;
}

BOOL CGridListCtrlEx::OnHeaderEndDrag(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	NMHEADER* pNMH = (NMHEADER*)pNMHDR;
	if (pNMH->pitem->mask & HDI_ORDER)
	{
		// Correct iOrder so it is just after the last hidden column
		int nColCount = GetHeaderCtrl()->GetItemCount();
		int* pOrderArray = new int[nColCount];
		VERIFY( GetColumnOrderArray(pOrderArray, nColCount) );

		for(int i = 0; i < nColCount ; ++i)
		{
			if (IsColumnVisible(pOrderArray[i]))
			{
                pNMH->pitem->iOrder = max(pNMH->pitem->iOrder,i);
				break;
			}
		}
		delete [] pOrderArray;
	}
	return FALSE;
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
}

// Define does not exist on VC6
#ifndef ListView_SortItemsEx
	#ifndef LVM_SORTITEMSEX
		#define LVM_SORTITEMSEX          (LVM_FIRST + 81)
	#endif
	#define ListView_SortItemsEx(hwndLV, _pfnCompare, _lPrm) \
	(BOOL)SNDMSG((hwndLV), LVM_SORTITEMSEX, (WPARAM)(LPARAM)(_lPrm), (LPARAM)(PFNLVCOMPARE)(_pfnCompare))
#endif

bool CGridListCtrlEx::SortColumn(int columnIndex, bool ascending)
{
	// virtual lists cannot be sorted with this method
	if (GetStyle() & LVS_OWNERDATA)
		return false;

	if (GetItemCount()<=0)
		return true;

	// Uses SortItemsEx because it requires no knowledge of datamodel
	//	- CListCtrl::SortItems() is faster with direct access to the datamodel
	PARAMSORT paramsort(m_hWnd, columnIndex, ascending);
	ListView_SortItemsEx(m_hWnd, SortFunc, &paramsort);
	return true;
}

BOOL CGridListCtrlEx::OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pLV = reinterpret_cast<NMLISTVIEW*>(pNMHDR);

	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing

	int nCol = pLV->iSubItem;
	CGridColumnTrait* pTrait = GetColumnTrait(nCol);
	CGridColumnTrait::ColumnState& columnState = pTrait->GetColumnState();
	if (!columnState.m_Sortable)
		return FALSE;	// Let parent-dialog get change

	if (m_SortCol==nCol)
	{
		m_Ascending = !m_Ascending;
	}
	else
	{
		m_SortCol = nCol;
		m_Ascending = true;
	}

	if (SortColumn(m_SortCol, m_Ascending))
		SetSortArrow(m_SortCol, m_Ascending);

	return FALSE;	// Let parent-dialog get chance
}

void CGridListCtrlEx::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing
	 CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CGridListCtrlEx::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing
	 CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CGridListCtrlEx::SetEmptyMarkupText(const CString& text)
{
	m_EmptyMarkupText = text;
}

void CGridListCtrlEx::OnPaint()
{
	if (GetItemCount()==0 && !m_EmptyMarkupText.IsEmpty())
	{
		// Show text string when list is empty
		CPaintDC dc(this);

		int nSavedDC = dc.SaveDC();

		//Set up variables
		COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);	//system text color
		COLORREF clrBack = GetBkColor();
		CRect rc;
		GetClientRect(&rc);	//get client area of the ListCtrl

        //Now we actually display the text
        dc.SetTextColor(clrText);	//set the text color
        dc.SetBkColor(clrBack);	//set the background color
		CBrush backBrush(clrBack);
		dc.FillRect(rc, &backBrush);	//fill the client area rect
		CFont* pOldFont = dc.SelectObject(GetCellFont());	//select a font
		dc.DrawText(m_EmptyMarkupText, -1, rc, 
                      DT_CENTER | DT_WORDBREAK | DT_NOPREFIX |
					  DT_NOCLIP | DT_VCENTER | DT_SINGLELINE); //and draw the text
		dc.SelectObject(pOldFont);

        // Restore dc
		dc.RestoreDC(nSavedDC);
	}
	else
	{
		CListCtrl::OnPaint();	// default
	}
}
