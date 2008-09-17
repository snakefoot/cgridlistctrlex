#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all
//
//! CGridListCtrlGroups extends the CGridListCtrlEx with grouping.
//! Placed in its own file as all features requires _WIN32_WINNT > 0x0501
//------------------------------------------------------------------------

class CGridListCtrlGroups : public CGridListCtrlEx
{
public:
#if _WIN32_WINNT >= 0x0501
	LRESULT InsertGroupHeader(int nIndex, int nGroupID, const CString& strHeader, DWORD dwState = 0, DWORD dwAlign = 0);

	CString GetGroupHeader(int nGroupID);
	int GetRowGroupId(int nRow);
	BOOL SetRowGroupId(int nRow, int nGroupID);
	int GroupHitTest(const CPoint& point);

	BOOL GroupByColumn(int nCol);
	void DeleteEntireGroup(int nGroupId);
	BOOL IsGroupStateEnabled();

	void CheckEntireGroup(int nGroupId, bool bChecked);

	bool SortColumn(int columnIndex, bool ascending);

	BOOL HasGroupState(int nGroupID, DWORD dwState);
	BOOL SetGroupState(int nGroupID, DWORD dwState);

	void CollapseAllGroups();
	void ExpandAllGroups();

	BOOL SetGroupFooter(int nGroupID, const CString& footer, DWORD dwAlign = 0);
	BOOL SetGroupTask(int nGroupID, const CString& task);
	BOOL SetGroupSubtitle(int nGroupID, const CString& subtitle);
	BOOL SetGroupTitleImage(int nGroupID, int nImage, const CString& topDesc, const CString& bottomDesc);

	// DataModel callbacks
	virtual bool CallbackCellGroup(int nRow, int nCol, int& groupId) { return false; }

protected:
	// Context Menu Handlers
	virtual void OnContextMenuGrid(CWnd* pWnd, CPoint point);
	virtual void OnContextMenuHeader(CWnd* pWnd, CPoint point, int nCol);
	virtual void OnContextMenuGroup(CWnd* pWnd, CPoint point, int nGroupId);

	//{{AFX_MSG(CGridListCtrlEx)
	virtual afx_msg void OnContextMenu(CWnd*, CPoint point);
	virtual afx_msg BOOL OnGroupTaskClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual afx_msg BOOL OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP();
#endif
};
