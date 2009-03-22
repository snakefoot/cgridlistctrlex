#include "stdafx.h"
#include "CGridColumnEditorProfile.h"

#include "CGridListCtrlEx.h"
#include "CGridColumnTrait.h"

//------------------------------------------------------------------------
//! CGridColumnConfig
//------------------------------------------------------------------------
CString CGridColumnConfig::GetSetting(const CString& name, const CString& defval) const
{
	return ReadSetting(GetSectionName(), name, defval);
}

void CGridColumnConfig::SetSetting(const CString& name, const CString& value)
{
	WriteSetting(GetSectionName(), name, value);
}

const CString& CGridColumnConfig::GetSectionName() const
{
	return m_ViewName;
}

void CGridColumnConfig::RemoveCurrentConfig()
{
	RemoveSection(GetSectionName());
}

bool CGridColumnConfig::GetBoolSetting(const CString& name, bool defval) const
{
	const CString& value = GetSetting(name, ConvertBoolSetting(defval));
	if (value==_T("TRUE"))
		return true;
	else
	if (value==_T("FALSE"))
		return false;
	else
		return defval;
}

CString CGridColumnConfig::ConvertBoolSetting(bool value) const
{
	return value ? _T("TRUE") : _T("FALSE");
}

void CGridColumnConfig::SetBoolSetting(const CString& name, bool value)
{
	SetSetting(name, ConvertBoolSetting(value));
}

int CGridColumnConfig::GetIntSetting(const CString& name, int defval) const
{
	const CString& value = GetSetting(name, ConvertIntSetting(defval));
	return _ttoi(value);
}

CString CGridColumnConfig::ConvertIntSetting(int value) const
{
	CString strValue;
	strValue.Format(_T("%d"), value);
	return strValue;
}

void CGridColumnConfig::SetIntSetting(const CString& name, int value)
{
	SetSetting(name, ConvertIntSetting(value));
}

double CGridColumnConfig::GetFloatSetting(const CString& name, double defval) const
{
	const CString& value = GetSetting(name, ConvertFloatSetting(defval));
	return _tstof(value);
}

CString CGridColumnConfig::ConvertFloatSetting(double value, int decimals) const
{
	CString dblFormat;
	dblFormat.Format(_T("%%.%df"), decimals);

	CString strValue;	
	strValue.Format(dblFormat, value);
	return strValue;
}

void CGridColumnConfig::SetFloatSetting(const CString& name, double value, int decimals)
{
	SetSetting(name, ConvertFloatSetting(value, decimals));
}

void CGridColumnConfig::SplitArraySetting(const CString& strArray, CSimpleArray<CString>& values, const CString& delimiter) const
{
	// Perform tokenize using delimiter
	int cur_pos = 0;
	int prev_pos = 0;
	int length = strArray.GetLength();
	while(cur_pos < length)
	{
		cur_pos = strArray.Find(delimiter, prev_pos);
		if (cur_pos==-1)
		{
			CString value = strArray.Mid(prev_pos, length - prev_pos);
			values.Add(value);
			break;
		}
		else
		{
			CString value = strArray.Mid(prev_pos, cur_pos - prev_pos);
			values.Add(value);
			prev_pos = cur_pos + delimiter.GetLength();
		}
	}
}

void CGridColumnConfig::GetArraySetting(const CString& name, CSimpleArray<CString>& values, const CString& delimiter) const
{
	const CString& strArray = GetSetting(name, _T(""));
	if (strArray.IsEmpty())
		return;

	SplitArraySetting(strArray, values, delimiter);
}

CString CGridColumnConfig::ConvertArraySetting(const CSimpleArray<CString>& values, const CString& delimiter) const
{
	CString strValue;
	for(int i = 0; i < values.GetSize() ; ++i)
	{
		if (!strValue.IsEmpty())
			strValue += delimiter;
		strValue += values[i];
	}
	return strValue;
}

void CGridColumnConfig::SetArraySetting(const CString& name, const CSimpleArray<CString>& values, const CString& delimiter)
{
	SetSetting(name, ConvertArraySetting(values, delimiter));
}

void CGridColumnConfig::GetArraySetting(const CString& name, CSimpleArray<int>& values, const CString& delimiter) const
{
	CSimpleArray<CString> strArray;
	GetArraySetting(name, strArray, delimiter);
	for(int i = 0 ; i < strArray.GetSize(); ++i)
	{
		int value = _ttoi(strArray[i]);
		values.Add(value);
	}
}

