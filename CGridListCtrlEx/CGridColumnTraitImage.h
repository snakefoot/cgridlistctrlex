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
//! editor. To get/set the checkbox value of a cell use the methods
//! GetCellImage()/SetCellImage() on CGridListCtrlEx
//------------------------------------------------------------------------
class CGridColumnTraitImage : public CGridColumnTraitText
{
public:
	CGridColumnTraitImage();
	CGridColumnTraitImage(int nImageIndex, int nImageCount);

	void AddImageIndex(int nImageIdx);
	void AddImageIndex(int nImageIdx, const CString& strImageText, bool bEditable = true);

	void SetImageText(int nImageIdx, const CString& strImageText, bool bEditable = true);

	void SetSortImageIndex(bool bValue);
	void SetToggleSelection(bool bValue);

	static int AppendStateImages(CGridListCtrlEx& owner, CImageList& imagelist);
	
protected:
	virtual int OnSortRows(int nLeftImageIdx, int nRightImageIdx, bool bAscending);
	virtual bool IsCellReadOnly(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt) const;
	virtual int OnClickEditStart(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt, bool bDblClick);
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt);
	virtual void Accept(CGridColumnTraitVisitor& visitor);
	virtual int FlipImageIndex(CGridListCtrlEx& owner, int nRow, int nCol);

	struct ImageCell
	{
		CString m_CellText;
		bool m_Editable;

		ImageCell()
			: m_Editable(true) {}
		ImageCell(const CString& cellText, bool editable)
			: m_CellText(cellText), m_Editable(editable) {}
	};
	CSimpleMap<int,ImageCell> m_ImageIndexes;	//!< Fixed list of image items to switch between

	bool m_SortImageIndex;
	bool m_ToggleSelection;
};