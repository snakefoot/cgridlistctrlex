// CGridListCtrlExDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CGridListCtrlExApp.h"
#include "CGridListCtrlExDlg.h"

#include "CGridColumnTraitEdit.h"
#include "CGridColumnTraitCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CGridListCtrlExDlg dialog



CGridListCtrlExDlg::CGridListCtrlExDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridListCtrlExDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGridListCtrlExDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CGridListCtrlExDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CGridListCtrlExDlg message handlers

BOOL CGridListCtrlExDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// Create Columns
	m_ListCtrl.InsertHiddenLabelColumn();	// Requires one never uses column 0

	for(int col = 0; col < m_DataModel.GetColCount() ; ++col)
	{
		const string& title = m_DataModel.GetColTitle(col);
		CGridColumnTrait* pTrait = NULL;
		if (col==0)	// City
		{
			pTrait = new CGridColumnTraitEdit;
		}
		if (col==1)	// State
		{
			CGridColumnTraitCombo* pComboTrait = new CGridColumnTraitCombo;
			const vector<string>& states = m_DataModel.GetStates();
			for(size_t i=0; i < states.size() ; ++i)
				pComboTrait->AddItem((int)i, states[i].c_str());
			pTrait = pComboTrait;
		}
		if (col==2)	// Country
		{
			CGridColumnTraitCombo* pComboTrait = new CGridColumnTraitCombo;
			const vector<string>& countries = m_DataModel.GetCountries();
			for(size_t i=0; i < countries.size() ; ++i)
				pComboTrait->AddItem((int)i, countries[i].c_str());
			pTrait = pComboTrait;
		}
		m_ListCtrl.InsertColumnTrait(col+1, title.c_str(), LVCFMT_LEFT, 100, col, pTrait);
	}

	// Insert data into list-control by copying from datamodel
	int nItem = 0;
	for(size_t rowId = 0; rowId < m_DataModel.GetRowIds() ; ++rowId)
	{
		nItem = m_ListCtrl.InsertItem(++nItem, m_DataModel.GetCellText(rowId, 0).c_str());
		m_ListCtrl.SetItemData(nItem, rowId);
		for(int col = 0; col < m_DataModel.GetColCount() ; ++col)
		{
			m_ListCtrl.SetItemText(nItem, col+1, m_DataModel.GetCellText(rowId, col).c_str());
		}
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGridListCtrlExDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGridListCtrlExDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGridListCtrlExDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
