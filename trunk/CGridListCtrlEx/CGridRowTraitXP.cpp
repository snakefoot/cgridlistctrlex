#include "stdafx.h"
#include "CGridRowTraitXP.h"

#include "CGridRowTraitVisitor.h"
#include "CGridListCtrlEx.h"
#include "CGridColumnTraitText.h"


void CGridRowTraitXP::Accept(CGridRowTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

bool CGridRowTraitXP::UpdateBackColor(CGridListCtrlEx& owner, int nRow, int nCol, COLORREF& backColor)
{
	bool result = false;

	// Check row-trait color
	if (CGridRowTraitText::UpdateBackColor(nRow, backColor))
		result = true;

	// Check grid-row color
	COLORREF textColor(-1);
	if (owner.CallbackRowCustomColor(nRow, textColor, backColor))
		result = true;

	// Check col-trait color
	CGridColumnTrait* pColTrait = owner.GetColumnTrait(nCol);
	CGridColumnTraitText* pColTraitText = dynamic_cast<CGridColumnTraitText*>(pColTrait);
	if (pColTraitText!=NULL && pColTraitText->UpdateBackColor(backColor))
		result = true;

	// Check grid-col color
	if (owner.CallbackCellCustomColor(nRow, nCol, textColor, backColor));
		result = true;

	return true;
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
				// Check if custom back-coloring is wanted
				if (!UpdateBackColor(owner, nRow, nCol, backColor))
					break;
			}
			else
			{
				if (owner.GetFocusRow()==nRow && owner.GetFocusCell()==nCol)
					break;	// No drawing of selection color for focus cell

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
					if (rcCell.right < 0)
						continue;
					if (rcCell.right > rcVisibleRect.right)
						continue;

					pDC->MoveTo(rcCell.right, rcCell.top);
					pDC->LineTo(rcCell.right, rcCell.bottom);
				}

				pDC->SelectObject(pOldPen);
			}
#endif
		} break;
	}

	CGridRowTraitText::OnCustomDraw(owner, pLVCD, pResult);
}