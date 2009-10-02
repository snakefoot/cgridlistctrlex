#include "stdafx.h"
#include "CGridColumnTraitCheckBox.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
//! CGridColumnTraitCheckBox - Constructor
//------------------------------------------------------------------------
CGridColumnTraitCheckBox::CGridColumnTraitCheckBox()
	:m_pOldImageList(NULL)
{
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
		m_ImageList.Add(pStateList->ExtractIcon(0));
		m_ImageList.Add(pStateList->ExtractIcon(1));
	}
	if (createdStateImages)
		owner.SetExtendedStyle(owner.GetExtendedStyle() & ~LVS_EX_CHECKBOXES);
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
			// Remove the selection color for the focus cell, to make it easier to see focus
			m_pOldImageList = owner.SetImageList(&m_ImageList, LVSIL_SMALL);
			*pResult |= CDRF_NOTIFYPOSTPAINT;	// We need to restore the original imagelist
		} break;

		// After painting a cell
		case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
		{
			if (m_pOldImageList!=NULL)
			{
				// Restore the original font
				owner.SetImageList(m_pOldImageList, LVSIL_SMALL);
			}
		} break;
	}

	CGridColumnTraitText::OnCustomDraw(owner, pLVCD, pResult);
}

