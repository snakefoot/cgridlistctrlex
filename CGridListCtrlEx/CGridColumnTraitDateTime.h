#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnTraitText.h"

//------------------------------------------------------------------------
//! CGridColumnTraitDateTime implements a CDateTimeCtrl as cell-editor
//------------------------------------------------------------------------
class CGridColumnTraitDateTime : public CGridColumnTraitText
{
public:
	CGridColumnTraitDateTime();

	void SetFormat(const CString& format);
	void SetParseDateTime(DWORD dwFlags = 0, LCID lcid = LANG_USER_DEFAULT);
	void SetStyle(DWORD dwStyle);

	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);

protected:
	virtual void Accept(CGridColumnTraitVisitor& visitor);
	virtual CDateTimeCtrl* CreateDateTimeCtrl(CGridListCtrlEx& owner, int nRow, int nCol, const CRect& rect);

	CString m_Format;
	DWORD	m_ParseDateTimeFlags;
	LCID	m_ParseDateTimeLCID;
	DWORD	m_DateTimeCtrlStyle;
};

//------------------------------------------------------------------------
//! CGridEditorDateTimeCtrl (For internal use)
//------------------------------------------------------------------------
class CGridEditorDateTimeCtrl : public CDateTimeCtrl
{
public:
	CGridEditorDateTimeCtrl(int nRow, int nCol, const CString& format, DWORD formatFlags, LCID lcid);
	
protected:
	virtual void EndEdit(bool bSuccess);

	afx_msg void OnKillFocus(CWnd *pNewWnd);
	afx_msg void OnNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMSG);

	int m_Row;
	int m_Col;
	bool m_Completed;

	CString m_Format;
	DWORD	m_FormatFlags;
	LCID	m_FormatLCID;

	DECLARE_MESSAGE_MAP();
};