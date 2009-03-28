#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

class CGridListCtrlEx;

//------------------------------------------------------------------------
//! CGridColumnEditor specifies the interface for the column editor
//! that can work together with CGridListCtrlEx
//------------------------------------------------------------------------
class CGridColumnEditor
{
public:
	//! Destructor
	virtual ~CGridColumnEditor() {}

	//! Is there a column configuration editor available for this column ?
	//!
	//! @param owner The list control with the column
	//! @param nCol The index of the column
	//! @param strTitle Title to show in the context menu when right-clicking the column
	//! @return Column editor available (true / false)
	virtual bool HasColumnEditor(CGridListCtrlEx& owner, int nCol, CString& strTitle) { return false; }
	//! Open the column configuration editor for the column
	//!
	//! @param owner The list control with the column
	//! @param nCol The index of the column
	virtual void OpenColumnEditor(CGridListCtrlEx& owner, int nCol){}

	//! Is there a column picker available that can add / remove columns
	//!
	//! @param owner The list control with the columns
	//! @param strTitle Title to show in the context menu when right-clicking the column
	//! @return Column picker available (true / false)
	virtual bool HasColumnPicker(CGridListCtrlEx& owner, CString& strTitle) { return false; }
	//! Open the column picker for the list control
	//!
	//! @param owner The list control with the column
	virtual void OpenColumnPicker(CGridListCtrlEx& owner) {}

	//! Is it possible to reset the column configuration to its default configuration ?
	//!
	//! @param owner The list control with the columns
	//! @param strTitle Title to show in the context menu when right-clicking the column
	//! @return Default column configuration is available (true / false)
	virtual bool HasColumnsDefault(CGridListCtrlEx& owner, CString& strTitle) { return false; }
	//! Reset the column configuration to its default configuration
	//!
	//! @param owner The list control with the columns
	virtual void ResetColumnsDefault(CGridListCtrlEx& owner) {}

	//! Is it possible to switch between multiple column configurations ?
	//!
	//! @param owner The list control with the columns
	//! @param profiles List of available column profiles
	//! @param strTitle Title to show in the context menu when right-clicking the column
	//! @return Name of the current column profile
	virtual CString HasColumnProfiles(CGridListCtrlEx& owner, CSimpleArray<CString>& profiles, CString& strTitle) { return _T(""); }
	//! Switch to different column configurations profile
	//!
	//! @param owner The list control with the columns
	//! @param strProfile List of available column profiles
	virtual void SwichColumnProfile(CGridListCtrlEx& owner, const CString& strProfile) {}

	//! Called when the column configuration is setup for the list control
	//!
	//! @param owner The list control with the columns
	virtual void OnColumnSetup(CGridListCtrlEx& owner){}
	//! Called when the list control looses focus to another control
	//!
	//! @param owner The list control with the columns
	virtual void OnOwnerKillFocus(CGridListCtrlEx& owner){}
	//! Called after a column has been resized
	//!
	//! @param owner The list control with the columns
	virtual void OnColumnResize(CGridListCtrlEx& owner){}
	//! Called after a column has been added / removed
	//!
	//! @param owner The list control with the columns
	virtual void OnColumnPick(CGridListCtrlEx& owner){}
};