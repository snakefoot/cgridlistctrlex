#pragma once

class CGridListCtrlEx;

class CGridColumnEditor
{
public:
	virtual ~CGridColumnEditor() {}

	virtual bool HasColumnEditor(CGridListCtrlEx& owner, int nCol, CString& title) { return false; }
	virtual void OpenColumnEditor(CGridListCtrlEx& owner, int nCol){}

	virtual bool HasColumnPicker(CGridListCtrlEx& owner, CString& title) { return false; }
	virtual void OpenColumnPicker(CGridListCtrlEx& owner) {}

	virtual bool HasColumnsDefault(CGridListCtrlEx& owner, CString& title) { return false; }
	virtual void ResetColumnsDefault(CGridListCtrlEx& owner) {}

	virtual CString HasColumnProfiles(CGridListCtrlEx& owner, CSimpleArray<CString>& profiles, CString& title) { return _T(""); }
	virtual void SwichColumnProfile(CGridListCtrlEx& owner, const CString& profile) {}

	virtual void OnColumnSetup(CGridListCtrlEx& owner){}
	virtual void OnOwnerKillFocus(CGridListCtrlEx& owner){}
	virtual void OnColumnResize(CGridListCtrlEx& owner){}
	virtual void OnColumnPick(CGridListCtrlEx& owner){}
};