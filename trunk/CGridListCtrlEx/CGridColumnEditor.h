#pragma once

class CGridColumnEditor
{
public:
	virtual ~CGridColumnEditor() {}
	virtual bool HasColumnEditor(CGridListCtrlEx& owner, int nCol, CString& title) { return false; }
	virtual void OpenColumnEditor(CGridListCtrlEx& owner, int nCol){}
	virtual bool HasColumnPicker(CGridListCtrlEx& owner, CString& title) { return false; }
	virtual void OpenColumnPicker(CGridListCtrlEx& owner) {}
	virtual void OnColumnDrag(CGridListCtrlEx& owner){}
	virtual void OnColumnResize(CGridListCtrlEx& owner){}
};