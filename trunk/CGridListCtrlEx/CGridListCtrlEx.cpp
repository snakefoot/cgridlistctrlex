#include "stdafx.h"
#include "CGridListCtrlEx.h"

#include <shlwapi.h>	// IsThemeEnabled
#include <afxole.h>		// 

#pragma warning(disable:4100)	// unreferenced formal parameter

#include "CGridColumnManager.h"
#include "CGridColumnTraitText.h"
#include "CGridRowTraitText.h"

//------------------------------------------------------------------------
//! @cond INTERNAL
//------------------------------------------------------------------------
template<class T>
class COleDropTargetWnd : public COleDropTarget
{
		T* m_pWnd;
		bool m_DragSource;
		bool m_DragDestination;

	public:
		COleDropTargetWnd()
			:m_pWnd(NULL)
			,m_DragSource(false)
			,m_DragDestination(false)
		{}

		BOOL Register(T* pWnd)
		{
			if (m_pWnd!=NULL)
			{
				ASSERT(m_pWnd==pWnd);
				return TRUE;	// Already registered
			}

			if (COleDropTarget::Register(pWnd)==FALSE)
				return FALSE;

			m_pWnd = pWnd;
			return TRUE;
		}

		bool IsDragSource() const { return m_DragSource; }
		bool IsDragDestination() const { return m_DragDestination; }
		void SetDragSource(bool value) { m_DragSource = value; m_DragDestination = false; }

		virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
		{
			ASSERT(m_pWnd==pWnd && m_pWnd!=NULL);
			m_DragDestination = true;
			return m_pWnd->OnDragEnter(pDataObject, dwKeyState, point);
		}

		virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
		{
			ASSERT(m_pWnd==pWnd && m_pWnd!=NULL);
			return m_pWnd->OnDragOver(pDataObject, dwKeyState, point);
		}

		virtual void OnDragLeave(CWnd* pWnd)
		{
			ASSERT(m_pWnd==pWnd && m_pWnd!=NULL);
			m_DragDestination = false;
			m_pWnd->OnDragLeave();
		}

		virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
		{
			ASSERT(m_pWnd==pWnd && m_pWnd!=NULL);
			m_DragDestination = false;
			return m_pWnd->OnDrop(pDataObject, dropEffect, point);
		}
};

template<class T>
class COleDataSourceWnd : public COleDataSource
{
	COleDropTargetWnd<T>* m_pTarget;

public:
	COleDataSourceWnd(COleDropTargetWnd<T>* pTarget)
		:m_pTarget(pTarget)
	{
		if (m_pTarget!=NULL)
			m_pTarget->SetDragSource(true);
	}

	~COleDataSourceWnd()
	{
		if (m_pTarget!=NULL)
			m_pTarget->SetDragSource(false);
	}
};
//------------------------------------------------------------------------
//! @endcond INTERNAL
//------------------------------------------------------------------------

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
	ON_NOTIFY_EX(HDN_ENDTRACKA, 0, OnHeaderEndResize)
	ON_NOTIFY_EX(HDN_ENDTRACKW, 0, OnHeaderEndResize)
	ON_NOTIFY_EX(HDN_BEGINDRAG, 0, OnHeaderBeginDrag)
	ON_NOTIFY_EX(HDN_ENDDRAG, 0, OnHeaderEndDrag)
	ON_NOTIFY_EX(HDN_DIVIDERDBLCLICKA, 0, OnHeaderDividerDblClick)
	ON_NOTIFY_EX(HDN_DIVIDERDBLCLICKW, 0, OnHeaderDividerDblClick)
	ON_NOTIFY_EX(TTN_NEEDTEXTA, 0, OnToolNeedText)
	ON_NOTIFY_EX(TTN_NEEDTEXTW, 0, OnToolNeedText)
	ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnHeaderClick)	// Column Click
	ON_NOTIFY_REFLECT_EX(NM_CLICK, OnItemClick)				// Cell Click
	ON_NOTIFY_REFLECT_EX(NM_DBLCLK, OnItemDblClick)			// Cell Double Click
	ON_NOTIFY_REFLECT_EX(LVN_ODFINDITEM, OnOwnerDataFindItem)	// Owner Data Find Item
	ON_NOTIFY_REFLECT_EX(LVN_BEGINDRAG, OnBeginDrag)		// Begin Drag Dropb
	ON_WM_CONTEXTMENU()	// OnContextMenu
	ON_WM_KEYDOWN()		// OnKeyDown
	ON_WM_LBUTTONDOWN()	// OnLButtonDown(UINT nFlags, CPoint point)
	ON_WM_RBUTTONDOWN()	// OnRButtonDown(UINT nFlags, CPoint point)
	ON_WM_HSCROLL()		// OnHScroll
	ON_WM_VSCROLL()		// OnVScroll
	ON_WM_CHAR()		// OnChar (Keyboard search)
	ON_WM_PAINT()		// OnPaint
	ON_WM_CREATE()		// OnCreate
	ON_WM_KILLFOCUS()	// OnKillFocus
	ON_MESSAGE(WM_COPY, OnCopy)	// Clipboard
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------------------------------
//! Constructor 
//------------------------------------------------------------------------
CGridListCtrlEx::CGridListCtrlEx()
	:m_FocusCell(-1)
	,m_SortCol(-1)
	,m_Ascending(false)
	,m_UsingVisualStyle(false)
	,m_pEditor(NULL)
	,m_LastSearchCell(-1)
	,m_LastSearchRow(-1)
	,m_RepeatSearchCount(-1)
	,m_EmptyMarkupText(_T("There are no items to show in this view."))
	,m_pOleDropTarget(NULL)
	,m_Margin(1)		// Higher row-height (more room for edit-ctrl border)
	,m_pDefaultRowTrait(new CGridRowTraitText)
	,m_pColumnManager(new CGridColumnManager)
{}

//------------------------------------------------------------------------
//! Destructor
//------------------------------------------------------------------------
CGridListCtrlEx::~CGridListCtrlEx()
{
	for(int nCol = GetColumnTraitSize()-1; nCol >= 0 ; --nCol)
		DeleteColumnTrait(nCol);

	delete m_pDefaultRowTrait;
	m_pDefaultRowTrait = NULL;

	delete m_pColumnManager;
	m_pColumnManager = NULL;

	delete m_pOleDropTarget;
	m_pOleDropTarget = NULL;
}

//------------------------------------------------------------------------
//! Sets the interface for handling column state persistence for the list control
//! 
//! @param pColumnManager The new column state interface handler
//------------------------------------------------------------------------
void CGridListCtrlEx::SetupColumnConfig(CGridColumnManager* pColumnManager)
{
	ASSERT(pColumnManager!=NULL);
	delete m_pColumnManager;
	m_pColumnManager = pColumnManager;
	m_pColumnManager->OnColumnSetup(*this);
}

//------------------------------------------------------------------------
//! Checks if the current OS version against the requested OS version
//!
//! @param requestOS The full version number of the OS required (Ex 0x0600)
//------------------------------------------------------------------------
bool CGridListCtrlEx::CheckOSVersion(WORD requestOS)
{
	static WORD fullver = 0;
	if (fullver==0)
	{
		OSVERSIONINFO osver = {0};
		osver.dwOSVersionInfoSize = sizeof(osver);
		GetVersionEx(&osver);
		 fullver = MAKEWORD(osver.dwMinorVersion, osver.dwMajorVersion);
	}
	return requestOS <= fullver;
}

namespace {
	bool IsCommonControlsEnabled()
	{
		bool commoncontrols = false;
	
		// Test if application has access to common controls
		HMODULE hinstDll = ::LoadLibrary(_T("comctl32.dll"));
		if (hinstDll)
		{
			DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hinstDll, "DllGetVersion");
			if (pDllGetVersion != NULL)
			{
				DLLVERSIONINFO dvi = {0};
				dvi.cbSize = sizeof(dvi);
				HRESULT hRes = pDllGetVersion ((DLLVERSIONINFO *) &dvi);
				if (SUCCEEDED(hRes))
					commoncontrols = dvi.dwMajorVersion >= 6;
			}
			::FreeLibrary(hinstDll);
		}
		return commoncontrols;
	}

	bool IsThemeEnabled()
	{
		bool XPStyle = false;
		bool (__stdcall *pIsAppThemed)();
		bool (__stdcall *pIsThemeActive)();

		// Test if operating system has themes enabled
		HMODULE hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			(FARPROC&)pIsAppThemed = ::GetProcAddress(hinstDll, "IsAppThemed");
			(FARPROC&)pIsThemeActive = ::GetProcAddress(hinstDll,"IsThemeActive");
			if (pIsAppThemed != NULL && pIsThemeActive != NULL)
			{
				if (pIsAppThemed() && pIsThemeActive())
				{
					// Test if application has themes enabled by loading the proper DLL
					XPStyle = IsCommonControlsEnabled();
				}
			}
			::FreeLibrary(hinstDll);
		}
		return XPStyle;
	}

	LRESULT EnableWindowTheme(HWND hwnd, LPCWSTR classList, LPCWSTR subApp, LPCWSTR idlist)
	{
		LRESULT lResult = S_FALSE;
	
		HRESULT (__stdcall *pSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
		HANDLE (__stdcall *pOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
		HRESULT (__stdcall *pCloseThemeData)(HANDLE hTheme);

		HMODULE hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			(FARPROC&)pOpenThemeData = ::GetProcAddress(hinstDll, "OpenThemeData");
			(FARPROC&)pCloseThemeData = ::GetProcAddress(hinstDll, "CloseThemeData");
			(FARPROC&)pSetWindowTheme = ::GetProcAddress(hinstDll, "SetWindowTheme");
			if (pSetWindowTheme && pOpenThemeData && pCloseThemeData)
			{
				HANDLE theme = pOpenThemeData(hwnd,classList);
				if (theme!=NULL)
				{
					VERIFY(pCloseThemeData(theme)==S_OK);
					lResult = pSetWindowTheme(hwnd, subApp, idlist);
				}
			}
			::FreeLibrary(hinstDll);
		}
		return lResult;
	}
}

//------------------------------------------------------------------------
//! Activate visual style for the list control (Vista Theme)
//! 
//! @param bValue Specifies whether the visual styles should be enabled or not
//! @return S_FALSE if visual styles could not be enabled
//------------------------------------------------------------------------
LRESULT CGridListCtrlEx::EnableVisualStyles(bool bValue)
{
	if (!IsThemeEnabled())
	{
		m_UsingVisualStyle = false;
		return S_FALSE;
	}

	if (!CheckOSVersion(0x0600))
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
		if (CheckOSVersion(0x501))
			SetExtendedStyle(LVS_EX_DOUBLEBUFFER | GetExtendedStyle());
#endif
	}
	else
	{
		m_UsingVisualStyle = false;
	}

	return rc;
}

//------------------------------------------------------------------------
//! Configures the initial style of the list control when the it is created
//------------------------------------------------------------------------
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
	if (CheckOSVersion(0x501))
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

	RegisterDropTarget();
}

//------------------------------------------------------------------------
//! WM_CREATE message handler. Called when inside a CView.
//!
//! @param lpCreateStruct Pointer to a CREATESTRUCT structure that contains information about the list control object being created. 
//------------------------------------------------------------------------
int CGridListCtrlEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Will not be called when placed inside a CDialog
	int rc = CListCtrl::OnCreate(lpCreateStruct);
	if (rc==0)
		OnCreateStyle();

	return rc;
}

//------------------------------------------------------------------------
//! Normally used for subclassing controls, but here used to configure
//! initial style when list control is created.
//------------------------------------------------------------------------
void CGridListCtrlEx::PreSubclassWindow()
{
	// Changes made to style will not having any effect when placed in a CView
	CListCtrl::PreSubclassWindow();
	OnCreateStyle();
}


