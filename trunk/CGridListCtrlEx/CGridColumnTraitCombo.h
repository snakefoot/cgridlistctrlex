#pragma once

#include "CGridColumnTraitText.h"

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all
//
//! CGridColumnTraitCombo implements a CComboBox as cell-editor
//------------------------------------------------------------------------
class CGridColumnTraitCombo : public CGridColumnTraitText
{
public:
	CGridColumnTraitCombo();

	void SetMaxItems(int items);
	int  GetMaxItems() const;

	void SetStyle(DWORD dwStyle);
	DWORD GetStyle() const;

	void SetMaxWidth(int width);
	int  GetMaxWidth() const;

	void LoadList(const CSimpleMap<int,CString>& comboList, int nCurSel);
	void AddItem(int itemData, const CString& itemText);

	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);
	virtual void  OnEditEnd();

protected:
	virtual void Accept(CGridColumnTraitVisitor& visitor);
	virtual CComboBox* CreateComboBox(CGridListCtrlEx& owner, int nRow, int nCol, const CRect& rect);

	CSimpleMap<int,CString> m_ComboList;
	CComboBox* m_pComboBox;
	DWORD m_ComboBoxStyle;
	int m_MaxItems;
	int m_MaxWidth;
};

//------------------------------------------------------------------------
//! CGridEditorComboBoxEdit (For internal use) 
//
// Taken from "MFC Grid control" credits Chris Maunder
// http://www.codeproject.com/KB/miscctrl/gridctrl.aspx
//------------------------------------------------------------------------
class CGridEditorComboBoxEdit : public CEdit
{
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP();
};

//------------------------------------------------------------------------
//! CGridEditorComboBox (For internal use)
//
// Taken from "MFC Grid control" credits Chris Maunder
// http://www.codeproject.com/KB/miscctrl/gridctrl.aspx
//------------------------------------------------------------------------
class CGridEditorComboBox : public CComboBox
{
public:
	CGridEditorComboBox(int nRow, int nCol, int nMaxWidth);

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual void EndEdit(bool bSuccess);

protected:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnDestroy();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnDropDown();
	afx_msg void OnCloseUp();

	DECLARE_MESSAGE_MAP();

	CGridEditorComboBoxEdit m_Edit;  // subclassed edit control
	bool	m_Completed;
	int		m_Row;
	int		m_Col;
	int		m_MaxWidth;
};