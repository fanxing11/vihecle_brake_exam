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
//-------------快排
void Swap(double A[], int i, int j)
{
	double temp = A[i];
	A[i] = A[j];
	A[j] = temp;
}

int Partition(double A[], int left, int right)  // 划分函数
{
	double pivot = A[right];               // 这里每次都选择最后一个元素作为基准
	int tail = left - 1;                // tail为小于基准的子数组最后一个元素的索引
	for (int i = left; i < right; i++)  // 遍历基准以外的其他元素
	{
		if (A[i] <= pivot)              // 把小于等于基准的元素放到前一个子数组末尾
		{
			Swap(A, ++tail, i);
		}
	}
	Swap(A, tail + 1, right);           // 最后把基准放到前一个子数组的后边，剩下的子数组既是大于基准的子数组
	// 该操作很有可能把后面元素的稳定性打乱，所以快速排序是不稳定的排序算法
	return tail + 1;                    // 返回基准的索引
}

void QuickSort(double A[], int left, int right)
{
	if (left >= right)
		return;
	int pivot_index = Partition(A, left, right); // 基准的索引
	QuickSort(A, left, pivot_index - 1);
	QuickSort(A, pivot_index + 1, right);
}
//-------------快排


Filter::Filter(UINT nSize)
	:m_dSum(0.0)
	,m_nCount(0)
	,m_pArrayDouble(NULL)
	,m_bInitialed(false)
{
	m_nBufSize = nSize;
	g_logger.TraceInfo("Filter::Filter -in");

	memset(m_Array,0,sizeof(double)*100000);
}

Filter::~Filter(void)
{
	g_logger.TraceInfo("Filter::~Filter");
	if (NULL != m_pArrayDouble)
	{
		delete[] m_pArrayDouble;
		m_pArrayDouble = NULL;
	}
}



void Filter::AddData(double dData)
{
	m_dSum += dData;
	m_nCount++;
}

void Filter::AddData1(double dData)
{
	try
	{
		if (!m_bInitialed)
		{
			memset(m_Array,0,sizeof(double)*100000);
			//g_logger.TraceInfo("Filter::AddData1 - init1");
			//Initial();
			//g_logger.TraceInfo("Filter::AddData1 - init2-%.3f,%d,",dData,m_nCount);
		}
		m_Array[m_nCount] = dData;
		//*(m_pArrayDouble+m_nCount) = dData;
		m_nCount++;
	}
	catch (exception &e)
	{
		DWORD dw = GetLastError();
		g_logger.TraceError("Filter::AddData:%d-%s",dw,e.what());
	}

}

void Filter::Initial()
{

	try{
		g_logger.TraceInfo("Filter::Initial1");
		if (NULL !=m_pArrayDouble)
		{
			g_logger.TraceInfo("Filter::Initial2");
			delete[] m_pArrayDouble;
			m_pArrayDouble = NULL;
		}
		g_logger.TraceInfo("Filter::Initial3");
		m_pArrayDouble = new double[m_nBufSize];
		g_logger.TraceInfo("Filter::Initial4");
		memset(m_pArrayDouble,0,sizeof(double)*m_nBufSize);
		g_logger.TraceInfo("Filter::Initial5");
		m_bInitialed = true;
		m_nCount = 0;
		g_logger.TraceInfo("Filter::Initial6");

	}
	catch (exception &e)
	{
		DWORD dw = GetLastError();
		g_logger.TraceError("Filter::GetMidValue:%d-%s",dw,e.what());
	}
}

double Filter::GetMidValue()
{
	try{
		g_logger.TraceInfo("Filter::GetMidValue1");
		//double* ptmp = new double[m_nBufSize];

		//double tmp[30000] = {0};
		////if ()
		////{
		////}
		//if (m_pArrayDouble == NULL)
		//{
		//}
		//g_logger.TraceInfo("Filter::GetMidValue2");
		//memcpy(tmp,m_pArrayDouble,sizeof(double)*m_nBufSize);
		//g_logger.TraceInfo("Filter::GetMidValue3");

		//LogTime();
		//g_logger.TraceInfo("Filter::GetMidValue5");
		//QuickSort(tmp, 0, m_nCount-1);//DAQCONTROLER::sectionLengthW - 1
		//g_logger.TraceInfo("Filter::GetMidValue3");
		//LogTime();
		////delete[] ptmp;
		//g_logger.TraceInfo("Filter::GetMidValue6");
		////ptmp = NULL;

		//QuickSort(m_pArrayDouble, 0, m_nCount-1);//DAQCONTROLER::sectionLengthW - 1
		QuickSort(m_Array, 0, m_nCount-1);//DAQCONTROLER::sectionLengthW - 1


		m_bInitialed = false;

		g_logger.TraceInfo("Filter::GetMidValue %d",m_nCount);

		double dret = m_Array[(m_nCount)/2];
		return dret;

		//return *( m_pArrayDouble + (m_nCount)/2 );//DAQCONTROLER::sectionLengthW

	}
	catch (exception &e)
	{
		DWORD dw = GetLastError();
		g_logger.TraceError("Filter::GetMidValue:%d-%s",dw,e.what());
	}

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