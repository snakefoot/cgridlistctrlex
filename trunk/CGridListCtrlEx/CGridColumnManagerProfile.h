#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnManager.h"

class CViewConfigSectionProfiles;
class CViewConfigSection;

//------------------------------------------------------------------------
//! Implementation of the CGridColumnManager interface, that supports
//! persistance of the column state.
//!
//! - Persistence of column order and width for the visible columns.
//! - Can reset the column configuration back to its default state.
//! - Can switch between multiple column configuration profiles.
//------------------------------------------------------------------------
class CGridColumnManagerProfile : public CGridColumnManager
{
protected:
	CViewConfigSectionProfiles* m_pColumnConfig;	//!< Interface for persisting the column configuration
	bool m_ApplyingConfiguration;				//!< Currently loading / saving the column configuration

	virtual void SaveColumnConfiguration(int nConfigCol, int nOwnerCol, CGridListCtrlEx& owner, CViewConfigSection& config);
	virtual void LoadColumnConfiguration(int nConfigCol, int nOwnerCol, CGridListCtrlEx& owner, CViewConfigSection& config);

public:
	explicit CGridColumnManagerProfile(const CString& strViewName);
	explicit CGridColumnManagerProfile(CViewConfigSectionProfiles* pColumnConfig);
	virtual ~CGridColumnManagerProfile();

	virtual bool HasColumnsDefault(CGridListCtrlEx& owner, CString& strTitle);
	virtual void ResetColumnsDefault(CGridListCtrlEx& owner);

	virtual void AddColumnProfile(const CString& strProfile);
	virtual void DeleteColumnProfile(const CString& strProfile);
	virtual CString HasColumnProfiles(CGridListCtrlEx& owner, CSimpleArray<CString>& profiles, CString& strTitle);
	virtual void SwichColumnProfile(CGridListCtrlEx& owner, const CString& strProfile);

	virtual void OnColumnSetup(CGridListCtrlEx& owner);
	virtual void OnOwnerKillFocus(CGridListCtrlEx& owner);
	virtual void OnColumnResize(CGridListCtrlEx& owner);
	virtual void OnColumnPick(CGridListCtrlEx& owner);

	virtual void LoadConfiguration(CGridListCtrlEx& owner, CViewConfigSection& config);
	virtual void SaveConfiguration(CGridListCtrlEx& owner, CViewConfigSection& config);
};

