#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnTraitImage.h"

#ifdef CGRIDLISTCTRLEX_AFX_EXT
// Using MFC Extension DLL
#define CGRIDLISTCTRLEX_AFX_EXT_CLASS AFX_EXT_CLASS
#undef AFX_DATA
#define AFX_DATA AFX_EXT_DATA
class CGRIDLISTCTRLEX_AFX_EXT_CLASS CEdit;
#else
#define CGRIDLISTCTRLEX_AFX_EXT_CLASS
#endif

class CGridEditorText;

//------------------------------------------------------------------------
//! CGridColumnTraitEdit implements a CEdit as cell-editor
//------------------------------------------------------------------------
class CGRIDLISTCTRLEX_AFX_EXT_CLASS CGridColumnTraitEdit : public CGridColumnTraitImage
{
public:
	CGridColumnTraitEdit();

	void SetStyle(DWORD dwStyle);
	DWORD GetStyle() const;

	void SetLimitText(UINT nMaxChars);
	UINT GetLimitText() const;

	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt) { return CGridColumnTraitImage::OnEditBegin(owner, nRow, nCol, pt); }

protected:
	virtual void Accept(CGridColumnTraitVisitor& visitor);
	virtual CEdit* CreateEdit(CGridListCtrlEx& owner, int nRow, int nCol, DWORD dwStyle, const CRect& rect);

	DWORD m_EditStyle;				//!< Style to use when creating CEdit
	UINT m_EditLimitText;			//!< Max number of characters the CEdit will accept
};

//------------------------------------------------------------------------
//! CEdit for inplace edit. For internal use by CGridColumnTraitEdit
//------------------------------------------------------------------------
class CGRIDLISTCTRLEX_AFX_EXT_CLASS CGridEditorText : public CEdit
{
	DECLARE_DYNAMIC(CGridEditorText)

public:
	CGridEditorText(int nRow, int nCol);
	virtual void EndEdit(bool bSuccess);

protected:
	afx_msg void OnKillFocus(CWnd *pNewWnd);
	afx_msg void OnEnChange();

	virtual void PostNcDestroy();
	virtual	BOOL PreTranslateMessage(MSG* pMsg);

	int		m_Row;					//!< The index of the row being edited
	int		m_Col;					//!< The index of the column being edited
	bool	m_Completed;			//!< Ensure the editor only reacts to a single close event
	bool	m_Modified;				//!< Register if text was modified while the editor was open
	bool	m_InitialModify;		//!< Initial text modication should not set that the editor text was modified

	DECLARE_MESSAGE_MAP();

private:
	CGridEditorText();
	CGridEditorText(const CGridEditorText&);
	CGridEditorText& operator=(const CGridEditorText&);
};

#ifdef CGRIDLISTCTRLEX_AFX_EXT
#undef AFX_DATA
#define AFX_DATA
#endif
#undef CGRIDLISTCTRLEX_AFX_EXT_CLASS