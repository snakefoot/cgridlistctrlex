#include "stdafx.h"
#include "CGridColumnTraitEdit.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
// CGridColumnTraitEdit
//------------------------------------------------------------------------
CGridColumnTraitEdit::CGridColumnTraitEdit()
	:m_EditStyle(ES_AUTOHSCROLL | ES_NOHIDESEL | WS_BORDER)
{
}

void CGridColumnTraitEdit::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

CEdit* CGridColumnTraitEdit::CreateEdit(CGridListCtrlEx& owner, int nRow, int nCol, const CRect& rect)
{
	// Get the text-style of the cell to edit
	DWORD dwStyle = m_EditStyle;
	HDITEM hd = {0};
	hd.mask = HDI_FORMAT;
	VERIFY( owner.GetHeaderCtrl()->GetItem(nCol, &hd) );
	if (hd.fmt & HDF_CENTER)
		dwStyle |= ES_CENTER;
	else if (hd.fmt & HDF_RIGHT)
		dwStyle |= ES_RIGHT;
	else
		dwStyle |= ES_LEFT;

	CEdit* pEdit = new CGridEditorText(nRow, nCol);
	VERIFY( pEdit->Create( WS_CHILD | dwStyle, rect, &owner, 0) );

	// Configure font
	pEdit->SetFont(owner.GetCellFont());

	// First item (Label) doesn't have a margin (Subitems does)
	if (nCol==0)
		pEdit->SetMargins(0, 0);
	else
		pEdit->SetMargins(4, 0);

	return pEdit;
}

CWnd* CGridColumnTraitEdit::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	// Get position of the cell to edit
	CRect rcItem = GetCellEditRect(owner, nRow, nCol);

	// Create edit control to edit the cell
	CEdit* pEdit = CreateEdit(owner, nRow, nCol, rcItem);
	VERIFY(pEdit!=NULL);

	pEdit->SetWindowText(owner.GetItemText(nRow, nCol));
	pEdit->SetSel(0, -1, 0);

	return pEdit;
}

//------------------------------------------------------------------------
// CGridEditorText (For internal use)
//------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CGridEditorText, CEdit)
	//{{AFX_MSG_MAP(CGridEditorText)
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CGridEditorText::CGridEditorText(int nRow, int nCol)
	:m_Row(nRow)
	,m_Col(nCol)
	,m_Completed(false)
{}

void CGridEditorText::EndEdit(bool bSuccess)
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
		dispinfo.item.mask = LVIF_TEXT;
		dispinfo.item.pszText = str.GetBuffer(0);
		dispinfo.item.cchTextMax = str.GetLength();
	}
	ShowWindow(SW_HIDE);
	GetParent()->GetParent()->SendMessage( WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo );
	PostMessage(WM_CLOSE);
}

void CGridEditorText::OnKillFocus(CWnd *pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	EndEdit(true);
}

void CGridEditorText::OnNcDestroy()
{
	CEdit::OnNcDestroy();
	delete this;
}

BOOL CGridEditorText::PreTranslateMessage(MSG* pMSG)
{
	switch(pMSG->message)
	{
		case WM_KEYDOWN:
		{
			switch(pMSG->wParam)
			{
				case VK_RETURN: EndEdit(true); return TRUE;
				case VK_TAB: EndEdit(true); return FALSE;
				case VK_ESCAPE: EndEdit(false);return TRUE;
			}
			break;
		};
		case WM_MOUSEWHEEL: EndEdit(true); return FALSE;	// Don't steal event
	}
	return CEdit::PreTranslateMessage(pMSG);
}
