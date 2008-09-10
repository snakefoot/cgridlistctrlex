#include "stdafx.h"
#include "CGridColumnTraitText.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

CGridColumnTraitText::CGridColumnTraitText()
	:m_pOldFont(NULL)
{}

void CGridColumnTraitText::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

void CGridColumnTraitText::OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult)
{
	int nRow = (int)pLVCD->nmcd.dwItemSpec;

	switch (pLVCD->nmcd.dwDrawStage)
	{
		// Before painting a cell
		case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		{
			// Remove the selection color for the focus cell, to make it easier to see focus
			int nCol = pLVCD->iSubItem;
			if (pLVCD->nmcd.uItemState & CDIS_SELECTED && owner.GetFocusCell()==nCol && owner.GetFocusRow()==nRow)
			{
				pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;
			}

			if (owner.CallbackCellCustomColor(nRow, nCol, pLVCD->clrText, pLVCD->clrTextBk))
				*pResult |= CDRF_NEWFONT;

			LOGFONT newFont = {0};
			if (owner.CallbackCellCustomFont(nRow, nCol, newFont))
			{
				CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
				CFont* pNewFont = new CFont;
				VERIFY( pNewFont->CreateFontIndirect(&newFont) );
				m_pOldFont = pDC->SelectObject(pNewFont);
				*pResult |= CDRF_NOTIFYPOSTPAINT;	// We need to restore the original font
				*pResult |= CDRF_NEWFONT;
			}
		} break;

		// After painting a cell
		case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
		{
			if (m_pOldFont!=NULL)
			{
				// Restore the original font
				CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
				CFont* pNewFont = pDC->SelectObject(m_pOldFont);
				delete pNewFont;
			}
		} break;
	}
}

//------------------------------------------------------------------------
//! Returns the proper row-height, which an editor should fit in
//------------------------------------------------------------------------
int CGridColumnTraitText::GetCellFontHeight(CGridListCtrlEx& owner)
{
	const CString testText = _T("yjpÍÁ");

	CRect rcRequired = CRect(0,0,0,0);

	CClientDC dc(&owner);
	dc.SelectObject(owner.GetCellFont());
	dc.DrawText(testText, &rcRequired, DT_CALCRECT|DT_SINGLELINE);

	return rcRequired.Height();
}

//------------------------------------------------------------------------
//! Returns the proper rectangle, which an editor should fit in
//------------------------------------------------------------------------
CRect CGridColumnTraitText::GetCellEditRect(CGridListCtrlEx& owner, int nRow, int nCol)
{
	// Find the required height according to font
	int requiredHeight = GetCellFontHeight(owner);

	// Get position of the cell to edit
	CRect rectCell;
	VERIFY( owner.GetCellRect(nRow, nCol, LVIR_LABEL, rectCell) );

	// Adjust position to font height
	if (!owner.UsingVisualStyle())
	{
		if ((requiredHeight + 2*::GetSystemMetrics(SM_CXEDGE)) > rectCell.Height())
		{
			rectCell.top -= ::GetSystemMetrics(SM_CXEDGE);
			rectCell.bottom += ::GetSystemMetrics(SM_CXEDGE);
		}
	}
	if (owner.GetExtendedStyle() & LVS_EX_GRIDLINES)
	{
		if ((requiredHeight + 2*::GetSystemMetrics(SM_CXEDGE) + ::GetSystemMetrics(SM_CXBORDER)) < rectCell.Height())
			rectCell.bottom -= ::GetSystemMetrics(SM_CXBORDER);
	}
	if (owner.GetExtendedStyle() & LVS_EX_SUBITEMIMAGES)
	{
		if (owner.GetImageList(LVSIL_SMALL)!=NULL && owner.GetCellImage(nRow,nCol)>=0)
			rectCell.left += ::GetSystemMetrics(SM_CXBORDER);
	}
	return rectCell;
}