CString CGridColumnConfig::ConvertArraySetting(const CSimpleArray<int>& values, const CString& delimiter) const
{
	CString strValue;
	CString strArray;
	for(int i = 0; i < values.GetSize(); ++i)
	{
		if (!strArray.IsEmpty())
			strArray += delimiter;
		strValue.Format( _T("%d"), values[i]);
		strArray += strValue;
	}
	return strArray;
}

void CGridColumnConfig::SetArraySetting(const CString& name, const CSimpleArray<int>& values, const CString& delimiter)
{
	SetSetting(name, ConvertArraySetting(values, delimiter));
}

LOGFONT CGridColumnConfig::GetLogFontSetting(const CString& name) const
{
	LOGFONT font = {0};

	CSimpleArray<CString> strArray;
	GetArraySetting(name, strArray);
	if (strArray.GetSize() != 13)
		return font;

#if __STDC_WANT_SECURE_LIB__
	_tcscpy_s(font.lfFaceName, sizeof(font.lfFaceName)/sizeof(TCHAR), strArray[0]);
#else
	_tcsncpy(font.lfFaceName, strArray[0], sizeof(font.lfFaceName)/sizeof(TCHAR));
#endif
	font.lfHeight = _ttoi(strArray[1]);
	font.lfWidth = _ttoi(strArray[2]);
	font.lfEscapement = _ttoi(strArray[3]);
	font.lfOrientation = _ttoi(strArray[4]);
	font.lfWeight = _ttoi(strArray[5]);
	font.lfItalic = (BYTE)_ttoi(strArray[6]);
	font.lfUnderline = (BYTE)_ttoi(strArray[7]);
	font.lfStrikeOut = (BYTE)_ttoi(strArray[8]);
	font.lfCharSet = (BYTE)_ttoi(strArray[9]);
	font.lfOutPrecision = (BYTE)_ttoi(strArray[10]);
	font.lfQuality = (BYTE)_ttoi(strArray[11]);
	font.lfPitchAndFamily = (BYTE)_ttoi(strArray[12]);
	return font;
}

CString CGridColumnConfig::ConvertLogFontSetting(const LOGFONT& font) const
{
	CSimpleArray<CString> strArray;

	CString value(font.lfFaceName, sizeof(font.lfFaceName)/sizeof(TCHAR));
	strArray.Add(value);
	value.Format(_T("%d"), font.lfHeight);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfWidth);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfEscapement);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfOrientation);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfWeight);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfItalic);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfUnderline);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfStrikeOut);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfCharSet);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfOutPrecision);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfQuality);
	strArray.Add(value);
	value.Format(_T("%d"), font.lfPitchAndFamily);
	strArray.Add(value);

	return ConvertArraySetting(strArray);
}

void CGridColumnConfig::SetLogFontSetting(const CString& name, const LOGFONT& font)
{
	SetSetting(name, ConvertLogFontSetting(font));
}

CRect CGridColumnConfig::GetRectSetting(const CString& name, const CRect& defval) const
{
	CSimpleArray<CString> strArray;
	GetArraySetting(name, strArray);
	if (strArray.GetSize() != 4)
		return defval;

	CRect rect(0,0,0,0);
    rect.left = _ttoi(strArray[0]);
    rect.top = _ttoi(strArray[1]);
    rect.right = _ttoi(strArray[2]);
    rect.bottom = _ttoi(strArray[3]);
	return rect;
}

CString CGridColumnConfig::ConvertRectSetting(const RECT& rect) const
{
	CSimpleArray<CString> strArray;
	CString value;
	value.Format(_T("%d"), rect.left);
	strArray.Add(value);
	value.Format(_T("%d"), rect.top);
	strArray.Add(value);
	value.Format(_T("%d"), rect.right);
	strArray.Add(value);
	value.Format(_T("%d"), rect.bottom);
	strArray.Add(value);

	return ConvertArraySetting(strArray);
}

void CGridColumnConfig::SetRectSetting(const CString& name, const RECT& rect)
{
	SetSetting(name, ConvertRectSetting(rect));
}

COLORREF CGridColumnConfig::GetColorSetting(const CString& name, const COLORREF defval) const
{
	CSimpleArray<CString> strArray;
	GetArraySetting(name, strArray);
	if (strArray.GetSize() != 3)
		return defval;

	int r = _ttoi(strArray[0]);
	int g = _ttoi(strArray[1]);
	int b = _ttoi(strArray[2]);

	return RGB(r,g,b);
}

