#pragma once

struct CListCtrl_DataRecord
{
	CListCtrl_DataRecord()
	{}

	CListCtrl_DataRecord(const CString& city, const CString& country, COleDateTime year)
		:m_City(city)
		,m_Country(country)
		,m_YearWon(year)
	{}

	CString	m_City;
	CString	m_Country;
	COleDateTime m_YearWon;

	CString GetCellText(int col, bool title) const
	{
		switch(col)
		{
		case 0: { static const CString title0(_T("Country")); return title ? title0 : m_Country; }
		case 1: { static const CString title1(_T("Capital")); return title ? title1 : m_City; }
		case 2: { static const CString title2(_T("European Championship")); return title ? title2 : m_YearWon.GetStatus()==m_YearWon.valid ? m_YearWon.Format() : CString(); }
		case 3: { static const CString title3(_T("Wiki")); return title ? title3 : m_YearWon.GetStatus()==m_YearWon.valid ? m_YearWon.Format(_T("%Y")) : CString(); }
		default:{ static const CString emptyStr; return emptyStr; }
		}
	}

	int  GetColCount() const { return 4; }
};

class CListCtrl_DataModel
{
	vector<CListCtrl_DataRecord> m_Records;
	int	m_LookupTime;
	int m_RowMultiplier;

public:
	CListCtrl_DataModel()
		:m_RowMultiplier(0)
		,m_LookupTime(0)
	{
		InitDataModel();
	}

	void InitDataModel()
	{
		m_Records.clear();
		m_Records.push_back( CListCtrl_DataRecord(_T("Copenhagen"), _T("Denmark"), COleDateTime(1992,6,26,0,0,0)) );
		m_Records.push_back( CListCtrl_DataRecord(_T("Berlin"), _T("Germany"), COleDateTime(1996,6,30,0,0,0)) );
		m_Records.push_back( CListCtrl_DataRecord(_T("Paris"), _T("France"), COleDateTime(2000,7,2,0,0,0)) );
		m_Records.push_back( CListCtrl_DataRecord(_T("Athen"), _T("Greece"), COleDateTime(2004,7,4,0,0,0)) );
		m_Records.push_back( CListCtrl_DataRecord(_T("Stockholm"), _T("Sweden"), COleDateTime(0,0,0,0,0,0)) );
		m_Records.push_back( CListCtrl_DataRecord(_T("Madrid"), _T("Spain"), COleDateTime(2008,6,29,0,0,0)) );

		if (m_RowMultiplier > 1)
		{
			vector<CListCtrl_DataRecord> rowset(m_Records);
			m_Records.reserve((m_RowMultiplier-1) * rowset.size());
			for(int i = 0 ; i < m_RowMultiplier ; ++i)
			{
				m_Records.insert(m_Records.end(), rowset.begin(), rowset.end());
			}
		}
	}

	CString GetCellText(size_t lookupId, int col) const
	{
		if (lookupId >= m_Records.size())
		{
			static CString oob(_T("Out of Bound"));
			return oob;
		}
		// How many times should we search sequential for the row ?
		for(int i=0; i < m_LookupTime; ++i)
		{
			for(size_t rowId = 0; rowId < m_Records.size(); ++rowId)
			{
				if (rowId==lookupId)
					break;
			}
		}
		return m_Records.at(lookupId).GetCellText(col, false);
	}

	vector<CString> GetCountries() const
	{
		vector<CString> countries;
		for(size_t rowId = 0 ; rowId < m_Records.size(); ++rowId)
			countries.push_back( m_Records[rowId].m_Country );
		sort(countries.begin(), countries.end());
		countries.erase(unique(countries.begin(), countries.end()), countries.end());
		return countries;
	}

	size_t GetRowIds() const { return m_Records.size(); }
	int GetColCount() const { return CListCtrl_DataRecord().GetColCount(); }
	CString GetColTitle(int col) const { return CListCtrl_DataRecord().GetCellText(col, true); }

	vector<CListCtrl_DataRecord>& GetRecords() { return m_Records; }
	void SetLookupTime(int lookupTimes) { m_LookupTime = lookupTimes; }
	void SetRowMultiplier(int multiply) { if (m_RowMultiplier != multiply ) { m_RowMultiplier = multiply; InitDataModel(); } }
};