
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

	//get max data(最大脚刹力是最大值后的1s/2000个点的均值)
	void AddData1_1(const double dData);
	double GetMaxValue_1();

	void GetPartIndex(UINT &nBegin,UINT &nEnd);
	double GetPartMeanValue(const UINT nBegin,const UINT bEnd);
	//for mean drag acc and max velocity1
	void AddData2(double dAcc,double dVel);
	//返回MFDD，最大速度（制动初速度），刹车距离;把最大速度点作为开始制动点
	bool GetData2(const double deltat,double &dAcc,double &dVel,double &BrakeDist);
	//for mean drag acc and max velocity
	void AddData3(double dAcc,double dVel,double dOriginFootBrakeForce);
	//返回MFDD，最大速度（制动初速度），刹车距离;取行车制动力大于某个值的点作为开始制动点
	bool GetData3(const double deltat, const double dOriginFootBrakeForce,
		double &dAcc,double &dVel,double &BrakeDist);

private:

	double m_dSum;
	int m_nCount;

	vector<double> m_vtData;//acc
	vector<double> m_vtData1;//vel
	vector< pair<double,int> >m_vtData2;//vel and its count -or- footbrakeforce and its count
	vector< pair<double,double> >m_vtData3;//vel and footbrakeforce
};