CString CGridColumnConfig::ConvertColorSetting(COLORREF color) const
{
	CSimpleArray<CString> strArray;
	CString value;
	value.Format(_T("%d"), GetRValue(color));
	strArray.Add(value);
	value.Format(_T("%d"), GetGValue(color));
	strArray.Add(value);
	value.Format(_T("%d"), GetBValue(color));
	strArray.Add(value);

	return ConvertArraySetting(strArray);
}

void CGridColumnConfig::SetColorSetting(const CString& name, COLORREF color)
{
	SetSetting(name, ConvertColorSetting(color));
}

//------------------------------------------------------------------------
//! CGridColumnConfigDefault
//------------------------------------------------------------------------
CGridColumnConfigDefault::CGridColumnConfigLocal::CGridColumnConfigLocal(const CString& viewname)
:CGridColumnConfig(viewname)
{}

CGridColumnConfigDefault::CGridColumnConfigLocal::CGridColumnConfigLocal(const CGridColumnConfigDefault::CGridColumnConfigLocal& other)
:CGridColumnConfig(other)
{
	*this = other;
}

CGridColumnConfigDefault::CGridColumnConfigLocal& CGridColumnConfigDefault::CGridColumnConfigLocal::operator=(const CGridColumnConfigDefault::CGridColumnConfigLocal& other)
{
	if (this==&other)
		return *this;

	static_cast<CGridColumnConfig&>(*this) = other;

	m_LocalSettings.RemoveAll();
	for(int i = 0; i < other.m_LocalSettings.GetSize(); ++i)
		m_LocalSettings.Add(other.m_LocalSettings.GetKeyAt(i), other.m_LocalSettings.GetValueAt(i));
	return *this;
}

CString CGridColumnConfigDefault::CGridColumnConfigLocal::ReadSetting(const CString& section, const CString& setting, const CString& defval) const
{
	for(int i = 0; i < m_LocalSettings.GetSize(); ++i)
		if (m_LocalSettings.GetKeyAt(i)==setting)
			return m_LocalSettings.GetValueAt(i);

	return defval;
}

void CGridColumnConfigDefault::CGridColumnConfigLocal::WriteSetting(const CString& section, const CString& setting, const CString& value)
{
	m_LocalSettings.Add(setting, value);
}

void CGridColumnConfigDefault::CGridColumnConfigLocal::RemoveSection(const CString& section)
{
	m_LocalSettings.RemoveAll();
}

bool CGridColumnConfigDefault::CGridColumnConfigLocal::HasDefaultSettings() const
{
	return m_LocalSettings.GetSize() > 0;
}

void CGridColumnConfigDefault::CGridColumnConfigLocal::CopySettings(CGridColumnConfig& destination) const
{
	for(int i = 0; i < m_LocalSettings.GetSize(); ++i)
		destination.SetSetting(m_LocalSettings.GetKeyAt(i), m_LocalSettings.GetValueAt(i));
}

CGridColumnConfigDefault::CGridColumnConfigDefault(const CString& viewname)
:CGridColumnConfig(viewname)
,m_DefaultSettings(viewname)
{}

CString CGridColumnConfigDefault::GetSetting(const CString& name, const CString& defval) const
{
	return CGridColumnConfig::GetSetting(name, m_DefaultSettings.GetSetting(name, defval));
}

//------------------------------------------------------------------------
//! CGridColumnConfigProfiles
//------------------------------------------------------------------------
CGridColumnConfigProfiles::CGridColumnConfigProfiles(const CString& viewname)
:CGridColumnConfigDefault(viewname)
{
	m_CurrentSection = viewname;
}

void CGridColumnConfigProfiles::SplitSectionName(const CString& section, CString& view, CString& profile)
{
	int pos_profile = section.Find(_T("__"));
	if (pos_profile > 0)
	{
		view = section.Mid(0, pos_profile);
		profile = section.Mid(pos_profile+2);
	}
	else
	{
		view = section;
	}
}

CString CGridColumnConfigProfiles::JoinSectionName(const CString& view, const CString& profile) const
{
	if (profile.IsEmpty())
		return view;
	else
		return view  + _T("__") + profile;
}

