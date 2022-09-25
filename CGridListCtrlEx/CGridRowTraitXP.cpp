//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "stdafx.h"
#pragma warning(disable:4100)	// unreferenced formal parameter

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
//!
//! @param owner The list control drawing
//! @param pLVCD Pointer to NMLVCUSTOMDRAW structure
//! @param pResult Modification to the drawing stage (CDRF_NEWFONT, etc.)
//------------------------------------------------------------------------
void CGridRowTraitXP::OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult)
{
	const int nRow = static_cast<int>(pLVCD->nmcd.dwItemSpec);

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
			const int nCol = pLVCD->iSubItem;

			if (CRect(pLVCD->nmcd.rc) == CRect(0, 0, 0, 0))
				break;

			int nImage = owner.GetCellImage(nRow, nCol);
			if (nImage == I_IMAGECALLBACK)
				break;

			CImageList* pImageList = owner.GetImageList(LVSIL_SMALL);
			if (pImageList == NULL)
				break;

			COLORREF backColor = COLORREF(-1);
			if (owner.GetExtendedStyle() & LVS_EX_TRACKSELECT && owner.GetHotItem() == nRow)
			{
	#if(WINVER >= 0x0500)
				backColor = ::GetSysColor(COLOR_HOTLIGHT);
	#else
				if (owner.IsRowSelected(nRow))
					backColor = ::GetSysColor(COLOR_HIGHLIGHT);
				else
					break;
	#endif
			}
			else if ((pLVCD->nmcd.uItemState & CDIS_SELECTED) && owner.IsRowSelected(nRow))
			{
				if (owner.GetExtendedStyle() & LVS_EX_FULLROWSELECT || nCol == 0)
				{
					// Selection must be visible for this cell
					if (owner.UsingVisualStyle())
						break;	// Row selection is not drawn by background color

					if (m_InvertCellSelection && owner.GetFocusRow() == nRow && owner.GetFocusCell() == nCol)
					{
						// No drawing of selection color for focus cell
						if (pLVCD->clrTextBk > RGB(255, 255, 255))
							break;

						backColor = pLVCD->clrTextBk;
					}
					else
					{
						if (owner.GetFocus() != &owner && !owner.IsCellEditorOpen())
						{
							// Selection color is different when not having focus
							if (owner.GetStyle() & LVS_SHOWSELALWAYS)
							{
								backColor = ::GetSysColor(COLOR_BTNFACE);
							}
							else
								break;	// No drawing of selection, when not in focus
						}
						else
						{
							backColor = ::GetSysColor(COLOR_HIGHLIGHT);
						}
					}
				}
				else
				{
					// Redraw with the given background color
					if (pLVCD->clrTextBk > RGB(255, 255, 255))
						break;	// If a color is more than white, then it is invalid

					backColor = pLVCD->clrTextBk;
				}
			}
			else
			{
				// Redraw with the given background color
				if (pLVCD->clrTextBk > RGB(255, 255, 255))
					break;	// If a color is more than white, then it is invalid

				backColor = pLVCD->clrTextBk;
			}

			CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);

			CRect rcIcon, rcIconArea;
			VERIFY(owner.GetCellRect(nRow, nCol, LVIR_ICON, rcIcon));
			VERIFY(owner.GetCellRect(nRow, nCol, LVIR_BOUNDS, rcIconArea));
			// When the label column is placed first it has a left-margin 
			if (nCol == 0 && (nCol == owner.GetFirstVisibleColumn() || owner.GetExtendedStyle() & LVS_EX_CHECKBOXES))
			{
				int cxborder = ::GetSystemMetrics(SM_CXBORDER);
				rcIconArea.left += cxborder * 2;
			}

			// Remove white margin between cell-image and cell-text
			rcIconArea.right = rcIcon.right + 2;

			// Stay within the limits of the column visible area
			if (rcIcon.right > pLVCD->nmcd.rc.right)
				rcIcon.right = pLVCD->nmcd.rc.right;

			if (rcIconArea.right > pLVCD->nmcd.rc.right)
				rcIconArea.right = pLVCD->nmcd.rc.right;

			CBrush brush(backColor);
			pDC->FillRect(&rcIconArea, &brush);

			IMAGEINFO iconSizeInfo = { 0 };
			VERIFY(pImageList->GetImageInfo(0, &iconSizeInfo));
			int iconHeight = iconSizeInfo.rcImage.bottom - iconSizeInfo.rcImage.top;
			if (rcIcon.Height() > iconHeight)
				rcIcon.top += (rcIcon.Height() - iconHeight) / 2;

			// Draw icon
			COLORREF oldBkColor = pImageList->SetBkColor(backColor);
			pImageList->Draw(pDC,
				nImage,
				rcIcon.TopLeft(),
				ILD_NORMAL);
			pImageList->SetBkColor(oldBkColor);

			if (nCol == 0 && owner.GetExtendedStyle() & LVS_EX_CHECKBOXES)
			{
				CImageList* pStateImageList = owner.GetImageList(LVSIL_STATE);
				if (pImageList == NULL)
					break;

				IMAGEINFO stateSizeInfo = { 0 };
				VERIFY(pStateImageList->GetImageInfo(0, &stateSizeInfo));
				int stateIconHeight = stateSizeInfo.rcImage.bottom - stateSizeInfo.rcImage.top;
				int stateIconWidth = stateSizeInfo.rcImage.right - stateSizeInfo.rcImage.left;
				if (rcIconArea.Height() > stateIconHeight)
					rcIconArea.top += (rcIconArea.Height() - stateIconHeight) / 2;

				if ((rcIcon.left - rcIconArea.left) > stateIconWidth)
					rcIconArea.left += ((rcIcon.left - rcIconArea.left) - stateIconWidth) / 2;

				int checkState = owner.GetCheck(nRow);
				COLORREF oldStateBkColor = pStateImageList->SetBkColor(backColor);
				pStateImageList->Draw(pDC,
					checkState,
					rcIconArea.TopLeft(),
					ILD_NORMAL);
				pStateImageList->SetBkColor(oldStateBkColor);
			}

		} break;
	}

	// Perform standard drawing
	CGridRowTraitText::OnCustomDraw(owner, pLVCD, pResult);
}