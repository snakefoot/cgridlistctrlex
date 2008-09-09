#include "stdafx.h"
#include "CGridColumnTraitEdit.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

namespace {
	// Posts end-label notification when finished
	class CInPlaceEdit : public CEdit
	{
	public:
		CInPlaceEdit(int nRow, int nCol)
			:m_Row(nRow)
			,m_Col(nCol)
			,m_Completed(false)
		{}

		void EndEdit(bool bSuccess)
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
				dispinfo.item.pszText = str.GetBuffer();
				dispinfo.item.cchTextMax = str.GetLength();
			}
			ShowWindow(SW_HIDE);
			GetParent()->GetParent()->SendMessage( WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo );
			PostMessage(WM_CLOSE);
		}

		void OnKillFocus(CWnd *pNewWnd)
		{
			EndEdit(true);
		}

		void OnNcDestroy()
		{
			CEdit::OnNcDestroy();
			delete this;
		}
		
		BOOL PreTranslateMessage(MSG* pMSG)
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

	private:
		int m_Row;
		int m_Col;
		bool m_Completed;

		DECLARE_MESSAGE_MAP()
	};

	BEGIN_MESSAGE_MAP(CInPlaceEdit, CEdit)
		//{{AFX_MSG_MAP(CInPlaceEdit)
		ON_WM_KILLFOCUS()
		ON_WM_NCDESTROY()
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()
}

void CGridColumnTraitEdit::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//-----------------------------------------------------------------------------
namespace {
	int GetEditFontHeight(CGridListCtrlEx& owner)
	{
		const CString testText = _T("yjpÍÁ");

		CRect rcRequired = CRect(0,0,0,0);

		CClientDC dc(&owner);
		dc.SelectObject(owner.GetCellFont());
		dc.DrawText(testText, &rcRequired, DT_CALCRECT|DT_SINGLELINE);

		return rcRequired.Height();
	}
}

CWnd* CGridColumnTraitEdit::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	// Get the text-style of the cell to edit
	DWORD dwStyle = 0;
	HDITEM hd = {0};
	hd.mask = HDI_FORMAT;
	VERIFY( owner.GetHeaderCtrl()->GetItem(nCol, &hd) );
	if (hd.fmt & HDF_CENTER)
		dwStyle = ES_CENTER;
	else if (hd.fmt & HDF_RIGHT)
		dwStyle = ES_RIGHT;
	else
		dwStyle = ES_LEFT;

	// Get position of the cell to edit
	CRect rcItem;
	VERIFY( owner.GetCellRect(nRow, nCol, LVIR_LABEL, rcItem) );

	// Adjust position to font height
	int requiredHeight = GetEditFontHeight(owner);
	if (!owner.UsingVisualStyle())
	{
		if ((requiredHeight + 2*::GetSystemMetrics(SM_CXEDGE)) > rcItem.Height())
		{
			rcItem.top -= ::GetSystemMetrics(SM_CXEDGE);
			rcItem.bottom += ::GetSystemMetrics(SM_CXEDGE);
		}
	}
	if (owner.GetExtendedStyle() & LVS_EX_GRIDLINES)
	{
		if ((requiredHeight + 2*::GetSystemMetrics(SM_CXEDGE) + ::GetSystemMetrics(SM_CXBORDER)) < rcItem.Height())
			rcItem.bottom -= ::GetSystemMetrics(SM_CXBORDER);
	}

	// Create edit control to edit the cell
	CEdit* pEdit = new CInPlaceEdit(nRow, nCol);
	VERIFY( pEdit->CreateEx( 0, _T ("EDIT"), _T(""), ES_AUTOHSCROLL | ES_NOHIDESEL | WS_CHILD | WS_BORDER | dwStyle, rcItem, &owner, 0) );

	// Configure font
	pEdit->SetFont(owner.GetCellFont(), TRUE);

	// First item (Label) doesn't have a margin (Subitems does)
	if (nCol==0)
		pEdit->SetMargins(0, 0);
	else
		pEdit->SetMargins(4, 0);

	pEdit->SetWindowText(owner.GetItemText(nRow, nCol));
	pEdit->SetSel(0, -1, 0);

	return pEdit;
}
