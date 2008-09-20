#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all
//
//! CGridColumnTrait specifies the methods needed for custom cell handling
//------------------------------------------------------------------------

class CGridRowTraitVisitor;
class CGridListCtrlEx;

class CGridRowTrait
{
public:
	virtual ~CGridRowTrait() {}

	//! Override OnCustomDraw() to provide your own special row-drawing
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult) {}

	//! Override Accept() and update CGridColumnTraitVisitor for new column-trait classes
	//!   - Will enable the use of the visitor-pattern ex. for serialization of column-traits
	virtual void Accept(CGridRowTraitVisitor& visitor) {}

	// Maintaining column visible state, etc.
	struct RowState
	{
		RowState()
			:m_MetaFlags(0)
		{}

		//! Meta-Flags (and data) can be used to store extra properties for a column
		//! when deriving from CGridListCtrlEx.
		DWORD m_MetaFlags;
		CSimpleMap<CString,CString> m_MetaData;
	};
	inline RowState& GetRowState() { return m_RowState; }

	inline BOOL HasMetaFlag(DWORD flag) { return m_RowState.m_MetaFlags & flag; }
	void SetMetaFlag(DWORD flag, bool enable)
	{
		if (enable)
			m_RowState.m_MetaFlags |= flag;
		else
			m_RowState.m_MetaFlags &= ~flag;
	}

protected:
	RowState m_RowState;
};