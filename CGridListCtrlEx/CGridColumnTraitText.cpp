#include "stdafx.h"
#include "CGridColumnTraitText.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
//! CGridColumnTraitText - Constructor
//------------------------------------------------------------------------
CGridColumnTraitText::CGridColumnTraitText()
	:m_pOldFont(NULL)
	,m_TextColor(COLORREF(-1))
	,m_BackColor(COLORREF(-1))
	,m_SortFormatNumber(false)
{}

//------------------------------------------------------------------------
//! Accept Visitor Pattern
//------------------------------------------------------------------------
void CGridColumnTraitText::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//------------------------------------------------------------------------
//! Changes the text color if one is specified
//!
//! @param textColor Current text color
//! @return New text color was specified (true / false)
//------------------------------------------------------------------------
bool CGridColumnTraitText::UpdateTextColor(COLORREF& textColor)
{
	if (m_TextColor!=COLORREF(-1))
	{
		textColor = m_TextColor;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
//! Changes the background color if one is specified
//!
//! @param backColor Current background color
//! @return New background color was specified (true / false)
//------------------------------------------------------------------------
bool CGridColumnTraitText::UpdateBackColor(COLORREF& backColor)
{
	if (m_BackColor!=COLORREF(-1))
	{
		backColor = m_BackColor;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
//! Overrides the custom draw handler, to allow custom coloring of cells
//! with this column trait.
//!
//! @param owner The list control drawing
//! @param pLVCD Pointer to NMLVCUSTOMDRAW structure
//! @param pResult Modification to the drawing stage (CDRF_NEWFONT, etc.)
//------------------------------------------------------------------------
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
				if (owner.GetExtendedStyle() & LVS_EX_FULLROWSELECT)
					pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;
			}

			if (!owner.IsRowSelected(nRow) && owner.GetHotItem()!=nRow)
			{
				if (UpdateTextColor(pLVCD->clrText))
					*pResult |= CDRF_NEWFONT;

				if (UpdateBackColor(pLVCD->clrTextBk))
					*pResult |= CDRF_NEWFONT;

				// Only change cell colors when not selected / in focus
				if (owner.OnDisplayCellColor(nRow, nCol, pLVCD->clrText, pLVCD->clrTextBk))
					*pResult |= CDRF_NEWFONT;
			}

			LOGFONT newFont = {0};
			if (owner.OnDisplayCellFont(nRow, nCol, newFont))
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

namespace {
	struct PARAMSORT
	{
		PARAMSORT(HWND hWnd, CGridColumnTraitText* pTrait, int nCol, bool bAscending)
			:m_hWnd(hWnd)
			,m_pTrait(pTrait)
			,m_ColumnIndex(nCol)
			,m_Ascending(bAscending)
		{}

		HWND m_hWnd;
		int  m_ColumnIndex;
		bool m_Ascending;
		CGridColumnTraitText* m_pTrait;
	};

	// Comparison extracts values from the List-Control
	int CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		PARAMSORT& ps = *(PARAMSORT*)lParamSort;

		TCHAR left[256] = _T(""), right[256] = _T("");
		ListView_GetItemText(ps.m_hWnd, lParam1, ps.m_ColumnIndex, left, sizeof(left));
		ListView_GetItemText(ps.m_hWnd, lParam2, ps.m_ColumnIndex, right, sizeof(right));

		return ps.m_pTrait->CompareCellValues(left, right, ps.m_Ascending);
	}
}

// Define does not exist on VC6
#ifndef ListView_SortItemsEx
	#ifndef LVM_SORTITEMSEX
		#define LVM_SORTITEMSEX          (LVM_FIRST + 81)
	#endif
	#define ListView_SortItemsEx(hwndLV, _pfnCompare, _lPrm) \
	(BOOL)SNDMSG((hwndLV), LVM_SORTITEMSEX, (WPARAM)(LPARAM)(_lPrm), (LPARAM)(PFNLVCOMPARE)(_pfnCompare))
#endif

//------------------------------------------------------------------------
//! Performs row sorting according to the cell contents
//!
//! @param owner The list control
//! @param nCol The index of the column to sort according to
//! @param bAscending Perform sorting in ascending or descending order
//! @return Can rows be sorted according to specified column (true / false)
//------------------------------------------------------------------------
bool CGridColumnTraitText::OnSortRows(CGridListCtrlEx& owner, int nCol, bool bAscending)
{
	// Uses SortItemsEx because it requires no knowledge of datamodel
	//	- CListCtrl::SortItems() is faster with direct access to the datamodel
	PARAMSORT paramsort(owner.m_hWnd, this, nCol, bAscending);
	ListView_SortItemsEx(owner.m_hWnd, SortFunc, &paramsort);
	return true;
}

//------------------------------------------------------------------------
//! Compares two cell values according to specified sort order
//!
//! @param pszLeftValue Left cell value
//! @param pszLeftValue Right cell value
//! @param bAscending Perform sorting in ascending or descending order
//! @return Is left value less than right value (-1) or equal (0) or larger (1)
//------------------------------------------------------------------------
int CGridColumnTraitText::CompareCellValues(LPCTSTR pszLeftValue, const CString& pszRightValue, bool bAscending)
{
	if (m_SortFormatNumber)
	{
		int nLeftValue = _ttoi(pszLeftValue);
		int nRightValue = _ttoi(pszRightValue);
		if (bAscending)
			return nLeftValue - nRightValue;
		else
			return nRightValue - nLeftValue;
	}
	else
	{
		if (bAscending)
			return _tcscmp( pszLeftValue, pszRightValue );
		else
			return _tcscmp( pszRightValue, pszLeftValue );
	}
}

//------------------------------------------------------------------------
//! Should cell values be compared as numbers when sorting
//!
//! @param bValue Left cell value
//------------------------------------------------------------------------
void CGridColumnTraitText::SetSortFormatNumber(bool bValue)
{
	m_SortFormatNumber = bValue;
}

//------------------------------------------------------------------------
//! Calculates the proper row-height according to font, which a cell value
//! editor should fit in.
//!
//! @param owner The list control for the inplace cell value editor
//! @return Height in pixels of the row.
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
//! Returns the proper rectangle, which a cell value editor should fit in
//!
//! @param owner The list control for the inplace cell value editor
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @return Rectangle where the inplace cell value editor should be placed.
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