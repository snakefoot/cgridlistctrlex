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

			if (owner.GetCellCustomColor(nRow, nCol, pLVCD->clrText, pLVCD->clrTextBk))
				*pResult |= CDRF_NEWFONT;

			LOGFONT newFont = {0};
			if (owner.GetCellCustomFont(nRow, nCol, newFont))
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
			int nCol = pLVCD->iSubItem;
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