//------------------------------------------------------------------------
//! Inserts a new column in the list control, and gives the option to customize the column using a trait
//!  
//! @param nCol Index of the new column
//! @param strColumnHeading Title of the new column
//! @param nFormat Text alignment of the new column
//! @param nWidth Width of the new column
//! @param nSubItem Unique identifier used to recognize the column independent of index
//! @param pTrait Column trait interface for the new column
//! @return The index of the new column if successful or -1 otherwise.
//------------------------------------------------------------------------
int CGridListCtrlEx::InsertColumnTrait(int nCol, const CString& strColumnHeading, int nFormat, int nWidth, int nSubItem, CGridColumnTrait* pTrait)
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

	int index = InsertColumn(nCol, static_cast<LPCTSTR>(strColumnHeading), nFormat, nWidth, nSubItem);
	if (index != -1)
	{
		VERIFY( index == nCol );
		GetColumnTrait(nCol)->OnInsertColumn(*this, nCol);
	}
	else
		DeleteColumnTrait(nCol);
	return index;
}

//------------------------------------------------------------------------
//! Inserts the label column (first column) with invisible state
//! The label column behaves differently from the rest of the columns,
//! and to get the uniform look, then it should be hidden away.
//!	- It has a special margin, which no other column has
//!	- When dragged to another position than the first, then it looses it special margin
//!
//! @return The index of the new column if successful or -1 otherwise.
//------------------------------------------------------------------------
int CGridListCtrlEx::InsertHiddenLabelColumn()
{
	// Must be the label column
	VERIFY(GetHeaderCtrl()->GetItemCount()==0);

	CGridColumnTrait* pColumTrait = new CGridColumnTrait;
	pColumTrait->GetColumnState().m_AlwaysHidden = true;
	return InsertColumnTrait(0, _T(""), LVCFMT_LEFT, -1, -1, pColumTrait);
}

//------------------------------------------------------------------------
//! Retrieves the header control of a list control.
//!
//! @return A pointer to the header control, used by the list control.
//------------------------------------------------------------------------
const CHeaderCtrl* CGridListCtrlEx::GetHeaderCtrl() const
{
	ASSERT(::IsWindow(m_hWnd));

	HWND hWnd = (HWND) ::SendMessage(m_hWnd, LVM_GETHEADER, 0, 0);
	if (hWnd == NULL)
		return NULL;
	else
		return (const CHeaderCtrl*) CHeaderCtrl::FromHandle(hWnd);
}

//------------------------------------------------------------------------
//! Retrieves the number of columns from the header control.
//!
//! @return Number of header control items if successful; otherwise � 1.
//------------------------------------------------------------------------
int CGridListCtrlEx::GetColumnCount() const
{
	return GetHeaderCtrl()->GetItemCount();
}

//------------------------------------------------------------------------
//! Retrieves the font used to draw cells in the list control
//!
//! @return A pointer to the current font used by the list control.
//------------------------------------------------------------------------
CFont* CGridListCtrlEx::GetCellFont()
{
	if (m_CellFont.GetSafeHandle()==NULL)
		return GetFont();
	return &m_CellFont;
}

//------------------------------------------------------------------------
//! Takes the current font and increases the font with the given margin
//! multiplier. Increases the row-height but keeps the cell font intact.
//! Gives more room for the grid-cell editors and their border.
//!
//! @param margin Multiplier for how much to increase the font size
//------------------------------------------------------------------------
void CGridListCtrlEx::SetCellMargin(double margin)
{
	m_Margin = margin;

	LOGFONT lf = {0};
	VERIFY(GetFont()->GetLogFont(&lf)!=0);
	VERIFY( m_CellFont.CreateFontIndirect(&lf) );

	lf.lfHeight = (int)(lf.lfHeight * m_Margin);
	lf.lfWidth = (int)(lf.lfWidth * m_Margin);
	VERIFY( m_GridFont.CreateFontIndirect(&lf) );

	CListCtrl::SetFont(&m_GridFont);
	GetHeaderCtrl()->SetFont(&m_CellFont);
	CToolTipCtrl* pToolTipCtrl = (CToolTipCtrl*)CWnd::FromHandle((HWND)::SendMessage(m_hWnd, LVM_GETTOOLTIPS, 0, 0L));
	if (pToolTipCtrl!=NULL && pToolTipCtrl->m_hWnd!=NULL)
		pToolTipCtrl->SetFont(&m_CellFont);
}

//------------------------------------------------------------------------
//! The column version of GetItemData(), one can specify an unique
//! identifier when using InsertColumn()
//!
//! @param nCol Index of the column
//! @return Unique identifier of the column specified
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
//!
//! @param nCol Index of the column
//! @return Column offset is in left-to-right order. For example, zero indicates the leftmost column.
//------------------------------------------------------------------------
int CGridListCtrlEx::GetColumnOrder(int nCol) const
{
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_ORDER;
	VERIFY( GetColumn(nCol, &lvc) );
	return lvc.iOrder;
}

//------------------------------------------------------------------------
//! Retrieve column title of a column in the list control 
//!
//! @param nCol Index of the column
//! @return Column header text of the specified column
//------------------------------------------------------------------------
CString CGridListCtrlEx::GetColumnHeading(int nCol) const
{
	// Retrieve column-title
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_TEXT;
	TCHAR sColText[256];
	lvc.pszText = sColText;
	lvc.cchTextMax = sizeof(sColText)-1;
	VERIFY( GetColumn(nCol, &lvc) );
	return lvc.pszText;
}

//------------------------------------------------------------------------
//! Retrieve row with the LVIS_FOCUSED state flag set
//!
//! @return The index of the row if successful, or -1 otherwise.
//------------------------------------------------------------------------
int CGridListCtrlEx::GetFocusRow() const
{
	return GetNextItem(-1, LVNI_FOCUSED);
}

//------------------------------------------------------------------------
//! Sets LVIS_FOCUSED state flag for the specified row
//!
//! @param nRow The index of the row
//------------------------------------------------------------------------
void  CGridListCtrlEx::SetFocusRow(int nRow)
{
	SetItemState(nRow, LVIS_FOCUSED, LVIS_FOCUSED);
}

//------------------------------------------------------------------------
//! Checks if the LVIS_SELECTED state flag set for the specified row
//!
//! @param nRow The index of the row
//! @return True if the row is selected
//------------------------------------------------------------------------
bool CGridListCtrlEx::IsRowSelected(int nRow) const
{
	return (GetItemState(nRow, LVIS_SELECTED) & LVIS_SELECTED) == LVIS_SELECTED;
}

//------------------------------------------------------------------------
//! Sets the LVIS_SELECTED state flag for the specified row
//!
//! @param nRow The index of the row. -1 means all rows
//! @param bSelect Whether row should be selected or not
//! @return Nonzero if successful; otherwise zero.
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::SelectRow(int nRow, bool bSelect)
{
	return SetItemState(nRow, bSelect ? LVIS_SELECTED : 0, LVIS_SELECTED);
}

//------------------------------------------------------------------------
//! Improved version of GetSubItemRect().
//!	- It doesn't return entire row-rect when using LVIR_BOUNDS for label-column (nCol==0)
//!	- It supports LVIR_LABEL for sub-items (nCol>0)
//!	- It supports LVIR_BOUNDS when column width is less than subitem image width
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param nCode Determines the portion of the bounding rectangle (of the list view subitem) to be retrieved.
//!	@param rect Reference to a CRect object that contains the coordinates of the cell's bounding rectangle.
//! @return Nonzero if successful; otherwise zero.
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

	if (nCode == LVIR_ICON)
	{
		if (nCol > 0 && !(GetExtendedStyle() & LVS_EX_SUBITEMIMAGES))
			return FALSE;	// no image in subitem

		int nImage = GetCellImage(nRow, nCol);
		if (nImage == I_IMAGECALLBACK)
			return FALSE;	// no image

		return TRUE;
	}

	if (nCode == LVIR_LABEL && nCol>0)
	{
		if (!(GetExtendedStyle() & LVS_EX_SUBITEMIMAGES))
			return TRUE;	// no image in subitem

		int nImage = GetCellImage(nRow, nCol);
		if (nImage == I_IMAGECALLBACK)
			return TRUE;	// No image in subitem

		CRect iconRect;
		if (GetSubItemRect(nRow, nCol, LVIR_ICON, iconRect)==FALSE)
			return FALSE;

		rect.left += iconRect.Width();
	}

	return TRUE;
}

//------------------------------------------------------------------------
//! Replicates the SubItemHitTest() but in a const version. Finds the cell
//! below the given mouse cursor position.
//!
//! @param pt The position to hit test, in client coordinates. 
//! @param nRow The index of the row (Returns -1 if no row)
//! @param nCol The index of the column (Returns -1 if no column)
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
//! Checks if the current cell is using callback to retrieve its text value
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @return Returns true if the cell is using call back to retrieve its text value
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
//! Retrieves the icon index of the specified cell
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @return Index of the cell's icon in the control's image list (I_IMAGECALLBACK means no image)
//------------------------------------------------------------------------
int CGridListCtrlEx::GetCellImage(int nRow, int nCol) const
{
	LV_ITEM lvi = {0};
	lvi.iItem = nRow;
	lvi.iSubItem = nCol;
	lvi.mask = LVIF_IMAGE;
	VERIFY( GetItem( &lvi ) );
	return lvi.iImage;
}

//------------------------------------------------------------------------
//! Sets the icon of the specified cell
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param nImageId The icon index in the list control image list
//! @return Nonzero if successful; otherwise zero.
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::SetCellImage(int nRow, int nCol, int nImageId)
{
	LV_ITEM lvitem = {0};
	lvitem.mask = LVIF_IMAGE;
	lvitem.iItem = nRow;
	lvitem.iSubItem = nCol;
	lvitem.iImage = nImageId;	// I_IMAGENONE (Indent but no image), I_IMAGECALLBACK
	return SetItem(&lvitem);
}

//------------------------------------------------------------------------
//! Changes the focus cell.
//! Override this method and set m_FocusCell = -1 if wanting to disable subitem focus
//!
//! @param nCol The index of the column
//! @param bRedraw Should the focus row be redrawn ? (true / false)
//------------------------------------------------------------------------
void CGridListCtrlEx::SetFocusCell(int nCol, bool bRedraw)
{
	m_FocusCell = nCol;
	if (bRedraw)
	{
		int nFocusRow = GetFocusRow();
		if (nFocusRow >= 0)
		{
			CRect itemRect;
			VERIFY( GetItemRect(nFocusRow, itemRect, LVIR_BOUNDS) );
			InvalidateRect(itemRect);
			UpdateWindow();
		}
	}
}

//------------------------------------------------------------------------
//! Shifts the cell focus left or right in the same row
//!
//! @param bMoveRight Specifies whether the cell focus should be left or right
//------------------------------------------------------------------------
void CGridListCtrlEx::MoveFocusCell(bool bMoveRight)
{
	if (GetItemCount()<=0)
	{
		SetFocusCell(-1);	// Entire row selected
		return;
	}

	if (GetFocusCell() == -1)
	{
		// Entire row already selected
		if (bMoveRight)
		{
			// Change to the first column in the current order
			SetFocusCell( GetFirstVisibleColumn() );
		}
	}
	else
	{
		// Convert focus-cell to order index
		int nOrderIndex = -1;
		for(int i = 0; i < GetHeaderCtrl()->GetItemCount(); ++i)
		{
			int nCol = GetHeaderCtrl()->OrderToIndex(i);
			if (nCol == GetFocusCell())
			{
				nOrderIndex = i;
				break;
			}
		}

		// Move to the following column
		if (bMoveRight)
			nOrderIndex++;
		else
			nOrderIndex--;

		// Convert order-index to focus cell
		if (nOrderIndex >= 0 && nOrderIndex < GetHeaderCtrl()->GetItemCount())
		{
			int nCol = GetHeaderCtrl()->OrderToIndex(nOrderIndex);
			if (IsColumnVisible(nCol))
				SetFocusCell(nCol);
			else
			if (!bMoveRight)
				SetFocusCell(-1);	// Entire row selection
		}
		else if (!bMoveRight)
			SetFocusCell(-1);	// Entire row selection
	}

	// Ensure the column is visible
	if (GetFocusCell() >= 0)
	{
		VERIFY( EnsureColumnVisible(GetFocusCell(), false) );
	}

	SetFocusCell(GetFocusCell(), true);
}


