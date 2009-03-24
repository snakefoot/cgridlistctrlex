#pragma once

#include "CGridRowTrait.h"

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//
//! CGridRowTraitText provides customization text and background at
//! row-level
//------------------------------------------------------------------------

class CGridRowTraitText : public CGridRowTrait
{
public:
	CGridRowTraitText();
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);

	void SetRowColor(COLORREF textColor, COLORREF backColor);
	void SetAltRowColor(COLORREF textColor, COLORREF backColor);

protected:
	CFont*	m_pOldFont;		// Backup of the original font while drawing with this font
	COLORREF m_TextColor;
	COLORREF m_BackColor;

	COLORREF m_AltTextColor;
	COLORREF m_AltBackColor;

	bool UpdateTextColor(int nRow, COLORREF& textColor);
	bool UpdateBackColor(int nRow, COLORREF& backColor);
	virtual void Accept(CGridRowTraitVisitor& visitor);
};
