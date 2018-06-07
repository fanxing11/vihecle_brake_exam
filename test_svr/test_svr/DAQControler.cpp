#include "main.h"

#include "time.h"

extern CtheApp* theApp;

#include "DAQControler.h"

namespace DAQCONTROLER
{
	void printTime()
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

		cout.precision( 6 );
		cout << fixed << showpoint << timeElapsedTotal << endl;

	}

	unsigned int WINAPI DAQThreadFunc(LPVOID lp)
	{
		g_logger.TraceInfo("DAQThreadFunc_funcin");

		//STIN_COMTHREAD stTep;
		//stTep = *( STIN_COMTHREAD* )lp;
		CDAQControler* pCDAQControler=(CDAQControler*)lp;

		while(1)
		{
			int nDataLen = 0;
			nDataLen = dll_GetDataLen();
			//cout<< "GetDataLen = " << nDataLen << endl;
			float fMultiple = nDataLen / 1024.0;
			if (fMultiple > 1)
			{
				//cout<< "- begin sample"<<endl;
				pCDAQControler->GetData();

				//-----
				LONGLONG nPoints = 0;
				int readBuffer[32768]={0};//32768 bits
				//int readBuffer[1024]={0};//32768 bits
				nPoints = dll_ReadBuf(readBuffer, 1024);
				cout<< "nPoints = " << nPoints << endl;


				int dFactor[8]={1,1,1,1,1,1,1,1};
				short nEnableCh_short[8]={1,1,1,1,1,1,1,1};
				double dXTime[8*1024]={0};
				double dYAm[8*1024]={0};
				//double dYAm[8][1024]={0};
				dll_OutLabDate(nPoints, readBuffer, dFactor, nEnableCh_short, dXTime, dYAm);

				//double dXTime_Get2Wave[1024] = {0};
				//double dYAm1[1024] = {0};
				//double dYAm2[1024] = {0};
				//double dt = dll_Get2Wave(1, dXTime, dYAm, dXTime_Get2Wave, dYAm1, dYAm2);
				//cout<<"dYAm1"<<'\t'<<"dYAm2"<<endl;
				//for (int i=0;i<100;i++)
				//{
				//	cout<<dYAm1[i]<<'\t'<<dYAm2[i]<<endl;
				//	cout<<"-"<<readBuffer[i]<<readBuffer[i+1024]<<endl;
				//	cout<<"="<<dYAm[i]<<dYAm[i+1024]<<endl;
				//	cout<<"+"<<dXTime[i]<<dXTime[i+1024]<<endl;
				//	cout<<"+"<<dXTime_Get2Wave[i]<<endl;
				//}

				if (WAIT_OBJECT_0 != WaitForSingleObject(m_gEvtSample,0))//是否在采集
				{
					continue;
				}
				//printTime();
				//2. data process
				if (WAIT_OBJECT_0 == WaitForSingleObject(m_gEvtInitGradient,0))
				{
					theApp->m_pDataController->HandleInitGradientDataW(dYAm, channelCountW,sectionLengthW);
				}
				if (WAIT_OBJECT_0 == WaitForSingleObject(m_gEvtMoveDetection,0))
				{
					theApp->m_pDataController->HandleMoveDetectionDataW(dYAm, channelCountW,sectionLengthW,deltatW);
				}
				if (WAIT_OBJECT_0 == WaitForSingleObject(m_gEvtStillDetection,0))
				{
					theApp->m_pDataController->HandleStillDetectionDataW(dYAm, channelCountW,sectionLengthW);
				}
				//printTime();
				//3. data save
				if (WAIT_OBJECT_0 == WaitForSingleObject(m_gEvtSaveFile,0))
				{
					//printf("Ready to save the data...%d\n\n",getDataCount);
					memcpy(buffer, dYAm, SingleSavingFileSizeW);

					if (WriteFile(hFile, buffer, SingleSavingFileSizeW, &WrittenBytes, NULL))
					{
						//printf("Saving has been executed!\n\n");
						RealFileSize += WrittenBytes; 
						//printf("The real-time size of file is %d byte\n\n", RealFileSize);
						//printf("Executed %d time.\n\n", i++);
					} 
					else
					{
						g_logger.TraceError("DAQThreadFunc:WriteFile error!");
					}	
				}
				//memcpy(DateOne, buffer, 10*sizeof(double));

				//4. send data to UI
				PostThreadMessage(theApp->m_dwMainThreadID, msg_DAQ_DATAONE, NULL, NULL);

			}
			else
			{
				//cout<<"!failed to sample "<<endl;
				Sleep(1);
			}

		}

		_endthreadex(0);

		g_logger.TraceInfo("UDPRevThreadFunc_funcout");

		return 0;
	}

	inline void waitAnyKey()
	{
		do{SLEEP(1);} while(!kbhit());
	} 

	inline void openFile()
	{
		struct tm *local;
		time_t t;
		t = time(NULL);
		local = localtime(&t);
		char stFilename[MAX_PATH] = {0};
		sprintf_s(stFilename,"%04d%02d%02d_%02d%02d%02d.bin",
			local->tm_year+1900,
			local->tm_mon+1,
			local->tm_mday,
			local->tm_hour,
			local->tm_min,
			local->tm_sec
			);

		string strPath;
		theApp->m_pDataController->GetProjectPath(strPath);
		strPath.append(stFilename);

		hFile = CreateFileA(
			//L"..\\C++_Data.bin",
			strPath.c_str(),
			GENERIC_WRITE | GENERIC_READ,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			g_logger.TraceError("openFile:Cannot open file (error %d)\n", GetLastError());
		}

		buffer = (double*)VirtualAlloc(NULL, SingleSavingFileSizeW, MEM_COMMIT, PAGE_READWRITE);
		if (!buffer)
		{
			g_logger.TraceError("openFile:Allocate buffer fail(error %d)\n",GetLastError());
		}
	}


	CDAQControler::CDAQControler(void)
		:m_wfAiCtrl(NULL)
		,m_bDAQInitialSuccessfully(false)
		,m_hDAQThread(NULL)
	{
		g_logger.TraceInfo("CDAQControler::CDAQControler");
		CreateSyncEvent();

		this->Initialize();
	}

	CDAQControler::~CDAQControler(void)
	{
		this->DisInitialize();
	}
	void CDAQControler::GetData()
	{
		
	}
	void CDAQControler::CloseEvtHandle()
	{
		if (!m_gEvtSample)
		{
			CloseHandle(m_gEvtSample);
			m_gEvtSample = NULL;
		}
		if (!m_gEvtMoveDetection)
		{
			CloseHandle(m_gEvtMoveDetection);
			m_gEvtMoveDetection = NULL;
		}
		if (!m_gEvtStillDetection)
		{
			CloseHandle(m_gEvtStillDetection);
			m_gEvtStillDetection = NULL;
		}
		if(!m_gEvtInitGradient)
		{
			CloseHandle(m_gEvtInitGradient);
			m_gEvtInitGradient = NULL;
		}
		if (!m_gEvtSaveFile)
		{
			CloseHandle(m_gEvtSaveFile);
			m_gEvtSaveFile = NULL;
		}

	}

	void CDAQControler::CreateSyncEvent()
	{
		CloseEvtHandle();

		m_gEvtSample = CreateEvent(NULL,TRUE,FALSE,L"");

		m_gEvtInitGradient = CreateEvent(NULL,TRUE,FALSE,L"");
	
		m_gEvtMoveDetection = CreateEvent(NULL,TRUE,FALSE,L"");

		m_gEvtStillDetection = CreateEvent(NULL,TRUE,FALSE,L"");

		m_gEvtSaveFile = CreateEvent(NULL,TRUE,FALSE,L"");

	}

	void CDAQControler::DisInitialize()
	{
		this->CloseEvtHandle();
	}

	void CDAQControler::Initialize()
	{
		g_logger.TraceInfo("CDAQControler::Initialize:-in.");
		try
		{

			bool ret = true;

			HMODULE hInst = NULL;
			//cout<<"LoadLibrary start"<<endl;
			hInst = LoadLibrary(L"UsbAD.dll"); //UsbAD.dll

			DWORD dw = GetLastError();

			if (hInst == NULL)
			{
				//cout<<"hInst = NULL"<<endl;
				g_logger.TraceError("CDAQControler::Initialize:hInst = NULL");
				ret = false;
				//return false;
			}
			else
			{
				cout<<"LoadLibrary ok"<<endl;
			}

			//*********************************Dll import start*****************************************//
			//-----Lab_ConnectUT89
			dll_ConnectUT89 = *(Dll_ConnectUT89)GetProcAddress(hInst, "Lab_ConnectUT89");
			//-----Lab_SetPar
			dll_SetPar = *(Dll_SetPar)GetProcAddress(hInst, "Lab_SetPar");
			//-----Lab_SetPar
			dll_Start = *(Dll_OnStart)GetProcAddress(hInst, "Lab_OnStart");
			//-----Lab_GetDataLen n_DateSize
			dll_GetDataLen = *(Dll_GetDataLen)GetProcAddress(hInst, "Lab_GetDataLen");
			//-----Lab_ReadBuf
			dll_ReadBuf = *(Dll_ReadBuf)GetProcAddress(hInst, "Lab_ReadBuf");
			//-----Lab_OutLabDate
			dll_OutLabDate = *(Dll_OutLabDate)GetProcAddress(hInst, "Lab_OutLabDate");
			//-----Lab_Get2Wave
			dll_Get2Wave = *(Dll_Get2Wave)GetProcAddress(hInst, "Lab_Get2Wave");
			//*********************************Dll import end*****************************************//

			//-----
			string strIP("192.168.0.187");

			char* pIP =NULL;
			pIP = (char*)(strIP.c_str());

			char cFilePath[MAX_PATH]={0};
			GetModuleFileNameA(NULL, cFilePath, MAX_PATH); 
			(strrchr(cFilePath, '\\'))[1] = 0;
			//cout<< cFilePath <<endl;

			bool bConnected = dll_ConnectUT89(cFilePath, pIP );
			if (!bConnected)
			{	
				g_logger.TraceError("CDAQControler::Initialize:can not connect to wireless IP");
				ret = false;
				//cout<<"!can not connect to IP."<<endl;
			}
			//else
			//{
			//	cout<<"-connect to IP:"<<strIP<<endl;
			//}

			//-----
			int nEnableCh[8]={1,1,1,1,1,1,1,1};
			int sCoupling[8]={0,0,0,0,0,0,0,0};
			int pComInput[8]={0,0,0,0,0,0,0,0};
			int pZoom[8]={0,0,0,0,0,0,0,0};
			bool bSetPara = dll_SetPar(WirelessDAQFrq, nEnableCh, sCoupling, pComInput, pZoom);
			if (!bSetPara)
			{
				ret = false;
				g_logger.TraceError("CDAQControler::Initialize:failed to set para");
				//cout<<"!failed to set para"<<endl;
			}
			//else
			//{
			//	cout<<"- set para success"<<endl;
			//}

			// If something wrong in this execution, print the error code on screen for tracking.
			m_bDAQInitialSuccessfully = true;
			if(!ret)
			{
				//u初始化错误，弹框或者返回错误信息----
				//g_logger.TraceError("CDAQControler::Initialize:Initial DAQ failed. And the last error code is 0x%X.\n", ret);
				//waitAnyKey();// wait any key to quit!
				m_bDAQInitialSuccessfully = false;
				//throw;
			}
			else
			{
				//-----
				if (dll_Start == 0)
				{
					g_logger.TraceError("CDAQControler::Initialize:dll_Start == 0");
					//cout<<"GetProcAddress dll_Start failed"<<endl;
				}
				else
				{
					int ret = dll_Start();
					if (!ret)
					{
						//cout<<"!failed to start"<<endl;s
						g_logger.TraceError("CDAQControler::Initialize:failed to start");
					}
					else
					{
						//cout<<"- start"<<endl;
						g_logger.TraceInfo("CDAQControler::Initialize:_beginthreadex");

						m_hDAQThread = (HANDLE)_beginthreadex(NULL, 0, DAQThreadFunc, (LPVOID)this, 0, NULL);  
					}
					//cout<<"DllStart return:"<<ret<<endl;
				}

			}
		}
		catch (exception &e)
		{
			g_logger.TraceError("CDAQControler::Initialize:(in catch)Initial DAQ failed.");
		}
	}
	void CDAQControler::InitGradientBegin()
	{
		if (!CheckDAQStarted())
		{
			return;
		}
		SetEvent(m_gEvtInitGradient);
		SampleBegin();
	}
	void CDAQControler::InitGradientEnd()
	{
		if (!CheckDAQStarted())
		{
			return;
		}
		ResetEvent(m_gEvtInitGradient);
		if (WAIT_OBJECT_0 != WaitForSingleObject(m_gEvtMoveDetection,0)
			|| WAIT_OBJECT_0 != WaitForSingleObject(m_gEvtStillDetection,0) )
		{
			SampleEnd();
		}
	}

	void CDAQControler::StillDetectionBegin()
	{
		if (!CheckDAQStarted())
		{
			return;
		}
		SetEvent(m_gEvtStillDetection);
		SampleBegin();
	}

	void CDAQControler::StillDetectionEnd()
	{
		if (!CheckDAQStarted())
		{
			return;
		}

		ResetEvent(m_gEvtStillDetection);
		if (WAIT_OBJECT_0 != WaitForSingleObject(m_gEvtInitGradient,0)
			&& WAIT_OBJECT_0 != WaitForSingleObject(m_gEvtMoveDetection,0) )
		{
			SampleEnd();
		}
	}

	void CDAQControler::MoveDetectionBegin()
	{
		if (!CheckDAQStarted())
		{
			return;
		}
		SetEvent(m_gEvtMoveDetection);
		SampleBegin();
	}

	void CDAQControler::MoveDetectionEnd()
	{
		if (!CheckDAQStarted())
		{
			return;
		}

		ResetEvent(m_gEvtMoveDetection);
		if (WAIT_OBJECT_0 != WaitForSingleObject(m_gEvtInitGradient,0)
			|| WAIT_OBJECT_0 != WaitForSingleObject(m_gEvtStillDetection,0) )
		{
			SampleEnd();
		}
	}

	void CDAQControler::SampleBegin(){
		SetEvent(m_gEvtSample);
	}

	void CDAQControler::SampleEnd(){
		ResetEvent(m_gEvtSample);
	}

	bool CDAQControler::NewProject(char cMode)
	{
		//20180313temp for new project
		//if (!CheckDAQStarted())
		//{
		//	g_logger.TraceError("CDAQControler::NewProject error: DAQ was NOT been initialized");
		//	return false;
		//}

		//开始保存数据
		if (0x02 == cMode)//完整检测
		{
			openFile();
			SetEvent(m_gEvtSaveFile);
		}
		else if(0x01 == cMode)//测试模式
		{
			ResetEvent(m_gEvtSaveFile);
		}
		return true;

	}

	void CDAQControler::TerminateProject()
	{
		if (!CheckDAQStarted())
		{
			return;
		}

		MoveDetectionEnd();
		StillDetectionEnd();

		ResetEvent(m_gEvtSaveFile);

		VirtualFree(buffer, SingleSavingFileSizeW, MEM_RELEASE);
		CloseHandle(hFile);
	}
	//void CDAQControler::SetInitAngleFlag()
	//{
	//	SetEvent(m_gEvtInitAngleFlag);
	//}
	bool CDAQControler::CheckDAQStarted()const
	{
		return m_bDAQInitialSuccessfully;
	}

}

