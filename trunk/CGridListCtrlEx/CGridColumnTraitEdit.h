#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all
//
//! CGridColumnTraitCombo implements a CEdit as cell-editor
//------------------------------------------------------------------------

#include "CGridColumnTraitText.h"

class CGridColumnTraitEdit : public CGridColumnTraitText
{
	virtual void Accept(CGridColumnTraitVisitor& visitor);

public:
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);
};