#include "AMeanFilter.h"


AMeanFilter::AMeanFilter(void)
	:m_dSum(0.0)
	,m_nCount(0)
{
}


AMeanFilter::~AMeanFilter(void)
{
}

void AMeanFilter::AddData(double dData)
{
	m_dSum += dData;
	m_nCount++;
}

double AMeanFilter::GetMeanData()
{
	if (0 != m_nCount)
	{
		return m_dSum / m_nCount;
	}
	else
	{
		return 0;
	}
}