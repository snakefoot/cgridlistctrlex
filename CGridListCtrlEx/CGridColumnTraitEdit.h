#pragma once

#include "CGridColumnTraitText.h"

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//
//! CGridColumnTraitEdit implements a CEdit as cell-editor
//------------------------------------------------------------------------
class CGridColumnTraitEdit : public CGridColumnTraitText
{
public:
	CGridColumnTraitEdit();

	void SetStyle(DWORD dwStyle);

	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);

protected:
	virtual void Accept(CGridColumnTraitVisitor& visitor);
	virtual CEdit* CreateEdit(CGridListCtrlEx& owner, int nRow, int nCol, const CRect& rect);

	DWORD m_EditStyle;
};

//------------------------------------------------------------------------
//! CGridEditorText (For internal use)
//------------------------------------------------------------------------
class CGridEditorText : public CEdit
{
public:
	CGridEditorText(int nRow, int nCol);
	virtual void EndEdit(bool bSuccess);

protected:
	afx_msg void OnKillFocus(CWnd *pNewWnd);
	afx_msg void OnNcDestroy();
	virtual	BOOL PreTranslateMessage(MSG* pMSG);

	int m_Row;
	int m_Col;
	bool m_Completed;

	DECLARE_MESSAGE_MAP();
};