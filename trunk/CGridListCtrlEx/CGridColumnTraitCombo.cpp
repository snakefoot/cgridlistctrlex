#include "stdafx.h"
#include "CGridColumnTraitCombo.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

namespace {
	// Posts end-label notification when finished
	class CInPlaceList : public CComboBox
	{
	public:
		CInPlaceList(int nRow, int nCol)
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
				dispinfo.item.mask = LVIF_TEXT | LVIF_PARAM;
				dispinfo.item.pszText = str.GetBuffer();
				dispinfo.item.cchTextMax = str.GetLength();
				dispinfo.item.lParam = GetItemData(GetCurSel());
			}
			ShowWindow(SW_HIDE);
			GetParent()->GetParent()->SendMessage( WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo );
			PostMessage(WM_CLOSE);
		}

		void OnKillFocus(CWnd *pNewWnd)
		{
			EndEdit(true);
		}

		void OnCloseUp()
		{
			EndEdit(true);
		}

		void OnNcDestroy()
		{
			CComboBox::OnNcDestroy();
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
				}
			}
			return CComboBox::PreTranslateMessage(pMSG);
		}

	private:
		int m_Row;
		int m_Col;
		bool m_Completed;

		DECLARE_MESSAGE_MAP()
	};

	BEGIN_MESSAGE_MAP(CInPlaceList, CEdit)
		//{{AFX_MSG_MAP(CInPlaceList)
		ON_WM_KILLFOCUS()
		ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseUp)
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

CWnd* CGridColumnTraitCombo::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	CRect rcItem;
	VERIFY( owner.GetCellRect(nRow, nCol, LVIR_LABEL, rcItem) );

	// Expand the size of the ComboBox according to 9 elements
	rcItem.bottom += (rcItem.bottom - rcItem.top) * 9;
	rcItem.top -= GetSystemMetrics(SM_CXEDGE); // Because of ComboBox 3D Style

	// Create edit control to edit the cell
	//	- Stores the pointer, so elements can be dynamically added laters
	m_pComboBox = new CInPlaceList(nRow, nCol);
	VERIFY( m_pComboBox->Create( WS_CHILD |WS_VSCROLL|WS_HSCROLL | CBS_SIMPLE |CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | CBS_NOINTEGRALHEIGHT, rcItem, &owner, 0) );

	// Configure font
	m_pComboBox->SetFont(owner.GetFont(), FALSE);

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
	CRect rcFinalSize;
	VERIFY( owner.GetCellRect(nRow, nCol, LVIR_LABEL, rcFinalSize) );
	rcFinalSize.bottom += (rcFinalSize.bottom - rcFinalSize.top) * min(9, m_pComboBox->GetCount() + 1);
	m_pComboBox->SetWindowPos(NULL,		// not relative to any other windows
							0, 0,		// TopLeft corner doesn't change
							rcFinalSize.Width(), rcFinalSize.bottom - rcFinalSize.top,   // existing width, new height
							SWP_NOMOVE | SWP_NOZORDER	// don't move box or change z-ordering.
							);
	return m_pComboBox;
}

void CGridColumnTraitCombo::OnEditEnd()
{
	m_pComboBox = NULL;
}