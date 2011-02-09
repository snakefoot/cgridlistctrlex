#include "stdafx.h"
//#pragma warning(disable:4100)	// unreferenced formal parameter

#include "CGridColumnManagerProfile.h"
#include "ViewConfigSection.h"

#include "CGridListCtrlEx.h"
#include "CGridColumnTrait.h"

//------------------------------------------------------------------------
//! CGridColumnManagerProfile - Constructor
//!
//! @param strViewName Name to identify and persist the configuration
//------------------------------------------------------------------------
CGridColumnManagerProfile::CGridColumnManagerProfile(const CString& strViewName)
{
	m_ApplyingConfiguration = false;
	m_pColumnConfig = new CViewConfigSectionWinApp(strViewName);
}

//------------------------------------------------------------------------
//! CGridColumnManagerProfile - Constructor
//!
//! @param pColumnConfig Interface for persisting the configuration
//------------------------------------------------------------------------
CGridColumnManagerProfile::CGridColumnManagerProfile(CViewConfigSectionProfiles* pColumnConfig)
{
	m_ApplyingConfiguration = false;
	m_pColumnConfig = pColumnConfig;
}

//------------------------------------------------------------------------
//! CGridColumnManagerProfile - Destructor
//------------------------------------------------------------------------
CGridColumnManagerProfile::~CGridColumnManagerProfile()
{
	delete m_pColumnConfig;
	m_pColumnConfig = NULL;
}

//------------------------------------------------------------------------
//! Saves the column configuration of the list control
//!
//! @param owner The list control
//! @param config The interface for persisting the configuration
//------------------------------------------------------------------------
void CGridColumnManagerProfile::SaveConfiguration(CGridListCtrlEx& owner, CViewConfigSection& config)
{
	if (m_ApplyingConfiguration)
		return;

	config.RemoveCurrentConfig();	// Reset the existing config

	int nColCount = owner.GetColumnCount();
	int* pOrderArray = new int[nColCount];
	owner.GetColumnOrderArray(pOrderArray, nColCount);

	int nVisibleCols = 0;
	for(int i = 0 ; i < nColCount; ++i)
	{
		int nCol = pOrderArray[i];
		int nColData = owner.GetColumnData(nCol);

		if (owner.IsColumnVisible(nCol))
		{
			CString colSetting;
			colSetting.Format(_T("ColumnData_%d"), nVisibleCols);
			config.SetIntSetting(colSetting, nColData);

			SaveColumnConfiguration(nVisibleCols, nCol, owner, config);

			nVisibleCols++;
		}
	}
	config.SetIntSetting(_T("ColumnCount"), nVisibleCols);

	delete [] pOrderArray;
}

