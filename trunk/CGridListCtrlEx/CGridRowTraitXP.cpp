#include "stdafx.h"
#include "CGridRowTraitXP.h"

#include "CGridRowTraitVisitor.h"
#include "CGridListCtrlEx.h"
#include "CGridColumnTraitText.h"


void CGridRowTraitXP::Accept(CGridRowTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

void CGridRowTraitXP::OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult)
{
	if (owner.UsingVisualStyle())
	{
		CGridRowTraitText::OnCustomDraw(owner, pLVCD, pResult);
		return;
	}

	// We are using classic- or XP-style
	int nRow = (int)pLVCD->nmcd.dwItemSpec;

	// Perform standard drawing
	CGridRowTraitText::OnCustomDraw(owner, pLVCD, pResult);
	if (*pResult & CDRF_SKIPDEFAULT)
		return;

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

			if (!owner.IsColumnVisible(nCol))
				break;

			COLORREF backColor = COLORREF(-1);
			if (!owner.IsRowSelected(nRow))
			{
				// Redraw with the given background color
				if (pLVCD->clrTextBk > RGB(255,255,255))
					break;	// If a color is more than white, then it is invalid

				if (CRect(pLVCD->nmcd.rc)==CRect(0,0,0,0))
					break;

				backColor = pLVCD->clrTextBk;
			}
			else
			{
				if (owner.GetFocusRow()==nRow && owner.GetFocusCell()==nCol)
					break;	// No drawing of selection color for focus cell

				if (!(owner.GetExtendedStyle() & LVS_EX_FULLROWSELECT))
					break;	// No drawing of selection color without full-row-select

				if (owner.GetFocus()!=&owner)
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

			int nImage = owner.GetCellImage(nRow, nCol);
			if (nImage < 0)
				break;

			CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);

			CRect rcIcon, rcCell;
			VERIFY( owner.GetCellRect(nRow, nCol, LVIR_ICON, rcIcon) );
			VERIFY( owner.GetCellRect(nRow, nCol, LVIR_BOUNDS, rcCell) );

			rcCell.right = rcIcon.right + 2;
			pDC->FillSolidRect(&rcCell, backColor);

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
}