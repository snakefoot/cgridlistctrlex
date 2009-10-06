#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnTraitText.h"

//------------------------------------------------------------------------
//! CGridColumnTraitCheckBox implements a checkbox as cell-editor.
//!
//! It hacks the cell-image, and transforms it into a checkbox.
//! To get/set the checkbox value of a cell use the methods
//! GetCellImage()/SetCellImage() on CGridListCtrlEx (0 = Uncheck/1 = Check).
//------------------------------------------------------------------------
class CGridColumnTraitCheckBox : public CGridColumnTraitText
{
public:
	CGridColumnTraitCheckBox();
	
protected:
	virtual void OnInsertColumn(CGridListCtrlEx& owner, int nCol);
	virtual bool OnClickEditStart(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt);
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);
	virtual void Accept(CGridColumnTraitVisitor& visitor);

	CImageList m_ImageList;			//!< Stores the checkbox state-images
	CImageList m_EmptyImageList;	//!< Default image list if one is not configured on the CListCtrl
	HICON m_OldFirstIcon;			//!< Stores the original image during drawing
	HICON m_OldSecondIcon;			//!< Stores the original image during drawing
};