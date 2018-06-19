#include "Filter.h"
#include "const.h"

static double m_Array[200000];

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
	//g_logger.TraceInfo("Partition,%d-%d",left,right);
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
	//g_logger.TraceInfo("QuickSort,%d-%d",left,right);
	if (left >= right)
		return;
	int pivot_index = Partition(A, left, right); // 基准的索引
	QuickSort(A, left, pivot_index - 1);
	QuickSort(A, pivot_index + 1, right);
}
//-------------快排

void BubbleSort(double A[], double n)
{
	for (int j = 0; j < n - 1; j++)         // 每次最大元素就像气泡一样"浮"到数组的最后
	{
		for (int i = 0; i < n - 1 - j; i++) // 依次比较相邻的两个元素,使较大的那个向后移
		{
			if (A[i] > A[i + 1])            // 如果条件改成A[i] >= A[i + 1],则变为不稳定的排序算法
			{
				Swap(A, i, i + 1);
			}
		}
	}
}

//-------堆排序----begin
void Heapify(double A[], int i, int size)  // 从A[i]向下进行堆调整
{
	int left_child = 2 * i + 1;         // 左孩子索引
	int right_child = 2 * i + 2;        // 右孩子索引
	int max = i;                        // 选出当前结点与其左右孩子三者之中的最大值
	if (left_child < size && A[left_child] > A[max])
		max = left_child;
	if (right_child < size && A[right_child] > A[max])
		max = right_child;
	if (max != i)
	{
		Swap(A, i, max);                // 把当前结点和它的最大(直接)子节点进行交换
		Heapify(A, max, size);          // 递归调用，继续从当前结点向下进行堆调整
	}
}

int BuildHeap(double A[], int n)           // 建堆，时间复杂度O(n)
{
	int heap_size = n;
	for (int i = heap_size / 2 - 1; i >= 0; i--) // 从每一个非叶结点开始向下进行堆调整
		Heapify(A, i, heap_size);
	return heap_size;
}

void HeapSort(double A[], int n)
{
	int heap_size = BuildHeap(A, n);    // 建立一个最大堆
	while (heap_size > 1)// 堆 无序区 元素个数大于1，未完成排序
	{   // 将堆顶元素与堆的最后一个元素互换，并从堆中去掉最后一个元素
		// 此处交换操作很有可能把后面元素的稳定性打乱，所以堆排序是不稳定的排序算法
		Swap(A, 0, --heap_size);
		Heapify(A, 0, heap_size);     // 从新的堆顶元素开始向下进行堆调整，时间复杂度O(logn)
	}
}
//-------堆排序----end

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

void Filter::ResetMid()
{
	m_nCount = 0;
	m_bInitialed = false;
	//g_logger.TraceInfo("Filter::ResetMid");
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
		g_logger.TraceInfo("Filter::GetMidValue,sum point count is %d.",m_nCount);
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
		//QuickSort(m_Array, 0, 1000-1);//data is big, calc failed.
		//BubbleSort(m_Array, m_nCount);//
		HeapSort(m_Array, m_nCount);//
		for (int i=0;i<m_nCount;)
		{
			for (int j=0;j<20;j++)
			{
				i+=20;
				g_logger.TraceError("%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t",
					m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],
					m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i],m_Array[i]);
				//g_logger.TraceError("%.3f",m_Array[i]);
			}
		}
		//m_bInitialed = false;

		//g_logger.TraceInfo("Filter::GetMidValue %d",m_nCount);

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

double Filter::GetMaxValue()
{
	try{

		HeapSort(m_Array, m_nCount);



		//m_bInitialed = false;

		//g_logger.TraceInfo("Filter::GetMidValue %d",m_nCount);

		double dret = m_Array[(m_nCount-1)];
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