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
protected:
	void OnPaint();
	virtual afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP();
};
