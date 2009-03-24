#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//
//! CGridColumnTrait specifies the methods needed for custom cell handling
//------------------------------------------------------------------------

class CGridColumnTraitVisitor;
class CGridListCtrlEx;

class CGridColumnTrait
{
public:
	virtual ~CGridColumnTrait() {}

	//! Override OnCustomDraw() to provide your own special cell-drawing
	//!   - Must handle selection drawing
	//!   - Must handle focus drawing
	//!	  - Must handle selection drawing when the CListCtrl doesn't have focus
	//!   - Must query owner if special foreground/background-color or font should be used
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult) {}

	//! Override OnEditBegin() to provide your own special cell-edit control
	//!   - The edit control must inherit from CWnd
	//!   - The edit control must delete itself when it looses focus
	//!   - The edit control must send a LVN_ENDLABELEDIT message when edit is complete
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol) { return NULL; }

	//! Override OnEditEnd() in case one need to change state after a cell-edit
	virtual void  OnEditEnd() {}

	//! Override Accept() and update CGridColumnTraitVisitor for new column-trait classes
	//!   - Will enable the use of the visitor-pattern ex. for serialization of column-traits
	virtual void Accept(CGridColumnTraitVisitor& visitor) {}

	// Maintaining column visible state, etc.
	struct ColumnState
	{
		ColumnState()
			:m_Visible(true)
			,m_OrgWidth(0)
			,m_OrgPosition(-1)
			,m_AlwaysHidden(false)
			,m_Sortable(true)
			,m_Editable(true)
			,m_Resizable(true)
			,m_MetaFlags(0)
		{}
		bool m_Visible;
		int  m_OrgWidth;	// Width it had before being hidden
		int  m_OrgPosition;	// Position it had before being hidden
		bool m_AlwaysHidden;
		bool m_Sortable;
		bool m_Editable;
		bool m_Resizable;

		//! Meta-Flags (and data) can be used to store extra properties for a column
		//! when deriving from CGridListCtrlEx.
		DWORD m_MetaFlags;
		CSimpleMap<CString,CString> m_MetaData;
	};
	inline ColumnState& GetColumnState() { return m_ColumnState; }

	inline BOOL HasMetaFlag(DWORD flag) { return m_ColumnState.m_MetaFlags & flag; }
	void SetMetaFlag(DWORD flag, bool enable)
	{
		if (enable)
			m_ColumnState.m_MetaFlags |= flag;
		else
			m_ColumnState.m_MetaFlags &= ~flag;
	}

protected:
	ColumnState m_ColumnState;
};