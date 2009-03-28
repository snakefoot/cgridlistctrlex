#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnEditor.h"

class CGridColumnConfigProfiles;
class CGridColumnConfig;

//------------------------------------------------------------------------
//! Implementation of the CGridColumnEditor interface, that supports
//! persistance of the column state.
//------------------------------------------------------------------------
class CGridColumnEditorProfile : public CGridColumnEditor
{
protected:
	CGridColumnConfigProfiles* m_pColumnConfig;	//!< Interface for persisting the column configuration
	bool m_ApplyingConfiguration;				//!< Currently loading / saving the column configuration

	virtual void SaveColumnConfiguration(int nConfigCol, int nOwnerCol, CGridListCtrlEx& owner, CGridColumnConfig& config);
	virtual void LoadColumnConfiguration(int nConfigCol, int nOwnerCol, CGridListCtrlEx& owner, CGridColumnConfig& config);

public:
	explicit CGridColumnEditorProfile(const CString& strViewName);
	explicit CGridColumnEditorProfile(CGridColumnConfigProfiles* pColumnConfig);
	virtual ~CGridColumnEditorProfile();

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

	virtual void LoadConfiguration(CGridListCtrlEx& owner, CGridColumnConfig& config);
	virtual void SaveConfiguration(CGridListCtrlEx& owner, CGridColumnConfig& config);
};

//------------------------------------------------------------------------
//! Abstract interface for persisting column configuration
//------------------------------------------------------------------------
class CGridColumnConfig
{
protected:
	CString m_ViewName;		//!< Configuration name used when persisting the state (Translates into a section name)

	//! Pure virtual interface for reading setting from persisting layer
	virtual CString ReadSetting(const CString& strSection, const CString& strSetting, const CString& strDefval) const = 0;
	//! Pure virtual interface for writing setting to persisting layer
	virtual void WriteSetting(const CString& strSection, const CString& strSetting, const CString& strValue) = 0;
	//! Pure virtual interface for removing setting section from persisting layer
	virtual void RemoveSection(const CString& strSection) = 0;

	// Converters
	virtual CString ConvertBoolSetting(bool bValue) const;
	virtual CString ConvertIntSetting(int nValue) const;
	virtual CString ConvertFloatSetting(double nValue, int nDecimals = 6) const;
	virtual CString ConvertArraySetting(const CSimpleArray<CString>& values, const CString& strDelimiter = _T(", ")) const;
	virtual CString ConvertArraySetting(const CSimpleArray<int>& values, const CString& strDelimiter = _T(", ")) const;
	virtual CString ConvertLogFontSetting(const LOGFONT& font) const;
	virtual CString ConvertRectSetting(const RECT& rect) const;
	virtual CString ConvertColorSetting(COLORREF color) const;
	virtual void	SplitArraySetting(const CString& strArray, CSimpleArray<CString>& values, const CString& strDelimiter = _T(", ")) const;

	virtual const CString& GetSectionName() const;

public:
	explicit CGridColumnConfig(const CString& strViewName);
	virtual ~CGridColumnConfig();

	// Getters
	virtual CString GetSetting(const CString& strName, const CString& strDefval = _T("")) const;
	virtual bool GetBoolSetting(const CString& strName, bool bDefval = false) const;
	virtual int GetIntSetting(const CString& strName, int nDefval = 0) const;
	virtual double GetFloatSetting(const CString& strName, double nDefval = 0.0) const;
	virtual LOGFONT GetLogFontSetting(const CString& strName) const;
	virtual CRect GetRectSetting(const CString& strName, const CRect& rectDefval = CRect(0,0,0,0)) const;
	virtual COLORREF GetColorSetting(const CString& strName, const COLORREF colorDefval = RGB(0,0,0)) const;
	virtual void GetArraySetting(const CString& strName, CSimpleArray<CString>& values, const CString& strDelimiter = _T(", ")) const;
	virtual void GetArraySetting(const CString& strName, CSimpleArray<int>& values, const CString& strDelimiter = _T(", ")) const;

