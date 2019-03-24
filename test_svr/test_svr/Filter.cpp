#include "Filter.h"
#include "const.h"


void LogTime()
{
	LARGE_INTEGER lv;

	// 获取每秒多少CPU Performance Tick
	QueryPerformanceFrequency( &lv );

	// 转换为每个Tick多少秒
	double secondsPerTick = 1.0 / lv.QuadPart;

	// 获取CPU运行到现在的Tick数
	QueryPerformanceCounter( &lv );

	// 计算CPU运行到现在的时间
	// 比GetTickCount和timeGetTime更加精确
	double timeElapsedTotal = secondsPerTick * lv.QuadPart;

	//cout.precision( 6 );
	//cout << fixed << showpoint << timeElapsedTotal << endl;

	g_logger.TraceWarning("time now is %.6f",timeElapsedTotal);

}

///----for data 2:vel and acc
double myCompair(pair<double, int>p1,pair<double,int>p2 )
{
	return p1.first < p2.first;
}

Filter::Filter(UINT nSize)
	:m_dSum(0.0)
	,m_nCount(0)
{
	//g_logger.TraceInfo("Filter::Filter -in");

}

Filter::~Filter(void)
{
	//g_logger.TraceInfo("Filter::~Filter");

	vector <double>().swap(m_vtData);
	vector<double>().swap(m_vtData1);
	vector< pair<double,int> >().swap(m_vtData2);
}

void Filter::AddData(double dData)
{
	m_dSum += dData;
	m_nCount++;
}

double Filter::GetMeanData()
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

void Filter::ResetMean()
{
	m_nCount = 0;
	m_dSum = 0;
}


void Filter::AddData1(const double dData)
{
	m_vtData.push_back(dData);
}
//v2.0.3
void Filter::AddData1_1(const double dData)
{
	pair<double,int>pp=make_pair(dData,m_nCount);
	m_vtData2.push_back(pp);
	m_nCount++;
}


void Filter::ResetMid()
{
	m_vtData.clear();
	m_vtData2.clear();
	m_vtData1.clear();
	m_nCount = 0;
}

double Filter::GetMidValue()
{
	try
	{
		UINT nData = m_vtData.size();
		//g_logger.TraceInfo("Filter::GetMidValue,sum point count is %d.",nData);

		if (!is_sorted(m_vtData.begin(),m_vtData.end()))
		{
			sort(m_vtData.begin(),m_vtData.end(),less<double>());
		}
		return m_vtData[nData/2];
	}
	catch (exception &e)
	{
		DWORD dw = GetLastError();
		g_logger.TraceError("Filter::GetMidValue:%d-%s",dw,e.what());
	}

}

double Filter::GetMaxValue()
{
	try{
		if (!is_sorted(m_vtData.begin(),m_vtData.end()))
		{
			sort(m_vtData.begin(),m_vtData.end(),less<double>());
		}
		return m_vtData[m_vtData.size()-1];
	}
	catch (exception &e)
	{
		DWORD dw = GetLastError();
		g_logger.TraceError("Filter::GetMidValue:%d-%s",dw,e.what());
	}
	
}
double Filter::GetMaxValue_1()
{
	vector< pair<double,int> >vtData2_1 = m_vtData2;
	double dBrakeForce = 0.0;
	LogTime();
	sort(vtData2_1.begin(),vtData2_1.end(),myCompair);
	LogTime();
	dBrakeForce = vtData2_1.back().first;

	int nIndex = vtData2_1.back().second;
	int nCOUNT = 2000;
	double dSum=0;

	//如果最大脚刹力之后没有200个力了,不再去均值,而是直接返回此最大值.
	if ((nIndex+nCOUNT) > (m_vtData2.size()-1))
	{
		g_logger.TraceError("Filter::GetMaxValue_1:overflowed,total num=%d,target=%d",(m_vtData2.size()),(nIndex+nCOUNT));
		return dBrakeForce;
	}

	for(int nbeg=0;nbeg<nCOUNT;nbeg++)
	{

		double dForce = m_vtData2.at(nbeg+nIndex).first;
		dSum += dForce;
	}
	return dSum/nCOUNT;
}

void Filter::GetPartIndex(UINT &nBegin,UINT &nEnd)
{
	if (!is_sorted(m_vtData.begin(),m_vtData.end()))
	{
		sort(m_vtData.begin(),m_vtData.end(),less<double>());
	}
	nBegin = 1;
}

double Filter::GetPartMeanValue(const UINT nBegin,const UINT bEnd)
{
	return 0;;
}