//------------------------------------------------------------------------
//! Scrolls the view, so the column becomes visible
//!
//! http://www.codeguru.com/cpp/controls/listview/columns/article.php/c931/
//!
//! @param nCol The index of the column
//! @param bPartialOK Is partially visible good enough ?
//! @return Nonzero if successful; otherwise zero.
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

//------------------------------------------------------------------------
//! Retrieves the column index of the first visible column
//!
//! @return Column index of the first visible column (-1 if no visible columns)
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
//! Changes the visible state of column.
//! Hides a column by resizing the column width to zero and moving it to
//! the outer left in the column order. Shows a column by returning it to
//! its original position.
//!
//! @param nCol The index of the column
//! @param bShow Specifies whether the column should be shown or hidden
//! @return Nonzero if successful; otherwise zero.
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::ShowColumn(int nCol, bool bShow)
{
	if (!bShow && IsColumnAlwaysVisible(nCol))
		return FALSE;

	if (bShow && IsColumnAlwaysHidden(nCol))
		return FALSE;

	CGridColumnTrait* pTrait = GetColumnTrait(nCol);
	CGridColumnTrait::ColumnState& columnState = pTrait->GetColumnState();

	SetRedraw(FALSE);

	int nColCount = GetColumnCount();
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
	m_pColumnManager->OnColumnPick(*this);
	SetRedraw(TRUE);
	Invalidate(FALSE);
	return TRUE;
}

//------------------------------------------------------------------------
//! Resizes the width of a column according the contents of the cells below
//!
//! @param nCol The index of the column
//! @param bIncludeHeader Include the column header text the column width calculation
//! @return Nonzero if successful; otherwise zero.
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::SetColumnWidthAuto(int nCol, bool bIncludeHeader)
{
	if (nCol == -1)
	{
		for(int i = 0; i < GetHeaderCtrl()->GetItemCount() ; ++i)
		{
			SetColumnWidthAuto(i, bIncludeHeader);
		}
		return TRUE;
	}
	else
	{
		if (bIncludeHeader)
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
			// Arrow pointing up
			CPoint Pt[3];
			Pt[0] = CPoint(7,  5);	// Top
			Pt[1] = CPoint(4,  8);	// Left
			Pt[2] = CPoint(10,  8);	// Right
			memDC.Polygon(Pt, 3);
		}
		else
		{
			// Arrow pointing down
			CPoint Pt[3];
			Pt[0] = CPoint(10, 6);	// Right
			Pt[1] = CPoint(4, 6);	// Left
			Pt[2] = CPoint(7, 9);	// Bottom
			memDC.Polygon(Pt, 3);
		}
		memDC.SelectObject(pOldBrush);

		//select old bitmap back into the memory dc
		memDC.SelectObject(pOldBitmap);

		return (HBITMAP)newbmp.Detach();
	}
}

//------------------------------------------------------------------------
//! Puts a sort-icon in the column header of the specified column
//!
//! @param nCol The index of the column
//! @param bAscending Should the arrow be up or down 
//------------------------------------------------------------------------
void CGridListCtrlEx::SetSortArrow(int nCol, bool bAscending)
{
	if (IsThemeEnabled())
	{
#if (_WIN32_WINNT >= 0x501)
		TRACE(_T("theme enabled\n"));
		for(int i = 0; i < GetHeaderCtrl()->GetItemCount(); ++i)
		{
			HDITEM hditem = {0};
			hditem.mask = HDI_FORMAT;
			VERIFY( GetHeaderCtrl()->GetItem( i, &hditem ) );
			hditem.fmt &= ~(HDF_SORTDOWN|HDF_SORTUP);
			if (i == nCol)
			{
				hditem.fmt |= bAscending ? HDF_SORTUP : HDF_SORTDOWN;
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
			if (i == nCol)
			{
				hditem.fmt |= HDF_BITMAP|HDF_BITMAP_ON_RIGHT;
				//UINT bitmapID = m_Ascending ? IDB_DOWNARROW : IDB_UPARROW; 
				//hditem.hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmapID), IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS);
				hditem.hbm = CreateSortBitmap(bAscending);
				VERIFY( hditem.hbm!=NULL );
				VERIFY( CListCtrl::GetHeaderCtrl()->SetItem( i, &hditem ) );
			}
		}
	}
}

//------------------------------------------------------------------------
//! WM_KEYDOWN message handler for performing keyboard navigation
//!
//! @param nChar Specifies the virtual key code of the given key.
//! @param nRepCnt Repeat count (the number of times the keystroke is repeated as a result of the user holding down the key).
//! @param nFlags Specifies the scan code, key-transition code, previous key state, and context code
//------------------------------------------------------------------------
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
		} break;
		case 0x43:		// CTRL+C (Copy to clipboard)
		{
			if (GetKeyState(VK_CONTROL) < 0)
			{
				OnCopyToClipboard();
			}
		} break;
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
			CRect rect;
			VERIFY( GetCellRect(GetFocusRow(), GetFocusCell(), LVIR_LABEL, rect) );
			EditCell(GetFocusRow(), GetFocusCell(), rect.TopLeft());
		} break;
		case VK_SPACE:
		{
			if (GetExtendedStyle() & LVS_EX_CHECKBOXES)
			{
				// Toggle checkbox for virtual list with checkbox style
				if (GetStyle() & LVS_OWNERDATA)
				{
					int nFocusRow = GetFocusRow();
					if (nFocusRow != -1)
						OnOwnerDataToggleCheckBox(nFocusRow);
				}
			}
			else
			{
				// Flip-cell image icon
				if (GetFocusRow()!=-1 && GetFocusCell()!=-1)
				{
					CRect rect;
					if (GetCellRect(GetFocusRow(), GetFocusCell(), LVIR_ICON, rect))
						EditCell(GetFocusRow(), GetFocusCell(), rect.TopLeft());
				}
			}
		} break;
	}
	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

//------------------------------------------------------------------------
//! Override this method to display checkbox state for LVS_OWNERDATA (virtual list).
//!
//! @param nRow The row index being displayed
//! @return Is row checked ?
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnOwnerDataDisplayCheckbox(int nRow)
{
	return false;
}

//------------------------------------------------------------------------
//! Override this method to react to check box being toggled for LVS_OWNERDATA (virtual list).
//! Remember to force a redraw to ensure the new checkbox state is displayed.
//!
//! @param nRow The row index where the checkbox should be toggled
//------------------------------------------------------------------------
void CGridListCtrlEx::OnOwnerDataToggleCheckBox(int nRow)
{
	// Force redraw so the new checkbox value is displayed
	Invalidate();
	UpdateWindow();
}

//------------------------------------------------------------------------
//! Override this method to optimize the keyboard search for LVS_OWNERDATA (virtual list)
//!
//! @param nCol Column where the search is performed
//! @param nStartRow Row index at which the search will start
//! @param strSearch String to search for
//! @return Row index matching the specified search string
//------------------------------------------------------------------------
int CGridListCtrlEx::OnKeyboardSearch(int nCol, int nStartRow, const CString& strSearch)
{
	int nRowCount = GetItemCount();

	// Perform the search loop twice
	//	- First search from current position down to bottom
	//	- Then search from top to current position
	for(int j = 0; j < 2; ++j)
	{
		for(int i = nStartRow + 1; i < nRowCount; ++i)
		{
			CString cellText = GetItemText(i, nCol);
			if (cellText.GetLength()>=strSearch.GetLength())
			{
				cellText = cellText.Left(strSearch.GetLength());
				if (cellText.CompareNoCase(strSearch)==0)
					return i;
			}
		}
		nRowCount = nStartRow;
		nStartRow = -1;
	}

	return -1;
}

//------------------------------------------------------------------------
//! LVN_ODFINDITEM message handler for performing keyboard search when
//! LVS_OWNERDATA (virtual list)
//!
//! @param pNMHDR Pointer to NMLVFINDITEM structure
//! @param pResult Set to the row-index matching the keyboard search (-1 for no match)
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnOwnerDataFindItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVFINDITEM* pFindItem = reinterpret_cast<NMLVFINDITEM*>(pNMHDR);

	*pResult = -1;

	if (pFindItem->lvfi.flags & LVFI_STRING)
	{
		*pResult = OnKeyboardSearch(0, pFindItem->iStart-1, pFindItem->lvfi.psz);
		return TRUE;
	}

	return FALSE;	// Allow parent to handle message
}

//------------------------------------------------------------------------
//! WM_CHAR message handler for performing keyboard search with subitems
//!
//! @param nChar Specifies the virtual key code of the given key.
//! @param nRepCnt Repeat count (the number of times the keystroke is repeated as a result of the user holding down the key).
//! @param nFlags Specifies the scan code, key-transition code, previous key state, and context code
//------------------------------------------------------------------------
void CGridListCtrlEx::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (GetFocusCell()<=0)
	{
		// Use the default keyboard search in the label-column
		CListCtrl::OnChar(nChar, nRepCnt, nFlags);
		return;
	}

	if (GetKeyState(VK_CONTROL) < 0)
	{
		m_LastSearchString = _T("");
		m_RepeatSearchCount = -1;
		return;
	}

	// No input within 2 seconds, resets the search
	if (m_LastSearchTime.GetCurrentTime() >= (m_LastSearchTime+2)
	 && m_LastSearchString.GetLength()>0)
		m_LastSearchString = _T("");

	// Changing cells, resets the search
	if (m_LastSearchCell!=GetFocusCell())
		m_LastSearchString = _T("");

	// Changing rows, resets the search
	if (m_LastSearchRow!=GetFocusRow())
		m_LastSearchString = _T("");

	if (m_LastSearchString.IsEmpty())
		m_RepeatSearchCount = -1;

	m_LastSearchCell = GetFocusCell();
	m_LastSearchTime = m_LastSearchTime.GetCurrentTime();

	if ( m_LastSearchString.GetLength()==1
	  && m_LastSearchString.GetAt(0)==(TCHAR)nChar)
	{
		// When the same first character is entered again,
		// then just repeat the search, but remember number of repeats
		m_RepeatSearchCount++;
	}
	else
	{
		for(int i=0; i <=m_RepeatSearchCount; ++i)
			m_LastSearchString.Insert(m_LastSearchString.GetLength()+1,m_LastSearchString.GetAt(0));
		m_RepeatSearchCount = -1;
		m_LastSearchString.Insert(m_LastSearchString.GetLength()+1, (TCHAR)nChar);
	}

	int nRow = GetFocusRow();
	if (nRow < 0)
		nRow = 0;
	int nCol = GetFocusCell();
	if (nCol < 0)
		nCol = GetFirstVisibleColumn();

	m_LastSearchRow = OnKeyboardSearch(nCol, nRow, m_LastSearchString);
	if (m_LastSearchRow!=-1)
	{
		// De-select all other rows
		SelectRow(-1, false);
		// Select row found
		SelectRow(m_LastSearchRow, true);
		// Focus row found
		SetFocusRow(m_LastSearchRow);
		// Scroll to row found
		EnsureVisible(m_LastSearchRow, FALSE);
	}
}

//------------------------------------------------------------------------
//! Override this method to provide text string when drawing cells
//! Only called when using LPSTR_TEXTCALLBACK with CListCtrl::SetItemText()
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param strText Text string to display in the cell
//! @return True if there is text string to display
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayCellText(int nRow, int nCol, CString& strText)
{
	return false;
}

//------------------------------------------------------------------------
//! Override this method to provide icon index when drawing cells
//! Only called when using I_IMAGECALLBACK with SetCellImage()
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param nImageId The icon index in the list control image list
//! @return True if there is an icon image to display
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayCellImage(int nRow, int nCol, int& nImageId)
{
	return false;
}

