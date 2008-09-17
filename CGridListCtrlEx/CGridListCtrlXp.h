#include "CGridListCtrlGroups.h"

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all
//
//! CGridListCtrlXp extends the CGridListCtrlGroups to perform bugfixes
//! that are visible when running CGridListCtrlEx on Windows XP
//------------------------------------------------------------------------

class CGridListCtrlXp : public CGridListCtrlGroups
{
	bool RedrawCell(int nRow, int nCol);
	virtual void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
};
