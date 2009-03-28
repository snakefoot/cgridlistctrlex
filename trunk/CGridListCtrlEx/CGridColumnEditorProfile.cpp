#include "stdafx.h"
#include "CGridColumnEditorProfile.h"

#include "CGridListCtrlEx.h"
#include "CGridColumnTrait.h"

//------------------------------------------------------------------------
//! CGridColumnConfig
//------------------------------------------------------------------------
CString CGridColumnConfig::GetSetting(const CString& strName, const CString& strDefval) const
{
	return ReadSetting(GetSectionName(), strName, strDefval);
}

void CGridColumnConfig::SetSetting(const CString& strName, const CString& strValue)
{
	WriteSetting(GetSectionName(), strName, strValue);
}

const CString& CGridColumnConfig::GetSectionName() const
{
	return m_ViewName;
}

void CGridColumnConfig::RemoveCurrentConfig()
{
	RemoveSection(GetSectionName());
}

bool CGridColumnConfig::GetBoolSetting(const CString& strName, bool bDefval) const
{
	const CString& strValue = GetSetting(strName, ConvertBoolSetting(bDefval));
	if (strValue==_T("TRUE"))
		return true;
	else
	if (strValue==_T("FALSE"))
		return false;
	else
		return bDefval;
}

CString CGridColumnConfig::ConvertBoolSetting(bool bValue) const
{
	return bValue ? _T("TRUE") : _T("FALSE");
}

void CGridColumnConfig::SetBoolSetting(const CString& strName, bool bValue)
{
	SetSetting(strName, ConvertBoolSetting(bValue));
}

int CGridColumnConfig::GetIntSetting(const CString& strName, int nDefval) const
{
	const CString& value = GetSetting(strName, ConvertIntSetting(nDefval));
	return _ttoi(value);
}

CString CGridColumnConfig::ConvertIntSetting(int nValue) const
{
	CString strValue;
	strValue.Format(_T("%d"), nValue);
	return strValue;
}

void CGridColumnConfig::SetIntSetting(const CString& strName, int nValue)
{
	SetSetting(strName, ConvertIntSetting(nValue));
}

double CGridColumnConfig::GetFloatSetting(const CString& strName, double nDefval) const
{
	const CString& value = GetSetting(strName, ConvertFloatSetting(nDefval));
	return _tstof(value);
}

CString CGridColumnConfig::ConvertFloatSetting(double nValue, int nDecimals) const
{
	CString strFormat;
	strFormat.Format(_T("%%.%df"), nDecimals);

	CString strValue;	
	strValue.Format(strFormat, nValue);
	return strValue;
}

void CGridColumnConfig::SetFloatSetting(const CString& strName, double nValue, int nDecimals)
{
	SetSetting(strName, ConvertFloatSetting(nValue, nDecimals));
}

void CGridColumnConfig::SplitArraySetting(const CString& strArray, CSimpleArray<CString>& values, const CString& strDelimiter) const
{
	// Perform tokenize using strDelimiter
	int cur_pos = 0;
	int prev_pos = 0;
	int length = strArray.GetLength();
	while(cur_pos < length)
	{
		cur_pos = strArray.Find(strDelimiter, prev_pos);
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
			prev_pos = cur_pos + strDelimiter.GetLength();
		}
	}
}

void CGridColumnConfig::GetArraySetting(const CString& strName, CSimpleArray<CString>& values, const CString& strDelimiter) const
{
	const CString& strArray = GetSetting(strName, _T(""));
	if (strArray.IsEmpty())
		return;

	SplitArraySetting(strArray, values, strDelimiter);
}

CString CGridColumnConfig::ConvertArraySetting(const CSimpleArray<CString>& values, const CString& strDelimiter) const
{
	CString strValue;
	for(int i = 0; i < values.GetSize() ; ++i)
	{
		if (!strValue.IsEmpty())
			strValue += strDelimiter;
		strValue += values[i];
	}
	return strValue;
}

void CGridColumnConfig::SetArraySetting(const CString& strName, const CSimpleArray<CString>& values, const CString& strDelimiter)
{
	SetSetting(strName, ConvertArraySetting(values, strDelimiter));
}

void CGridColumnConfig::GetArraySetting(const CString& strName, CSimpleArray<int>& values, const CString& strDelimiter) const
{
	CSimpleArray<CString> strArray;
	GetArraySetting(strName, strArray, strDelimiter);
	for(int i = 0 ; i < strArray.GetSize(); ++i)
	{
		int value = _ttoi(strArray[i]);
		values.Add(value);
	}
}

CString CGridColumnConfig::ConvertArraySetting(const CSimpleArray<int>& values, const CString& strDelimiter) const
{
	CString strValue;
	CString strArray;
	for(int i = 0; i < values.GetSize(); ++i)
	{
		if (!strArray.IsEmpty())
			strArray += strDelimiter;
		strValue.Format( _T("%d"), values[i]);
		strArray += strValue;
	}
	return strArray;
}

