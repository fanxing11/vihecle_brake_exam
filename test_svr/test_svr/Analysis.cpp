#include "main.h"
#include "Analysis.h"

extern CtheApp theApp;

namespace ANALYSISSPACE
{

	inline	bool CheckFolderExist(const string& strPath)
	{
		WIN32_FIND_DATAA wfd;
		bool bRet = false;
		HANDLE hFind = FindFirstFileA(strPath.c_str(),&wfd);
		if ( (hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			bRet = true;
		}
		FindClose(hFind);
		return bRet;
	}

	inline bool CheckFileExist(const string& strFileFullPath)
	{
		WIN32_FIND_DATAA wfd;
		bool bRet = false;
		HANDLE hFind = FindFirstFileA(strFileFullPath.c_str(),&wfd);
		//if ( (hFind != INVALID_HANDLE_VALUE) &&(wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) )
		if ( (hFind != INVALID_HANDLE_VALUE) )
		{
			bRet = true;
		}
		FindClose(hFind);
		return bRet;
	}

	unsigned int WINAPI AnalysisThreadFunc(LPVOID lp)
	{
		CAnalysis* pAnalysis = (CAnalysis*)lp;

		MSG msg;
		bool bRet = false;
		while(GetMessage(&msg,NULL,0,0))
		{
			if (msg_ANA_ANALYSIS_BEGIN == msg.message)
			{
				string strInfo;
				if( !(pAnalysis->_BeginaAnalysis(strInfo)) )
				{
					char* pBuf = new char[strInfo.length()+1];
					memset(pBuf,0,strInfo.length()+1);
					memcpy(pBuf,strInfo.c_str(),strInfo.length());
					if( !PostThreadMessage(theApp.m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_THREE,(LPARAM)pBuf) )
					{
						delete[] pBuf;
						pBuf = NULL;
					}
				}
				else
				{
					PostThreadMessage(theApp.m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_TWO,NULL);
				}
			}
		}



		return 0;
	}

	CAnalysis::CAnalysis(void)
		:m_hAnalysisThread(NULL)
	{
	}


	CAnalysis::~CAnalysis(void)
	{
		if (NULL != m_hBeginEvent)
		{
			CloseHandle(m_hBeginEvent);
		}
		if( WAIT_OBJECT_0 == WaitForSingleObject(m_hAnalysisThread,1000) )
		{
			////nomal
		}

	}

	void CAnalysis::BeginAnalysis(const string strProjectPath)
	{
		if ( !CheckFolderExist(strProjectPath))
		{
			g_logger.TraceError("CAnalysis::BeginAnalysis - ProjectPath(%s) is not exist",strProjectPath.c_str());
			//send msg
			return;
		}
		string strConfigFile = strProjectPath+"\\config.ini";
		if ( !CheckFileExist(strConfigFile))
		{
			g_logger.TraceError("CAnalysis::BeginAnalysis - ConfigFile(%s) is not exist",strConfigFile.c_str());
			//send msg
			return;
		}
		string strBinFile = strProjectPath + "\\*.bin";
		if ( !CheckFileExist(strBinFile))
		{
			g_logger.TraceError("CAnalysis::BeginAnalysis - *.bin File(%s) is not exist",strBinFile.c_str());
			//send msg
			return;
		}
		
		if (NULL == m_hAnalysisThread)
		{
			m_hAnalysisThread = (HANDLE)_beginthreadex(NULL, 0, AnalysisThreadFunc, (LPVOID)this, 0, NULL);  
		}
		if (NULL == m_hBeginEvent)
		{
			m_hBeginEvent = CreateEvent(NULL,TRUE,FALSE,L"");
		}

		PostThreadMessage(GetThreadId(m_hAnalysisThread),msg_ANA_ANALYSIS_BEGIN ,NULL,NULL);


	}
	bool CAnalysis::_BeginaAnalysis(string &strErrInfo)
	{
		if( WAIT_OBJECT_0 == WaitForSingleObject(m_hBeginEvent,0) )
		{
			//post msg in analysing
			strErrInfo = ("有一个分析尚未完成");
			return false;
		}
		SetEvent(m_hBeginEvent);

		//open file
		//read and transform data
		//send result to main thread msg_ANA_ANALYSIS_RESULT

		ResetEvent(m_hBeginEvent);
		return true;

	}


			//if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hAnalysisThread,3000) )
			//{
			//	m_hAnalysisThread = NULL;
			//}
			//else
			//{
			//	g_logger.TraceError("CAnalysis::BeginAnalysis ; WaitForSingleObject error");


}

