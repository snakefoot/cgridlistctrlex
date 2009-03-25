#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

class CGridColumnEditor;
class CGridColumnTrait;
class CGridRowTrait;

//------------------------------------------------------------------------
//! GridListCtrlEx extends the CListCtrl with the following functionality:
//!	- Sortable
//!	- Simple column picker
//!	- Cell navigation
//!	- Keyboard search navigation
//!	- Cell tooltip
//!	- Cell editing and customization through use of CGridColumnTrait's
//!	- Clipboard (copy only)
//------------------------------------------------------------------------
class CGridListCtrlEx : public CListCtrl
{
public:
	CGridListCtrlEx();
	~CGridListCtrlEx();

	// CListCtrl
	LRESULT EnableVisualStyles(bool bValue);
	inline bool UsingVisualStyle() const { return m_UsingVisualStyle; }
	virtual CFont* GetCellFont();
	virtual void SetCellMargin(double margin);
	void SetEmptyMarkupText(const CString& strText);

	// Row
	int GetFocusRow() const;
	void SetFocusRow(int nRow);
	bool IsRowSelected(int nRow) const;
	BOOL SelectRow(int nRow, bool bSelect);
	virtual CGridRowTrait* GetRowTrait(int nRow);
	virtual void SetDefaultRowTrait(CGridRowTrait* pRowTrait);

	// Column
	const CHeaderCtrl* GetHeaderCtrl() const;
	CHeaderCtrl* GetHeaderCtrl() { return CListCtrl::GetHeaderCtrl(); }
	int GetColumnCount() const;
	int GetColumnData(int nCol) const;
	int GetColumnOrder(int nCol) const;
	CString GetColumnHeading(int nCol) const;
	virtual BOOL EnsureColumnVisible(int nCol, bool bPartialOK);
	virtual BOOL SetColumnWidthAuto(int nCol = -1, bool bIncludeHeader = false);
	virtual void SetSortArrow(int nCol, bool bAscending);
	virtual BOOL ShowColumn(int nCol, bool bShow);
	virtual bool IsColumnVisible(int nCol);
	virtual int GetFirstVisibleColumn();
	virtual int InsertHiddenLabelColumn();
	virtual int InsertColumnTrait(int nCol, const CString& strColumnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1, CGridColumnTrait* pTrait = NULL);
	virtual CGridColumnTrait* GetColumnTrait(int nCol);
	virtual int GetColumnTraitSize() const;
	virtual void SetupColumnConfig(CGridColumnEditor* pColumnEditor);

	// Cell / Subitem 
	void CellHitTest(const CPoint& pt, int& nRow, int& nCol) const;
	BOOL GetCellRect(int nRow, int nCol, UINT nCode, CRect& rect);
	inline int GetFocusCell() const { return m_FocusCell; }
	virtual CWnd* EditCell(int nRow, int nCol);
	bool IsCellEditorOpen() const;
	bool IsCellCallback(int nRow, int nCol) const;
	int GetCellImage(int nRow, int nCol) const;
	BOOL SetCellImage(int nRow, int nCol, int nImageId);
	virtual CGridColumnTrait* GetCellColumnTrait(int nRow, int nCol);

	// DataModel callbacks
	virtual bool OnDisplayCellText(int nRow, int nCol, CString& strResult);
	virtual bool OnDisplayCellImage(int nRow, int nCol, int& nImageId);
	virtual bool OnDisplayCellTooltip(const CPoint& point) const;
	virtual bool OnDisplayCellTooltip(int nRow, int nCol, CString& strResult);
	virtual bool OnDisplayCellColor(int nRow, int nCol, COLORREF& textColor, COLORREF& backColor);
	virtual bool OnDisplayCellFont(int nRow, int nCol, LOGFONT& font);
	virtual bool OnDisplayRowColor(int nRow, COLORREF& textColor, COLORREF& backColor);
	virtual bool OnDisplayRowFont(int nRow, LOGFONT& font);
	virtual bool OnDisplayToClipboard(CString& strResult);
	virtual bool OnDisplayToClipboard(int nRow, CString& strResult);
	virtual bool OnDisplayToClipboard(int nRow, int nCol, CString& strResult);

protected:
	// Maintaining column traits (and visible state)
	CSimpleArray<CGridColumnTrait*> m_ColumnTraits;
	virtual void InsertColumnTrait(int nCol, CGridColumnTrait* pTrait);
	virtual void DeleteColumnTrait(int nCol);
	CGridColumnEditor* m_pColumnEditor;
	int InternalColumnPicker(CMenu& menu, int offset);
	int InternalColumnProfileSwitcher(CMenu& menu, int offset, CSimpleArray<CString>& profiles);

	// Maintaining row traits
	CGridRowTrait* m_pDefaultRowTrait;

	// Maintaining cell/subitem focus
	int m_FocusCell;
	void MoveFocusCell(bool bMoveRight);
	void UpdateFocusCell(int nCol);

	// Maintaining Keyboard search
	CString m_LastSearchString;
	CTime	m_LastSearchTime;
	int		m_LastSearchCell;
	int		m_LastSearchRow;

	// Maintaining row sorting
	int m_SortCol;
	bool m_Ascending;
	virtual bool SortColumn(int nCol, bool bAscending);

	// Maintaining cell editing
	CWnd* m_pEditor;

	bool m_UsingVisualStyle;

	// Maintaining margin
	CFont* m_pGridFont;
	CFont* m_pCellFont;
	double m_Margin;

	CString m_EmptyMarkupText;

	// Global column trait methods
	virtual void OnTraitCustomDraw(CGridColumnTrait* pTrait, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);
	virtual CWnd* OnTraitEditBegin(CGridColumnTrait* pTrait, CWnd* pEditor, int nRow, int nCol);
	virtual bool OnTraitEditComplete(CGridColumnTrait* pTrait, CWnd* pEditor, LV_DISPINFO* pLVDI);

	// Context Menu Handlers
	virtual void OnContextMenuGrid(CWnd* pWnd, CPoint point);
	virtual void OnContextMenuHeader(CWnd* pWnd, CPoint point, int nCol);
	virtual void OnContextMenuKeyboard(CWnd* pWnd, CPoint point);
	virtual void OnContextMenuCell(CWnd* pWnd, CPoint point, int nRow, int nCol);

	virtual void OnCreateStyle();

	virtual void OnCopyToClipboard();

	//{{AFX_VIRTUAL(CGridListCtrlEx)
	virtual void PreSubclassWindow();
#if defined(_WIN64)
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
#else
	virtual int OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
#endif
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CGridListCtrlEx)
	virtual afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual afx_msg LRESULT OnDeleteColumn(WPARAM wParam, LPARAM lParam);
	virtual afx_msg LRESULT OnInsertColumn(WPARAM wParam, LPARAM lParam);
	virtual afx_msg BOOL OnItemClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnItemDblClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	virtual afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg LRESULT OnSetColumnWidth(WPARAM wParam, LPARAM lParam);
	virtual afx_msg BOOL OnHeaderDividerDblClick(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderBeginResize(UINT id, NMHDR* pNmhdr, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderBeginDrag(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderEndResize(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderEndDrag(UINT id, NMHDR* pNmhdr, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnToolNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnBeginLabelEdit(NMHDR* pNMHDR,LRESULT* pResult);
	virtual afx_msg BOOL OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual afx_msg void OnContextMenu(CWnd*, CPoint point);
	virtual afx_msg void OnPaint();
	virtual afx_msg void OnKillFocus(CWnd* pNewWnd);
	virtual afx_msg LRESULT OnCopy(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP();
};

//{{AFX_INSERT_LOCATION}}
