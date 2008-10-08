#include "stdafx.h"
#include "CGridColumnTraitDateTime.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
// CGridColumnTraitDateTime
//------------------------------------------------------------------------
CGridColumnTraitDateTime::CGridColumnTraitDateTime()
	:m_ParseDateTimeFlags(0)
	,m_ParseDateTimeLCID(LANG_USER_DEFAULT)
	,m_DateTimeCtrlStyle(0)
{}

void CGridColumnTraitDateTime::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

void CGridColumnTraitDateTime::SetFormat(const CString& format)
{
	m_Format = format;
}

void CGridColumnTraitDateTime::SetParseDateTime(DWORD dwFlags, LCID lcid)
{
	m_ParseDateTimeFlags = dwFlags;
	m_ParseDateTimeLCID = lcid;
}

void CGridColumnTraitDateTime::SetStyle(DWORD dwStyle)
{
	m_DateTimeCtrlStyle = dwStyle;
}

CDateTimeCtrl* CGridColumnTraitDateTime::CreateDateTimeCtrl(CGridListCtrlEx& owner, int nRow, int nCol, const CRect& rect)
{
	// Get the text-style of the cell to edit
	DWORD dwStyle = m_DateTimeCtrlStyle;
	HDITEM hd = {0};
	hd.mask = HDI_FORMAT;
	VERIFY( owner.GetHeaderCtrl()->GetItem(nCol, &hd) );
	if (hd.fmt & HDF_RIGHT)
		dwStyle = DTS_RIGHTALIGN;

	// Create control to edit the cell
	CDateTimeCtrl* pDateTimeCtrl = new CGridEditorDateTimeCtrl(nRow, nCol, m_Format, m_ParseDateTimeFlags, m_ParseDateTimeLCID);
	VERIFY( pDateTimeCtrl->Create(WS_CHILD | dwStyle, rect, &owner, 0) );

	// Configure font
	pDateTimeCtrl->SetFont(owner.GetCellFont());

	return pDateTimeCtrl;
}

CWnd* CGridColumnTraitDateTime::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	// Convert cell-text to date/time format
	CString cellText = owner.GetItemText(nRow, nCol);
	COleDateTime dt;
	if(dt.ParseDateTime(cellText, m_ParseDateTimeFlags, m_ParseDateTimeLCID)==FALSE)
		return NULL;

	// Get position of the cell to edit
	CRect rcItem = GetCellEditRect(owner, nRow, nCol);

	// Create control to edit the cell
	CDateTimeCtrl* pDateTimeCtrl = CreateDateTimeCtrl(owner, nRow, nCol, rcItem);
	VERIFY(pDateTimeCtrl!=NULL);

	pDateTimeCtrl->SetTime(dt);

	return pDateTimeCtrl;
}

//------------------------------------------------------------------------
//! CGridEditorDateTimeCtrl (For internal use)
//------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CGridEditorDateTimeCtrl, CDateTimeCtrl)
	//{{AFX_MSG_MAP(CGridEditorDateTimeCtrl)
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CGridEditorDateTimeCtrl::CGridEditorDateTimeCtrl(int nRow, int nCol, const CString& format, DWORD formatFlags, LCID lcid)
	:m_Row(nRow)
	,m_Col(nCol)
	,m_Format(format)
	,m_FormatFlags(formatFlags)
	,m_FormatLCID(lcid)
	,m_Completed(false)
{}

void CGridEditorDateTimeCtrl::EndEdit(bool bSuccess)
{
	// Avoid two messages if key-press is followed by kill-focus
	if (m_Completed)
		return;

	m_Completed = true;

	// Format time back to string
	COleDateTime dt;
	GetTime(dt);
	CString str;
	if (m_Format.IsEmpty())
		str = dt.Format(m_FormatFlags, m_FormatLCID);
	else
		str = dt.Format(static_cast<LPCTSTR>(m_Format));

	// Send Notification to parent of ListView ctrl
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

void CGridEditorDateTimeCtrl::OnKillFocus(CWnd *pNewWnd)
{
	EndEdit(true);
}

void CGridEditorDateTimeCtrl::OnNcDestroy()
{
	CDateTimeCtrl::OnNcDestroy();
	delete this;
}

BOOL CGridEditorDateTimeCtrl::PreTranslateMessage(MSG* pMSG)
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
	return CDateTimeCtrl::PreTranslateMessage(pMSG);
}
