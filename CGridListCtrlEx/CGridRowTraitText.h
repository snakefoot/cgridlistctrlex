#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridRowTrait.h"

#ifdef CGRIDLISTCTRLEX_AFX_EXT
// Using MFC Extension DLL
#define CGRIDLISTCTRLEX_AFX_EXT_CLASS AFX_EXT_CLASS
#undef AFX_DATA
#define AFX_DATA AFX_EXT_DATA
#else
#define CGRIDLISTCTRLEX_AFX_EXT_CLASS
#endif

//------------------------------------------------------------------------
//! CGridRowTraitText provides customization text and background at
//! row-level
//------------------------------------------------------------------------
class CGRIDLISTCTRLEX_AFX_EXT_CLASS CGridRowTraitText : public CGridRowTrait
{
public:
	CGridRowTraitText();
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);

	void SetRowColor(COLORREF textColor, COLORREF backColor);
	void SetAltRowColor(COLORREF textColor, COLORREF backColor);

	void SetInvertCellSelection(bool bValue);
	bool GetInvertCellSelection() const;

protected:
	COLORREF m_TextColor;	//!< Text color to use for this row
	COLORREF m_BackColor;	//!< Background color to use for this row

	COLORREF m_AltTextColor;//!< Alternate text color to use for every second row
	COLORREF m_AltBackColor;//!< Alternate background color to use for every second row

	bool m_InvertCellSelection;//!< When cell has focus in column, then the selection color is removed

	virtual bool UpdateTextColor(int nRow, COLORREF& textColor);
	virtual bool UpdateBackColor(int nRow, COLORREF& backColor);
	virtual void Accept(CGridRowTraitVisitor& visitor);
};

#ifdef CGRIDLISTCTRLEX_AFX_EXT
#undef AFX_DATA
#define AFX_DATA
#endif
#undef CGRIDLISTCTRLEX_AFX_EXT_CLASS