//------------------------------------------------------------------------
//! LVN_GETDISPINFO message handler, which is called when details are
//! needed for an item that specifies callback.
//!		- Cell-Text, when item is using LPSTR_TEXTCALLBACK
//!		- Cell-Image, when item is using I_IMAGECALLBACK
//!
//! @param pNMHDR Pointer to an NMLVDISPINFO structure
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pNMW = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	int nRow = pNMW->item.iItem;
	int nCol = pNMW->item.iSubItem;

	if (nRow< 0 || nRow >= GetItemCount())
		return FALSE;	// requesting invalid item

	if (nCol < 0 || nCol >= GetHeaderCtrl()->GetItemCount())
		return FALSE;	// requesting invalid item

	int nColItemData = GetColumnData(nCol);
	nColItemData;	// Avoid unreferenced variable warning
	int nRowItemData = (int)GetItemData(nRow);
	nRowItemData;	// Avoid unreferenced variable warning

	if(pNMW->item.mask & LVIF_TEXT)
	{
		// Request text
		CString result;
		if (OnDisplayCellText(nRow, nCol, result))
		{
#if __STDC_WANT_SECURE_LIB__
			_tcscpy_s(pNMW->item.pszText, pNMW->item.cchTextMax, static_cast<LPCTSTR>(result) );
#else
			_tcsncpy(pNMW->item.pszText, static_cast<LPCTSTR>(result), pNMW->item.cchTextMax);
#endif
		}
	}

	if (pNMW->item.mask & LVIF_IMAGE)
	{
		// Request-Image
		int result = -1;
		if (OnDisplayCellImage(nRow, nCol, result))
            pNMW->item.iImage = result;
		else
			pNMW->item.iImage = I_IMAGECALLBACK;

		// Support checkboxes when using LVS_OWNERDATA (virtual list)
		if (nCol==0)
		{
			if (GetStyle() & LVS_OWNERDATA && GetExtendedStyle() & LVS_EX_CHECKBOXES)
			{
				pNMW->item.mask |= LVIF_STATE;
				pNMW->item.stateMask = LVIS_STATEIMAGEMASK;
				bool bChecked = OnOwnerDataDisplayCheckbox(nRow);
				pNMW->item.state = bChecked ? INDEXTOSTATEIMAGEMASK(2) : INDEXTOSTATEIMAGEMASK(1);
			}
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

//------------------------------------------------------------------------
//! Override this method if wanting to specify whether a tooltip is available.
//! Called constantly while the mouse is moving over the list-control.
//!
//! @param point Current mouse position relative to the upper-left corner of the window
//! @return Start tooltip timer for displaying tooltip (true / false)
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayCellTooltip(const CPoint& point) const
{
	return true;
}

//------------------------------------------------------------------------
//! Override this method to display a custom tooltip text when holding the
//! mouse over a cell. Called after the tooltip timer has fired. 
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param strResult The text value to display in the tooltip
//! @return Is tooltip available for current cell (true / false)
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayCellTooltip(int nRow, int nCol, CString& strResult)
{
	if (nRow!=-1 && nCol!=-1)
	{
		strResult = GetItemText(nRow, nCol);	// Cell-ToolTip
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
//! Called by the MFC framework during mouse over to detemine whether a
//! point is in the bounding rectangle of the specified tool.
//! It requests a TTN_NEEDTEXT notification when the tooltip text is needed
//! instead of building and allocating the tooltip text at mouse over.
//!
//! If needing to display more than 80 characters, then the easy solution is
//! to override this method and allocate and build the tooltip right away.
//!
//! \code
//! int MyListCtrl::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
//! {
//!		int rc = CGridListCtrlEx::OnToolHitTest(point, pTI);
//!		if (rc==-1)
//!			return -1;
//!		
//!		pTI->lpszText = new TCHAR[256];	// Will automatically be deallocated by MFC
//!		return rc;
//! }
//! \endcode
//!
//! @param point Current mouse position relative to the upper-left corner of the window
//! @param pTI A pointer to a TOOLINFO structure
//! @return Window control ID of the tooltip control (-1 if no tooltip control was found)
//------------------------------------------------------------------------
#if defined(_WIN64)
INT_PTR CGridListCtrlEx::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
#else
int CGridListCtrlEx::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
#endif
{
	if (!OnDisplayCellTooltip(point))
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

	return (int)pTI->uId; // Must return a unique value for each cell (Marks a new tooltip control)
}

//------------------------------------------------------------------------
//! TTN_NEEDTEXT message handler called when the tooltip timer fires.
//! It uses the default tooltip buffer, that limits the tooltip to 80 characters.
//!
//! If needing to display more than 80 characters, then the easy solution is
//! to override CGridListCtrlEx::OnToolHitTest().
//!
//! @param id Not used
//! @param pNMHDR Pointer to an TOOLTIPTEXT structure
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnToolNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt(GetMessagePos());
	ScreenToClient(&pt);

	int nRow, nCol;
	CellHitTest(pt, nRow, nCol);

	// Make const-reference to the returned anonymous CString-object,
	// will keep it alive until reaching scope end
	CString tooltip;
	if (!OnDisplayCellTooltip(nRow, nCol,tooltip) || tooltip.IsEmpty())
		return FALSE;

	// Non-unicode applications can receive requests for tooltip-text in unicode
	TOOLTIPTEXTA* pTTTA = reinterpret_cast<TOOLTIPTEXTA*>(pNMHDR);
	TOOLTIPTEXTW* pTTTW = reinterpret_cast<TOOLTIPTEXTW*>(pNMHDR);
#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTA->szText));
	else
#if __STDC_WANT_SECURE_LIB__
		mbstowcs_s(NULL, pTTTW->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTW->szText)/sizeof(WCHAR)-1);
#else
		mbstowcs(pTTTW->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTW->szText)/sizeof(WCHAR)-1);
#endif
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTA->szText)-1);
	else
		lstrcpyn(pTTTW->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTW->szText)/sizeof(WCHAR));
#endif
	return TRUE;
}

//------------------------------------------------------------------------
//! Override this method to control whether cell edit should be started
//! when clicked with the mouse.
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param pt The position clicked, in client coordinates.
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnClickEditStart(int nRow, int nCol, CPoint pt)
{
	if (GetKeyState(VK_CONTROL) < 0)
		return false;	// Row selection should not trigger cell edit
	if (GetKeyState(VK_SHIFT) < 0)
		return false;	// Row selection should not trigger cell edit

	// Begin edit if the same cell is clicked twice
	bool startEdit = nRow!=-1 && nCol!=-1 && GetFocusRow()==nRow && GetFocusCell()==nCol;

	CGridColumnTrait* pTrait = GetCellColumnTrait(nRow, nCol);
	if (pTrait==NULL)
		return startEdit;

	if (pTrait->IsCellReadOnly(*this, nRow, nCol, pt))
		return false;

	if (!pTrait->OnClickEditStart(*this, nRow, nCol, pt))
		return false;

	return true;
}

//------------------------------------------------------------------------
//! Override this method to control whether cell editing is allowed for a cell.
//! Called when start editing a cell value
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param pt The position clicked, in client coordinates.
//! @return Pointer to the cell editor (If NULL then block cell editing)
//------------------------------------------------------------------------
CWnd* CGridListCtrlEx::OnEditBegin(int nRow, int nCol, CPoint pt)
{
	CGridColumnTrait* pTrait = GetCellColumnTrait(nRow, nCol);
	if (pTrait==NULL)
		return NULL;

	if (pTrait->IsCellReadOnly(*this, nRow, nCol, pt))
		return NULL;

	return pTrait->OnEditBegin(*this, nRow, nCol, pt);
}

//------------------------------------------------------------------------
//! Override this method to validate the new value after a cell edit.
//! Called when completed editing of a cell value
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param pEditor Pointer to the cell editor created by the column trait
//! @param pLVDI Specifies the properties of the new cell value
//! @return Is the new cell value accepted
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnEditComplete(int nRow, int nCol, CWnd* pEditor, LV_DISPINFO* pLVDI)
{
	CGridColumnTrait* pTrait = GetCellColumnTrait(nRow, nCol);
	if (pTrait!=NULL)
		pTrait->OnEditEnd();

	return true;	// Accept edit
}

//------------------------------------------------------------------------
//! Starts the edit of a cell and sends a message to the parent window.
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param pt The position clicked, in client coordinates.
//! @return Pointer to the cell editor (If NULL then block cell editing)
//------------------------------------------------------------------------
CWnd* CGridListCtrlEx::EditCell(int nRow, int nCol, CPoint pt)
{
	if (nCol==-1 || nRow==-1)
		return NULL;

	m_pEditor = OnEditBegin(nRow, nCol, pt);
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
		OnEditComplete(nRow, nCol, NULL, NULL);
		m_pEditor->PostMessage(WM_CLOSE);
		m_pEditor = NULL;
		return NULL;
	}

	// Show editor
	m_pEditor->ShowWindow(SW_SHOW);
	m_pEditor->SetFocus();
	return m_pEditor;
}

//------------------------------------------------------------------------
//! Checks if the cell value editor is open for a cell
//!
//! @return Is currently editing a cell value (true / false)
//------------------------------------------------------------------------
bool CGridListCtrlEx::IsCellEditorOpen() const
{
	return m_pEditor!=NULL;
}

//------------------------------------------------------------------------
//! LVN_BEGINLABELEDIT message handler called when start editing a cell.
//! Blocks cell edit events from the parent CListCtrl for the label column.
//!
//! @param pNMHDR Pointer to an LV_DISPINFO structure
//! @param pResult Set to TRUE prevents the user from editing the label, else FALSE
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = reinterpret_cast<LV_DISPINFO*>(pNMHDR);

	// Parent Clistctrl might start a cell-edit on its own (Ignore this)
	if (!IsCellEditorOpen())
	{
		*pResult = TRUE;// Block for edit
		return TRUE;	// Block message from reaching parent-dialog
	}

	int nCol = pDispInfo->item.iSubItem;
	int nRow = pDispInfo->item.iItem;
	nRow;	// Avoid unreferenced variable warning
	nCol;	// Avoid unreferenced variable warning

	*pResult = FALSE;// Accept editing
	return FALSE;	// Let parent-dialog get chance
}

//------------------------------------------------------------------------
//! LVN_ENDLABELEDIT message handler called when completed a cell edit.
//! Makes it possible to validate input, and reject invalid values.
//!
//! @param pNMHDR Pointer to an LV_DISPINFO structure
//! @param pResult Set to TRUE accepts the cell edit, else FALSE
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = reinterpret_cast<LV_DISPINFO*>(pNMHDR);

	*pResult = FALSE;	// Reject edit by default

	int nRow = pDispInfo->item.iItem;
	int nCol = pDispInfo->item.iSubItem;

	if((pDispInfo->item.mask & LVIF_TEXT)&&
	   (pDispInfo->item.pszText != NULL)&&
       (nRow != -1) &&
	   (nCol != -1))
    {
		// Label edit completed by user
		if (OnEditComplete(nRow, nCol, m_pEditor, pDispInfo))
		{
			*pResult = TRUE;	// Accept edit

			// Handle situation where data is stored inside the CListCtrl
			if (!IsCellCallback(nRow,nCol))
				SetItemText(nRow, nCol, pDispInfo->item.pszText);
		}
	}

	if((pDispInfo->item.mask & LVIF_IMAGE)&&
	   (pDispInfo->item.iImage != I_IMAGECALLBACK)&&
       (nRow != -1) &&
	   (nCol != -1))
    {
		// Label edit completed by user
		if (OnEditComplete(nRow, nCol, m_pEditor, pDispInfo))
		{
			*pResult = TRUE;	// Accept edit

			// Handle situation where data is stored inside the CListCtrl
			if (!(GetStyle() & LVS_OWNERDATA))
			{
				LV_ITEM lvi = {0};
				lvi.iItem = nRow;
				lvi.iSubItem = nCol;
				lvi.mask = LVIF_IMAGE | LVIF_NORECOMPUTE;
				VERIFY( GetItem( &lvi ) );
				if (lvi.iImage!=I_IMAGECALLBACK)
					SetCellImage(nRow, nCol, pDispInfo->item.iImage);
			}
		}
	}

	// Editor Control automatically kills themselves after posting this message
	m_pEditor = NULL;
	SetFocus();
	return FALSE;		// Parent dialog should get a chance
}

