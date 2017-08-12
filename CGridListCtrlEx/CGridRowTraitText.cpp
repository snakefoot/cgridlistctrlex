//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "stdafx.h"
#pragma warning(disable:4100)	// unreferenced formal parameter

#include "CGridRowTraitText.h"

#include "CGridRowTraitVisitor.h"
#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
//! CGridRowTraitText - Constructor
//------------------------------------------------------------------------
CGridRowTraitText::CGridRowTraitText()
	: m_TextColor((COLORREF)-1)
	, m_BackColor((COLORREF)-1)
	, m_AltTextColor((COLORREF)-1)
	, m_AltBackColor((COLORREF)-1)
	, m_InvertCellSelection(false)
{}

//------------------------------------------------------------------------
//! Sets the same row coloring for all rows
//!
//! @param backColor The background color to use for all rows
//! @param textColor The text color to use for all rows
//------------------------------------------------------------------------
void CGridRowTraitText::SetRowColor(COLORREF textColor, COLORREF backColor)
{
	m_TextColor = textColor;
	m_BackColor = backColor;
}

//------------------------------------------------------------------------
//! Activates alternate row coloring
//!
//! @param backColor The background color to use for every second row
//! @param textColor The text color to use for every second row
//------------------------------------------------------------------------
void CGridRowTraitText::SetAltRowColor(COLORREF textColor, COLORREF backColor)
{
	m_AltTextColor = textColor;
	m_AltBackColor = backColor;
}

//------------------------------------------------------------------------
//! Should focus-cell in a selected row have the selection-color removed
//!
//! @param bValue Enabled / Disabled
//------------------------------------------------------------------------
void CGridRowTraitText::SetInvertCellSelection(bool bValue)
{
	m_InvertCellSelection = bValue;
}

//------------------------------------------------------------------------
//! Get whether focus-cell in a selected row should have the selection-color
//! removed
//!
//! @return Enabled / Disabled
//------------------------------------------------------------------------
bool CGridRowTraitText::GetInvertCellSelection() const
{
	return m_InvertCellSelection;
}

