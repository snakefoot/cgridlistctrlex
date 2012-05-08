//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "stdafx.h"
#pragma warning(disable:4100)	// unreferenced formal parameter

#include "CGridColumnTraitEdit.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
//! CGridColumnTraitEdit - Constructor
//------------------------------------------------------------------------
CGridColumnTraitEdit::CGridColumnTraitEdit()
	:m_EditStyle(ES_AUTOHSCROLL | ES_NOHIDESEL | WS_BORDER)
	,m_EditLimitText(UINT_MAX)
	,m_EditMaxLines(0)
{
}

//------------------------------------------------------------------------
//! Accept Visitor Pattern
//------------------------------------------------------------------------
void CGridColumnTraitEdit::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//------------------------------------------------------------------------
//! Set style used when creating CEdit for cell value editing
//!
//! @param dwStyle Style flags
//------------------------------------------------------------------------
void CGridColumnTraitEdit::SetStyle(DWORD dwStyle)
{
	m_EditStyle = dwStyle;
}

//------------------------------------------------------------------------
//! Get style used when creating CEdit for cell value editing
//------------------------------------------------------------------------
DWORD CGridColumnTraitEdit::GetStyle() const
{
	return m_EditStyle;
}

//------------------------------------------------------------------------
//! Set max number of characters the CEdit will accept
//!
//! @param nMaxChars The text limit, in characters.
//------------------------------------------------------------------------
void CGridColumnTraitEdit::SetLimitText(UINT nMaxChars)
{
	m_EditLimitText = nMaxChars;
}

//------------------------------------------------------------------------
//! Get max number of characters the CEdit will accept
//------------------------------------------------------------------------
UINT CGridColumnTraitEdit::GetLimitText() const
{
	return m_EditLimitText;
}

//------------------------------------------------------------------------
//! Set max number of lines that can the CEdit will display at a time
//!	For multiline editing then add these styles ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL
//!
//! @param nMaxLines The text limit, in lines.
//------------------------------------------------------------------------
void CGridColumnTraitEdit::SetMaxLines(UINT nMaxLines)
{
	m_EditMaxLines = nMaxLines;
}

//------------------------------------------------------------------------
//! Get max number of lines that can the CEdit will display at a time
//------------------------------------------------------------------------
UINT CGridColumnTraitEdit::GetMaxLines() const
{
	return m_EditMaxLines;
}

namespace
{
	int CharacterCount(const CString& csHaystack, LPCTSTR sNeedle)
	{
		if (csHaystack.IsEmpty())
			return 0;

		int nFind = -1;
		int nCount = 0;
		do
		{
			nCount++;
			nFind = csHaystack.Find( sNeedle, nFind + 1 );
		} while (nFind != -1);
		
		return nCount-1;
	}
}

//------------------------------------------------------------------------
//! Create a CEdit as cell value editor
//!
//! @param owner The list control starting a cell edit
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param rect The rectangle where the inplace cell value editor should be placed
//! @param cellText The text which is going to be initially displayed in the CEdit
//! @return Pointer to the cell editor to use
//------------------------------------------------------------------------
CEdit* CGridColumnTraitEdit::CreateEdit(CGridListCtrlEx& owner, int nRow, int nCol, const CRect& rect, const CString& cellText)
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

	CGridEditorText* pEdit = new CGridEditorText(nRow, nCol);

	CRect limitRect(rect);
	if (m_EditMaxLines > 1 && dwStyle & ES_MULTILINE)
	{
		// Calculate the number of lines in the cell text, expand the CEdit to match this
		int nLineHeight = GetCellFontHeight(owner);
		int nLineCount = CharacterCount(cellText, _T("\n"));
		if (nLineCount > 0)
		{
			if ((UINT)nLineCount > m_EditMaxLines-1)
				nLineCount = m_EditMaxLines-1;
			limitRect.bottom += nLineHeight*nLineCount;
		}

		pEdit->SetMaxLines(m_EditMaxLines);
		pEdit->SetLineHeight(nLineHeight);
	}

	VERIFY( pEdit->Create( WS_CHILD | dwStyle, limitRect, &owner, 0) );

	// Configure font
	pEdit->SetFont(owner.GetCellFont());

	// First item (Label) doesn't have a margin (Subitems does)
	if (nCol==0 || (hd.fmt & HDF_CENTER))
		pEdit->SetMargins(0, 0);
	else
	if (hd.fmt & HDF_RIGHT)
		pEdit->SetMargins(0, 7);
	else
		pEdit->SetMargins(4, 0);

	if (m_EditLimitText!=UINT_MAX)
		pEdit->SetLimitText(m_EditLimitText);

	pEdit->SetInitialText(cellText);

	return pEdit;
}

