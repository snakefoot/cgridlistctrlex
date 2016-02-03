// CGridListCtrlEx.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CGridListCtrlExApp:
// See CGridListCtrlEx.cpp for the implementation of this class
//

class CGridListCtrlExApp : public CWinApp
{
public:
	CGridListCtrlExApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

private:
	CGridListCtrlExApp(const CGridListCtrlExApp&);
	CGridListCtrlExApp& operator=(const CGridListCtrlExApp&);
};

extern CGridListCtrlExApp theApp;