//------------------------------------------------------------------------
//! Accept Visitor Pattern
//------------------------------------------------------------------------
void CGridRowTraitText::Accept(CGridRowTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//------------------------------------------------------------------------
//! Changes the text color if one is specified
//!
//! @param nRow The index of the row
//! @param textColor Current text color
//! @return New text color was specified (true / false)
//------------------------------------------------------------------------
bool CGridRowTraitText::UpdateTextColor(int nRow, COLORREF& textColor)
{
	if (m_AltTextColor != COLORREF(-1) && nRow % 2)
	{
		textColor = m_AltTextColor;
		return true;
	}

	if (m_TextColor != COLORREF(-1))
	{
		textColor = m_TextColor;
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
//! Changes the background color if one is specified
//!
//! @param nRow The index of the row
//! @param backColor Current background color
//! @return New background color was specified (true / false)
//------------------------------------------------------------------------
bool CGridRowTraitText::UpdateBackColor(int nRow, COLORREF& backColor)
{
	if (m_AltBackColor != COLORREF(-1) && nRow % 2)
	{
		backColor = m_AltBackColor;
		return true;
	}

	if (m_BackColor != COLORREF(-1))
	{
		backColor = m_BackColor;
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
//! Overrides the custom draw handler, to allow custom coloring of rows.
//!		- Focus rectangle display
//!		- Use font size to increase row-height, but keep cell font-size
//!		- Alternate row coloring
//!
//! @param owner The list control drawing
//! @param pLVCD Pointer to NMLVCUSTOMDRAW structure
//! @param pResult Modification to the drawing stage (CDRF_NEWFONT, etc.)
//------------------------------------------------------------------------
void CGridRowTraitText::OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult)
{
	const int nRow = static_cast<int>(pLVCD->nmcd.dwItemSpec);

	switch (pLVCD->nmcd.dwDrawStage)
	{
		case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		{
			// Remove the selection color for the focus cell, to make it easier to see focus
			if (m_InvertCellSelection)
			{
				const int nCol = pLVCD->iSubItem;
				if (pLVCD->nmcd.uItemState & CDIS_SELECTED)
				{
					if (owner.GetFocusCell() == nCol && owner.GetFocusRow() == nRow)
					{
						pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;
					}
				}
			}

			// Bug in Vista causes the cell color from previous cell to be used in the next
			// even if having reverted the cell coloring in subitem-post-paint
			if (pLVCD->clrText <= RGB(255, 255, 255) || pLVCD->clrTextBk <= RGB(255, 255, 255))
			{
				pLVCD->clrText = CLR_DEFAULT;
				pLVCD->clrTextBk = CLR_DEFAULT;

				if (UpdateTextColor(nRow, pLVCD->clrText))
					*pResult |= CDRF_NEWFONT;

				if (UpdateBackColor(nRow, pLVCD->clrTextBk))
					*pResult |= CDRF_NEWFONT;

				if (owner.OnDisplayRowColor(nRow, pLVCD->clrText, pLVCD->clrTextBk))
					*pResult |= CDRF_NEWFONT;
			}
		} break;

		// Before painting a row
		case CDDS_ITEMPREPAINT:
		{
			if (UpdateTextColor(nRow, pLVCD->clrText))
				*pResult |= CDRF_NEWFONT;

			if (UpdateBackColor(nRow, pLVCD->clrTextBk))
				*pResult |= CDRF_NEWFONT;

			if (owner.OnDisplayRowColor(nRow, pLVCD->clrText, pLVCD->clrTextBk))
				*pResult |= CDRF_NEWFONT;

			if (pLVCD->nmcd.uItemState & CDIS_FOCUS)
			{
				if (owner.GetFocus() != &owner)
					break;

				// If drawing focus row, then remove focus state and request to draw it later
				//	- Row paint request can come twice, with and without focus flag
				//	- Only respond to the one with focus flag, else DrawFocusRect XOR will cause solid or blank focus-rectangle
				if (owner.GetFocusRow() == nRow)
				{
					if (owner.GetFocusCell() >= 0)
					{
						// We want to draw a cell-focus-rectangle instead of row-focus-rectangle
						pLVCD->nmcd.uItemState &= ~CDIS_FOCUS;
						*pResult |= CDRF_NOTIFYPOSTPAINT;
					}
					else if (owner.GetExtendedStyle() & LVS_EX_GRIDLINES)
					{
						// Avoid bug where bottom of focus rectangle is missing when using grid-lines
						//	- Draw the focus-rectangle for the entire row (explicit)
						pLVCD->nmcd.uItemState &= ~CDIS_FOCUS;
						*pResult |= CDRF_NOTIFYPOSTPAINT;
					}
				}
			}
		} break;

		// After painting a row
		case CDDS_ITEMPOSTPAINT:
		{
			if (CRect(pLVCD->nmcd.rc) == CRect(0, 0, 0, 0))
				break;

			if (owner.GetFocusRow() != nRow)
				break;

			if (owner.GetFocus() != &owner)
				break;

			// Perform the drawing of the focus rectangle
			if (owner.GetFocusCell() >= 0)
			{
				// Draw the focus-rectangle for a single-cell
				CRect rcHighlight;
				CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);

				VERIFY(owner.GetCellRect(nRow, owner.GetFocusCell(), LVIR_BOUNDS, rcHighlight));

				int cxborder = ::GetSystemMetrics(SM_CXBORDER);

				// When the label column is placed first it has a left-margin 
				if (owner.GetFocusCell() == 0 && owner.GetFocusCell() == owner.GetFirstVisibleColumn())
				{
					rcHighlight.left += cxborder * 2;
				}
				else
					// Prevent focus rectangle to overlap with cell-image (Only room for this when not first column)
					if (owner.GetFirstVisibleColumn() != owner.GetFocusCell())
					{
						rcHighlight.left -= cxborder;
					}

				// Adjust rectangle according to grid-lines
				if (owner.GetExtendedStyle() & LVS_EX_GRIDLINES)
				{
					rcHighlight.bottom -= cxborder;
				}

				pDC->DrawFocusRect(rcHighlight);
			}
			else if (owner.GetExtendedStyle() & LVS_EX_GRIDLINES)
			{
				// Avoid bug where bottom of focus rectangle is missing when using grid-lines
				//	- Draw the focus-rectangle for the entire row (explicit)
				CRect rcHighlight;
				CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
				// Using LVIR_BOUNDS to get the entire row-rectangle
				VERIFY(owner.GetItemRect(nRow, rcHighlight, LVIR_BOUNDS));

				// Adjust rectangle according to grid-lines
				int cxborder = ::GetSystemMetrics(SM_CXBORDER);
				rcHighlight.bottom -= cxborder;
				pDC->DrawFocusRect(rcHighlight);
			}
		} break;
	}
}