void CGridColumnConfig::SetArraySetting(const CString& strName, const CSimpleArray<int>& values, const CString& strDelimiter)
{
	SetSetting(strName, ConvertArraySetting(values, strDelimiter));
}

LOGFONT CGridColumnConfig::GetLogFontSetting(const CString& strName) const
{
	LOGFONT font = {0};

	CSimpleArray<CString> strArray;
	GetArraySetting(strName, strArray);
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

	CString strValue(font.lfFaceName, sizeof(font.lfFaceName)/sizeof(TCHAR));
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfHeight);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfWidth);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfEscapement);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfOrientation);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfWeight);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfItalic);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfUnderline);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfStrikeOut);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfCharSet);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfOutPrecision);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfQuality);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), font.lfPitchAndFamily);
	strArray.Add(strValue);

	return ConvertArraySetting(strArray);
}

void CGridColumnConfig::SetLogFontSetting(const CString& strName, const LOGFONT& font)
{
	SetSetting(strName, ConvertLogFontSetting(font));
}

CRect CGridColumnConfig::GetRectSetting(const CString& strName, const CRect& rectDefval) const
{
	CSimpleArray<CString> strArray;
	GetArraySetting(strName, strArray);
	if (strArray.GetSize() != 4)
		return rectDefval;

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
	CString strValue;
	strValue.Format(_T("%d"), rect.left);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), rect.top);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), rect.right);
	strArray.Add(strValue);
	strValue.Format(_T("%d"), rect.bottom);
	strArray.Add(strValue);

	return ConvertArraySetting(strArray);
}

void CGridColumnConfig::SetRectSetting(const CString& strName, const RECT& rect)
{
	SetSetting(strName, ConvertRectSetting(rect));
}

COLORREF CGridColumnConfig::GetColorSetting(const CString& strName, const COLORREF colorDefval) const
{
	CSimpleArray<CString> strArray;
	GetArraySetting(strName, strArray);
	if (strArray.GetSize() != 3)
		return colorDefval;

	int r = _ttoi(strArray[0]);
	int g = _ttoi(strArray[1]);
	int b = _ttoi(strArray[2]);

	return RGB(r,g,b);
}

CString CGridColumnConfig::ConvertColorSetting(COLORREF color) const
{
	CSimpleArray<CString> strArray;
	CString strValue;
	strValue.Format(_T("%d"), GetRValue(color));
	strArray.Add(strValue);
	strValue.Format(_T("%d"), GetGValue(color));
	strArray.Add(strValue);
	strValue.Format(_T("%d"), GetBValue(color));
	strArray.Add(strValue);

	return ConvertArraySetting(strArray);
}

void CGridColumnConfig::SetColorSetting(const CString& strName, COLORREF color)
{
	SetSetting(strName, ConvertColorSetting(color));
}

//------------------------------------------------------------------------
//! CGridColumnConfigDefault
//------------------------------------------------------------------------
CGridColumnConfigDefault::CGridColumnConfigLocal::CGridColumnConfigLocal(const CString& strViewName)
:CGridColumnConfig(strViewName)
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

CString CGridColumnConfigDefault::CGridColumnConfigLocal::ReadSetting(const CString& strSection, const CString& strSetting, const CString& strDefval) const
{
	for(int i = 0; i < m_LocalSettings.GetSize(); ++i)
		if (m_LocalSettings.GetKeyAt(i)==strSetting)
			return m_LocalSettings.GetValueAt(i);

	return strDefval;
}

void CGridColumnConfigDefault::CGridColumnConfigLocal::WriteSetting(const CString& strSection, const CString& strSetting, const CString& strValue)
{
	m_LocalSettings.Add(strSetting, strValue);
}

void CGridColumnConfigDefault::CGridColumnConfigLocal::RemoveSection(const CString& strSection)
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

CGridColumnConfigDefault::CGridColumnConfigDefault(const CString& strViewName)
:CGridColumnConfig(strViewName)
,m_DefaultSettings(strViewName)
{}

CString CGridColumnConfigDefault::GetSetting(const CString& strName, const CString& strDefval) const
{
	return CGridColumnConfig::GetSetting(strName, m_DefaultSettings.GetSetting(strName, strDefval));
}

//------------------------------------------------------------------------
//! CGridColumnConfigProfiles
//------------------------------------------------------------------------
CGridColumnConfigProfiles::CGridColumnConfigProfiles(const CString& strViewName)
:CGridColumnConfigDefault(strViewName)
{
	m_CurrentSection = strViewName;
}

void CGridColumnConfigProfiles::SplitSectionName(const CString& strSection, CString& strViewName, CString& strProfile)
{
	int pos_profile = strSection.Find(_T("__"));
	if (pos_profile > 0)
	{
		strViewName = strSection.Mid(0, pos_profile);
		strProfile = strSection.Mid(pos_profile+2);
	}
	else
	{
		strViewName = strSection;
	}
}