//------------------------------------------------------------------------
//! Overrides OnEditBegin() to provide a CEdit cell value editor
//!
//! @param owner The list control starting edit
//! @param nRow The index of the row for the cell to edit
//! @param nCol The index of the column for the cell to edit
//! @return Pointer to the cell editor to use (NULL if cell edit is not possible)
//------------------------------------------------------------------------
CWnd* CGridColumnTraitEdit::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	// Get position of the cell to edit
	CRect rectCell = GetCellEditRect(owner, nRow, nCol);

	CString cellText = owner.GetItemText(nRow, nCol);

	// Create edit control to edit the cell
	CEdit* pEdit = CreateEdit(owner, nRow, nCol, rectCell, cellText);
	VERIFY(pEdit!=NULL);
	if (pEdit==NULL)
		return NULL;

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
	ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------------------------------
//! CGridEditorText - Constructor
//------------------------------------------------------------------------
CGridEditorText::CGridEditorText(int nRow, int nCol)
	:m_Row(nRow)
	,m_Col(nCol)
	,m_Completed(false)
	,m_Modified(false)
	,m_InitialText(true)
	,m_LineHeight(0)
	,m_MaxLines(0)
{}

//------------------------------------------------------------------------
//! The CEdit will fire an EN_CHANGE event for the initial cell text.
//! The initial EN_CHANGE event should not be seen as a text modification
//!
//! @param cellText Initial CEdit text
//------------------------------------------------------------------------
void CGridEditorText::SetInitialText(const CString& cellText)
{
	SetWindowText(cellText);
	m_InitialText = false;
}

//------------------------------------------------------------------------
//! The cell value editor was closed and the entered should be saved.
//!
//! @param bSuccess Should the entered cell value be saved
//------------------------------------------------------------------------
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
	dispinfo.hdr.idFrom = (UINT_PTR)GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;

	dispinfo.item.iItem = m_Row;
	dispinfo.item.iSubItem = m_Col;
	if (bSuccess && m_Modified)
	{
		dispinfo.item.mask = LVIF_TEXT;
		dispinfo.item.pszText = str.GetBuffer(0);
		dispinfo.item.cchTextMax = str.GetLength();
	}
	ShowWindow(SW_HIDE);
	GetParent()->GetParent()->SendMessage( WM_NOTIFY, (WPARAM)GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo );
	PostMessage(WM_CLOSE);
}

//------------------------------------------------------------------------
//! WM_KILLFOCUS message handler called when CEdit is loosing focus
//! to other control. Used register that cell value editor should close.
//!
//! @param pNewWnd Pointer to the window that receives the input focus (may be NULL or may be temporary).
//------------------------------------------------------------------------
void CGridEditorText::OnKillFocus(CWnd *pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	EndEdit(true);
}

//------------------------------------------------------------------------
//! WM_NCDESTROY message handler called when CEdit window is about to
//! be destroyed. Used to delete the inplace CEdit editor object as well.
//! This is necessary when the CDateTimeCtrl is created dynamically.
//------------------------------------------------------------------------
void CGridEditorText::OnNcDestroy()
{
	CEdit::OnNcDestroy();
	delete this;
}

//------------------------------------------------------------------------
//! EN_CHANGE notification handler to monitor text modifications
//------------------------------------------------------------------------
void CGridEditorText::OnEnChange()
{
	if (!m_InitialText)
	{
		if (m_MaxLines > 1 && GetStyle() & ES_MULTILINE && m_LineHeight > 0)
		{
			// Get number of text lines
			CString cellText;
			GetWindowText(cellText);
			int nLineCount = CharacterCount(cellText, _T("\n"));
			if (nLineCount > 0)
				if ((UINT)nLineCount > m_MaxLines-1)
					nLineCount = m_MaxLines-1;

			// Check if the current rect matches the number of lines
			CRect rect;
			GetWindowRect(&rect);
			if (rect.Height() / m_LineHeight != nLineCount + 1)
			{
				rect.bottom += (nLineCount + 1 - rect.Height() / m_LineHeight) * m_LineHeight;
				GetParent()->ScreenToClient(&rect);
				MoveWindow(rect.left, rect.top, rect.Width(), rect.Height(), TRUE);
			}
		}
		m_Modified = true;
	}
}

//------------------------------------------------------------------------
//! Hook to proces windows messages before they are dispatched.
//! Catch keyboard events that can should cause the cell value editor to close
//!
//! @param pMsg Points to a MSG structure that contains the message to process
//! @return Nonzero if the message was translated and should not be dispatched; 0 if the message was not translated and should be dispatched.
//------------------------------------------------------------------------
BOOL CGridEditorText::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
		case WM_KEYDOWN:
		{
			switch(pMsg->wParam)
			{
				case VK_RETURN:
				{
					if (GetStyle() & ES_WANTRETURN)
						break;

					EndEdit(true);
					return TRUE;
				}
				case VK_TAB: EndEdit(true); return FALSE;
				case VK_ESCAPE: EndEdit(false);return TRUE;
			}
			break;
		};
		case WM_MOUSEWHEEL: EndEdit(true); return FALSE;	// Don't steal event
	}
	return CEdit::PreTranslateMessage(pMsg);
}
