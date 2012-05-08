#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnTraitImage.h"

//------------------------------------------------------------------------
//! CGridColumnTraitEdit implements a CEdit as cell-editor
//------------------------------------------------------------------------
class CGridColumnTraitEdit : public CGridColumnTraitImage
{
public:
	CGridColumnTraitEdit();

	void SetStyle(DWORD dwStyle);
	DWORD GetStyle() const;

	void SetLimitText(UINT nMaxChars);
	UINT GetLimitText() const;

	void SetMaxLines(UINT nMaxLines);
	UINT GetMaxLines() const;

	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt) { return CGridColumnTraitImage::OnEditBegin(owner, nRow, nCol, pt); }

protected:
	virtual void Accept(CGridColumnTraitVisitor& visitor);
	virtual CEdit* CreateEdit(CGridListCtrlEx& owner, int nRow, int nCol, const CRect& rect, const CString& cellText);

	DWORD m_EditStyle;				//!< Style to use when creating CEdit
	UINT m_EditLimitText;			//!< Max number of characters the CEdit will accept
	UINT m_EditMaxLines;			//!< Max number of lines the CEdit will display at a time
};

//------------------------------------------------------------------------
//! CEdit for inplace edit. For internal use by CGridColumnTraitEdit
//------------------------------------------------------------------------
class CGridEditorText : public CEdit
{
public:
	CGridEditorText(int nRow, int nCol);
	virtual void EndEdit(bool bSuccess);

	void SetLineHeight(int nLineHeight)	{ m_LineHeight = nLineHeight; }
	void SetMaxLines(UINT nMaxLines)	{ m_MaxLines = nMaxLines; }
	void SetInitialText(const CString& cellText);

protected:
	afx_msg void OnKillFocus(CWnd *pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnEnChange();
	virtual	BOOL PreTranslateMessage(MSG* pMsg);

	int		m_Row;					//!< The index of the row being edited
	int		m_Col;					//!< The index of the column being edited
	bool	m_Completed;			//!< Ensure the editor only reacts to a single close event
	bool	m_Modified;				//!< Register if text was modified while the editor was open
	bool	m_InitialText;			//!< Initial text modication should not set that the editor text was modified
	int		m_LineHeight;			//!< The height of a single line (depends on current font)
	UINT	m_MaxLines;				//!< Max number of lines the CEdit will display at a time

	DECLARE_MESSAGE_MAP();

private:
	CGridEditorText();
	CGridEditorText(const CGridEditorText&);
	CGridEditorText& operator=(const CGridEditorText&);
};