void Filter::AddData2(double dAcc,double dVel)
{
	pair<double,int>pp=make_pair(dVel,m_nCount);
	m_vtData2.push_back(pp);
	m_vtData.push_back(dAcc);
	m_vtData1.push_back(dVel);
	m_nCount++;
}

bool Filter::GetData2(const double deltat,double &dAcc,double &dVel,double &BrakeDist)
{
	try
	{
		LogTime();
		sort(m_vtData2.begin(),m_vtData2.end(),myCompair);
		LogTime();
		dVel = m_vtData2.back().first;

		int nIndex = m_vtData2.back().second;

		double dDist0= DOUBLE_ZERO;//总行程
		double dDist1= DOUBLE_ZERO;//加速行程
		for (int i=0;i<nIndex;i++)
		{
			dDist0 += m_vtData1[i]*deltat;
			dDist1 = dDist0;
		}
		double dSumAcc2 = DOUBLE_ZERO;
		int nAmount = m_vtData.size() - nIndex;
		for (int i= nIndex;i<m_vtData.size();i++)
		{
			dDist0 += m_vtData1[i]*deltat;
			dSumAcc2 += m_vtData[i];
		}
		dAcc = dSumAcc2 / nAmount;
		BrakeDist = dDist0-dDist1;

		g_logger.TraceInfo("Filter::GetData2:totalAmount=%d,MaxVelIndex=%d,dist0=%d,dist1=%d",
			m_nCount,nIndex,dDist0,dDist1);
		LogTime();

		return true;
	}
	catch (exception* e)
	{
		return false;
	}
}
void Filter::AddData3(double dAcc,double dVel,double dOriginFootBrakeForce)
{
	pair<double,double>pp=make_pair(dVel,dOriginFootBrakeForce);
	m_vtData3.push_back(pp);
	m_vtData.push_back(dAcc);
	m_vtData1.push_back(dVel);
	m_nCount++;
}
/*
//返回MFDD，最大速度（制动初速度），刹车距离
@deltat[in] 采样时间间隔
@dOriginFootBrakeForce[in], 脚刹力计算阈值,from config file
@dAcc[out] MFDD
@dVel[out] 制动初速度
@BrakeDist[out] 刹车距离,取行车制动力大于某个值的点作为开始制动点
*/
bool Filter::GetData3(const double deltat, const double dOriginFootBrakeForce, double &dAcc,double &dVel,double &BrakeDist)
{
	try
	{
		LogTime();
		
		double dDist0= DOUBLE_ZERO;//总行程
		double dDist1= DOUBLE_ZERO;//非减速行程

		double dSumAcc2 = DOUBLE_ZERO;//减速度之和
		int nAmount = 0;//减速点数
		bool bStartBrake = false;//flag for 是否已经开始刹车
		for (int i= 0;i<m_vtData.size();i++)
		{
			dDist0 += m_vtData1[i]*deltat;//速度积分累加得到总距离
			double dFootForce = m_vtData3[i].second;//当前点的脚刹力
			if (!bStartBrake && dFootForce > dOriginFootBrakeForce)
			{
				bool bRealMax = true;
				//如果此点之后300个点均满足条件,则认为是真实的刹车起始点
				for (int ii=0;ii<300;ii++)
				{
					//确认当前点是否存在
					if ((i+ii) > (m_vtData3.size()-1))
					{
						g_logger.TraceError("Filter::GetData3:i=%d,ii=%d,overflowed and can not find valid footbrake force.",i,ii);
						return false;
					}
					double dFootForceii = m_vtData3[i+ii].second;
					//检查当前点是否满足要求
					if (dFootForceii <= dOriginFootBrakeForce)
					{
						bRealMax = false;
					}
				}
				if (bRealMax)
				{
					bStartBrake = true;
					g_logger.TraceWarning("Filter::GetData3:i=%d;datasize=%d",i,m_vtData.size());
				}
			}
			if (bStartBrake)//更新减速信息
			{
				dSumAcc2 += m_vtData[i];
				nAmount ++;
			}
			else
			{
				dVel = m_vtData3[i].first;
				dDist1 = dDist0;
			}
		}
		if (nAmount)
		{
			dAcc = dSumAcc2 / nAmount;
		}
		else
		{
			dAcc = 0;
		}
		BrakeDist = dDist0-dDist1;

		g_logger.TraceInfo("Filter::GetData3:totalAmount=%d,dVel=%.2f,dist0=%.2f,dist1=%.2f",
			m_vtData.size(),dVel,dDist0,dDist1);
		LogTime();

		return true;
	}
	catch (exception* e)
	{
		return false;
	}
}