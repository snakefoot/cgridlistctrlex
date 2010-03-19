#include "stdafx.h"
#pragma warning(disable:4100)	// unreferenced formal parameter

#include "CGridColumnTraitCombo.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
//! CGridColumnTraitCombo - Constructor
//------------------------------------------------------------------------
CGridColumnTraitCombo::CGridColumnTraitCombo()
	:m_MaxItems(7)
	,m_MaxWidth(200)
	,m_ComboBoxStyle(WS_VSCROLL | WS_HSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL | CBS_NOINTEGRALHEIGHT)
	,m_pComboBox(NULL)
{}

//------------------------------------------------------------------------
//! Accept Visitor Pattern
//------------------------------------------------------------------------
void CGridColumnTraitCombo::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//------------------------------------------------------------------------
//! Set max height (in items) of the CComboBox when doing dropdown
//!
//! @param nMaxItems Max number of items to show at once in the dropdown list
//------------------------------------------------------------------------
void CGridColumnTraitCombo::SetMaxItems(int nMaxItems)
{
	m_MaxItems = nMaxItems;
}

//------------------------------------------------------------------------
//! Retrieves max height (in items) of the CComboBox when doing dropdown
//!
//! @return Number of items
//------------------------------------------------------------------------
int CGridColumnTraitCombo::GetMaxItems() const
{
	return m_MaxItems;
}

//------------------------------------------------------------------------
//! Set max width (in pixels) of the CComboBox when doing dropdown
//!
//! @param nMaxWidth Max pixels in width to show when expanding the dropdown list
//------------------------------------------------------------------------
void CGridColumnTraitCombo::SetMaxWidth(int nMaxWidth)
{
	m_MaxWidth = nMaxWidth;
}

//------------------------------------------------------------------------
//! Retrieves max width (in pixels) of the CComboBox when doing dropdown
//!
//! @return Number of items
//------------------------------------------------------------------------
int CGridColumnTraitCombo::GetMaxWidth() const
{
	return m_MaxWidth;
}

//------------------------------------------------------------------------
//! Set style used when creating CComboBox for cell value editing
//!
//! @param dwStyle Style flags
//------------------------------------------------------------------------
void CGridColumnTraitCombo::SetStyle(DWORD dwStyle)
{
	m_ComboBoxStyle = dwStyle;
}

//------------------------------------------------------------------------
//! Get style used when creating CComboBox for cell value editing
//!
//! @return Style flags
//------------------------------------------------------------------------
DWORD CGridColumnTraitCombo::GetStyle() const
{
	return m_ComboBoxStyle;
}

//------------------------------------------------------------------------
//! Create a CComboBox as cell value editor
//!
//! @param owner The list control starting a cell edit
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param rect The rectangle where the inplace cell value editor should be placed
//! @return Pointer to the cell editor to use
//------------------------------------------------------------------------
CComboBox* CGridColumnTraitCombo::CreateComboBox(CGridListCtrlEx& owner, int nRow, int nCol, const CRect& rect)
{
	CGridEditorComboBox* pComboBox = new CGridEditorComboBox(nRow, nCol, m_MaxWidth);
	VERIFY( pComboBox->Create( WS_CHILD | m_ComboBoxStyle, rect, &owner, 0) );

	// Configure font
	pComboBox->SetFont(owner.GetCellFont());
	return pComboBox;
}

