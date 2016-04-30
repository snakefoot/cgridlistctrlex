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

class CGridColumnTrait;
class CGridColumnTraitImage;
class CGridColumnTraitCombo;
class CGridColumnTraitDateTime;
class CGridColumnTraitEdit;
class CGridColumnTraitText;

//------------------------------------------------------------------------
//! CGridColumnTraitVisitor enables the use of the visitor-pattern to
//! add extra behavior to the CGridColumnTrait classes
//------------------------------------------------------------------------
class CGRIDLISTCTRLEX_AFX_EXT_CLASS CGridColumnTraitVisitor
{
public:
	void Visit(CGridColumnTrait&) {}
	void Visit(CGridColumnTraitImage&) {}
	void Visit(CGridColumnTraitCombo&) {}
	void Visit(CGridColumnTraitDateTime&) {}
	void Visit(CGridColumnTraitEdit&) {}
	void Visit(CGridColumnTraitText&) {}
};

#ifdef CGRIDLISTCTRLEX_AFX_EXT
#undef AFX_DATA
#define AFX_DATA
#endif
#undef CGRIDLISTCTRLEX_AFX_EXT_CLASS