//------------------------------------------------------------------------
//! Loads and applies the column configuration for the list control
//!
//! @param owner The list control
//! @param config The interface for persisting the configuration
//------------------------------------------------------------------------
void CGridColumnManagerProfile::LoadConfiguration(CGridListCtrlEx& owner, CViewConfigSection& config)
{
	if (!m_pColumnConfig->HasDefaultConfig())
	{
		// Validate that column data is setup correctly
		CSimpleMap<int,int> uniqueChecker;
		for(int nCol = 0; nCol < owner.GetColumnCount(); ++nCol)
		{
			if (owner.IsColumnAlwaysHidden(nCol))
				continue;
			int nColData = owner.GetColumnData(nCol);
			ASSERT(uniqueChecker.FindKey(nColData)==-1);
			uniqueChecker.Add(nColData,nCol);
		}

		SaveConfiguration(owner, m_pColumnConfig->GetDefaultConfig());
	}

	m_ApplyingConfiguration = true;

	int nVisibleCols = config.GetIntSetting(_T("ColumnCount"));

	int nColCount = owner.GetColumnCount();
	int* pOrderArray = new int[nColCount];
	owner.GetColumnOrderArray(pOrderArray, nColCount);

	owner.SetRedraw(FALSE);

	// All invisible columns must be place in the begining of the order-array
	int nColOrder = nColCount;
	for(int i = nVisibleCols-1; i >= 0; --i)
	{
		CString colSetting;
		colSetting.Format(_T("ColumnData_%d"), i);
		int nColData = config.GetIntSetting(colSetting);
		for(int nCol = 0; nCol < nColCount; ++nCol)
		{
			// Check if already in array
			bool alreadyIncluded = false;
			for(int j = nColOrder; j < nColCount; ++j)
				if (pOrderArray[j]==nCol)
				{
					alreadyIncluded = true;
					break;
				}
			if (alreadyIncluded)
				continue;

			if (nColData==owner.GetColumnData(nCol))
			{
				// Column still exists
				if (owner.IsColumnAlwaysHidden(nCol))
					continue;

				CGridColumnTrait::ColumnState& columnState = owner.GetColumnTrait(nCol)->GetColumnState();
				columnState.m_Visible = true;
				LoadColumnConfiguration(i, nCol, owner, config);
				pOrderArray[--nColOrder] = nCol;
				break;
			}
		}
	}

	// Are there any always visible columns, that we must ensure are visible ?
	for(int nCol = nColCount-1; nCol >= 0; --nCol)
	{
		if (!owner.IsColumnAlwaysVisible(nCol))
			continue;

		bool visible = false;
		for(int i = nColOrder; i < nColCount; ++i)
		{
			if (pOrderArray[i]==nCol)
			{
				visible = true;
				break;
			}
		}
		if (!visible)
		{
			CGridColumnTrait::ColumnState& columnState = owner.GetColumnTrait(nCol)->GetColumnState();
			columnState.m_Visible = true;
			pOrderArray[--nColOrder] = nCol;
		}
	}

	// Did we find any visible columns in the saved configuration ?
	if (nColOrder < nColCount)
	{
		// All remaining columns are marked as invisible
		while(nColOrder > 0)
		{
			// Find nCol som ikke er i array
			int nCol = -1;
			for(nCol = nColCount-1; nCol >= 0; --nCol)
			{
				bool visible = false;
				for(int j = nColOrder; j < nColCount; ++j)
				{
					if (pOrderArray[j]==nCol)
					{
						visible = true;
						break;
					}
				}
				if (!visible)
					break;
			}
			ASSERT(nCol!=-1);
			CGridColumnTrait::ColumnState& columnState = owner.GetColumnTrait(nCol)->GetColumnState();
			columnState.m_OrgPosition = -1;
			columnState.m_OrgWidth = owner.GetColumnWidth(nCol);
			owner.SetColumnWidth(nCol, 0);
			columnState.m_Visible = false;
			ASSERT(nColOrder>0);
			pOrderArray[--nColOrder] = nCol;
		}

		// Only update the column configuration if there are visible columns
		ASSERT(nColOrder==0);	// All entries in the order-array must be set
		owner.SetColumnOrderArray(nColCount, pOrderArray);
	}

	delete [] pOrderArray;

	m_ApplyingConfiguration = false;

	owner.SetRedraw(TRUE);
	owner.Invalidate(TRUE);
	owner.UpdateWindow();
}

//------------------------------------------------------------------------
//! Saves the column configuration of a single column
//!
//! @param nConfigCol The column index in the persisting interface
//! @param nOwnerCol The column index in the owner list control
//! @param owner The list control
//! @param config The interface for persisting the configuration
//------------------------------------------------------------------------
void CGridColumnManagerProfile::SaveColumnConfiguration(int nConfigCol, int nOwnerCol, CGridListCtrlEx& owner, CViewConfigSection& config)
{
	CString colSetting;
	colSetting.Format(_T("ColumnWidth_%d"), nConfigCol);
	config.SetIntSetting(colSetting, owner.GetColumnWidth(nOwnerCol));
}

//------------------------------------------------------------------------
//! Loads the column configuration of a single column
//!
//! @param nConfigCol The column index in the persisting interface
//! @param nOwnerCol The column index in the owner list control
//! @param owner The list control
//! @param config The interface for persisting the configuration
//------------------------------------------------------------------------
void CGridColumnManagerProfile::LoadColumnConfiguration(int nConfigCol, int nOwnerCol, CGridListCtrlEx& owner, CViewConfigSection& config)
{
	CString colSetting;
	if (owner.IsColumnResizable(nOwnerCol))
	{
		colSetting.Format(_T("ColumnWidth_%d"), nConfigCol);
		int width = config.GetIntSetting(colSetting);
		owner.SetColumnWidth(nOwnerCol, width);
	}
}