const CString& CGridColumnConfigProfiles::GetSectionName() const
{
	if (m_CurrentSection==m_ViewName)
	{
		CString profile = ReadSetting(m_ViewName, _T("ActiveProfile"), _T(""));
		if (profile.IsEmpty())
		{
			CSimpleArray<CString> profiles;
			GetProfiles(profiles);
			if (profiles.GetSize()>0)
				profile = profiles[0];
		}
		m_CurrentSection = JoinSectionName(m_ViewName, profile);
	}
	return m_CurrentSection;
}

void CGridColumnConfigProfiles::GetProfiles(CSimpleArray<CString>& profiles) const
{
	const CString& strProfiles = ReadSetting(m_ViewName, _T("CurrentProfiles"), _T(""));
	SplitArraySetting(strProfiles, profiles, _T(", "));
}

CString CGridColumnConfigProfiles::GetActiveProfile()
{
	CString view, profile;
	SplitSectionName(m_CurrentSection, view, profile);
	return profile;
}

void CGridColumnConfigProfiles::SetActiveProfile(const CString& profile)
{
	// Make the new profile the active ones
	WriteSetting(m_ViewName, _T("ActiveProfile"), profile);
	m_CurrentSection = JoinSectionName(m_ViewName,profile);
	if (profile.IsEmpty())
		return;

	AddProfile(profile);
}

void CGridColumnConfigProfiles::AddProfile(const CString& profile)
{
	// Add the profile to the list if not already there
	CSimpleArray<CString> profiles;
	GetProfiles(profiles);
	for(int i=0; i < profiles.GetSize(); ++i)
		if (profiles[i]==profile)
			return;

	CString noconst(profile);
	profiles.Add(noconst);

	WriteSetting(m_ViewName, _T("CurrentProfiles"), ConvertArraySetting(profiles, _T(", ")));
}

void CGridColumnConfigProfiles::DeleteProfile(const CString& profile)
{
	if (profile.IsEmpty())
		return;

	// Remove any settings
	RemoveSection(JoinSectionName(m_ViewName,profile));

	// Remove the profile from the list
	CSimpleArray<CString> profiles;
	GetProfiles(profiles);
	for(int i=0; i < profiles.GetSize(); ++i)
		if (profiles[i]==profile)
			profiles.RemoveAt(i);
	WriteSetting(m_ViewName, _T("CurrentProfiles"), ConvertArraySetting(profiles, _T(", ")));
}

void CGridColumnConfigProfiles::RemoveCurrentConfig()
{
	if (GetSectionName()==m_ViewName)
	{
		// Backup profile-settings and reset the other settings
		const CString& strProfiles = ReadSetting(m_ViewName, _T("CurrentProfiles"), _T(""));
		const CString& activeProfile = ReadSetting(m_ViewName, _T("ActiveProfile"), _T(""));
		CGridColumnConfigDefault::RemoveCurrentConfig();
		WriteSetting(m_ViewName, _T("CurrentProfiles"), strProfiles);
		WriteSetting(m_ViewName, _T("ActiveProfile"), activeProfile);
	}
	else
	{
		CGridColumnConfigDefault::RemoveCurrentConfig();
	}
}


CGridColumnConfigWinApp::CGridColumnConfigWinApp(const CString& viewname)
	:CGridColumnConfigProfiles(viewname)
{
}

CString CGridColumnConfigWinApp::ReadSetting(const CString& section, const CString& setting, const CString& defval) const
{
	return AfxGetApp()->GetProfileString(section, setting, defval);
}

void CGridColumnConfigWinApp::WriteSetting(const CString& section, const CString& setting, const CString& value)
{
	AfxGetApp()->WriteProfileString(section, setting, value);
}

void CGridColumnConfigWinApp::RemoveSection(const CString& section)
{
	// Section is deleted when providing NULL as entry
	AfxGetApp()->WriteProfileString(section, NULL, NULL);
}

CGridColumnEditorProfile::CGridColumnEditorProfile(const CString& viewname)
{
	m_ApplyingConfiguration = false;
	m_pColumnConfig = new CGridColumnConfigWinApp(viewname);
}

CGridColumnEditorProfile::CGridColumnEditorProfile(CGridColumnConfigProfiles* pColumnConfig)
{
	m_ApplyingConfiguration = false;
	m_pColumnConfig = pColumnConfig;
}

CGridColumnEditorProfile::~CGridColumnEditorProfile()
{
	delete m_pColumnConfig;
	m_pColumnConfig = NULL;
}

void CGridColumnEditorProfile::SaveConfiguration(CGridListCtrlEx& owner, CGridColumnConfig& config)
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

