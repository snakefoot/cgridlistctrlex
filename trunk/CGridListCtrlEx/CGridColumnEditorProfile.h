#pragma once

#include "CGridColumnEditor.h"

// Independent of view-type and persistence-layer
class CGridColumnConfig
{
protected:
	CString m_ViewName;

	// Persistence of settings
	virtual CString ReadSetting(const CString& section, const CString& setting, const CString& defval) const = 0;
	virtual void WriteSetting(const CString& section, const CString& setting, const CString& value) = 0;
	virtual void RemoveSection(const CString& section) = 0;

	// Converters
	virtual CString ConvertBoolSetting(bool value) const;
	virtual CString ConvertIntSetting(int value) const;
	virtual CString ConvertFloatSetting(double value, int decimals = 6) const;
	virtual CString ConvertArraySetting(const CSimpleArray<CString>& values, const CString& delimiter = _T(", ")) const;
	virtual CString ConvertArraySetting(const CSimpleArray<int>& values, const CString& delimiter = _T(", ")) const;
	virtual CString ConvertLogFontSetting(const LOGFONT& font) const;
	virtual CString ConvertRectSetting(const RECT& rect) const;
	virtual CString ConvertColorSetting(COLORREF color) const;
	virtual void	SplitArraySetting(const CString& strArray, CSimpleArray<CString>& values, const CString& delimiter = _T(", ")) const;

	virtual const CString& GetSectionName() const;

public:
	explicit CGridColumnConfig(const CString& name) : m_ViewName(name) {}
	virtual ~CGridColumnConfig() {}

	// Getters
	virtual CString GetSetting(const CString& name, const CString& defval = _T("")) const;
	virtual bool GetBoolSetting(const CString& name, bool defval = false) const;
	virtual int GetIntSetting(const CString& name, int defval = 0) const;
	virtual double GetFloatSetting(const CString& name, double defval = 0.0) const;
	virtual LOGFONT GetLogFontSetting(const CString& name) const;
	virtual CRect GetRectSetting(const CString& name, const CRect& defval = CRect(0,0,0,0)) const;
	virtual COLORREF GetColorSetting(const CString& name, const COLORREF defval = RGB(0,0,0)) const;
	virtual void GetArraySetting(const CString& name, CSimpleArray<CString>& values, const CString& delimiter = _T(", ")) const;
	virtual void GetArraySetting(const CString& name, CSimpleArray<int>& values, const CString& delimiter = _T(", ")) const;

	// Setters
	virtual void SetSetting(const CString& name, const CString& value);
	virtual void SetBoolSetting(const CString& name, bool value);
	virtual void SetIntSetting(const CString& name, int value);
	virtual void SetFloatSetting(const CString& name, double value, int decimals = 6);
	virtual void SetArraySetting(const CString& name, const CSimpleArray<CString>& values, const CString& delimiter = _T(", "));
	virtual void SetArraySetting(const CString& name, const CSimpleArray<int>& values, const CString& delimiter = _T(", "));
	virtual void SetLogFontSetting(const CString& name, const LOGFONT& font);
	virtual void SetRectSetting(const CString& name, const RECT& rect);
	virtual void SetColorSetting(const CString& name, COLORREF color);

	virtual void RemoveCurrentConfig();
};

// Provides the ability to store an in-memory default-configuration
// It will use the values in the default-config if nothing else can be found
class CGridColumnConfigDefault : public CGridColumnConfig
{
protected:
	class CGridColumnConfigLocal : public CGridColumnConfig
	{
	protected:
		CSimpleMap<CString,CString> m_LocalSettings;

		// Persistence of settings
		virtual CString ReadSetting(const CString& section, const CString& name, const CString& defval) const;
		virtual void WriteSetting(const CString& section, const CString& name, const CString& value);
		virtual void RemoveSection(const CString& section);

	public:
		explicit CGridColumnConfigLocal(const CString& viewname);
		CGridColumnConfigLocal(const CGridColumnConfigLocal& other);
		CGridColumnConfigLocal& operator=(const CGridColumnConfigLocal& other);

		bool HasDefaultSettings() const;
		void CopySettings(CGridColumnConfig& destination) const;
	};
	CGridColumnConfigLocal m_DefaultSettings;

public:
	explicit CGridColumnConfigDefault(const CString& viewname);

	virtual CGridColumnConfig& GetDefaultConfig() { return m_DefaultSettings; }
	virtual bool HasDefaultSettings() const { return m_DefaultSettings.HasDefaultSettings(); }
	virtual void ResetSettingsDefault() { RemoveSection(GetSectionName()); m_DefaultSettings.CopySettings(*this); }

	virtual CString GetSetting(const CString& name, const CString& defval = _T("")) const;
};

// Provides the ability to change between multiple configurations for the same view
class CGridColumnConfigProfiles : public CGridColumnConfigDefault
{
protected:
	mutable CString m_CurrentSection;
	virtual const CString& GetSectionName() const;

	virtual void SplitSectionName(const CString& section, CString& view, CString& profile);
	virtual CString JoinSectionName(const CString& view, const CString& profile) const;

public:
	explicit CGridColumnConfigProfiles(const CString& viewname);

	virtual void RemoveCurrentConfig();

	virtual void GetProfiles(CSimpleArray<CString>& profiles) const;
	virtual CString GetActiveProfile();
	virtual void SetActiveProfile(const CString& profile);
	virtual void AddProfile(const CString& profile);
	virtual void DeleteProfile(const CString& profile);
};

class CGridColumnConfigWinApp : public CGridColumnConfigProfiles
{
protected:
	virtual CString ReadSetting(const CString& section, const CString& setting, const CString& defval) const;
	virtual void WriteSetting(const CString& section, const CString& setting, const CString& value);
	virtual void RemoveSection(const CString& section);

public:
	CGridColumnConfigWinApp(const CString& viewname);
};

class CGridColumnEditorProfile : public CGridColumnEditor
{
protected:
	CGridColumnConfigProfiles* m_pColumnConfig;
	bool m_ApplyingConfiguration;

	virtual void SaveColumnConfiguration(int nConfigCol, int nOwnerCol, CGridListCtrlEx& owner, CGridColumnConfig& config);
	virtual void LoadColumnConfiguration(int nConfigCol, int nOwnerCol, CGridListCtrlEx& owner, CGridColumnConfig& config);

public:
	explicit CGridColumnEditorProfile(const CString& viewname);
	explicit CGridColumnEditorProfile(CGridColumnConfigProfiles* pColumnConfig);
	virtual ~CGridColumnEditorProfile();

	virtual bool HasColumnsDefault(CGridListCtrlEx& owner, CString& title);
	virtual void ResetColumnsDefault(CGridListCtrlEx& owner);

	virtual void AddColumnProfile(const CString& profile);
	virtual void DeleteColumnProfile(const CString& profile);
	virtual CString HasColumnProfiles(CGridListCtrlEx& owner, CSimpleArray<CString>& profiles, CString& title);
	virtual void SwichColumnProfile(CGridListCtrlEx& owner, const CString& profile);

	virtual void OnColumnSetup(CGridListCtrlEx& owner);
	virtual void OnOwnerKillFocus(CGridListCtrlEx& owner);
	virtual void OnColumnResize(CGridListCtrlEx& owner);
	virtual void OnColumnPick(CGridListCtrlEx& owner);

	virtual void LoadConfiguration(CGridListCtrlEx& owner, CGridColumnConfig& config);
	virtual void SaveConfiguration(CGridListCtrlEx& owner, CGridColumnConfig& config);
};