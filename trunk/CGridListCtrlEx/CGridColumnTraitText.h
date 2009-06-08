#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnTrait.h"

//------------------------------------------------------------------------
//! CGridColumnTraitText provides customization of cell text and background
//------------------------------------------------------------------------
class CGridColumnTraitText : public CGridColumnTrait
{
public:
	CGridColumnTraitText();
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);	
	virtual int OnSortRows(LPCTSTR pszLeftValue, LPCTSTR pszRightValue, bool bAscending);

	bool UpdateTextColor(COLORREF& textColor);
	bool UpdateBackColor(COLORREF& backColor);

	void SetSortFormatNumber(bool bValue);

protected:
	CFont*	m_pOldFont;		//!< Backup of the original font while drawing with specified font
	COLORREF m_TextColor;	//!< Text color to use for this column
	COLORREF m_BackColor;	//!< Background color to use for this column
	bool m_SortFormatNumber;//!< Column contains integers

	virtual void Accept(CGridColumnTraitVisitor& visitor);
	virtual int GetCellFontHeight(CGridListCtrlEx& owner);
	virtual CRect GetCellEditRect(CGridListCtrlEx& owner, int nRow, int nCol);
};
