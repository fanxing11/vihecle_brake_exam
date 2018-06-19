
#pragma once

#include "DAQControler.h"
using namespace DAQCONTROLER;

class Filter
{
public:
	Filter(UINT nSize=DAQCONTROLER::sectionLengthW);
	~Filter(void);

public:
	//求均值
	void AddData(double dData);
	double GetMeanData();

	//求中位数
	void AddData1(double dData);
	double GetMidValue();
	double GetMaxValue();
	void ResetMid();
private:
	UINT m_nBufSize;
	void Initial();
	bool m_bInitialed;
	double m_dSum;
	int m_nCount;

	double* m_pArrayDouble;
};