//------------------------------------------------------------------------
//! Overrides OnEditBegin() to provide a CComboBox cell value editor
//!
//! @param owner The list control starting edit
//! @param nRow The index of the row for the cell to edit
//! @param nCol The index of the column for the cell to edit
//! @return Pointer to the cell editor to use (NULL if cell edit is not possible)
//------------------------------------------------------------------------
CWnd* CGridColumnTraitCombo::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	// Get position of the cell to edit
	CRect rectCell = GetCellEditRect(owner, nRow, nCol);
	int requiredHeight = GetCellFontHeight(owner);

	// Expand the size of the ComboBox according to max-elements
	CRect rectExpanded(rectCell);
	rectExpanded.bottom += rectCell.Height() + requiredHeight * m_MaxItems;

	// Create edit control to edit the cell
	//	- Stores the pointer, so elements can be dynamically added later
	m_pComboBox = CreateComboBox(owner, nRow, nCol, rectExpanded);
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

	// Resize combobox according to actual element count
	int visibleItemCount = 	m_MaxItems < m_pComboBox->GetCount() ? m_MaxItems : m_pComboBox->GetCount(); // min(m_MaxItems, m_pComboBox->GetCount());
	rectExpanded.bottom += rectCell.Height() + requiredHeight * (visibleItemCount + 1);
	m_pComboBox->SetWindowPos(NULL,		// not relative to any other windows
							0, 0,		// TopLeft corner doesn't change
							rectExpanded.Width(), rectExpanded.Height(),   // existing width, new height
							SWP_NOMOVE | SWP_NOZORDER	// don't move box or change z-ordering.
							);

	// Adjust the item-height to font-height (Must be done after resizing)
	CRect rectCombo;
	m_pComboBox->GetClientRect(&rectCombo);
	if (rectCombo.Height() < rectCell.Height())
	{
		// Expand to fit cell
		int padding = rectCell.Height() - rectCombo.Height();
		m_pComboBox->SetItemHeight(-1, m_pComboBox->GetItemHeight(-1)+padding);
	}
	else
	if (rectCombo.Height() > rectCell.Height() + ::GetSystemMetrics(SM_CXBORDER))
	{
		// Compress to fit cell
		int margin = rectCombo.Height() - m_pComboBox->GetItemHeight(-1);
		int padding = margin - 2*::GetSystemMetrics(SM_CXEDGE);
		if ((m_pComboBox->GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)
			padding -= ::GetSystemMetrics(SM_CXEDGE);
		if (padding > 0)
			m_pComboBox->SetItemHeight(-1, m_pComboBox->GetItemHeight(-1)-padding);
	}
	return m_pComboBox;
}

//------------------------------------------------------------------------
//! Overrides OnEditEnd() to ensure that temporary combobox variable
//! is reset when cell value editing is completed.
//------------------------------------------------------------------------
void CGridColumnTraitCombo::OnEditEnd()
{
	m_pComboBox = NULL;		// CGridEditorComboBoxEdit automatically deletes itself
}

//------------------------------------------------------------------------
//! Adds combobox item to the fixed combobox item-list
//!
//! @param nItemData Unique identifier of the item
//! @param strItemText Text identifier of the item
//------------------------------------------------------------------------
void CGridColumnTraitCombo::AddItem(int nItemData, const CString& strItemText)
{
	m_ComboList.Add(nItemData, strItemText);
}

//------------------------------------------------------------------------
//! Fills the combobox with the items of the fixed item-list
//!
//! @param comboList List of CComboBox items
//! @param nCurSel Unique identifier of the item currently selected
//------------------------------------------------------------------------
void CGridColumnTraitCombo::LoadList(const CSimpleMap<int,CString>& comboList, int nCurSel)
{
	VERIFY(m_pComboBox!=NULL);

	m_pComboBox->SetRedraw(FALSE);
	m_pComboBox->InitStorage(comboList.GetSize(), 32);

	for(int i = 0; i < comboList.GetSize(); ++i)
	{
		int nIndex = m_pComboBox->AddString(comboList.GetValueAt(i));
		m_pComboBox->SetItemData(nIndex, comboList.GetKeyAt(i));
	}
	m_pComboBox->SetRedraw(TRUE);
	m_pComboBox->Invalidate();
	m_pComboBox->UpdateWindow();
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------------------------------
//! WM_KILLFOCUS message handler called when CEdit is loosing focus
//! to other control. Used register that cell value editor should close.
//!
//! @param pNewWnd Pointer to the window that receives the input focus (may be NULL or may be temporary).
//------------------------------------------------------------------------
void CGridEditorComboBoxEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);

	CWnd* pOwner = GetOwner();
	if (pOwner && pOwner!=pNewWnd)
		pOwner->SendMessage(WM_CHAR, VK_RETURN, 0);
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
	ON_WM_DESTROY()
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
	ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseUp)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropDown)
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnChangeSelection)
	ON_CONTROL_REFLECT(CBN_SELENDOK, OnChangeSelection)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------------------------------
//! CGridEditorComboBox - Constructor
//------------------------------------------------------------------------
CGridEditorComboBox::CGridEditorComboBox(int nRow, int nCol, int nMaxWidth)
	:m_Row(nRow)
	,m_Col(nCol)
	,m_Completed(false)
	,m_Modified(false)
	,m_MaxWidth(nMaxWidth)
{}

