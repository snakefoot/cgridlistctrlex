// CGridListCtrlExDlg.h : header file
//

#pragma once
#include "afxcmn.h"

#include "..\CGridListCtrlEx\CGridListCtrlGroups.h"
#include "CListCtrl_DataModel.h"


// CGridListCtrlExDlg dialog
class CGridListCtrlExDlg : public CDialog
{
// Construction
public:
	CGridListCtrlExDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CGRIDLISTCTRLEX_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CGridListCtrlGroups m_ListCtrl;
	CListCtrl_DataModel m_DataModel;
	CImageList m_ImageList;
};
