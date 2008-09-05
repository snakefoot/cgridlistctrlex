#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all
//
//! CGridColumnTraitCombo implements a CComboBox as cell-editor
//------------------------------------------------------------------------

#include "CGridColumnTraitText.h"

class CGridColumnTraitCombo : public CGridColumnTraitText
{
protected:
	CSimpleMap<int,CString> m_ComboList;
	CComboBox* m_pComboBox;

	virtual void Accept(CGridColumnTraitVisitor& visitor);

public:
	CGridColumnTraitCombo();

	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);
	virtual void  OnEditEnd();

	void LoadList(const CSimpleMap<int,CString>& comboList, int nCurSel);
	void AddItem(int itemData, const CString& itemText) { m_ComboList.Add(itemData, itemText); }
};