//------------------------------------------------------------------------
//! Creates the CComboBox control, and subclasses the internal CEdit control
//! to implement special behavior for completing cell value editing.
//!
//! @param dwStyle Specifies the style of the combo box
//! @param rect Points to the position and size of the combo box
//! @param pParentWnd pecifies the combo box's parent window. It must not be NULL.
//! @param nID Specifies the combo box's control ID.
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
//! The cell value editor was closed and the entered should be saved.
//!
//! @param bSuccess Should the entered cell value be saved
//------------------------------------------------------------------------
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
	if (bSuccess && m_Modified)
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

//------------------------------------------------------------------------
//! WM_KILLFOCUS message handler called when CComboBox is loosing focus
//! to other control. Used register that cell value editor should close.
//!
//! @param pNewWnd Pointer to the window that receives the input focus (may be NULL or may be temporary).
//------------------------------------------------------------------------
void CGridEditorComboBox::OnKillFocus(CWnd* pNewWnd)
{
	CComboBox::OnKillFocus(pNewWnd);

	if (this == pNewWnd)
		return;

	if (&m_Edit==pNewWnd)
		return;

	EndEdit(true);
}

//------------------------------------------------------------------------
//! WM_NCDESTROY message handler called when CComboBox window is about to
//! be destroyed. Used to delete the inplace CComboBox editor object as well.
//! This is necessary when the CComboBox is created dynamically.
//------------------------------------------------------------------------
void CGridEditorComboBox::OnNcDestroy()
{
	CComboBox::OnNcDestroy();
	delete this;
}

//------------------------------------------------------------------------
//! WM_DESTROY message handler called when CComboBox window is about to
//! be destroyed. Used to unsubclass the internal CEdit control.
//------------------------------------------------------------------------
void CGridEditorComboBox::OnDestroy()
{
	if (!m_Completed)
		EndEdit(false);

	if (m_Edit.GetSafeHwnd() != NULL)
		m_Edit.UnsubclassWindow();

	CComboBox::OnDestroy();
}

//------------------------------------------------------------------------
//! CBN_DROPDOWN message handler called when the CComboBox control
//! is expanded into a dropdown list. Used to restrict the width of
//! the dropdown list to the max width.
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
//! CBN_CLOSEUP message handler called when the CComboBox control
//! dropdown list is closed up.
//------------------------------------------------------------------------
void CGridEditorComboBox::OnCloseUp()
{
}

//------------------------------------------------------------------------
//! CBN_SELCHANGE, CBN_SELENDOK message handler called when the selection
//! in the CComboBox control has been modified.
//------------------------------------------------------------------------
void CGridEditorComboBox::OnChangeSelection()
{
	m_Modified = true;
}

//------------------------------------------------------------------------
//! WM_CHAR message handler to monitor selection modifications
//!
//! @param nChar Specifies the virtual key code of the given key.
//! @param nRepCnt Repeat count (the number of times the keystroke is repeated as a result of the user holding down the key).
//! @param nFlags Specifies the scan code, key-transition code, previous key state, and context code
//------------------------------------------------------------------------
void CGridEditorComboBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar==VK_RETURN)
	{
		// By default the WM_CHAR handler will not receive VK_RETURN,
		// but it is sent by the internal CEdit when it looses focus
		EndEdit(true);
		return;
	}
	m_Modified = true;
	CComboBox::OnChar(nChar, nRepCnt, nFlags);
}

//------------------------------------------------------------------------
//! Hook to proces windows messages before they are dispatched.
//! Catch keyboard events that can should cause the cell value editor to close
//!
//! @param pMsg Points to a MSG structure that contains the message to process
//! @return Nonzero if the message was translated and should not be dispatched; 0 if the message was not translated and should be dispatched.
//------------------------------------------------------------------------
BOOL CGridEditorComboBox::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
		case WM_KEYDOWN:
		{
			// We also catch the message for the internal CEdit
			switch(pMsg->wParam)
			{
				case VK_RETURN: EndEdit(true); return TRUE;
				case VK_TAB: EndEdit(true); return FALSE;
				case VK_ESCAPE: EndEdit(false);return TRUE;
			}
			break;
		};
		case WM_CHAR:
		{
			// We also catch the message for the internal CEdit
			m_Modified = true;
			break;
		}
	}
	return CComboBox::PreTranslateMessage(pMsg);
}