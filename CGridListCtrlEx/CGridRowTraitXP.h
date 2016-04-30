#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridRowTraitText.h"

#ifdef CGRIDLISTCTRLEX_AFX_EXT
// Using MFC Extension DLL
#define CGRIDLISTCTRLEX_AFX_EXT_CLASS AFX_EXT_CLASS
#undef AFX_DATA
#define AFX_DATA AFX_EXT_DATA
#else
#define CGRIDLISTCTRLEX_AFX_EXT_CLASS
#endif

//------------------------------------------------------------------------
//! CGridRowTraitXP fixes drawing of rows when the application is using
//! classic- or XP-style.
//------------------------------------------------------------------------
class CGRIDLISTCTRLEX_AFX_EXT_CLASS CGridRowTraitXP : public  CGridRowTraitText
{
public:
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);

protected:
	virtual void Accept(CGridRowTraitVisitor& visitor);
};

#ifdef CGRIDLISTCTRLEX_AFX_EXT
#undef AFX_DATA
#define AFX_DATA
#endif
#undef CGRIDLISTCTRLEX_AFX_EXT_CLASS