#pragma once
class AMeanFilter
{
public:
	AMeanFilter(void);
	~AMeanFilter(void);

public:
	void AddData(double dData);
	double GetMeanData();

private:
	double m_dSum;
	int m_nCount;
};

