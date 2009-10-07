#include "stdafx.h"
#include "CGridColumnTraitImage.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
//! CGridColumnTraitImage - Constructor
//------------------------------------------------------------------------
CGridColumnTraitImage::CGridColumnTraitImage()
{
	// Checkbox should be flipped without needing cell-focus first
	m_ColumnState.m_EditFocusFirst = false;
}


//------------------------------------------------------------------------
//! CGridColumnTraitImage - Constructor
//!
//! @param nImageIndex The first index in list control imagelist
//! @param nImageCount The number of images to switch between in the imagelist
//------------------------------------------------------------------------
CGridColumnTraitImage::CGridColumnTraitImage(int nImageIndex, int nImageCount)
{
	// Checkbox should be flipped without needing cell-focus first
	m_ColumnState.m_EditFocusFirst = false;
	for(int i = nImageIndex; i < nImageIndex + nImageCount; ++i)
		AddImageIndex(i);
}

//------------------------------------------------------------------------
//! Adds image index to the list of images to switch between
//!
//! @param nImageIdx The index of the image in the list control imagelist
//------------------------------------------------------------------------
void CGridColumnTraitImage::AddImageIndex(int nImageIdx)
{
	m_ImageIndexes.Add(nImageIdx);
}

//------------------------------------------------------------------------
//! Accept Visitor Pattern
//------------------------------------------------------------------------
void CGridColumnTraitImage::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//------------------------------------------------------------------------
//! Appends the checkbox state images to the list control image list
//!
//! @param owner The list control adding column
//! @param imagelist The image list assigned to the list control
//! @return Image index where the two state images (unchecked/checked) was inserted
//------------------------------------------------------------------------
int CGridColumnTraitImage::AppendStateImages(CGridListCtrlEx& owner, CImageList& imagelist)
{
	if (!imagelist)
		imagelist.Create(16, 16, ILC_COLOR16 | ILC_MASK, 1, 0);
	bool createdStateImages = false;
	CImageList* pStateList = owner.GetImageList(LVSIL_STATE);
	if (pStateList==NULL)
	{
		if (!(owner.GetExtendedStyle() & LVS_EX_CHECKBOXES))
		{
			createdStateImages = true;
			owner.SetExtendedStyle(owner.GetExtendedStyle() | LVS_EX_CHECKBOXES);
			pStateList = owner.GetImageList(LVSIL_STATE);
		}
	}
	int imageCount = -1;
	ASSERT(pStateList!=NULL);
	if (pStateList!=NULL)
	{
		imageCount = imagelist.GetImageCount();
		HICON uncheckedIcon = pStateList->ExtractIcon(0);
		imagelist.Add(uncheckedIcon);
		DestroyIcon(uncheckedIcon);
		HICON checkedIcon = pStateList->ExtractIcon(1);
		imagelist.Add(checkedIcon);
		DestroyIcon(checkedIcon);
	}
	if (createdStateImages)
		owner.SetExtendedStyle(owner.GetExtendedStyle() & ~LVS_EX_CHECKBOXES);
	return imageCount;
}

//------------------------------------------------------------------------
//! Only flip checkbox when cell-image is clicked
//!
//! @param owner The list control being clicked
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param pt The position clicked, in client coordinates.
//------------------------------------------------------------------------
bool CGridColumnTraitImage::OnClickEditStart(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt)
{
	CRect rect;
	owner.GetCellRect(nRow, nCol, LVIR_ICON, rect);
	if (!rect.PtInRect(pt))
		return false;

	return true;
}

//------------------------------------------------------------------------
//! Reacts to clicking on the checkbox image, and flips the checkbox
//!
//! @param owner The list control starting edit
//! @param nRow The index of the row for the cell to edit
//! @param nCol The index of the column for the cell to edit
//! @return Pointer to the cell editor to use (NULL if cell edit is not possible)
//------------------------------------------------------------------------
CWnd* CGridColumnTraitImage::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	CPoint pt(GetMessagePos());
	owner.ScreenToClient(&pt);
	CRect rect;
	owner.GetCellRect(nRow, nCol, LVIR_ICON, rect);
	if (rect.PtInRect(pt))
	{
		// Send Notification to parent of ListView ctrl
		int image = owner.GetCellImage(nRow, nCol);
		int newImage = -1;
		for(int i=0; i < m_ImageIndexes.GetSize(); ++i)
		{
			if (m_ImageIndexes[i]==image)
			{
				newImage = i;
				break;
			}
		}
		if (newImage==-1)
			return NULL;

		if (newImage+1 == m_ImageIndexes.GetSize())
			newImage = m_ImageIndexes[0];
		else
			newImage = m_ImageIndexes[newImage+1];

		LV_DISPINFO dispinfo = {0};
		dispinfo.item.iImage = newImage;
		dispinfo.hdr.hwndFrom = owner.m_hWnd;
		dispinfo.hdr.idFrom = owner.GetDlgCtrlID();
		dispinfo.hdr.code = LVN_ENDLABELEDIT;

		dispinfo.item.iItem = nRow;
		dispinfo.item.iSubItem = nCol;
		dispinfo.item.mask = LVIF_IMAGE;

		owner.GetParent()->SendMessage( WM_NOTIFY, owner.GetDlgCtrlID(), (LPARAM)&dispinfo );
	}
	return NULL;	// Editor is never really started
}