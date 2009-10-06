#include "stdafx.h"
#include "CGridColumnTraitCheckBox.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
//! CGridColumnTraitCheckBox - Constructor
//------------------------------------------------------------------------
CGridColumnTraitCheckBox::CGridColumnTraitCheckBox()
	:m_OldFirstIcon(NULL)
	,m_OldSecondIcon(NULL)
{
	// Checkbox should be flipped without needing cell-focus first
	m_ColumnState.m_EditFocusFirst = false;
}

//------------------------------------------------------------------------
//! Accept Visitor Pattern
//------------------------------------------------------------------------
void CGridColumnTraitCheckBox::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//------------------------------------------------------------------------
//! Setups state images to display checkboxes
//!
//! @param owner The list control adding column
//! @param nCol The index of the column just added
//------------------------------------------------------------------------
void CGridColumnTraitCheckBox::OnInsertColumn(CGridListCtrlEx& owner, int nCol)
{
	m_ImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 1, 0);
	bool createdStateImages = false;
	CImageList* pStateList = owner.GetImageList(LVSIL_STATE);
	if (pStateList==NULL)
	{
		if (!(owner.GetExtendedStyle() & LVS_EX_CHECKBOXES))
		{
			createdStateImages = true;
			owner.SetExtendedStyle(owner.GetExtendedStyle() | LVS_EX_CHECKBOXES);
			pStateList = owner.GetImageList(LVSIL_STATE);
		}
	}
	ASSERT(pStateList!=NULL);
	if (pStateList!=NULL)
	{
		HICON uncheckedIcon = pStateList->ExtractIcon(0);
		m_ImageList.Add(uncheckedIcon);
		DestroyIcon(uncheckedIcon);
		HICON checkedIcon = pStateList->ExtractIcon(1);
		m_ImageList.Add(checkedIcon);
		DestroyIcon(checkedIcon);
	}
	if (createdStateImages)
		owner.SetExtendedStyle(owner.GetExtendedStyle() & ~LVS_EX_CHECKBOXES);
}

//------------------------------------------------------------------------
//! Only flip checkbox when cell-image is clicked
//!
//! @param owner The list control being clicked
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param pt The position clicked, in client coordinates.
//------------------------------------------------------------------------
bool CGridColumnTraitCheckBox::OnClickEditStart(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt)
{
	CRect rect;
	owner.GetCellRect(nRow, nCol, LVIR_ICON, rect);
	if (!rect.PtInRect(pt))
		return false;

	return true;
}

//------------------------------------------------------------------------
//! Reacts to clicking on the checkbox image, and flips the checkbox
//!
//! @param owner The list control starting edit
//! @param nRow The index of the row for the cell to edit
//! @param nCol The index of the column for the cell to edit
//! @return Pointer to the cell editor to use (NULL if cell edit is not possible)
//------------------------------------------------------------------------
CWnd* CGridColumnTraitCheckBox::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	CPoint pt(GetMessagePos());
	owner.ScreenToClient(&pt);
	CRect rect;
	owner.GetCellRect(nRow, nCol, LVIR_ICON, rect);
	if (rect.PtInRect(pt))
	{
		// Send Notification to parent of ListView ctrl
		int image = owner.GetCellImage(nRow, nCol);

		LV_DISPINFO dispinfo = {0};
		if (image==0)
			dispinfo.item.iImage = 1;
		else
		if (image==1)
			dispinfo.item.iImage = 0;
		else
			return NULL;

		dispinfo.hdr.hwndFrom = owner.m_hWnd;
		dispinfo.hdr.idFrom = owner.GetDlgCtrlID();
		dispinfo.hdr.code = LVN_ENDLABELEDIT;

		dispinfo.item.iItem = nRow;
		dispinfo.item.iSubItem = nCol;
		dispinfo.item.mask = LVIF_IMAGE;

		owner.GetParent()->SendMessage( WM_NOTIFY, owner.GetDlgCtrlID(), (LPARAM)&dispinfo );
	}
	return NULL;	// Editor is never really started
}

//------------------------------------------------------------------------
//! Swaps in a custom state-imagelist before drawing the checkbox-cell
//!
//! @param owner The list control drawing
//! @param pLVCD Pointer to NMLVCUSTOMDRAW structure
//! @param pResult Modification to the drawing stage (CDRF_NEWFONT, etc.)
//------------------------------------------------------------------------
void CGridColumnTraitCheckBox::OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult)
{
	int nRow = (int)pLVCD->nmcd.dwItemSpec;

	switch (pLVCD->nmcd.dwDrawStage)
	{
		// Before painting a cell
		case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		{
			// Check if there is an imagelist already
			CImageList* pImageList = owner.GetImageList(LVSIL_SMALL);
			if (pImageList==NULL)
			{
				// If no default imagelist, then use our own empty list
				if (m_EmptyImageList==NULL)
					m_EmptyImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 1, 0);
				owner.SetImageList(&m_EmptyImageList,LVSIL_SMALL);
				pImageList = owner.GetImageList(LVSIL_SMALL);
			}

			// Replace the first icon, when unchecked-state icon
			m_OldFirstIcon = pImageList->ExtractIcon(0);
			HICON uncheckedIcon = m_ImageList.ExtractIcon(0);
			if (m_OldFirstIcon==NULL)
				pImageList->Add(uncheckedIcon);
			else
				pImageList->Replace(0, uncheckedIcon);
			DestroyIcon(uncheckedIcon);

			// Replace the second icon, when checked-state icon
			m_OldSecondIcon = pImageList->ExtractIcon(1);
			HICON checkedIcon = m_ImageList.ExtractIcon(1);
			if (m_OldSecondIcon==NULL)
				pImageList->Add(checkedIcon);
			else
				pImageList->Replace(1, checkedIcon);
			DestroyIcon(checkedIcon);

			*pResult |= CDRF_NOTIFYPOSTPAINT;	// We need to restore the original imagelist
		} break;

		// After painting a cell
		case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
		{
			// Restore the second icon
			CImageList* pImageList = owner.GetImageList(LVSIL_SMALL);
			if (m_OldSecondIcon==NULL)
				pImageList->Remove(1);
			else
			{
				pImageList->Replace(1, m_OldSecondIcon);
				DestroyIcon(m_OldSecondIcon);
			}
			
			// Restore the first icon
			if (m_OldFirstIcon==NULL)
				pImageList->Remove(0);
			else
			{
				pImageList->Replace(0, m_OldFirstIcon);
				DestroyIcon(m_OldFirstIcon);
			}
		} break;
	}

	CGridColumnTraitText::OnCustomDraw(owner, pLVCD, pResult);
}