//------------------------------------------------------------------------
//! Has the ability to reset the column configuration to its default configuration
//!
//! @param owner The list control with the columns
//! @param strTitle Title to show in the context menu when right-clicking the column
//! @return Default column configuration is available (true / false)
//------------------------------------------------------------------------
bool CGridColumnManagerProfile::HasColumnsDefault(CGridListCtrlEx& owner, CString& strTitle)
{
	strTitle = _T("Reset columns");
	return m_pColumnConfig->HasDefaultConfig();
}

//------------------------------------------------------------------------
//! Reset the column configuration to its default configuration
//!
//! @param owner The list control with the columns
//------------------------------------------------------------------------
void CGridColumnManagerProfile::ResetColumnsDefault(CGridListCtrlEx& owner)
{
	m_pColumnConfig->ResetConfigDefault();
	LoadConfiguration(owner, *m_pColumnConfig);
}

//------------------------------------------------------------------------
//! Adds a profile to the official list of column configuration profiles
//!
//! @param strProfile Name of the profile
//------------------------------------------------------------------------
void CGridColumnManagerProfile::AddColumnProfile(const CString& strProfile)
{
	m_pColumnConfig->AddProfile(strProfile);
}

//------------------------------------------------------------------------
//! Removes a profile from the official list of column configuration profiles
//!
//! @param strProfile Name of the profile
//------------------------------------------------------------------------
void CGridColumnManagerProfile::DeleteColumnProfile(const CString& strProfile)
{
	m_pColumnConfig->DeleteProfile(strProfile);
}

//------------------------------------------------------------------------
//! Can switch between multiple column configurations
//!
//! @param owner The list control with the columns
//! @param profiles List of available column profiles
//! @param strTitle Title to show in the context menu when right-clicking the column
//! @return Name of the current column profile
//------------------------------------------------------------------------
CString CGridColumnManagerProfile::HasColumnProfiles(CGridListCtrlEx& owner, CSimpleArray<CString>& profiles, CString& strTitle)
{
	strTitle = _T("Column Profiles");
	m_pColumnConfig->GetProfiles(profiles);
	return m_pColumnConfig->GetActiveProfile();
}

//------------------------------------------------------------------------
//! Switch to different column configurations profile
//!
//! @param owner The list control with the columns
//! @param strProfile List of available column profiles
//------------------------------------------------------------------------
void CGridColumnManagerProfile::SwichColumnProfile(CGridListCtrlEx& owner, const CString& strProfile)
{
	// Save the current configuration before switching to the new one
	SaveConfiguration(owner, *m_pColumnConfig);
	m_pColumnConfig->SetActiveProfile(strProfile);
	LoadConfiguration(owner, *m_pColumnConfig);
}

//------------------------------------------------------------------------
//! Called when the column configuration is setup for the list control
//!
//! @param owner The list control with the columns
//------------------------------------------------------------------------
void CGridColumnManagerProfile::OnColumnSetup(CGridListCtrlEx& owner)
{
	LoadConfiguration(owner, *m_pColumnConfig);
}

//------------------------------------------------------------------------
//! Called when the list control looses focus to another control
//!
//! @param owner The list control with the columns
//------------------------------------------------------------------------
void CGridColumnManagerProfile::OnOwnerKillFocus(CGridListCtrlEx& owner)
{
	SaveConfiguration(owner, *m_pColumnConfig);
}

//------------------------------------------------------------------------
//! Called after a column has been resized
//!
//! @param owner The list control with the columns
//------------------------------------------------------------------------
void CGridColumnManagerProfile::OnColumnResize(CGridListCtrlEx& owner)
{
	SaveConfiguration(owner, *m_pColumnConfig);
}

//------------------------------------------------------------------------
//! Called after a column has been added / removed
//!
//! @param owner The list control with the columns
//------------------------------------------------------------------------
void CGridColumnManagerProfile::OnColumnPick(CGridListCtrlEx& owner)
{
	SaveConfiguration(owner, *m_pColumnConfig);
}
