#include "stdafx.h"
#include "CGridColumnTraitCombo.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

namespace {
	/////////////////////////////////////////////////////////////////////////////
	// CComboEdit window
	class CComboEdit : public CEdit
	{
	// Implementation
	protected:
		
		// Stoopid win95 accelerator key problem workaround - Matt Weagle.
		virtual BOOL PreTranslateMessage(MSG* pMsg)
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

		afx_msg void CComboEdit::OnKillFocus(CWnd* pNewWnd) 
		{
			CEdit::OnKillFocus(pNewWnd);

			CWnd* pOwner = GetOwner();
			if (pOwner && pOwner!=pNewWnd)
				pOwner->SendMessage(WM_KEYUP, VK_RETURN, 0 + (((DWORD)0)<<16));
		}

		afx_msg void CComboEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
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

		DECLARE_MESSAGE_MAP()
	};

	BEGIN_MESSAGE_MAP(CComboEdit, CEdit)
		//{{AFX_MSG_MAP(CComboEdit)
		ON_WM_KILLFOCUS()
		ON_WM_KEYDOWN()
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()


	/////////////////////////////////////////////////////////////////////////////
	// CInPlaceList window
	class CInPlaceList : public CComboBox
	{
	public:
		CInPlaceList(int nRow, int nCol)
			:m_Row(nRow)
			,m_Col(nCol)
			,m_Completed(false)
		{}

		BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
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
				dispinfo.item.mask = LVIF_TEXT | LVIF_PARAM;
				dispinfo.item.pszText = str.GetBuffer();
				dispinfo.item.cchTextMax = str.GetLength();
				dispinfo.item.lParam = GetItemData(GetCurSel());
			}
			ShowWindow(SW_HIDE);
			GetParent()->GetParent()->SendMessage( WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo );
			PostMessage(WM_CLOSE);
		}

	protected:
		afx_msg void OnKillFocus(CWnd* pNewWnd)
		{
			CComboBox::OnKillFocus(pNewWnd);

			if (this == pNewWnd)
				return;

			if (&m_Edit==pNewWnd)
				return;

			EndEdit(true);
		}

		afx_msg void OnNcDestroy()
		{
			CComboBox::OnNcDestroy();
			delete this;
		}

		afx_msg void OnDestroy()
		{
			if (!m_Completed)
				EndEdit(false);

			if (m_Edit.GetSafeHwnd() != NULL)
				m_Edit.UnsubclassWindow();

			CComboBox::OnDestroy();
		}

		afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
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

		afx_msg UINT OnGetDlgCode() { return DLGC_WANTALLKEYS; }

		DECLARE_MESSAGE_MAP()

	private:
		CComboEdit m_Edit;  // subclassed edit control
		bool	m_Completed;
		int		m_Row;
		int		m_Col;
	};

	BEGIN_MESSAGE_MAP(CInPlaceList, CComboBox)
		//{{AFX_MSG_MAP(CInPlaceList)
		ON_WM_KILLFOCUS()
		ON_WM_GETDLGCODE()
		ON_WM_DESTROY()
		ON_WM_KEYUP()
		ON_WM_NCDESTROY()
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()
}

CGridColumnTraitCombo::CGridColumnTraitCombo()
	:m_pComboBox(NULL)
{
}

void CGridColumnTraitCombo::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
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

CWnd* CGridColumnTraitCombo::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
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

	// Expand the size of the ComboBox according to 9 elements
	CRect rcFinalSize = rcItem;
	rcFinalSize.bottom += rcItem.Height() + requiredHeight * 9;

	// Create edit control to edit the cell
	//	- Stores the pointer, so elements can be dynamically added laters
	m_pComboBox = new CInPlaceList(nRow, nCol);
	VERIFY( m_pComboBox->Create( WS_CHILD|WS_VSCROLL|CBS_AUTOHSCROLL|CBS_DROPDOWN, rcFinalSize, &owner, 0) );

	// Configure font
	m_pComboBox->SetFont(owner.GetCellFont(), FALSE);

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
	rcFinalSize.bottom += rcItem.Height() + requiredHeight * min(9, m_pComboBox->GetCount() + 1);
	m_pComboBox->SetWindowPos(NULL,		// not relative to any other windows
							0, 0,		// TopLeft corner doesn't change
							rcFinalSize.Width(), rcFinalSize.Height(),   // existing width, new height
							SWP_NOMOVE | SWP_NOZORDER	// don't move box or change z-ordering.
							);

	// Adjust the item-height to font-height
	CRect comboRect;
	m_pComboBox->GetClientRect(&comboRect);
	int itemHeight = max(requiredHeight + 2*::GetSystemMetrics(SM_CXEDGE), rcItem.Height());
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
	m_pComboBox = NULL;
}
