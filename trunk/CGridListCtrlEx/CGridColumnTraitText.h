#pragma once

#include "CGridColumnTrait.h"

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all
//
//! CGridColumnTraitText provides customization of cell text and background
//------------------------------------------------------------------------

class CGridColumnTraitText : public CGridColumnTrait
{
protected:
	CFont*	m_pOldFont;		// Backup of the original font while drawing with this font

	virtual void Accept(CGridColumnTraitVisitor& visitor);

public:
	CGridColumnTraitText();
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);	
};