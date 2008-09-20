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
	CGridListCtrlGroups();

	virtual LRESULT InsertGroupHeader(int nIndex, int nGroupID, const CString& strHeader, DWORD dwState = 0, DWORD dwAlign = 0);

	virtual CString GetGroupHeader(int nGroupID);
	virtual int GetRowGroupId(int nRow);
	virtual BOOL SetRowGroupId(int nRow, int nGroupID);
	virtual int GroupHitTest(const CPoint& point);

	virtual BOOL GroupByColumn(int nCol);
	virtual void DeleteEntireGroup(int nGroupId);
	virtual BOOL IsGroupStateEnabled();

	virtual void CheckEntireGroup(int nGroupId, bool bChecked);

	virtual bool SortColumn(int columnIndex, bool ascending);

	virtual BOOL HasGroupState(int nGroupID, DWORD dwState);
	virtual BOOL SetGroupState(int nGroupID, DWORD dwState);

	virtual void CollapseAllGroups();
	virtual void ExpandAllGroups();

	virtual BOOL SetGroupFooter(int nGroupID, const CString& footer, DWORD dwAlign = 0);
	virtual BOOL SetGroupTask(int nGroupID, const CString& task);
	virtual BOOL SetGroupSubtitle(int nGroupID, const CString& subtitle);
	virtual BOOL SetGroupTitleImage(int nGroupID, int nImage, const CString& topDesc, const CString& bottomDesc);

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
	virtual afx_msg BOOL OnGetEmptyMarkup(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg void OnPaint();
	//}}AFX_MSG

	int m_GroupHeight;

	DECLARE_MESSAGE_MAP();
#endif	
};
