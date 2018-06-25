
#pragma once

#include "DAQControler.h"
using namespace DAQCONTROLER;

#include <algorithm>
#include <functional>

using namespace std;

class Filter
{
public:
	Filter(UINT nSize=DAQCONTROLER::sectionLengthW);
	~Filter(void);

public:
	//get mean data
	void AddData(double dData);
	double GetMeanData();
	void ResetMean();

	//get mid data
	void AddData1(const double dData);
	double GetMidValue();
	void ResetMid();
	//get max data-sametime with mid data
	double GetMaxValue();
	void GetPartIndex(UINT &nBegin,UINT &nEnd);
	double GetPartMeanValue(const UINT nBegin,const UINT bEnd);
	//for mean drag acc and max velocity
	void AddData2(double dAcc,double dVel);
	//返回MFDD，最大速度（制动初速度），刹车距离
	bool GetData2(const double deltat,double &dAcc,double &dVel,double &BrakeDist);
private:

	double m_dSum;
	int m_nCount;

	vector<double> m_vtData;
	vector<double> m_vtData1;
	vector< pair<double,int> >m_vtData2;
};

