#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#ifdef CGRIDLISTCTRLEX_AFX_EXT
// Using MFC Extension DLL
#define CGRIDLISTCTRLEX_AFX_EXT_CLASS AFX_EXT_CLASS
#undef AFX_DATA
#define AFX_DATA AFX_EXT_DATA
#else
#define CGRIDLISTCTRLEX_AFX_EXT_CLASS
#endif

class CGridRowTraitVisitor;
class CGridListCtrlEx;

#pragma warning(push)
#pragma warning(disable:4100)	// unreferenced formal parameter

//------------------------------------------------------------------------
//! CGridRowTrait specifies an interface for handling custom drawing at
//! row-level
//------------------------------------------------------------------------
class CGRIDLISTCTRLEX_AFX_EXT_CLASS CGridRowTrait
{
public:
	virtual ~CGridRowTrait() {}

	//! Override OnCustomDraw() to provide your own special row-drawing
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult) {}

	//! Override Accept() and update CGridColumnTraitVisitor for new column-trait classes
	//!   - Will enable the use of the visitor-pattern ex. for serialization of column-traits
	virtual void Accept(CGridRowTraitVisitor& visitor) {}
};

#pragma warning(pop)

#ifdef CGRIDLISTCTRLEX_AFX_EXT
#undef AFX_DATA
#define AFX_DATA
#endif
#undef CGRIDLISTCTRLEX_AFX_EXT_CLASS