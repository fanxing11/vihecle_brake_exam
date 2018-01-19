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
					PostThreadMessage(theApp.m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_TWO,NULL);
				}
			}
		}


		return 0;
	}

	CAnalysis::CAnalysis(void)
		:m_hAnalysisThread(NULL)
		,m_hFileReader(NULL)
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

		////send result to main thread msg_ANA_ANALYSIS_RESULT
		if( !AnalyseResult() )
		{
			return false;
		}

		return true;

	}

	bool CAnalysis::ReadParaFromINI(string &strErrInfo)
	{
		char stInitX[50]={0};  
		char stInitY[50]={0};  

		DWORD dwRet1,dwRet2;
		dwRet1 = GetPrivateProfileStringA(gc_strInitialAngle.c_str(), gc_strInitXAngle.c_str(), "", stInitX, 50, m_strConfigFile.c_str());  
		dwRet2 =GetPrivateProfileStringA(gc_strInitialAngle.c_str(), gc_strInitYAngle.c_str(), "", stInitY, 50, m_strConfigFile.c_str());  
		if (dwRet2 <= 0 || dwRet1 <= 0)
		{
			strErrInfo = "Get InitialX or InitialY in the INI file failed";
			g_logger.TraceError("CAnalysis::ReadParaFromINI - %s",strErrInfo.c_str() );
			return false;
		}
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
		bool bRet = true;

		string strDataFileName;
		this->GetDataFile( strDataFileName );

		m_hFileReader = CreateFileA(
			strDataFileName.c_str(),
			GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);

		if (m_hFileReader == INVALID_HANDLE_VALUE)
		{
			g_logger.TraceError("CAnalysis::ReadDataFromFile - %s", strErrInfo.c_str());
			bRet = false;
		}
		DWORD dwFileSize = GetFileSize(m_hFileReader,NULL); 

		double* buffer = (double*)VirtualAlloc(NULL, dwFileSize, MEM_COMMIT, PAGE_READWRITE);
		if (!buffer)
		{
			g_logger.TraceError("CAnalysis::ReadDataFromFile - Allocate buffer fail(error %d)\n",GetLastError());
			bRet = false;
		}

		double *tmpBuf = buffer;
		DWORD dwBytesRead = 0;
		DWORD dwBytesToRead = dwFileSize;//,tmpLen;
		do{ //循环读文件，确保读出完整的文件    
			static int nCount=0;
			++nCount;
			g_logger.TraceWarning("CAnalysis::ReadDataFromFile - begin read %d times",nCount);

			if(!ReadFile(m_hFileReader,tmpBuf,dwBytesToRead,&dwBytesRead,NULL))
			{
				g_logger.TraceWarning("CAnalysis::ReadDataFromFile - ReadFile failed:error=%d",GetLastError());
			}

			if (dwBytesRead == 0)
				break;

			dwBytesToRead -= dwBytesRead;
			tmpBuf += dwBytesRead;

		} while (dwBytesToRead > 0);

		HandleData(buffer, DAQCONTROLER::channelCount, dwFileSize, DAQCONTROLER::deltat);

		if (NULL != buffer)
		{
			VirtualFree(buffer, DAQCONTROLER::SingleSavingFileSize, MEM_RELEASE);
			buffer = NULL;
		}
		if (INVALID_HANDLE_VALUE != m_hFileReader)
		{
			CloseHandle(m_hFileReader);
		}

		if ( !bRet )
		{
			strErrInfo = "Read Data From File failed.";
		}

		return bRet;
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

	void CAnalysis::HandleData(const double* pData, const int channelCount, const DWORD dwDataSize/*Byte*/, const double deltat)
	{
		DWORD doubleNum = dwDataSize / (sizeof(double)) / channelCount;//几组10通道double值
		double dSumA = 0.0;
		double dCompoundA = 0.0;

		m_stResult.MaxAccelaration = sqrt((*(pData+7))*(*(pData+7))+(*(pData+8))*(*(pData+8))+(*(pData+9))*(*(pData+9)));
		double dSumVel=0.0;
		double dCurrentVel=0.0;
		double dCurrentDist=0.0;

		STRESSINFO stStressInfo;
		m_stResult.MaxFootBrakeForce = *(pData+0) - *(pData+1);
		m_stResult.Gradient = *(pData+2);//暂时使用一个方向的角度
		m_stResult.MaxHandBrakeForce = *(pData+4) - *(pData+5);
		m_stResult.PedalDistance = *(pData+6);

		for (DWORD i=0;i<doubleNum;++i)
		{
			dCompoundA = sqrt((*(pData+(i*channelCount)+7))*(*(pData+(i*channelCount)+7))+(*(pData+(i*channelCount)+8))*(*(pData+(i*channelCount)+8))+(*(pData+(i*channelCount)+9))*(*(pData+(i*channelCount)+9)));
			dSumA += dCompoundA;
			dCurrentVel = dSumA*deltat;
			dSumVel += dCurrentVel;
			dCurrentDist = dSumVel*deltat;

			stStressInfo.MaxFootBrakeForce = *(pData+(i*channelCount)) - *(pData+(i*channelCount)+1);
			stStressInfo.MaxHandBrakeForce = *(pData+(i*channelCount)+4) - *(pData+(i*channelCount)+5);
			stStressInfo.Gradient = *(pData+(i*channelCount)+2);//暂时使用一个方向的角度
			stStressInfo.PedalDistance = *(pData+(i*channelCount)+6);

			if (m_stResult.MaxFootBrakeForce < stStressInfo.MaxFootBrakeForce)
			{
				m_stResult.MaxFootBrakeForce = stStressInfo.MaxFootBrakeForce;
			}
			if (m_stResult.MaxHandBrakeForce< stStressInfo.MaxHandBrakeForce)
			{
				m_stResult.MaxHandBrakeForce = stStressInfo.MaxHandBrakeForce;
			}
			if (m_stResult.Gradient < stStressInfo.Gradient)
			{
				m_stResult.Gradient = stStressInfo.Gradient;
			}
			if (m_stResult.PedalDistance < stStressInfo.PedalDistance)
			{
				m_stResult.PedalDistance = stStressInfo.PedalDistance;
			}

		}
		m_stResult.AverageVelocity = dSumVel / (doubleNum/channelCount);
		m_stResult.BrakeDistance = dCurrentDist;
		m_stResult.MaxAccelaration = dCompoundA;

		theApp.m_pDataC->TransformAcceleration(m_stResult.MaxAccelaration);
		theApp.m_pDataC->TransformVelocity(m_stResult.AverageVelocity);

		theApp.m_pDataC->TransformFootBrakeForce(m_stResult.MaxFootBrakeForce);
		theApp.m_pDataC->TransformHandBrakeForce(m_stResult.MaxHandBrakeForce);
		theApp.m_pDataC->TransformGradient(m_stResult.Gradient);//暂时使用一个方向的角度
		//需要减去初始地面倾角
		theApp.m_pDataC->TransformPedalDistance(m_stResult.PedalDistance);

	}
	bool CAnalysis::AnalyseResult()
	{
		int nResult = 1;
		int nDataNum=7;
		double* pBuf = new double[nDataNum+1];
		memset(pBuf,0,(nDataNum+1)*sizeof(double));
		memcpy(pBuf,&m_stResult,nDataNum*sizeof(double));
		if(!PostThreadMessage(theApp.m_dwMainThreadID,msg_ANA_ANALYSIS_RESULT ,(WPARAM)&nResult,(LPARAM)pBuf))
		{
			g_logger.TraceError("CAnalysis::AnalyseResult - PostThreadMessage failed");
			delete[] pBuf;
			pBuf = NULL;
		}
		return true;
	}

}

