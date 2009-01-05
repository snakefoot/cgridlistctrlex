#include "stdafx.h"
#include "CGridColumnTraitCombo.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
// CGridColumnTraitCombo
//------------------------------------------------------------------------
CGridColumnTraitCombo::CGridColumnTraitCombo()
	:m_MaxItems(7)
	,m_MaxWidth(200)
	,m_ComboBoxStyle(WS_VSCROLL | WS_HSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL | CBS_NOINTEGRALHEIGHT)
	,m_pComboBox(NULL)
{}

void CGridColumnTraitCombo::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

void CGridColumnTraitCombo::SetMaxItems(int items)
{
	m_MaxItems = items;
}

int CGridColumnTraitCombo::GetMaxItems() const
{
	return m_MaxItems;
}

void CGridColumnTraitCombo::SetMaxWidth(int width)
{
	m_MaxWidth = width;
}

int CGridColumnTraitCombo::GetMaxWidth() const
{
	return m_MaxWidth;
}

void CGridColumnTraitCombo::SetStyle(DWORD dwStyle)
{
	m_ComboBoxStyle = dwStyle;
}

DWORD CGridColumnTraitCombo::GetStyle() const
{
	return m_ComboBoxStyle;
}

CComboBox* CGridColumnTraitCombo::CreateComboBox(CGridListCtrlEx& owner, int nRow, int nCol, const CRect& rect)
{
	CGridEditorComboBox* pComboBox = new CGridEditorComboBox(nRow, nCol, m_MaxWidth);
	VERIFY( pComboBox->Create( WS_CHILD | m_ComboBoxStyle, rect, &owner, 0) );

	// Configure font
	pComboBox->SetFont(owner.GetCellFont());
	return pComboBox;
}

CWnd* CGridColumnTraitCombo::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	// Get position of the cell to edit
	CRect rcItem = GetCellEditRect(owner, nRow, nCol);
	int requiredHeight = GetCellFontHeight(owner);

	// Expand the size of the ComboBox according to max-elements
	CRect rcFinalSize = rcItem;
	rcFinalSize.bottom += rcItem.Height() + requiredHeight * m_MaxItems;

	// Create edit control to edit the cell
	//	- Stores the pointer, so elements can be dynamically added later
	m_pComboBox = CreateComboBox(owner, nRow, nCol, rcFinalSize);
	VERIFY(m_pComboBox!=NULL);

	// Add all items to list
	if (m_ComboList.GetSize()>0)
	{
		LoadList(m_ComboList, -1);

		// Guess the currently selected item in the list
		CString item = owner.GetItemText(nRow, nCol);
		int nCurSel = m_pComboBox->FindString(-1, item);
		if (nCurSel!=-1)
			m_pComboBox->SetCurSel(nCurSel);
		else
			m_pComboBox->SetWindowText(item);
	}
	else
	{
		CString item = owner.GetItemText(nRow, nCol);
		m_pComboBox->SetWindowText(item);
	}

	// Resize combobox according to element count
	VERIFY( owner.GetCellRect(nRow, nCol, LVIR_LABEL, rcFinalSize) );
	int visibleItemCount = 	m_MaxItems < m_pComboBox->GetCount() ? m_MaxItems : m_pComboBox->GetCount(); // min(m_MaxItems, m_pComboBox->GetCount());
	rcFinalSize.bottom += rcItem.Height() + requiredHeight * (visibleItemCount + 1);
	m_pComboBox->SetWindowPos(NULL,		// not relative to any other windows
							0, 0,		// TopLeft corner doesn't change
							rcFinalSize.Width(), rcFinalSize.Height(),   // existing width, new height
							SWP_NOMOVE | SWP_NOZORDER	// don't move box or change z-ordering.
							);

	// Adjust the item-height to font-height
	CRect comboRect;
	m_pComboBox->GetClientRect(&comboRect);
	int fontHeightWithMargin = requiredHeight + 2*::GetSystemMetrics(SM_CXEDGE);
	int itemHeight = fontHeightWithMargin > rcItem.Height() ? fontHeightWithMargin : rcItem.Height(); // max(fontHeightWithMargin, rcItem.Height());
	if (owner.GetExtendedStyle() & LVS_EX_GRIDLINES)
	{
		if (itemHeight > (requiredHeight + 2*::GetSystemMetrics(SM_CXEDGE) + ::GetSystemMetrics(SM_CXBORDER)))
			itemHeight -= ::GetSystemMetrics(SM_CXBORDER);
	}
	m_pComboBox->SetItemHeight(-1, itemHeight - 2*::GetSystemMetrics(SM_CXEDGE));

	return m_pComboBox;
}

void CGridColumnTraitCombo::OnEditEnd()
{
	m_pComboBox = NULL;		// CGridEditorComboBoxEdit automatically deletes itself
}

void CGridColumnTraitCombo::AddItem(int itemData, const CString& itemText)
{
	m_ComboList.Add(itemData, itemText);
}

void CGridColumnTraitCombo::LoadList(const CSimpleMap<int,CString>& comboList, int nCurSel)
{
	VERIFY(m_pComboBox!=NULL);

	for(int i = 0; i < comboList.GetSize(); ++i)
	{
		int nIndex = m_pComboBox->AddString(comboList.GetValueAt(i));
		m_pComboBox->SetItemData(nIndex, comboList.GetKeyAt(i));
	}
	if (nCurSel!=-1)
		m_pComboBox->SetCurSel(nCurSel);
}


