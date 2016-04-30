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

class CGridRowTrait;
class CGridRowTraitText;
class CGridRowTraitXP;

//------------------------------------------------------------------------
//! CGridRowTraitVisitor enables the use of the visitor-pattern to
//! add extra behavior to the CGridRowTrait classes
//------------------------------------------------------------------------
class CGRIDLISTCTRLEX_AFX_EXT_CLASS CGridRowTraitVisitor
{
public:
	void Visit(CGridRowTrait&) {}
	void Visit(CGridRowTraitText&) {}
	void Visit(CGridRowTraitXP&) {}
};

#ifdef CGRIDLISTCTRLEX_AFX_EXT
#undef AFX_DATA
#define AFX_DATA
#endif
#undef CGRIDLISTCTRLEX_AFX_EXT_CLASS