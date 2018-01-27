#include "main.h"

#include "time.h"

extern CtheApp theApp;

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

	// This function is used to deal with 'DataReady' Event. 
	void BDAQCALL OnDataReadyEvent(void * sender, BfdAiEventArgs * args, void *userParam)
	{
		//static int i = 1;
		//if (i>1)
		//{
		//	return;
		//}
		//1. get data
		WaveformAiCtrl * waveformAiCtrl = NULL;
		waveformAiCtrl = (WaveformAiCtrl *)sender;
		int32 getDataCount = min(USER_BUFFER_SIZE, args->Count);
		//cout<<USER_BUFFER_SIZE<<"  "<<args->Count<<"  "<<getDataCount<<endl;
		waveformAiCtrl->GetData(getDataCount, Data);

		if (WAIT_OBJECT_0 != WaitForSingleObject(m_gEvtSample,0))//是否在采集
		{
			return;
		}
		//printTime();
		//2. data process
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_gEvtInitGradient,0))
		{
			theApp.m_pDataController->HandleInitGradientData(Data, channelCount,sectionLength);
		}
		else if (WAIT_OBJECT_0 == WaitForSingleObject(m_gEvtMoveDetection,0))
		{
			theApp.m_pDataController->HandleMoveDetectionData(Data, channelCount,sectionLength,deltat);
		}
		else if (WAIT_OBJECT_0 == WaitForSingleObject(m_gEvtStillDetection,0))
		{
			theApp.m_pDataController->HandleStillDetectionData(Data, channelCount,sectionLength);
		}
		//printTime();
		//3. data save
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_gEvtSaveFile,0))
		{
			//printf("Ready to save the data...%d\n\n",getDataCount);
			memcpy(buffer, Data, SingleSavingFileSize);
			
			if (WriteFile(hFile, buffer, SingleSavingFileSize, &WrittenBytes, NULL))
			{
				//printf("Saving has been executed!\n\n");
				RealFileSize += WrittenBytes; 
				//printf("The real-time size of file is %d byte\n\n", RealFileSize);
				//printf("Executed %d time.\n\n", i++);
			} 
			else
			{
				g_logger.TraceError("OnDataReadyEvent:error!");
			}	
		}
		//memcpy(DateOne, buffer, 10*sizeof(double));

		//4. send data to UI
		PostThreadMessage(theApp.m_dwMainThreadID, msg_DAQ_DATAONE, NULL, NULL);

	}
	//The function is used to deal with 'OverRun' Event.
	void BDAQCALL OnOverRunEvent(void * sender, BfdAiEventArgs * args, void *userParam)
	{
		printf("Streaming AI Overrun: offset = %d, count = %d\n", args->Offset, args->Count);
	}
	//The function is used to deal with 'CacheOverflow' Event.
	void BDAQCALL OnCacheOverflowEvent(void * sender, BfdAiEventArgs * args, void *userParam)
	{
		printf(" Streaming AI Cache Overflow: offset = %d, count = %d\n", args->Offset, args->Count);
	}
	//The function is used to deal with 'Stopped' Event.
	void BDAQCALL OnStoppedEvent(void * sender, BfdAiEventArgs * args, void *userParam)
	{
		printf("Streaming AI stopped: offset = %d, count = %d\n", args->Offset, args->Count);
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
		theApp.m_pDataController->GetProjectPath(strPath);
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

		buffer = (double*)VirtualAlloc(NULL, SingleSavingFileSize, MEM_COMMIT, PAGE_READWRITE);
		if (!buffer)
		{
			g_logger.TraceError("openFile:Allocate buffer fail(error %d)\n",GetLastError());
		}
	}


	CDAQControler::CDAQControler(void)
		:m_wfAiCtrl(NULL)
		,m_bDAQInitialSuccessfully(false)
	{
		this->Initialize();
	}

	CDAQControler::~CDAQControler(void)
	{
		this->DisInitialize();
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
		ErrorCode        ret = Success;

		do 
		{
			// step 8: Stop the operation if it is running.
			ret = m_wfAiCtrl->Stop(); 
			CHK_RESULT(ret);
		} while (false);

		// Step 9: Close device, release any allocated resource.
		m_wfAiCtrl->Dispose();

		// If something wrong in this execution, print the error code on screen for tracking.
		if(BioFailed(ret))
		{
			g_logger.TraceWarning("CDAQControler::DisInitialize:Some error occurred. And the last error code is 0x%X.\n", ret);
			//waitAnyKey();// wait any key to quit!
		}

		this->CloseEvtHandle();
	}

	void CDAQControler::Initialize()
	{
		try
		{
			CreateSyncEvent();

			ErrorCode        ret = Success;

			// Step 1: Create a 'WaveformAiCtrl' for buffered AI function.
			m_wfAiCtrl = WaveformAiCtrl::Create();

			////Step 2: Open file
			//openFile();

			// Step 3: Set the notification event Handler by which we can known the state of operation effectively.
			m_wfAiCtrl->addDataReadyHandler(OnDataReadyEvent, NULL);
			m_wfAiCtrl->addOverrunHandler(OnOverRunEvent, NULL);
			m_wfAiCtrl->addCacheOverflowHandler(OnCacheOverflowEvent, NULL);
			m_wfAiCtrl->addStoppedHandler(OnStoppedEvent, NULL);
			do
			{
				// Step 4: Select a device by device number or device description and specify the access mode.
				// in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
				DeviceInformation devInfo(deviceDescription);
				ret = m_wfAiCtrl->setSelectedDevice(devInfo);
				CHK_RESULT(ret);
				ret = m_wfAiCtrl->LoadProfile(profilePath);//Loads a profile to initialize the device.
				CHK_RESULT(ret);

				// Step 5: Set necessary parameters.
				Conversion * conversion = m_wfAiCtrl->getConversion();
				ret = conversion->setChannelStart(startChannel);
				CHK_RESULT(ret);
				ret = conversion->setChannelCount(channelCount);
				CHK_RESULT(ret);
				Record * record = m_wfAiCtrl->getRecord();
				ret = record->setSectionCount(sectionCount);//The 0 means setting 'streaming' mode.
				CHK_RESULT(ret);
				ret = record->setSectionLength(sectionLength);
				CHK_RESULT(ret);

				// Step 6: The operation has been started.
				// We can get samples via event handlers.
				ret = m_wfAiCtrl->Prepare();
				CHK_RESULT(ret);
				ret = m_wfAiCtrl->Start();
				CHK_RESULT(ret);

				//// Step 7: The device is acquiring data.
				//printf("Streaming AI is in progress.\nplease wait...  any key to quit!\n\n");
				//do
				//{
				//	SLEEP(1);
				//}	while((RealFileSize < RequirementFileSize) ? true : false);
				//printf("Saving completely!\n");

				//// step 8: Stop the operation if it is running.
				//ret = m_wfAiCtrl->Stop(); 
				//CHK_RESULT(ret);
			}while(false);

			//// Step 9: Close device, release any allocated resource.
			//m_wfAiCtrl->Dispose();
			//VirtualFree(buffer, SingleSavingFileSize, MEM_RELEASE);
			//CloseHandle(hFile);

			// If something wrong in this execution, print the error code on screen for tracking.
			m_bDAQInitialSuccessfully = true;
			if(BioFailed(ret))
			{
				//u初始化错误，弹框或者返回错误信息----
				g_logger.TraceError("CDAQControler::Initialize:Initial DAQ failed. And the last error code is 0x%X.\n", ret);
				//waitAnyKey();// wait any key to quit!
				m_bDAQInitialSuccessfully = false;
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
		if (!CheckDAQStarted())
		{
			g_logger.TraceError("CDAQControler::NewProject error: DAQ was NOT been initialized");
			return false;
		}

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

		VirtualFree(buffer, SingleSavingFileSize, MEM_RELEASE);
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