//------------------------------------------------------------------------
// CGridEditorComboBoxEdit (For internal use)
//
// Taken from "MFC Grid control" (CComboEdit). Credits Chris Maunder
// http://www.codeproject.com/KB/miscctrl/gridctrl.aspx
//------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CGridEditorComboBoxEdit, CEdit)
	//{{AFX_MSG_MAP(CGridEditorComboBoxEdit)
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Stoopid win95 accelerator key problem workaround - Matt Weagle.
BOOL CGridEditorComboBoxEdit::PreTranslateMessage(MSG* pMsg)
{
	// Make sure that the keystrokes continue to the appropriate handlers
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
	{
		::TranslateMessage(pMsg);
		::DispatchMessage(pMsg);
		return TRUE;
	}	

	// Catch the Alt key so we don't choke if focus is going to an owner drawn button
	if (pMsg->message == WM_SYSCHAR)
		return TRUE;

	return CEdit::PreTranslateMessage(pMsg);
}

void CGridEditorComboBoxEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);

	CWnd* pOwner = GetOwner();
	if (pOwner && pOwner!=pNewWnd)
		pOwner->SendMessage(WM_KEYUP, VK_RETURN, 0 + (((DWORD)0)<<16));
}

void CGridEditorComboBoxEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_TAB || nChar == VK_RETURN || nChar == VK_ESCAPE)
	{
		CWnd* pOwner = GetOwner();
		if (pOwner)
			pOwner->SendMessage(WM_KEYUP, nChar, nRepCnt + (((DWORD)nFlags)<<16));
		return;
	}

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

//------------------------------------------------------------------------
// CGridEditorComboBox (For internal use)
//
// Taken from "MFC Grid control" (CInPlaceList). Credits Chris Maunder
// http://www.codeproject.com/KB/miscctrl/gridctrl.aspx
//------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CGridEditorComboBox, CComboBox)
	//{{AFX_MSG_MAP(CGridEditorComboBox)
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
	ON_WM_DESTROY()
	ON_WM_KEYUP()
	ON_WM_NCDESTROY()
	ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseUp)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CGridEditorComboBox::CGridEditorComboBox(int nRow, int nCol, int nMaxWidth)
	:m_Row(nRow)
	,m_Col(nCol)
	,m_Completed(false)
	,m_MaxWidth(nMaxWidth)
{}

BOOL CGridEditorComboBox::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	BOOL bRes = CComboBox::Create(dwStyle, rect, pParentWnd, nID);
	if (!bRes)
		return FALSE;

	// Subclass the combobox edit control if style includes CBS_DROPDOWN
	//	- Must handle the focus of the internal CEdit
	if ((dwStyle & CBS_DROPDOWN) && !((dwStyle & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST))
	{
		m_Edit.SubclassWindow(*GetWindow(GW_CHILD));
	}

	return bRes;
}

void CGridEditorComboBox::EndEdit(bool bSuccess)
{
	// Avoid two messages if key-press is followed by kill-focus
	if (m_Completed)
		return;

	m_Completed = true;

	// Send Notification to parent of ListView ctrl
	CString str;
	GetWindowText(str);

	LV_DISPINFO dispinfo = {0};
	dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;

	dispinfo.item.iItem = m_Row;
	dispinfo.item.iSubItem = m_Col;
	if (bSuccess)
	{
		dispinfo.item.mask = LVIF_TEXT | LVIF_PARAM;
		dispinfo.item.pszText = str.GetBuffer(0);
		dispinfo.item.cchTextMax = str.GetLength();
		dispinfo.item.lParam = GetItemData(GetCurSel());
	}
	ShowWindow(SW_HIDE);
	GetParent()->GetParent()->SendMessage( WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo );
	PostMessage(WM_CLOSE);
}

void CGridEditorComboBox::OnKillFocus(CWnd* pNewWnd)
{
	CComboBox::OnKillFocus(pNewWnd);

	if (this == pNewWnd)
		return;

	if (&m_Edit==pNewWnd)
		return;

	EndEdit(true);
}

void CGridEditorComboBox::OnNcDestroy()
{
	CComboBox::OnNcDestroy();
	delete this;
}

void CGridEditorComboBox::OnDestroy()
{
	if (!m_Completed)
		EndEdit(false);

	if (m_Edit.GetSafeHwnd() != NULL)
		m_Edit.UnsubclassWindow();

	CComboBox::OnDestroy();
}

void CGridEditorComboBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_TAB || nChar == VK_RETURN)
	{
		EndEdit(true);
		return;
	}
	else
	if (nChar == VK_ESCAPE)
	{
		EndEdit(false);
		return;
	}

	CComboBox::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CGridEditorComboBox::OnDropDown()
{
	int itemHeight = GetItemHeight(-1);

	// Resize combo-box width to fit contents
	int nNumEntries = GetCount();
	int nWidth = 0;
	CString str;

	CClientDC dc(this);
	int nSave = dc.SaveDC();
	dc.SelectObject(GetFont());

	for (int i = 0; i < nNumEntries; i++)
	{
		GetLBText(i, str);
		int nLength = dc.GetTextExtent(str).cx;
		nWidth = nWidth > nLength ? nWidth : nLength;	// max(nWidth, nLength);
		if (nWidth > m_MaxWidth)
		{
			nWidth = m_MaxWidth;
			break;
		}
	}

	// check if the current height is large enough for the items in the list
	CRect rect;
	GetDroppedControlRect(&rect);
	if (rect.Height() <= nNumEntries*GetItemHeight(0))
		nWidth +=::GetSystemMetrics(SM_CXVSCROLL);

	// Add margin space to the calculations
	nWidth += dc.GetTextExtent(_T("0")).cx;

	dc.RestoreDC(nSave);
	SetDroppedWidth(nWidth);
	SetItemHeight(-1, itemHeight);
}

void CGridEditorComboBox::OnCloseUp()
{
}

UINT CGridEditorComboBox::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}