CString CGridColumnConfigProfiles::JoinSectionName(const CString& strViewName, const CString& strProfile) const
{
	if (strProfile.IsEmpty())
		return strViewName;
	else
		return strViewName  + _T("__") + strProfile;
}

const CString& CGridColumnConfigProfiles::GetSectionName() const
{
	if (m_CurrentSection==m_ViewName)
	{
		CString strProfile = ReadSetting(m_ViewName, _T("ActiveProfile"), _T(""));
		if (strProfile.IsEmpty())
		{
			CSimpleArray<CString> profiles;
			GetProfiles(profiles);
			if (profiles.GetSize()>0)
				strProfile = profiles[0];
		}
		m_CurrentSection = JoinSectionName(m_ViewName, strProfile);
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
	CString strViewName, strProfile;
	SplitSectionName(m_CurrentSection, strViewName, strProfile);
	return strProfile;
}

void CGridColumnConfigProfiles::SetActiveProfile(const CString& strProfile)
{
	// Make the new strProfile the active ones
	WriteSetting(m_ViewName, _T("ActiveProfile"), strProfile);
	m_CurrentSection = JoinSectionName(m_ViewName,strProfile);
	if (strProfile.IsEmpty())
		return;

	AddProfile(strProfile);
}

void CGridColumnConfigProfiles::AddProfile(const CString& strProfile)
{
	// Add the strProfile to the list if not already there
	CSimpleArray<CString> profiles;
	GetProfiles(profiles);
	for(int i=0; i < profiles.GetSize(); ++i)
		if (profiles[i]==strProfile)
			return;

	CString noconst(strProfile);
	profiles.Add(noconst);

	WriteSetting(m_ViewName, _T("CurrentProfiles"), ConvertArraySetting(profiles, _T(", ")));
}

void CGridColumnConfigProfiles::DeleteProfile(const CString& strProfile)
{
	if (strProfile.IsEmpty())
		return;

	// Remove any settings
	RemoveSection(JoinSectionName(m_ViewName,strProfile));

	// Remove the strProfile from the list
	CSimpleArray<CString> profiles;
	GetProfiles(profiles);
	for(int i=0; i < profiles.GetSize(); ++i)
		if (profiles[i]==strProfile)
			profiles.RemoveAt(i);
	WriteSetting(m_ViewName, _T("CurrentProfiles"), ConvertArraySetting(profiles, _T(", ")));
}

void CGridColumnConfigProfiles::RemoveCurrentConfig()
{
	if (GetSectionName()==m_ViewName)
	{
		// Backup strProfile-settings and reset the other settings
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


CGridColumnConfigWinApp::CGridColumnConfigWinApp(const CString& strViewName)
	:CGridColumnConfigProfiles(strViewName)
{
}

CString CGridColumnConfigWinApp::ReadSetting(const CString& strSection, const CString& strSetting, const CString& strDefval) const
{
	return AfxGetApp()->GetProfileString(strSection, strSetting, strDefval);
}

void CGridColumnConfigWinApp::WriteSetting(const CString& strSection, const CString& strSetting, const CString& strValue)
{
	AfxGetApp()->WriteProfileString(strSection, strSetting, strValue);
}

void CGridColumnConfigWinApp::RemoveSection(const CString& strSection)
{
	// Section is deleted when providing NULL as entry
	AfxGetApp()->WriteProfileString(strSection, NULL, NULL);
}

CGridColumnEditorProfile::CGridColumnEditorProfile(const CString& strViewName)
{
	m_ApplyingConfiguration = false;
	m_pColumnConfig = new CGridColumnConfigWinApp(strViewName);
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

bool CGridColumnEditorProfile::HasColumnsDefault(CGridListCtrlEx& owner, CString& strTitle)
{
	strTitle = _T("Reset columns");
	return m_pColumnConfig->HasDefaultSettings();
}

void CGridColumnEditorProfile::ResetColumnsDefault(CGridListCtrlEx& owner)
{
	m_pColumnConfig->ResetSettingsDefault();
	LoadConfiguration(owner, *m_pColumnConfig);
}

void CGridColumnEditorProfile::AddColumnProfile(const CString& strProfile)
{
	m_pColumnConfig->AddProfile(strProfile);
}

void CGridColumnEditorProfile::DeleteColumnProfile(const CString& strProfile)
{
	m_pColumnConfig->DeleteProfile(strProfile);
}

CString CGridColumnEditorProfile::HasColumnProfiles(CGridListCtrlEx& owner, CSimpleArray<CString>& profiles, CString& strTitle)
{
	strTitle = _T("Column Profiles");
	m_pColumnConfig->GetProfiles(profiles);
	return m_pColumnConfig->GetActiveProfile();
}

void CGridColumnEditorProfile::SwichColumnProfile(CGridListCtrlEx& owner, const CString& strProfile)
{
	SaveConfiguration(owner, *m_pColumnConfig);
	m_pColumnConfig->SetActiveProfile(strProfile);
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