//------------------------------------------------------------------------
//! WM_LBUTTONDOWN message handler called when the user presses the left
//! mouse button while the cursor is in the client area of a window.
//! Used to activate the cell editor of the cell clicked using the mouse.
//!
//! @param nFlags Indicates whether various virtual keys are down (MK_CONTROL, MK_SHIFT, etc.)
//! @param point Mouse cursor position relative to the upper-left corner of the client area.
//------------------------------------------------------------------------
void CGridListCtrlEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	bool startEdit = true;
	if (IsCellEditorOpen())
		startEdit = false;	// If the cell-editor is already open, then it should just be closed

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

	if (startEdit)
		startEdit = OnClickEditStart(nRow, nCol, point);

	// Update the focused cell before calling CListCtrl::OnLButtonDown()
	// as it might cause a row-repaint
	SetFocusCell(nCol);

	CListCtrl::OnLButtonDown(nFlags, point);
	// LVN_BEGINDRAG message can be fired when calling parent OnLButtonDown(),
	// this should not result in a start edit operation
	if (GetFocusCell() != nCol)
	{
		SetFocusCell(nCol);
		startEdit = false;
	}

	// CListCtrl::OnLButtonDown() doesn't change row if clicking on subitem without fullrow selection
	if (!(GetExtendedStyle() & LVS_EX_FULLROWSELECT))
	{
		if (nRow!=GetFocusRow())
		{
			SetFocusRow(nRow);
			if (!(GetKeyState(VK_CONTROL) < 0) && !(GetKeyState(VK_SHIFT) < 0))
			{
				SelectRow(-1, false);
				SelectRow(nRow, true);
			}
		}
	}

	// CListCtrl::OnLButtonDown() doesn't always cause a row-repaint
	// call our own method to ensure the row is repainted
	SetFocusCell(nCol, true);

	if (startEdit)
	{
		// This will steal the double-click event when double-clicking a cell that already have focus,
		// but we cannot guess after the first click, whether the user will click a second time.
		// A timer could be used but it would cause slugish behavior (http://blogs.msdn.com/oldnewthing/archive/2004/10/15/242761.aspx)
		EditCell(nRow, nCol, point);
	}
}

//------------------------------------------------------------------------
//! WM_RBUTTONDOWN message handler called when the user presses the right
//! mouse button while the cursor is in the client area of a window.
//! Used to change focus to the cell clicked using the mouse.
//!
//! @param nFlags Indicates whether various virtual keys are down (MK_CONTROL, MK_SHIFT, etc.)
//! @param point Mouse cursor position relative to the upper-left corner of the client area.
//------------------------------------------------------------------------
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
		SetFocusCell(nCol);
	}

	CListCtrl::OnRButtonDown(nFlags, point);
}

//------------------------------------------------------------------------
//! Override this method to change the colors used for drawing a cell
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param textColor The text color used when drawing the cell
//! @param backColor The background color when drawing the cell
//! @return Color is overrided
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayCellColor(int nRow, int nCol, COLORREF& textColor, COLORREF& backColor)
{
	return false;
}

//------------------------------------------------------------------------
//! Override this method to change the font used for drawing a cell
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param font The font description to use for drawing the cell
//! @return Font is overrided
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayCellFont(int nRow, int nCol, LOGFONT& font)
{
	return false;
}

//------------------------------------------------------------------------
//! Override this method to change the color used for drawing a row
//!
//! @param nRow The index of the row
//! @param textColor The text color used when drawing the row
//! @param backColor The background color when drawing the row
//! @return Color is overrided
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayRowColor(int nRow, COLORREF& textColor, COLORREF& backColor)
{
	return false;
}

//------------------------------------------------------------------------
//! Override this method to change the font used for drawing a row
//!
//! @param nRow The index of the row
//! @param font The font description to use for drawing the row
//! @return Font is overrided
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayRowFont(int nRow, LOGFONT& font)
{
	return false;
}

//------------------------------------------------------------------------
//! Override this method to react to mouse over event during drag drop
//!
//! @param nRow The index of the row
//------------------------------------------------------------------------
void CGridListCtrlEx::OnDisplayDragOverRow(int nRow)
{
	if (UsingVisualStyle())
	{
		SetHotItem(nRow);
		return;
	}

	SetItemState(-1, 0, LVIS_DROPHILITED | LVIS_FOCUSED);
	SetHotItem(-1);
	if (nRow!=-1)
	{
		if (!(GetExtendedStyle() & (LVS_EX_TRACKSELECT | LVS_EX_TWOCLICKACTIVATE)))
			SetExtendedStyle(GetExtendedStyle() | LVS_EX_TRACKSELECT | LVS_EX_TWOCLICKACTIVATE);
		SetItemState(nRow, LVIS_DROPHILITED | LVIS_FOCUSED, LVIS_DROPHILITED | LVIS_FOCUSED);
		SetHotItem(nRow);
	}
	else
	{
		SetExtendedStyle(GetExtendedStyle()  & ~LVS_EX_TRACKSELECT);
		SetExtendedStyle(GetExtendedStyle()  & ~LVS_EX_TWOCLICKACTIVATE);
	}
}

//------------------------------------------------------------------------
//! Performs custom drawing of the CListCtrl using CGridRowTrait
//!
//! @param nRow The index of the row
//! @param pLVCD Pointer to NMLVCUSTOMDRAW structure
//! @param pResult Modification to the drawing stage (CDRF_NEWFONT, etc.)
//------------------------------------------------------------------------
void CGridListCtrlEx::OnCustomDrawRow(int nRow, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult)
{
	CGridRowTrait* pTrait = GetRowTrait(nRow);
	if (pTrait==NULL)
		return;

	pTrait->OnCustomDraw(*this, pLVCD, pResult);
}

//------------------------------------------------------------------------
//! Performs custom drawing of the CListCtrl using CGridColumnTrait
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param pLVCD Pointer to NMLVCUSTOMDRAW structure
//! @param pResult Modification to the drawing stage (CDRF_NEWFONT, etc.)
//------------------------------------------------------------------------
void CGridListCtrlEx::OnCustomDrawCell(int nRow, int nCol, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult)
{
	CGridColumnTrait* pTrait = GetCellColumnTrait(nRow, nCol);
	if (pTrait==NULL)
		return;

	if (!pTrait->GetColumnState().m_Visible)
	{
		*pResult = CDRF_SKIPDEFAULT;
		return;
	}

	pTrait->OnCustomDraw(*this, pLVCD, pResult);
}

//------------------------------------------------------------------------
//! Performs custom drawing of the CListCtrl.
//!  - Ensures the CGridColumnTrait's can do their thing
//!
//! @param pNMHDR Pointer to NMLVCUSTOMDRAW structure
//! @param pResult Modification to the drawing stage (CDRF_NEWFONT, etc.)
//------------------------------------------------------------------------
void CGridListCtrlEx::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	int nRow = (int)pLVCD->nmcd.dwItemSpec;

	*pResult = CDRF_DODEFAULT;

	// Allow column-traits to perform their custom drawing
	if (pLVCD->nmcd.dwDrawStage & CDDS_SUBITEM)
	{
		OnCustomDrawRow(nRow, pLVCD, pResult);
		if (*pResult & CDRF_SKIPDEFAULT)
			return;	// Everything is handled by the row-trait

		int nCol = pLVCD->iSubItem;
		OnCustomDrawCell(nRow, nCol, pLVCD, pResult);
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
			OnCustomDrawRow(nRow, pLVCD, pResult);
		} break;

		// After painting the entire row
		case CDDS_ITEMPOSTPAINT:
		{
			OnCustomDrawRow(nRow, pLVCD, pResult);
		} break;
	}
}

//------------------------------------------------------------------------
//! Retrieves the row trait for the specified row.
//! Override this to provide a custom row trait for a certain row
//!
//! @param nRow The index of the row
//! @return Pointer to row trait
//------------------------------------------------------------------------
CGridRowTrait* CGridListCtrlEx::GetRowTrait(int nRow)
{
	return m_pDefaultRowTrait;
}

//------------------------------------------------------------------------
//! Sets the default row trait used by default for drawing rows
//!
//! @param pRowTrait Pointer to row trait
//------------------------------------------------------------------------
void CGridListCtrlEx::SetDefaultRowTrait(CGridRowTrait* pRowTrait)
{
	ASSERT(pRowTrait!=NULL);
	delete m_pDefaultRowTrait;
	m_pDefaultRowTrait = pRowTrait;
}

//------------------------------------------------------------------------
//! Retrieves the column visible state from the column trait.
//!
//! @param nCol The index of the column
//! @return Is the column visible or not (true / false)
//------------------------------------------------------------------------
bool CGridListCtrlEx::IsColumnVisible(int nCol)
{
	return GetColumnTrait(nCol)->GetColumnState().m_Visible;
}

//------------------------------------------------------------------------
//! Checks if a column is allowed to be resized
//!
//! @param nCol The index of the column
//! @return Can the column be resized or not (true / false)
//------------------------------------------------------------------------
bool CGridListCtrlEx::IsColumnResizable(int nCol)
{
	return GetColumnTrait(nCol)->GetColumnState().m_Resizable;
}

//------------------------------------------------------------------------
//! Checks if a column is fixed to be always visible
//!
//! @param nCol The index of the column
//! @return Is the column always visible (true / false)
//------------------------------------------------------------------------
bool CGridListCtrlEx::IsColumnAlwaysVisible(int nCol)
{
	return GetColumnTrait(nCol)->GetColumnState().m_AlwaysVisible;
}

//------------------------------------------------------------------------
//! Checks if a column is fixed to be always hidden
//!
//! @param nCol The index of the column
//! @return Is the column always visible (true / false)
//------------------------------------------------------------------------
bool CGridListCtrlEx::IsColumnAlwaysHidden(int nCol)
{
	return GetColumnTrait(nCol)->GetColumnState().m_AlwaysHidden;
}

//------------------------------------------------------------------------
//! Retrieves the column trait of a specified cell.
//! Makes it possible to override the column trait of a single cell
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @return The column trait pointer of the cell
//------------------------------------------------------------------------
CGridColumnTrait* CGridListCtrlEx::GetCellColumnTrait(int nRow, int nCol)
{
	return GetColumnTrait(nCol);
}

//------------------------------------------------------------------------
//! Retrieves the column trait for the entire column
//!
//! @param nCol The index of the column
//! @return The column trait pointer for the entire column
//------------------------------------------------------------------------
CGridColumnTrait* CGridListCtrlEx::GetColumnTrait(int nCol)
{
	VERIFY( nCol >=0 && nCol < m_ColumnTraits.GetSize() );
	return m_ColumnTraits[nCol];
}

//------------------------------------------------------------------------
//! Retrieves the number of column traits registered
//!
//! @return The number or column traits
//------------------------------------------------------------------------
int CGridListCtrlEx::GetColumnTraitSize() const
{
	return m_ColumnTraits.GetSize();
}

//------------------------------------------------------------------------
//! Internal maintenance function for the column trait container
//!
//! @param nCol The index of the new column to get a column trait
//! @param pTrait The column trait of the new column
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
//! Internal maintenance function for the column trait container
//!
//! @param nCol The index of the column just deleted
//------------------------------------------------------------------------
void CGridListCtrlEx::DeleteColumnTrait(int nCol)
{
	delete GetColumnTrait(nCol);
	m_ColumnTraits.RemoveAt(nCol);
}

