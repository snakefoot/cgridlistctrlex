#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnTraitText.h"

//------------------------------------------------------------------------
//! CGridColumnTraitImage implements an image switcher (can mimic a checkbox)
//!
//! By adding checkbox state-images to the official imagelist using
//! AppendStateImages(), then one can use this column trait as checkbox
//! editor. //! To get/set the checkbox value of a cell use the methods
//! GetCellImage()/SetCellImage() on CGridListCtrlEx
//------------------------------------------------------------------------
class CGridColumnTraitImage : public CGridColumnTraitText
{
public:
	CGridColumnTraitImage();
	CGridColumnTraitImage(int nImageIndex, int nImageCount);

	void AddImageIndex(int nImageIdx);
	
	static int AppendStateImages(CGridListCtrlEx& owner, CImageList& imagelist);
	
protected:
	virtual bool OnClickEditStart(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt);
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);
	virtual void Accept(CGridColumnTraitVisitor& visitor);

	CSimpleArray<int> m_ImageIndexes;	//!< 
};