void CGridColumnEditorProfile::LoadConfiguration(CGridListCtrlEx& owner, CGridColumnConfig& config)
{
	if (!m_pColumnConfig->HasDefaultSettings())
	{
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
			if (nColData==owner.GetColumnData(nCol))
			{
				// Column still exists
				if (owner.GetColumnTrait(nCol)->GetColumnState().m_AlwaysHidden)
					continue;

				CGridColumnTrait::ColumnState& columnState = owner.GetColumnTrait(nCol)->GetColumnState();
				columnState.m_Visible = true;
				LoadColumnConfiguration(i, nCol, owner, config);
				pOrderArray[--nColOrder] = nCol;
				break;
			}
		}
	}

	// Did we find any visible columns in the saved configuration ?
	if (nColOrder < nColCount)
	{
		// All remaining columns are marked as invisible
		for(int nCol = nColCount-1; nCol >= 0; --nCol)
		{
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
				columnState.m_OrgPosition = owner.GetColumnOrder(nCol);
				columnState.m_OrgWidth = owner.GetColumnWidth(nCol);
				owner.SetColumnWidth(nCol, 0);
				columnState.m_Visible = false;
				pOrderArray[--nColOrder] = nCol;
			}
		}
		owner.SetColumnOrderArray(nColCount, pOrderArray);
	}
	ASSERT(nColOrder==-1);
	delete [] pOrderArray;

	m_ApplyingConfiguration = false;

	owner.SetRedraw(TRUE);
	owner.Invalidate(TRUE);
	owner.UpdateWindow();
}

void CGridColumnEditorProfile::SaveColumnConfiguration(int nConfigCol, int nOwnerCol, CGridListCtrlEx& owner, CGridColumnConfig& config)
{
	CString colSetting;
	colSetting.Format(_T("ColumnWidth_%d"), nConfigCol);
	config.SetIntSetting(colSetting, owner.GetColumnWidth(nOwnerCol));
}

void CGridColumnEditorProfile::LoadColumnConfiguration(int nConfigCol, int nOwnerCol, CGridListCtrlEx& owner, CGridColumnConfig& config)
{
	CString colSetting;
	if (owner.GetColumnTrait(nOwnerCol)->GetColumnState().m_Resizable)
	{
		colSetting.Format(_T("ColumnWidth_%d"), nConfigCol);
		int width = config.GetIntSetting(colSetting);
		owner.SetColumnWidth(nOwnerCol, width);
	}
}

bool CGridColumnEditorProfile::HasColumnsDefault(CGridListCtrlEx& owner, CString& title)
{
	title = _T("Reset columns");
	return m_pColumnConfig->HasDefaultSettings();
}

void CGridColumnEditorProfile::ResetColumnsDefault(CGridListCtrlEx& owner)
{
	m_pColumnConfig->ResetSettingsDefault();
	LoadConfiguration(owner, *m_pColumnConfig);
}

void CGridColumnEditorProfile::AddColumnProfile(const CString& profile)
{
	m_pColumnConfig->AddProfile(profile);
}

void CGridColumnEditorProfile::DeleteColumnProfile(const CString& profile)
{
	m_pColumnConfig->DeleteProfile(profile);
}

CString CGridColumnEditorProfile::HasColumnProfiles(CGridListCtrlEx& owner, CSimpleArray<CString>& profiles, CString& title)
{
	title = _T("Column Profiles");
	m_pColumnConfig->GetProfiles(profiles);
	return m_pColumnConfig->GetActiveProfile();
}

void CGridColumnEditorProfile::SwichColumnProfile(CGridListCtrlEx& owner, const CString& profile)
{
	SaveConfiguration(owner, *m_pColumnConfig);
	m_pColumnConfig->SetActiveProfile(profile);
	LoadConfiguration(owner, *m_pColumnConfig);
}

void CGridColumnEditorProfile::OnColumnSetup(CGridListCtrlEx& owner)
{
	LoadConfiguration(owner, *m_pColumnConfig);
}

void CGridColumnEditorProfile::OnOwnerKillFocus(CGridListCtrlEx& owner)
{
	SaveConfiguration(owner, *m_pColumnConfig);
}

void CGridColumnEditorProfile::OnColumnResize(CGridListCtrlEx& owner)
{
	SaveConfiguration(owner, *m_pColumnConfig);
}

void CGridColumnEditorProfile::OnColumnPick(CGridListCtrlEx& owner)
{
	SaveConfiguration(owner, *m_pColumnConfig);
}