//------------------------------------------------------------------------
//! LVM_DELETECOLUMN message handler to ensure the column trait
//! container is updated when columns are removed.
//!
//! @param wParam The index of the column just deleted
//! @param lParam Not used
//! @return Whether the column was succesfully deleted
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
//! LVM_INSERTCOLUMN message handler to ensure the column trait
//! container is updated when columns are inserted.
//!
//! @param wParam The index of the column just inserted
//! @param lParam Not used
//! @return Whether the column was succesfully inserted
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
//! WM_CONTEXTMENU message handler to show popup menu when mouse right
//! click is used (or SHIFT+F10 on the keyboard)
//!
//! @param pWnd Handle to the window in which the user right clicked the mouse
//! @param point Position of the cursor, in screen coordinates, at the time of the mouse click.
//------------------------------------------------------------------------
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
		SetFocusCell(GetFocusCell(), true);

		CPoint pt = point;
		ScreenToClient(&pt);

		if (pWnd==GetHeaderCtrl())
		{
			HDHITTESTINFO hdhti = {0};
			hdhti.pt = pt;
			hdhti.pt.x += GetScrollPos(SB_HORZ);
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

//------------------------------------------------------------------------
//! Override this method to change the context menu when using the keyboard
//! shortcut SHIFT+F10
//!
//! @param pWnd Handle to the window in which the user right clicked the mouse
//! @param point Position of the cursor, in screen coordinates, at the time of the mouse click.
//------------------------------------------------------------------------
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
			VERIFY( GetItemRect(nRow, cellRect, LVIR_BOUNDS) );
		else
			VERIFY( GetCellRect(nRow, nCol, LVIR_BOUNDS, cellRect) );
		ClientToScreen(cellRect);

		// Adjust point so context-menu doesn't cover row / cell
		point = cellRect.TopLeft();
		int cellHeightCenter = cellRect.Height() / 2;
		int cellWidthCenter = cellRect.Width() / 2;
		point.x += cellHeightCenter < cellWidthCenter ? cellHeightCenter : cellWidthCenter; // min(cellHeightCenter, cellHeightCenter);
		point.y += cellHeightCenter;
		OnContextMenuCell(pWnd, point, nRow, nCol);
	}
}

//------------------------------------------------------------------------
//! Override this method to change the context menu when activating context
//! menu in client area with no rows
//!
//! @param pWnd Handle to the window in which the user right clicked the mouse
//! @param point Position of the cursor, in screen coordinates, at the time of the mouse click.
//------------------------------------------------------------------------
void CGridListCtrlEx::OnContextMenuGrid(CWnd* pWnd, CPoint point)
{
}

//------------------------------------------------------------------------
//! Internal method to add all available columns to the context menu
//!
//! @param menu The popup context menu
//! @param offset Start offset to use when adding new menu items to the context menu
//! @return Number of menu items added to the context menu
//------------------------------------------------------------------------
int CGridListCtrlEx::InternalColumnPicker(CMenu& menu, int offset)
{
	for( int i = 0 ; i < GetColumnCount(); ++i)
	{
		if (IsColumnAlwaysHidden(i))
			continue;	// Cannot be shown

		UINT uFlags = MF_STRING;

		// Put check-box on context-menu
		if (IsColumnVisible(i))
			uFlags |= MF_CHECKED;
		else
			uFlags |= MF_UNCHECKED;

		if (IsColumnAlwaysVisible(i))
			uFlags |= MF_DISABLED;

		// Retrieve column-title
		const CString& columnTitle = GetColumnHeading(i);
		VERIFY( menu.AppendMenu(uFlags, offset+i, static_cast<LPCTSTR>(columnTitle)) );
	}

	return GetColumnTraitSize();
}

//------------------------------------------------------------------------
//! Internal method to add all available column profiles to the context menu
//!
//! @param menu The popup context menu
//! @param offset Start offset to use when adding new menu items to the context menu
//! @param profiles List of column profiles that one can change between
//! @return Number of menu items added to the context menu
//------------------------------------------------------------------------
int CGridListCtrlEx::InternalColumnProfileSwitcher(CMenu& menu, int offset, CSimpleArray<CString>& profiles)
{
	CString title_profiles;
	CString active_profile = m_pColumnManager->HasColumnProfiles(*this, profiles, title_profiles);
	if (profiles.GetSize()>0)
	{
		menu.AppendMenu(MF_SEPARATOR, 0, _T(""));

		CMenu submenu;
		submenu.CreatePopupMenu();
		for(int i=0;i<profiles.GetSize();++i)
		{
			UINT uFlags = MF_STRING;
			// Put check-box on context-menu
			if (active_profile==profiles[i])
				uFlags |= MF_CHECKED;
			else
				uFlags |= MF_UNCHECKED;
			VERIFY( submenu.AppendMenu(uFlags, offset + i, static_cast<LPCTSTR>(profiles[i])) );
		}

		VERIFY( menu.AppendMenu(MF_POPUP, (UINT_PTR)submenu.Detach(), static_cast<LPCTSTR>(title_profiles)) );
	}
	return profiles.GetSize();
}

//------------------------------------------------------------------------
//! Override this method to change the context menu when activating context
//! menu for the column headers
//!
//! @param pWnd Handle to the window in which the user right clicked the mouse
//! @param point Position of the cursor, in screen coordinates, at the time of the mouse click.
//! @param nCol The index of the column
//------------------------------------------------------------------------
void CGridListCtrlEx::OnContextMenuHeader(CWnd* pWnd, CPoint point, int nCol)
{
	// Show context-menu with the option to show hide columns
	CMenu menu;
	VERIFY( menu.CreatePopupMenu() );

	CString title_editor;
	if (m_pColumnManager->HasColumnEditor(*this, nCol, title_editor))
	{
		menu.AppendMenu(MF_STRING, 1, static_cast<LPCTSTR>(title_editor));
	}

	CString title_picker;
	if (m_pColumnManager->HasColumnPicker(*this, title_picker))
	{
		menu.AppendMenu(MF_STRING, 2, static_cast<LPCTSTR>(title_picker));
	}
	else
	{
		if (menu.GetMenuItemCount()>0)
			menu.AppendMenu(MF_SEPARATOR, 0, _T(""));

		InternalColumnPicker(menu, 4);
	}

	CSimpleArray<CString> profiles;
	InternalColumnProfileSwitcher(menu, GetColumnCount() + 5, profiles);

	CString title_resetdefault;
	if (m_pColumnManager->HasColumnsDefault(*this, title_resetdefault))
	{
		if (profiles.GetSize()==0)
			menu.AppendMenu(MF_SEPARATOR, 0, _T(""));
		menu.AppendMenu(MF_STRING, 3, title_resetdefault);
	}

	// Will return zero if no selection was made (TPM_RETURNCMD)
	int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this, 0);
	switch(nResult)
	{
		case 0: break;
		case 1:	m_pColumnManager->OpenColumnEditor(*this, nCol); break;
		case 2: m_pColumnManager->OpenColumnPicker(*this); break;
		case 3: m_pColumnManager->ResetColumnsDefault(*this); break;
		default:
		{
			int nCol = nResult-4;
			if (nCol < GetColumnCount())
			{
				ShowColumn(nCol, !IsColumnVisible(nCol));
			}
			else
			{
				int nProfile = nResult-GetColumnCount()-5;
				m_pColumnManager->SwichColumnProfile(*this, profiles[nProfile]);
			}
		} break;
	}
}

//------------------------------------------------------------------------
//! Override this method to change the context menu when activating context
//! menu for a single cell
//!
//! @param pWnd Handle to the window in which the user right clicked the mouse
//! @param point Position of the cursor, in screen coordinates, at the time of the mouse click.
//! @param nRow The index of the row
//! @param nCol The index of the column
//------------------------------------------------------------------------
void CGridListCtrlEx::OnContextMenuCell(CWnd* pWnd, CPoint point, int nRow, int nCol)
{
}


//------------------------------------------------------------------------
//! HDN_DIVIDERDBLCLICK message handler called when double clicking the
//! divider in the columns of the CHeaderCtrl. Used to prevent resizing
//! of hidden columns.
//!
//! @param pNMHDR Pointer to an NMHEADER structure with information about the divider was double-clicked
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnHeaderDividerDblClick(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing

	NMHEADER* pNMH = reinterpret_cast<NMHEADER*>(pNMHDR);
	SetColumnWidthAuto(pNMH->iItem);
	return TRUE;	// Don't let parent handle the event
}

//------------------------------------------------------------------------
//! HDN_BEGINTRACK message handler called when resizing columns. Used to
//! prevent resizing of hidden columns.
//!
//! @param pNMHDR Pointer to an NMHEADER structure with information about the column being resized
//! @param pResult Set to FALSE to allow tracking of the divider, or TRUE to prevent tracking
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnHeaderBeginResize(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing

	// Check that column is allowed to be resized
	NMHEADER* pNMH = reinterpret_cast<NMHEADER*>(pNMHDR);
	int nCol = (int)pNMH->iItem;

	if (!IsColumnVisible(nCol))
	{
		*pResult = TRUE;	// Block resize
		return TRUE;		// Block event
	}

	if (!IsColumnResizable(nCol))
	{
		*pResult = TRUE;	// Block resize
		return TRUE;		// Block event
	}

	return FALSE;
}

//------------------------------------------------------------------------
//! HDN_ENDTRACK message handler called after column resize. Used to
//! persist the new column state.
//!
//! @param pNMHDR Pointer to an NMHEADER structure with information about the column just resized
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnHeaderEndResize(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	m_pColumnManager->OnColumnResize(*this);
	return FALSE;
}

//------------------------------------------------------------------------
//! LVM_SETCOLUMNWIDTH message handler called when wanting to resize a column.
//! Used to prevent resize of hidden columns.
//!
//! @param wParam The index of the column
//! @param lParam New width of the column (High word)
//! @return Nonzero indicates success. Zero indicates otherwise.
//------------------------------------------------------------------------
LRESULT CGridListCtrlEx::OnSetColumnWidth(WPARAM wParam, LPARAM lParam)
{
	// Check that column is allowed to be resized
	int nCol = (int)wParam;

	if (!IsColumnVisible(nCol))
		return FALSE;

	if (!IsColumnResizable(nCol))
		return FALSE;

	m_pColumnManager->OnColumnResize(*this);

	// Let CListCtrl handle the event
	return DefWindowProc(LVM_SETCOLUMNWIDTH, wParam, lParam);
}

//------------------------------------------------------------------------
//! WM_KILLFOCUS message handler called when list control is loosing focus
//! to other control. Used to persist the new column state.
//!
//! @param pNewWnd Pointer to the window that receives the input focus (may be NULL or may be temporary).
//------------------------------------------------------------------------
void CGridListCtrlEx::OnKillFocus(CWnd* pNewWnd)
{
	m_pColumnManager->OnOwnerKillFocus(*this);
}

//------------------------------------------------------------------------
//! HDN_BEGINDRAG message handler called when about to move a column to
//! a new position. Used to ensure that any cell value editing is completed.
//!
//! @param pNMHDR Pointer to an NMHEADER structure with information about the column just resized
//! @param pResult Set to FALSE to allow header control to automatically manage column order. Set to TRUE if manually wanting to manage column order.
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnHeaderBeginDrag(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing
	return FALSE;
}

