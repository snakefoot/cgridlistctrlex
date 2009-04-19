#include "stdafx.h"
#include "CGridRowTraitXP.h"

#include "CGridRowTraitVisitor.h"
#include "CGridListCtrlEx.h"
#include "CGridColumnTraitText.h"

//------------------------------------------------------------------------
//! Accept Visitor Pattern
//------------------------------------------------------------------------
void CGridRowTraitXP::Accept(CGridRowTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//------------------------------------------------------------------------
//! Overrides the custom draw handler, to allow custom coloring of rows.
//!		- Fix white background for icon images
//!		- Fix white background between icon and cell text
//!		- Fix drawing of column grid lines when slowly scrolling
//!
//! @param owner The list control drawing
//! @param pLVCD Pointer to NMLVCUSTOMDRAW structure
//! @param pResult Modification to the drawing stage (CDRF_NEWFONT, etc.)
//------------------------------------------------------------------------
void CGridRowTraitXP::OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult)
{
	if (owner.UsingVisualStyle())
	{
		// Perform standard drawing
		CGridRowTraitText::OnCustomDraw(owner, pLVCD, pResult);
		return;
	}
	
	// We are using classic- or XP-style
	int nRow = (int)pLVCD->nmcd.dwItemSpec;

	// Repair the standard drawing
	switch (pLVCD->nmcd.dwDrawStage)
	{
		case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		{
			// We want to fix cell images
			*pResult |= CDRF_NOTIFYPOSTPAINT;
		} break;

		case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
		{
			// Fix CListCtrl selection drawing bug with white background for icon image
			// Fix CListCtrl selection drawing bug with white margin between icon and text
			int nCol = pLVCD->iSubItem;

			if (CRect(pLVCD->nmcd.rc)==CRect(0,0,0,0))
				break;

			int nImage = owner.GetCellImage(nRow, nCol);
			if (nImage < 0)
				break;

			COLORREF backColor = COLORREF(-1);
			if (!owner.IsRowSelected(nRow))
			{
				backColor = pLVCD->clrTextBk;
			}
			else
			{
				if (owner.GetFocusRow()==nRow && owner.GetFocusCell()==nCol)
					break;	// No drawing of selection color for focus cell

				if (!(owner.GetExtendedStyle() & LVS_EX_FULLROWSELECT))
					break;	// No drawing of selection color without full-row-select

				if (owner.GetFocus()!=&owner && !owner.IsCellEditorOpen())
				{
					// Selection color is different when not having focus
					if (owner.GetStyle() & LVS_SHOWSELALWAYS)
						backColor = ::GetSysColor(COLOR_BTNFACE);
					else
						break;	// no drawing of selection color when not in focus
				}
				else
					backColor = ::GetSysColor(COLOR_HIGHLIGHT);
			}

			CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);

			CRect rcIcon, rcCell;
			VERIFY( owner.GetCellRect(nRow, nCol, LVIR_ICON, rcIcon) );
			VERIFY( owner.GetCellRect(nRow, nCol, LVIR_BOUNDS, rcCell) );

			rcCell.right = rcIcon.right + 2;
			CBrush brush(backColor);
			pDC->FillRect(&rcCell, &brush);

			// Draw icon
			CImageList* pImageList = owner.GetImageList(LVSIL_SMALL);
			COLORREF oldBkColor = pImageList->SetBkColor(backColor);
			pImageList->Draw (	pDC,  
								nImage,  
								rcIcon.TopLeft(),  
								ILD_BLEND50 );
			pImageList->SetBkColor(oldBkColor);
		} break;

		case CDDS_ITEMPOSTPAINT:
		{
			// Fix CListCtrl grid drawing bug where vertical grid-border disappears
			//	- To reproduce the bug one needs atleast 2 columns:
			//		1) Resize the second column so a scrollbar appears
			//		2) Scroll to the right so the first column disappear
			//		3) When scrolling slowly to the left, the right border of first column is not drawn
#if (_WIN32_WINNT >= 0x501)
			if ( (owner.GetExtendedStyle() & LVS_EX_GRIDLINES)
			  && (owner.GetExtendedStyle() & LVS_EX_DOUBLEBUFFER)
			   )
			{
				CRect rcVisibleRect;
				owner.GetClientRect(rcVisibleRect);

				CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
				CPen Pen;
				Pen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNFACE));
				CPen* pOldPen = pDC->SelectObject(&Pen);

				// Loop through the columns without regard of display order
				int nColCount = owner.GetHeaderCtrl()->GetItemCount();
				for(int nCol = 0; nCol < nColCount; ++nCol)
				{
					CRect rcCell;
					VERIFY( owner.GetCellRect(nRow, nCol, LVIR_BOUNDS, rcCell) );
					if (rcCell.right==0 && rcCell.left!=rcCell.right)
					{
						// Only redraw when the border is about to show, and the column has a width
						pDC->MoveTo(rcCell.right, rcCell.top);
						pDC->LineTo(rcCell.right, rcCell.bottom);
					}
				}

				pDC->SelectObject(pOldPen);
			}
#endif
		} break;
	}

	// Perform standard drawing
	CGridRowTraitText::OnCustomDraw(owner, pLVCD, pResult);
}