	// Setters
	virtual void SetSetting(const CString& strName, const CString& strValue);
	virtual void SetBoolSetting(const CString& strName, bool bValue);
	virtual void SetIntSetting(const CString& strName, int nValue);
	virtual void SetFloatSetting(const CString& strName, double nValue, int nDecimals = 6);
	virtual void SetArraySetting(const CString& strName, const CSimpleArray<CString>& values, const CString& strDelimiter = _T(", "));
	virtual void SetArraySetting(const CString& strName, const CSimpleArray<int>& values, const CString& strDelimiter = _T(", "));
	virtual void SetLogFontSetting(const CString& strName, const LOGFONT& font);
	virtual void SetRectSetting(const CString& strName, const RECT& rect);
	virtual void SetColorSetting(const CString& strName, COLORREF color);

	virtual void RemoveCurrentConfig();
};

//------------------------------------------------------------------------
//! Abstract interface for persisting column configuration, that can use
//! an in-memory default-configuration.
//!
//! It will use the values in the default-config if nothing else can be found
//------------------------------------------------------------------------
class CGridColumnConfigDefault : public CGridColumnConfig
{
protected:
	//! Inner class that stores the default configuration in memory
	class CGridColumnConfigLocal : public CGridColumnConfig
	{
	protected:
		CSimpleMap<CString,CString> m_LocalSettings;	//! Default configuration

		// Persistence of settings
		virtual CString ReadSetting(const CString& strSection, const CString& strName, const CString& strDefval) const;
		virtual void WriteSetting(const CString& strSection, const CString& strName, const CString& strValue);
		virtual void RemoveSection(const CString& strSection);

	public:
		explicit CGridColumnConfigLocal(const CString& strViewName);
		CGridColumnConfigLocal(const CGridColumnConfigLocal& other);
		CGridColumnConfigLocal& operator=(const CGridColumnConfigLocal& other);

		bool HasDefaultSettings() const;
		void CopySettings(CGridColumnConfig& destination) const;
	};
	CGridColumnConfigLocal m_DefaultSettings;

public:
	explicit CGridColumnConfigDefault(const CString& strViewName);

	virtual CGridColumnConfig& GetDefaultConfig();
	virtual bool HasDefaultSettings() const;
	virtual void ResetSettingsDefault();

	virtual CString GetSetting(const CString& strName, const CString& strDefval = _T("")) const;
};

//------------------------------------------------------------------------
//! Abstract interface for persisting column configuration, that can switch
//! between different column configuration profiles.
//------------------------------------------------------------------------
class CGridColumnConfigProfiles : public CGridColumnConfigDefault
{
protected:
	mutable CString m_CurrentSection;	//! Section name combined from the viewname and the current profile name
	virtual const CString& GetSectionName() const;

	virtual void SplitSectionName(const CString& strSection, CString& strViewName, CString& strProfile);
	virtual CString JoinSectionName(const CString& strViewName, const CString& strProfile) const;

public:
	explicit CGridColumnConfigProfiles(const CString& strViewName);

	virtual void RemoveCurrentConfig();

	virtual void GetProfiles(CSimpleArray<CString>& profiles) const;
	virtual CString GetActiveProfile();
	virtual void SetActiveProfile(const CString& strProfile);
	virtual void AddProfile(const CString& strProfile);
	virtual void DeleteProfile(const CString& strProfile);
};

//------------------------------------------------------------------------
//! Can persist the column configuration using CWinApp::WriteProfile()
//------------------------------------------------------------------------
class CGridColumnConfigWinApp : public CGridColumnConfigProfiles
{
protected:
	virtual CString ReadSetting(const CString& strSection, const CString& strSetting, const CString& strDefval) const;
	virtual void WriteSetting(const CString& strSection, const CString& strSetting, const CString& strValue);
	virtual void RemoveSection(const CString& strSection);

public:
	CGridColumnConfigWinApp(const CString& strViewName);
};