//------------------------------------------------------------------------
//! HDN_ENDDRAG message handler called after a column have been dragged,
//! but before the column order has been updated. Used to ensure that
//! visible columns are not dragged in between invisible columns.
//!
//! @param pNMHDR Pointer to an NMHEADER structure with information about the column just resized
//! @param pResult If the owner is performing external (manual) drag-and-drop management, it must be set to FALSE
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnHeaderEndDrag(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	NMHEADER* pNMH = reinterpret_cast<NMHEADER*>(pNMHDR);
	if (pNMH->pitem->mask & HDI_ORDER)
	{
		// Correct iOrder so it is just after the last hidden column
		int nColCount = GetColumnCount();
		int* pOrderArray = new int[nColCount];
		VERIFY( GetColumnOrderArray(pOrderArray, nColCount) );

		for(int i = 0; i < nColCount ; ++i)
		{
			if (IsColumnVisible(pOrderArray[i]))
			{
				pNMH->pitem->iOrder = pNMH->pitem->iOrder > i ? pNMH->pitem->iOrder : i; // max(pNMH->pitem->iOrder,i);
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
		PARAMSORT(HWND hWnd, int nCol, bool bAscending, CGridColumnTrait* pTrait)
			:m_hWnd(hWnd)
			,m_pTrait(pTrait)
			,m_ColumnIndex(nCol)
			,m_Ascending(bAscending)
		{}

		HWND m_hWnd;
		int  m_ColumnIndex;
		bool m_Ascending;
		CGridColumnTrait* m_pTrait;
	};

	// Comparison extracts values from the List-Control
	int CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		PARAMSORT& ps = *(PARAMSORT*)lParamSort;

		TCHAR left[256] = _T(""), right[256] = _T("");
		ListView_GetItemText(ps.m_hWnd, lParam1, ps.m_ColumnIndex, left, sizeof(left));
		ListView_GetItemText(ps.m_hWnd, lParam2, ps.m_ColumnIndex, right, sizeof(right));

		return ps.m_pTrait->OnSortRows(left, right, ps.m_Ascending);
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

//------------------------------------------------------------------------
//! Changes the row sorting in regard to the specified column
//!
//! @param nCol The index of the column
//! @param bAscending Perform sorting in ascending or descending order
//! @return True / false depending on whether sort is possible
//------------------------------------------------------------------------
bool CGridListCtrlEx::SortColumn(int nCol, bool bAscending)
{
	// virtual lists cannot be sorted with this method
	if (GetStyle() & LVS_OWNERDATA)
		return false;

	if (GetItemCount()<=0)
		return true;

	CWaitCursor waitCursor;

	// Uses SortItemsEx because it requires no knowledge of datamodel
	//	- CListCtrl::SortItems() is faster with direct access to the datamodel
	PARAMSORT paramsort(m_hWnd, nCol, bAscending, GetColumnTrait(nCol));
	ListView_SortItemsEx(m_hWnd, SortFunc, &paramsort);
	return true;
}

//------------------------------------------------------------------------
//! LVN_COLUMNCLICK message handler called when clicking a column header.
//! Used to update the row sorting according to the clicked column.
//!
//! @param pNMHDR Pointer to an NMLISTVIEW structure specifying the column
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
//! Copies the contents of the selected rows into the global clipboard
//------------------------------------------------------------------------
void CGridListCtrlEx::OnCopyToClipboard()
{
	CString result;
	if (!OnDisplayToClipboard(result))
		return;

	int nlength = (result.GetLength()+1)*sizeof(TCHAR);	// +1 for null-term

	// Allocate a global memory object for the text.
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, nlength);
	if (hglbCopy==NULL)
		return;

	// Lock the handle to the memory object
	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	if (lptstrCopy==NULL)
		return;

	// Copy the text to the memory object.
	memcpy(lptstrCopy, result, nlength);
	if (GlobalUnlock(hglbCopy)!=NO_ERROR)
		return;

	// Open clipboard
	if (!OpenClipboard())
	{
		GlobalFree(hglbCopy);
		return;
	}

	// Clear clipboard
	if (!EmptyClipboard())
	{
		CloseClipboard();
		GlobalFree(hglbCopy);
		return;
	}

	// Set new clipboard content
#ifndef _UNICODE
	if (SetClipboardData(CF_TEXT, hglbCopy)==NULL)
	{
		CloseClipboard();
		GlobalFree(hglbCopy);
		return;
	}
#else
	if (SetClipboardData(CF_UNICODETEXT, hglbCopy)==NULL)
	{
		CloseClipboard();
		GlobalFree(hglbCopy);
		return;
	}
#endif

	// Close clipboard (Now owns the memory object)
	CloseClipboard();
}

//------------------------------------------------------------------------
//! Override this method to control what is placed in the global clipboard
//!
//! @param strResult Text to place in the clipboard
//! @param includeHeader Include column headers when copying to clipboard
//! @return Text is available for the clipboard
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayToClipboard(CString& strResult, bool includeHeader)
{
	if (GetSelectedCount()==1 && GetFocusCell()!=-1)
		return OnDisplayToClipboard(GetSelectionMark(), GetFocusCell(), strResult);

	POSITION pos = GetFirstSelectedItemPosition();
	if (pos==NULL)
		return false;

	if (includeHeader)
		OnDisplayToClipboard(-1, strResult);

	while(pos!=NULL)
	{
		int nRow = GetNextSelectedItem(pos);

		CString strLine;
		if (!OnDisplayToClipboard(nRow, strLine))
			continue;

		if (!strResult.IsEmpty())
			strResult += _T("\r\n");
		strResult += strLine;
	}
	return true;
}

//------------------------------------------------------------------------
//! Override this method to control what to place in the clipboard for
//! a single row.
//!
//! @param nRow The index of the row
//! @param strResult Text to place in the clipboard
//! @return Text is available for the clipboard
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayToClipboard(int nRow, CString& strResult)
{
	int nColCount = GetHeaderCtrl()->GetItemCount();
	for(int i = 0; i < nColCount; ++i)
	{
		int nCol = GetHeaderCtrl()->OrderToIndex(i);
		if (!IsColumnVisible(nCol))
			continue;
		CString strCellText;
		if (!OnDisplayToClipboard(nRow, nCol, strCellText))
			continue;
		if (!strResult.IsEmpty())
			strResult += _T("\t");
		strResult += strCellText;
	}
	return true;
}

//------------------------------------------------------------------------
//! Override this method to control what to place in the clipboard for
//! a single cell.
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param strResult Text to place in the clipboard
//! @return Text is available for the clipboard
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayToClipboard(int nRow, int nCol, CString& strResult)
{
	if (nRow==-1)
		strResult = GetColumnHeading(nCol);
	else
		strResult = GetItemText(nRow, nCol);
	return true;
}

//------------------------------------------------------------------------
//! WM_COPY message handler. Not sent by default, but just incase
//!
//! @param wParam Not used
//! @param lParam Not used
//! @return Not used
//------------------------------------------------------------------------
LRESULT CGridListCtrlEx::OnCopy(WPARAM wParam, LPARAM lParam)
{
	OnCopyToClipboard();
	return DefWindowProc(WM_COPY, wParam, lParam); 
}

namespace {
	class CMyOleDropSource : public COleDropSource
	{
		CImageList* m_pDragImage;

	public:
		explicit CMyOleDropSource(CImageList* pDragImage)
			:m_pDragImage(pDragImage)
		{}

		~CMyOleDropSource()
		{
			if (m_pDragImage!=NULL)
			{
				::ReleaseCapture();
				m_pDragImage->DragLeave(CWnd::FromHandle(GetDesktopWindow()));
				m_pDragImage->EndDrag();
				m_pDragImage->DeleteImageList();
				delete m_pDragImage;
			}
		}

		virtual SCODE GiveFeedback(DROPEFFECT dropEffect)
		{
			return COleDropSource::GiveFeedback(dropEffect);
		}

		virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState)
		{
			if (m_pDragImage!=NULL)
			{
				CPoint ptDropPoint;
				::GetCursorPos(&ptDropPoint);
				m_pDragImage->DragMove(ptDropPoint);
			}
			return COleDropSource::QueryContinueDrag(bEscapePressed, dwKeyState);
		}
	};
}

//------------------------------------------------------------------------
//! LVN_BEGINDRAG message handler called when performing left-click drag.
//! Used to perform drag drop from the list control. Override this method
//! to disable drag drop of rows.
//!
//! @param pNMHDR Pointer to an NMLISTVIEW structure specifying the column
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Update cell focus to show what is being dragged
	SetFocusCell(GetFocusCell(), true);

	int nRow = GetFocusRow();

	// Notify that drag operation was started (don't start edit),
	// also it will ensure the entire row is dragged (and not a single cell)
	SetFocusCell(-1);

	NMLISTVIEW* pLV = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	pLV;	// Avoid unreferenced variable warning

	COleDataSourceWnd<CGridListCtrlEx> oleDataSource(m_pOleDropTarget);
	DROPEFFECT dropEffect = DoDragDrop(oleDataSource);
	OnDisplayDragOverRow(-1);
	if (dropEffect==DROPEFFECT_NONE)
	{
		SetFocusRow(nRow);
		return FALSE;
	}

	return FALSE;
}

//------------------------------------------------------------------------
//! Override this method to control what data is selected for drag drop
//! operation.
//!
//! @param strResult Text to place in the drag drop cache
//! @return Text is available for the drag drop operation
//------------------------------------------------------------------------
bool CGridListCtrlEx::OnDisplayToDragDrop(CString& strResult)
{
	return OnDisplayToClipboard(strResult, false);
}

//------------------------------------------------------------------------
//! Takes the contents of the selected rows, and starts a drag-drop operation
//!
//! @param oleDataSource Cache for placing the data selected for drag-drop operation
//! @return Drop effect generated by the drag-and-drop operation; otherwise DROPEFFECT_NONE
//------------------------------------------------------------------------
DROPEFFECT CGridListCtrlEx::DoDragDrop(COleDataSource& oleDataSource)
{
	if (GetSelectedCount()<1)
		return DROPEFFECT_NONE;

	CString result;
	if (!OnDisplayToDragDrop(result))
		return DROPEFFECT_NONE;

	int nlength = (result.GetLength()+1)*sizeof(TCHAR);	// +1 for null-term

	// Allocate a global memory object for the text.
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, nlength);
	if (hglbCopy==NULL)
		return FALSE;

	// Lock the handle to the memory object
	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	if (lptstrCopy==NULL)
		return DROPEFFECT_NONE;

	// Copy the text to the memory object.
	memcpy(lptstrCopy, result, nlength);
	if (GlobalUnlock(hglbCopy)!=NO_ERROR)
		return DROPEFFECT_NONE;

#ifndef _UNICODE
	oleDataSource.CacheGlobalData(CF_TEXT,hglbCopy);
#else
	oleDataSource.CacheGlobalData(CF_UNICODETEXT,hglbCopy);
#endif

	DROPEFFECT dropEffect = oleDataSource.DoDragDrop();
	GlobalFree(hglbCopy);
	return dropEffect;
}

//------------------------------------------------------------------------
//! Registers the CListCtrl as a valid OLE drag drop target
//!
//! @return Nonzero if successful; otherwise zero.
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::RegisterDropTarget()
{
	if (m_pOleDropTarget!=NULL)
		return TRUE;
	
	m_pOleDropTarget = new COleDropTargetWnd<CGridListCtrlEx>;
	if (!m_pOleDropTarget->Register(this))
	{
		// Was AfxOleInit() called in derived CWinApp::InitInstance() ?
		delete m_pOleDropTarget;
		m_pOleDropTarget = NULL;
		return FALSE;
	}
	return TRUE;
}

//------------------------------------------------------------------------
//! Called by the framework when the cursor is first dragged into the window.
//!
//! @param pDataObject Points to the data object containing the data that can be dropped
//! @param dwKeyState Contains the state of the modifier keys (MK_SHIFT, MK_CONTROL, etc.)
//! @param point Contains the current location of the cursor in client coordinates.
//! @return The effect that would result if a drop were attempted at the location specified by point.
//------------------------------------------------------------------------
DROPEFFECT CGridListCtrlEx::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return DROPEFFECT_MOVE;
}

