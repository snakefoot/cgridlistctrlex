#include "stdafx.h"
#pragma warning(disable:4100)	// unreferenced formal parameter

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
	m_ImageIndexes.Add(nImageIdx, _T(""));
}

//------------------------------------------------------------------------
//! Adds image index to the list of images to switch between
//!
//! @param nImageIdx The index of the image in the list control imagelist
//! @param strImageText The associated cell text to the image
//------------------------------------------------------------------------
void CGridColumnTraitImage::AddImageIndex(int nImageIdx, const CString& strImageText)
{
	m_ImageIndexes.Add(nImageIdx, strImageText);
}

//------------------------------------------------------------------------
//! Updates the image text for the specified image index
//!
//! @param nImageIdx The index of the image in the list control imagelist
//! @param strImageText The associated cell text to the image
//------------------------------------------------------------------------
void CGridColumnTraitImage::SetImageText(int nImageIdx, const CString& strImageText)
{
	int nIndex = m_ImageIndexes.FindKey(nImageIdx);
	if (nIndex==-1)
		AddImageIndex(nImageIdx, strImageText);
	else
		m_ImageIndexes.GetValueAt(nIndex) = strImageText;
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
//! Only flip the cell-image when the actual image is clicked
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
//! Reacts to clicking on the cell image, and switch to the next image index
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
		int nImageIdx = owner.GetCellImage(nRow, nCol);
		int nOldImagePos = -1;
		CString strOldImageText;
		for(int i=0; i < m_ImageIndexes.GetSize(); ++i)
		{
			if (m_ImageIndexes.GetKeyAt(i)==nImageIdx)
			{
				nOldImagePos = i;
				strOldImageText = m_ImageIndexes.GetValueAt(i);
				break;
			}
		}
		if (nOldImagePos==-1)
			return NULL;

		CString strNewImageText;
		int nNewImageIdx = -1;
		if (nOldImagePos+1 == m_ImageIndexes.GetSize())
		{
			nNewImageIdx = m_ImageIndexes.GetKeyAt(0);
			strNewImageText = m_ImageIndexes.GetValueAt(0);
		}
		else
		{
			nNewImageIdx = m_ImageIndexes.GetKeyAt(nOldImagePos+1);
			strNewImageText = m_ImageIndexes.GetValueAt(nOldImagePos+1);
		}

		LV_DISPINFO dispinfo = {0};
		dispinfo.item.iImage = nNewImageIdx;
		dispinfo.hdr.hwndFrom = owner.m_hWnd;
		dispinfo.hdr.idFrom = owner.GetDlgCtrlID();
		dispinfo.hdr.code = LVN_ENDLABELEDIT;

		dispinfo.item.iItem = nRow;
		dispinfo.item.iSubItem = nCol;
		dispinfo.item.mask = LVIF_IMAGE;
		if (strNewImageText!=strOldImageText)
		{
			dispinfo.item.mask |= LVIF_TEXT;
			dispinfo.item.pszText = strNewImageText.GetBuffer(0);
			dispinfo.item.cchTextMax = strNewImageText.GetLength();
		}

		owner.GetParent()->SendMessage( WM_NOTIFY, owner.GetDlgCtrlID(), (LPARAM)&dispinfo );
	}
	return NULL;	// Editor is never really started
}