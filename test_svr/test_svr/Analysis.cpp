#include "main.h"
#include "Analysis.h"
#include <sstream>

extern CtheApp theApp;

namespace ANALYSISSPACE
{

	unsigned int WINAPI AnalysisThreadFunc(LPVOID lp)
	{
		CAnalysis* pAnalysis = (CAnalysis*)lp;

		//新建的第一次也需要分析
		PostThreadMessage(GetCurrentThreadId(),msg_ANA_ANALYSIS_BEGIN ,NULL,NULL);

		MSG msg;
		bool bRet = false;
		while(GetMessage(&msg,NULL,0,0))
		{
			if (msg_ANA_ANALYSIS_BEGIN == msg.message)
			{
				string strInfo;
				if( !(pAnalysis->_BeginaAnalysis(strInfo)) )
				{

					char* pBuf = NULL;
					int nLen = strInfo.length();
					pBuf = new char[nLen+1];
					memset(pBuf,0,nLen+1);
					memcpy(pBuf,strInfo.c_str(),nLen);
					if(!PostThreadMessage(theApp.m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_THREE,(LPARAM)pBuf))
					{
						g_logger.TraceError("AnalysisThreadFunc - PostThreadMessage failed");
						delete[] pBuf;
						pBuf = NULL;
					}

				}
				else
				{
					PostThreadMessage(theApp.m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_TWO,(LPARAM)"");//注意最后一个参数不能是NULL，否则解析出错
				}
			}
		}


		return 0;
	}

	CAnalysis::CAnalysis(void)
		:m_hAnalysisThread(NULL)
		,m_pF(NULL)
		,m_dInitXAngle(0.0)
		,m_dInitYAngle(0.0)
	{
	}


	CAnalysis::~CAnalysis(void)
	{
		if( WAIT_OBJECT_0 == WaitForSingleObject(m_hAnalysisThread,1000) )
		{
			////normal
		}

	}

	void CAnalysis::BeginAnalysis(const string &strProjectPath)
	{
		m_strProjectPath = strProjectPath;
		if ( m_strProjectPath.at(m_strProjectPath.length()-1) != '\\' )
		{
			m_strProjectPath.append(1,'\\');
		}
		bool bGood = true;
		string strInfo;
		m_strConfigFile = m_strProjectPath+gc_strProjectParaINI_FileName;
		string strBinFile = m_strProjectPath + "*.bin";

		if ( !CheckFolderExist(m_strProjectPath))
		{
			bGood = false;
			strInfo = ("ProjectPath(");
			strInfo = strInfo+m_strProjectPath+string(") is not exist");
		}
		else if ( !CheckFileExist(m_strConfigFile))
		{
			bGood = false;
			strInfo = ("ProjectParaFile(");
			strInfo = strInfo+m_strConfigFile+string(") is not exist");
		}
		else if ( !CheckFileExist(strBinFile))
		{
			bGood = false;
			strInfo = ("DataFile(");
			strInfo = strInfo+ strBinFile +string(") is not exist");
		}

		if (!bGood)
		{
			g_logger.TraceWarning("CAnalysis::BeginAnalysis - %s",strInfo.c_str());

			char* pBuf = NULL;
			int nLen = strInfo.length();
			pBuf = new char[nLen+1];
			memset(pBuf,0,nLen+1);
			memcpy(pBuf,strInfo.c_str(),nLen);
			if(!PostThreadMessage(theApp.m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_THREE,(LPARAM)pBuf))
			{
				g_logger.TraceError("CAnalysis::BeginAnalysis - PostThreadMessage failed");
				delete[] pBuf;
				pBuf = NULL;
			}

			return;
		}
		
		if (NULL == m_hAnalysisThread)
		{
			m_hAnalysisThread = (HANDLE)_beginthreadex(NULL, 0, AnalysisThreadFunc, (LPVOID)this, CREATE_SUSPENDED, NULL);  
			ResumeThread(m_hAnalysisThread);
		}
		else
		{
			PostThreadMessage(GetThreadId(m_hAnalysisThread),msg_ANA_ANALYSIS_BEGIN ,NULL,NULL);
		}
	}

	bool CAnalysis::_BeginaAnalysis(string &strErrInfo)
	{
		//get para from INI
		if(!ReadParaFromINI(strErrInfo))
		{
			return false;
		}

		//read and transform data
		if (!ReadDataFromFile(strErrInfo))
		{
			return false;
		}

		//send result to main thread msg_ANA_ANALYSIS_RESULT
		return true;

	}

	bool CAnalysis::ReadParaFromINI(string &strErrInfo)
	{
		char stInitX[50]={0};  
		char stInitY[50]={0};  

		GetPrivateProfileStringA(gc_strInitialAngle.c_str(), gc_strInitXAngle.c_str(), "", stInitX, 50, m_strConfigFile.c_str());  
		GetPrivateProfileStringA(gc_strInitialAngle.c_str(), gc_strInitYAngle.c_str(), "", stInitY, 50, m_strConfigFile.c_str());  
		std::stringstream stream;
		stream<<stInitX;
		stream>>m_dInitXAngle;
		stream.clear();
		stream<<stInitY;
		stream>>m_dInitYAngle;

		return true;
	}

	bool CAnalysis::ReadDataFromFile(string &strErrInfo)
	{
		string strDataFileName;
		this->GetDataFile( strDataFileName );

		m_pF = fopen( strDataFileName.c_str(),"rb");
		if (NULL == m_pF)
		{
			strErrInfo = string("Data File ")+strDataFileName+string(" open failed");
			return false;
		}
		double Data[10] = {0};
		while(!feof(m_pF))
		{
			if(1!=fread(Data,10*sizeof(double),1,m_pF))
			{
				break;
			}
			//parse the data 
		}
		return true;
	}

	bool CAnalysis::GetDataFile(string& strFileName)
	{
		vector<string> files;
		string format = ".bin";  
		GetAllFiles(m_strProjectPath, files,format);
		int nFileNum = files.size();
		if (NUM_ONE != nFileNum)
		{
			g_logger.TraceWarning("CAnalysis::GetDataFile - there are more than 1 data File");
		}
		strFileName = files.at(0);
		return true;
	}

}