//------------------------------------------------------------------------
//! Called by the framework when the cursor is dragged over the window.
//!
//! @param pDataObject Points to the data object containing the data that can be dropped
//! @param dwKeyState Contains the state of the modifier keys (MK_SHIFT, MK_CONTROL, etc.)
//! @param point Contains the current location of the cursor in client coordinates.
//! @return The effect that would result if a drop were attempted at the location specified by point.
//------------------------------------------------------------------------
DROPEFFECT CGridListCtrlEx::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	CPoint pt(GetMessagePos());
	ScreenToClient(&pt);

	// Use screen position and scroll bars instead
	int nRow, nCol;
	CellHitTest(pt, nRow, nCol);

	if (GetItemCount()>0)
	{
		// Check if we need to perform auto scroll
		CRect gridRect;
		GetClientRect(&gridRect);

		CRect headerRect;
		GetHeaderCtrl()->GetClientRect(&headerRect);

		CRect cellRect;
		VERIFY( GetCellRect(0, 0, LVIR_BOUNDS, cellRect) );

		int minPos, maxPos;
		GetScrollRange(SB_VERT, &minPos, &maxPos);
		int scrollPos = GetScrollPos(SB_VERT);

		if (pt.y < gridRect.top+cellRect.Height()+headerRect.Height())
		{
			if (scrollPos > minPos)
			{
				Sleep(100);
				Scroll(CSize(0,cellRect.Height()*-1));
				UpdateWindow();
				return DROPEFFECT_SCROLL;
			}
		}
		else
		if (pt.y > gridRect.bottom-cellRect.Height())
		{
			Sleep(100);
			Scroll(CSize(0,cellRect.Height()));
			// For some strange reason, then GetScrollPos() never returns
			// a value near maxPos. Instead we check if scrollpos changed
			// to detect if we are at the bottom.
			if (GetScrollPos(SB_VERT)!=scrollPos)
			{
				UpdateWindow();
				return DROPEFFECT_SCROLL;
			}
		}
	}

	OnDisplayDragOverRow(nRow);
	return DROPEFFECT_MOVE;
}

//------------------------------------------------------------------------
//! Called by the framework when the cursor leaves the window while a dragging operation is in effect.
//------------------------------------------------------------------------
void CGridListCtrlEx::OnDragLeave()
{
}

//------------------------------------------------------------------------
//! Called by the framework when a drop operation is to occur
//!
//! @param pDataObject Points to the data object containing the data that can be dropped
//! @param dropEffect The effect that the user chose for the drop operation (DROPEFFECT_COPY, DROPEFFECT_MOVE, DROPEFFECT_LINK)
//! @param point Contains the current location of the cursor in client coordinates.
//! @return Nonzero if the drop is successful; otherwise 0
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	if (m_pOleDropTarget->IsDragSource())
		return OnDropSelf(pDataObject, dropEffect, point);
	else
		return OnDropExternal(pDataObject, dropEffect, point);
}

//------------------------------------------------------------------------
//! Called by the framework when a drop operation is to occur, where the
//! origin is the CGridListCtrlEx itself
//!
//! @param pDataObject Points to the data object containing the data that can be dropped
//! @param dropEffect The effect that the user chose for the drop operation (DROPEFFECT_COPY, DROPEFFECT_MOVE, DROPEFFECT_LINK)
//! @param point Contains the current location of the cursor in client coordinates.
//! @return Nonzero if the drop is successful; otherwise 0
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnDropSelf(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	// Internal drag (Change item position)
	int nRow, nCol;
	CellHitTest(point, nRow, nCol);
	if (MoveSelectedRows(nRow))
	{
		EnsureVisible(nRow, FALSE);
		SetFocusRow(nRow);
	}
	return TRUE;
}

//------------------------------------------------------------------------
//! Called by the framework when a drop operation is to occur, where the
//! origin is an external source
//!
//! @param pDataObject Points to the data object containing the data that can be dropped
//! @param dropEffect The effect that the user chose for the drop operation (DROPEFFECT_COPY, DROPEFFECT_MOVE, DROPEFFECT_LINK)
//! @param point Contains the current location of the cursor in client coordinates.
//! @return Nonzero if the drop is successful; otherwise 0
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnDropExternal(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	return FALSE;
}

namespace {
	struct PARAMMOVESORT
	{
		PARAMMOVESORT(HWND hWnd, int nRow, bool groupSelection)
			:m_hWnd(hWnd)
			,m_nRow(nRow)
			,m_GroupSelection(groupSelection)
		{}

		HWND m_hWnd;
		int m_nRow;
		bool m_GroupSelection;
	};

	// Comparison extracts values from the List-Control
	//	- Groups the selected rows around the drop-row
	int CALLBACK MoveSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		PARAMMOVESORT& ps = *(PARAMMOVESORT*)lParamSort;

		// If part of selection, then it must be moved
		bool selected1 = (ListView_GetItemState(ps.m_hWnd,lParam1,LVIS_SELECTED) & LVIS_SELECTED) == LVIS_SELECTED;
		bool selected2 = (ListView_GetItemState(ps.m_hWnd,lParam2,LVIS_SELECTED) & LVIS_SELECTED) == LVIS_SELECTED;

		// If both selected then no change in positioning
		if (selected1 && selected2)
			return (int)(lParam1 - lParam2);
		else
		if (selected1)
		{
			if (lParam2==ps.m_nRow)
			{
				if (ps.m_GroupSelection)
					return 1;	// Place entire selection after drop-row
				else
				if (lParam1 < ps.m_nRow)
					return 1;	// Place selected items before drop-row
				else
					return -1;	// Place selected items after drop-row
			}
			else
			if (lParam2 > ps.m_nRow)
				return -1;
			else
				return 1;
		}
		else
		if (selected2)
		{
			if (lParam1 == ps.m_nRow)
			{
				if (ps.m_GroupSelection)
					return -1;	// Place entire selection after drop-row
				else
				if (ps.m_nRow < lParam2)
					return 1;	// Place selected items before drop-row
				else
					return -1;	// Place selected items after drop-row
			}
			else
			if (lParam1 > ps.m_nRow)
				return 1;
			else
				return -1;
		}
		// If none selected then no change in positioning
		return (int)(lParam1 - lParam2);
	}
}

//------------------------------------------------------------------------
//! Inserts the selected rows before the specified row
//!
//! @param nDropRow Insert the selected rows before this row
//! @return Was rows rearranged ? (true / false)
//------------------------------------------------------------------------
bool CGridListCtrlEx::MoveSelectedRows(int nDropRow)
{
	// All selected rows should be place above this row (pushing the row down)
	if (GetStyle() & LVS_OWNERDATA)
		return false;

	POSITION pos = GetFirstSelectedItemPosition();
	if (pos==NULL)
		return false;
	
	int nFirstSelectedRow = GetNextSelectedItem(pos);

	if (GetSelectedCount()==1)
	{
		// Do nothing if dragging a single row without moving position
		if (nFirstSelectedRow==nDropRow)
			return false;
	}

	// Check if dropping in the bottom of the list
	if (nDropRow==-1)
		nDropRow = GetItemCount();

	bool groupSelection = false;
	if (GetSelectedCount() > 1)
	{
		if (nFirstSelectedRow < nDropRow && GetNextItem(nDropRow, LVNI_SELECTED)!=-1)
		{
			groupSelection = true;
		}
	}

	// Uses SortItemsEx because it requires no knowledge of datamodel
	CWaitCursor waitCursor;
	PARAMMOVESORT paramsort(m_hWnd, nDropRow, groupSelection);
	ListView_SortItemsEx(m_hWnd, MoveSortFunc, &paramsort);
	return true;
}

//------------------------------------------------------------------------
//! NM_CLICK message handler called when left-clicking in a cell.
//! Just to show how to catch the single click event.
//!
//! @param pNMHDR Pointer to NMITEMACTIVATE structure
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnItemClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMITEMACTIVATE* pItem = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);

	// The iItem member of pItem is only valid if the icon or first-column label has been clicked
	int nRow = pItem->iItem;
	int nCol = pItem->iSubItem;

	// Toggle checkbox for virtual list with checkbox style
	if (GetStyle() & LVS_OWNERDATA && GetExtendedStyle() & LVS_EX_CHECKBOXES)
	{
		// Verify that the checkbox-area was clicked
		CellHitTest(pItem->ptAction, nRow, nCol);
		if (nRow!=-1 && nCol==0)
		{
			// Checkbox area is between the item-bounds and the item-icon
			CRect iconRect, itemRect;
			VERIFY( GetCellRect(nRow, nCol, LVIR_ICON, iconRect) );
			VERIFY( GetCellRect(nRow, nCol, LVIR_BOUNDS, itemRect) );
			CRect checkboxRect(itemRect.left, itemRect.top, iconRect.left, itemRect.bottom);
			if (checkboxRect.PtInRect(pItem->ptAction))
				OnOwnerDataToggleCheckBox(nRow);
		}
	}

	CellHitTest(pItem->ptAction, nRow, nCol);

	return FALSE;	// Let parent-dialog get chance
}

//------------------------------------------------------------------------
//! NM_DBLCLK message handler called when double-clicking in a cell.
//! Just to show how to catch the double click event.
//!
//! @param pNMHDR Pointer to NMITEMACTIVATE structure
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlEx::OnItemDblClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMITEMACTIVATE* pItem = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);

	// The iItem member of pItem is only valid if the icon or first-column label has been clicked
	int nRow = pItem->iItem;
	int nCol = pItem->iSubItem;

	CellHitTest(pItem->ptAction, nRow, nCol);
	return FALSE;	// Let parent-dialog get chance
}

//------------------------------------------------------------------------
//! WM_HSCROLL message handler called when scrolling in the list control.
//! Used to ensure that any cell value editing is completed.
//!
//! @param nSBCode Specifies a scroll-bar code that indicates the user's scrolling request
//! @param nPos Specifies the scroll-box position if the scroll-bar code is SB_THUMBPOSITION or SB_THUMBTRACK (Can be negative)
//! @param pScrollBar If the scroll message came from a scroll-bar control, contains a pointer to the control
//------------------------------------------------------------------------
void CGridListCtrlEx::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing
	
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);

	if (!UsingVisualStyle())
	{
		// Only when using the mouse to scroll
		if ((::GetKeyState(VK_LBUTTON) & 0x8000)!=0)
		{
			// Fix CListCtrl grid drawing bug where vertical grid-border disappears
			//	- To reproduce the bug one needs atleast 2 columns:
			//		1) Resize the second column so a scrollbar appears
			//		2) Scroll to the right so the first column disappear
			//		3) When scrolling slowly to the left, the right border of first column is not drawn
			if (GetExtendedStyle() & LVS_EX_GRIDLINES)
			{
				Invalidate(FALSE);
				UpdateWindow();
			}
		}
	}
}

//------------------------------------------------------------------------
//! WM_VSCROLL message handler called when scrolling in the list control.
//! Used to ensure that any cell value editing is completed.
//!
//! @param nSBCode Specifies a scroll-bar code that indicates the user's scrolling request
//! @param nPos Specifies the scroll-box position if the scroll-bar code is SB_THUMBPOSITION or SB_THUMBTRACK (Can be negative)
//! @param pScrollBar If the scroll message came from a scroll-bar control, contains a pointer to the control
//------------------------------------------------------------------------
void CGridListCtrlEx::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing

	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

	if (!UsingVisualStyle())
	{
		// Only when using the mouse to scroll
		if ((::GetKeyState(VK_LBUTTON) & 0x8000)!=0)
		{
			// Fix bug where it doesn't erase the background properly
			if (GetExtendedStyle() & LVS_EX_GRIDLINES)
			{
				Invalidate(FALSE);
				UpdateWindow();
			}
		}
	}
}

//------------------------------------------------------------------------
//! Update the markup text displayed when the list control is empty
//!
//! @param strText Text to display when list control is empty
//------------------------------------------------------------------------
void CGridListCtrlEx::SetEmptyMarkupText(const CString& strText)
{
	m_EmptyMarkupText = strText;
}

//------------------------------------------------------------------------
//! WM_PAINT message handler called when needing to redraw list control.
//! Used to display text when the list control is empty
//------------------------------------------------------------------------
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
