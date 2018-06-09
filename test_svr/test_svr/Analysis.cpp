#include "main.h"
#include "Analysis.h"
#include <sstream>

#include <algorithm>

extern CtheApp* theApp;

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
					if(!PostThreadMessage(theApp->m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_THREE,(LPARAM)pBuf))
					{
						g_logger.TraceError("AnalysisThreadFunc - PostThreadMessage failed");
						delete[] pBuf;
						pBuf = NULL;
					}

				}
				else
				{
					pAnalysis->PostAnalysisStateMsg(NUM_TWO);
					//PostThreadMessage(theApp->m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_TWO,NULL);
				}
			}
		}


		return 0;
	}

	CAnalysis::CAnalysis(void)
		:m_hAnalysisThread(NULL)
		,m_hFileReader(NULL)
		,m_dCarInitXAngle(0.0)
		,m_dCarInitYAngle(0.0)
		,m_dMaxHandBrakeForce(0.0)
	{
		g_logger.TraceInfo("CAnalysis::CAnalysis");
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
			if(!PostThreadMessage(theApp->m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_THREE,(LPARAM)pBuf))
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
		//---------read car angle from ini------
		char stInitX[50]={0};  
		char stInitY[50]={0};  

		DWORD dwRet1,dwRet2;
		dwRet1 = GetPrivateProfileStringA(gc_strInitialCarAngle.c_str(), gc_strInitXAngle.c_str(), "", stInitX, 50, m_strConfigFile.c_str());  
		dwRet2 =GetPrivateProfileStringA(gc_strInitialCarAngle.c_str(), gc_strInitYAngle.c_str(), "", stInitY, 50, m_strConfigFile.c_str());  
		if (dwRet2 <= 0 || dwRet1 <= 0)
		{
			strErrInfo = "Get InitialX or InitialY in the INI file failed";
			g_logger.TraceError("CAnalysis::ReadParaFromINI - %s",strErrInfo.c_str() );
			return false;
		}
		std::stringstream stream;
		stream<<stInitX;
		stream>>m_dCarInitXAngle;
		stream.clear();
		stream<<stInitY;
		stream>>m_dCarInitYAngle;

		//---------read max hand brake force from ini------
		char stMaxHandBrakeForce[50]={0};  
		DWORD dwRet3;
		dwRet3 = GetPrivateProfileStringA(gc_strResult.c_str(), gc_strMaxHandBrakeForce.c_str(), "", stMaxHandBrakeForce, 50, m_strConfigFile.c_str());  
		if ( dwRet3 <= 0 )
		{
			strErrInfo = "Get MaxHandBrakeForce in the INI file failed";
			g_logger.TraceError("CAnalysis::ReadParaFromINI - %s",strErrInfo.c_str() );
			return false;
		}
		stream.clear();
		stream<<stMaxHandBrakeForce;
		stream>>m_dMaxHandBrakeForce;

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

		if (theApp->m_pDataController->DAQIsWirelessType())
		{
			HandleDataW(buffer, DAQCONTROLER::channelCountW, dwFileSize, DAQCONTROLER::deltatW);
		}
		else
		{
			HandleData(buffer, DAQCONTROLER::channelCount, dwFileSize, DAQCONTROLER::deltat);
		}

		if (NULL != buffer)
		{
			VirtualFree(buffer, DAQCONTROLER::SingleSavingFileSize, MEM_RELEASE);
			buffer = NULL;
		}
		if (INVALID_HANDLE_VALUE != m_hFileReader)
		{
			CloseHandle(m_hFileReader);
			m_hFileReader = NULL;
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
		this->PostAnalysisStateMsg();

		m_vAnalysisData.clear();
		m_vAccelaration.clear();
		m_vVelocity.clear();
		m_vFootBrakeForce.clear();
		m_vPedalDistance.clear();

		DWORD doubleNum = dwDataSize / (sizeof(double)) / channelCount;//几组10通道double值
		double dSumA = 0.0;
		double dCompoundA = 0.0;

		m_stResult.MaxAccelaration = sqrt((*(pData+7))*(*(pData+7))+(*(pData+8))*(*(pData+8))+(*(pData+9))*(*(pData+9)));
		double dSumVel=0.0;
		double dCurrentVel=0.0;
		double dCurrentDist=0.0;

		m_stResult.MaxFootBrakeForce = *(pData+0) - *(pData+1);
		m_stResult.GradientX = *(pData+2);
		m_stResult.GradientY = *(pData+3);
		m_stResult.PedalDistance = *(pData+6);

		ANALYSISRESULT stAnalysisInfo;
		for (DWORD i=0;i<doubleNum;++i)
		{
			dCompoundA = sqrt((*(pData+(i*channelCount)+7))*(*(pData+(i*channelCount)+7))+(*(pData+(i*channelCount)+8))*(*(pData+(i*channelCount)+8))+(*(pData+(i*channelCount)+9))*(*(pData+(i*channelCount)+9)));
			dSumA += dCompoundA;
			dCurrentVel = dSumA*deltat;
			dSumVel += dCurrentVel;
			dCurrentDist = dSumVel*deltat;

			stAnalysisInfo.MaxFootBrakeForce = *(pData+(i*channelCount)) - *(pData+(i*channelCount)+1);
			//stAnalysisInfo.MaxHandBrakeForce = *(pData+(i*channelCount)+4) - *(pData+(i*channelCount)+5);
			stAnalysisInfo.GradientX = *(pData+(i*channelCount)+2);
			stAnalysisInfo.GradientY = *(pData+(i*channelCount)+3);
			stAnalysisInfo.PedalDistance = *(pData+(i*channelCount)+6);

			if (m_stResult.MaxFootBrakeForce < stAnalysisInfo.MaxFootBrakeForce)
			{
				m_stResult.MaxFootBrakeForce = stAnalysisInfo.MaxFootBrakeForce;
			}
			//if (m_stResult.MaxHandBrakeForce< stStressInfo.MaxHandBrakeForce)
			//{
			//	m_stResult.MaxHandBrakeForce = stStressInfo.MaxHandBrakeForce;
			//}
			if (m_stResult.GradientX < stAnalysisInfo.GradientX)
			{
				m_stResult.GradientX = stAnalysisInfo.GradientX;
			}
			if (m_stResult.GradientY < stAnalysisInfo.GradientY)
			{
				m_stResult.GradientY = stAnalysisInfo.GradientY;
			}
			if (m_stResult.PedalDistance < stAnalysisInfo.PedalDistance)
			{
				m_stResult.PedalDistance = stAnalysisInfo.PedalDistance;
			}

			//save file data to vector for send to UI curve
			static int nCountBetweenSend = 0;
			++nCountBetweenSend;
			if (nCountBetweenSend > COUNTBETWEENSEND)//每COUNTBETWEENSEND个数发送一个数
			{
				nCountBetweenSend = 0;
				ANALYSISDATA stData = {0.0};
				stData.Accelaration = dCompoundA;
				stData.Velocity = dCurrentVel;
				stData.FootBrakeForce = stAnalysisInfo.MaxFootBrakeForce;
				stData.PedalDistance = m_stResult.PedalDistance;
				m_vAnalysisData.push_back(stData);
				m_vAccelaration.push_back(stData.Accelaration);
				m_vFootBrakeForce.push_back(stData.FootBrakeForce);
				m_vPedalDistance.push_back(stData.PedalDistance);
				m_vVelocity.push_back(stData.Velocity);	
			}


		}
		m_stResult.AverageVelocity = dSumVel / (doubleNum/channelCount);
		m_stResult.BrakeDistance = dCurrentDist;
		m_stResult.MaxAccelaration = dCompoundA;

		NormalizData();
		SendAnalysisData();


		theApp->m_pDataController->TransformAcceleration(m_stResult.MaxAccelaration);
		theApp->m_pDataController->TransformVelocity(m_stResult.AverageVelocity);

		theApp->m_pDataController->TransformFootBrakeForce(m_stResult.MaxFootBrakeForce);
		//theApp->m_pDataController->TransformHandBrakeForce(m_stResult.MaxHandBrakeForce);
		theApp->m_pDataController->TransformGradient(m_stResult.GradientX);
		theApp->m_pDataController->TransformGradient(m_stResult.GradientY);
		//需要减去检测时车辆参考面的相对倾角，得到地面实际倾角
		m_stResult.GradientX -= m_dCarInitXAngle;
		m_stResult.GradientY -= m_dCarInitYAngle;
		theApp->m_pDataController->TransformPedalDistance(m_stResult.PedalDistance);

		m_stResult.MaxHandBrakeForce = m_dMaxHandBrakeForce;//from ini，不再需要转换

		g_logger.TraceWarning("CAnalysis::HandleData - Result=%2f_%2f_%2f_%2f_%2f_%2f_%2f_%2f",
			m_stResult.MaxAccelaration,
			m_stResult.BrakeDistance,
			m_stResult.AverageVelocity,
			m_stResult.GradientX,
			m_stResult.GradientY,
			m_stResult.PedalDistance,
			m_stResult.MaxHandBrakeForce,
			m_stResult.MaxFootBrakeForce);

	}
	void CAnalysis::HandleDataW(const double* pData, const int channelCount, const DWORD dwDataSize/*Byte*/, const double deltat)
	{
		this->PostAnalysisStateMsg();

		m_vAnalysisData.clear();
		m_vAccelaration.clear();
		m_vVelocity.clear();
		m_vFootBrakeForce.clear();
		m_vPedalDistance.clear();

		DWORD doubleNum = dwDataSize / (sizeof(double)) / channelCount;//几组8通道double值
		double dSumA = 0.0;
		double dCompoundA = 0.0;

		m_stResult.MaxAccelaration = sqrt((*(pData)) * (*(pData)) + (*(pData+2*1024) * (*(pData+2*1024))) + (*(pData+3*1024)) * (*(pData+3*1024)) );
		double dSumVel=0.0;
		double dCurrentVel=0.0;
		double dCurrentDist=0.0;

		m_stResult.MaxFootBrakeForce = *(pData+7*1024);
		m_stResult.GradientX = *(pData+1*1024);
		m_stResult.GradientY = *(pData+4*1024);
		m_stResult.PedalDistance = *(pData+5*1024);

		ANALYSISRESULT stAnalysisInfo;
		for (DWORD i=0;i<doubleNum;++i)
		{
			dCompoundA = sqrt((*(pData+i)) * (*(pData+i)) + (*(pData+2*1024+i) * (*(pData+2*1024+i))) + (*(pData+3*1024+i)) * (*(pData+3*1024+i)) );
			dSumA += dCompoundA;
			dCurrentVel = dSumA*deltat;
			dSumVel += dCurrentVel;
			dCurrentDist = dSumVel*deltat;

			stAnalysisInfo.MaxFootBrakeForce = *(pData+7*1024+i);
			//stAnalysisInfo.MaxHandBrakeForce = *(pData+(i*channelCount)+4) - *(pData+(i*channelCount)+5);
			stAnalysisInfo.GradientX = *(pData+1*1024+i);
			stAnalysisInfo.GradientY = *(pData+4*1024+i);
			stAnalysisInfo.PedalDistance = *(pData+5*1024+i);

			if (m_stResult.MaxFootBrakeForce < stAnalysisInfo.MaxFootBrakeForce)
			{
				m_stResult.MaxFootBrakeForce = stAnalysisInfo.MaxFootBrakeForce;
			}
			//if (m_stResult.MaxHandBrakeForce< stStressInfo.MaxHandBrakeForce)
			//{
			//	m_stResult.MaxHandBrakeForce = stStressInfo.MaxHandBrakeForce;
			//}
			if (m_stResult.GradientX < stAnalysisInfo.GradientX)
			{
				m_stResult.GradientX = stAnalysisInfo.GradientX;
			}
			if (m_stResult.GradientY < stAnalysisInfo.GradientY)
			{
				m_stResult.GradientY = stAnalysisInfo.GradientY;
			}
			if (m_stResult.PedalDistance < stAnalysisInfo.PedalDistance)
			{
				m_stResult.PedalDistance = stAnalysisInfo.PedalDistance;
			}

			//save file data to vector for send to UI curve
			static int nCountBetweenSend = 0;
			++nCountBetweenSend;
			if (nCountBetweenSend > COUNTBETWEENSEND)//每COUNTBETWEENSEND个数发送一个数
			{
				nCountBetweenSend = 0;
				ANALYSISDATA stData = {0.0};
				stData.Accelaration = dCompoundA;
				stData.Velocity = dCurrentVel;
				stData.FootBrakeForce = stAnalysisInfo.MaxFootBrakeForce;
				stData.PedalDistance = m_stResult.PedalDistance;
				m_vAnalysisData.push_back(stData);
				m_vAccelaration.push_back(stData.Accelaration);
				m_vFootBrakeForce.push_back(stData.FootBrakeForce);
				m_vPedalDistance.push_back(stData.PedalDistance);
				m_vVelocity.push_back(stData.Velocity);	
			}


		}
		m_stResult.AverageVelocity = dSumVel / (doubleNum/channelCount);
		m_stResult.BrakeDistance = dCurrentDist;
		m_stResult.MaxAccelaration = dCompoundA;

		NormalizData();
		SendAnalysisData();


		theApp->m_pDataController->TransformAcceleration(m_stResult.MaxAccelaration);
		theApp->m_pDataController->TransformVelocity(m_stResult.AverageVelocity);

		theApp->m_pDataController->TransformFootBrakeForce(m_stResult.MaxFootBrakeForce);
		//theApp->m_pDataController->TransformHandBrakeForce(m_stResult.MaxHandBrakeForce);
		theApp->m_pDataController->TransformGradient(m_stResult.GradientX);
		theApp->m_pDataController->TransformGradient(m_stResult.GradientY);
		//需要减去检测时车辆参考面的相对倾角，得到地面实际倾角
		m_stResult.GradientX -= m_dCarInitXAngle;
		m_stResult.GradientY -= m_dCarInitYAngle;
		theApp->m_pDataController->TransformPedalDistance(m_stResult.PedalDistance);

		m_stResult.MaxHandBrakeForce = m_dMaxHandBrakeForce;//from ini，不再需要转换

		g_logger.TraceWarning("CAnalysis::HandleData - Result=%2f_%2f_%2f_%2f_%2f_%2f_%2f_%2f",
			m_stResult.MaxAccelaration,
			m_stResult.BrakeDistance,
			m_stResult.AverageVelocity,
			m_stResult.GradientX,
			m_stResult.GradientY,
			m_stResult.PedalDistance,
			m_stResult.MaxHandBrakeForce,
			m_stResult.MaxFootBrakeForce);

	}
	
	bool CAnalysis::AnalyseResult()
	{
		g_logger.TraceInfo("CAnalysis::AnalyseResult-in");
		int nResult = NUM_ONE;//need add judge

		if(!PostThreadMessage(theApp->m_dwMainThreadID,msg_ANA_ANALYSIS_RESULT ,nResult,(LPARAM)&m_stResult))
		{
			g_logger.TraceError("CAnalysis::AnalyseResult - PostThreadMessage failed");
		}
		return true;
	}
	void CAnalysis::NormalizData()
	{//归一化到1-100之间的char类型
		//accelaration - -0.625~0.625
		g_logger.TraceInfo("CAnalysis::NormalizData-in");
		double dminAccelaration=0,dmaxAccelaration=0;
		FindMaxMinRange(m_vAccelaration,dminAccelaration,dmaxAccelaration);
		double dminVelocity=0,dmaxVelocity=0;
		FindMaxMinRange(m_vVelocity,dminVelocity,dmaxVelocity);
		double dminFootBrakeForce=0,dmaxFootBrakeForce=0;
		FindMaxMinRange(m_vFootBrakeForce,dminFootBrakeForce,dmaxFootBrakeForce);
		double dminPedalDistance=0,dmaxPedalDistance=0;
		FindMaxMinRange(m_vPedalDistance,dminPedalDistance,dmaxPedalDistance);

		for( std::vector<ANALYSISDATA>::iterator itData = m_vAnalysisData.begin();
			itData!=m_vAnalysisData.end();
			++itData )
		{
			NormalizDataOne(itData->Accelaration, dminAccelaration, dmaxAccelaration);
			NormalizDataOne(itData->Velocity, dminVelocity, dmaxVelocity);
			NormalizDataOne(itData->FootBrakeForce, dminFootBrakeForce, dmaxFootBrakeForce);
			NormalizDataOne(itData->PedalDistance, dminPedalDistance, dmaxPedalDistance);
		}


	}
	void CAnalysis::NormalizDataOne(double& dData, const double dOriginMin, const double dOriginMax, const int nNewRange)
	{
		double dOriginRange = dOriginMax - dOriginMin;
		if (dOriginRange == 0)//采集到数据是恒值
		{
			if (dData>=nNewRange)
			{
				dData = nNewRange;
			}
			else if(dData <= 0)
			{
				dData = 0;
			}
			else
			{
				//do nothing
			}
		}
		else
		{
			dData = (dData-dOriginMin) / dOriginRange * nNewRange;
		}
	}

	void CAnalysis::FindMaxMinRange(vector<double> &vData/*in*/, double& minData, double& maxData)
	{
		g_logger.TraceInfo("CAnalysis::FindMaxMinRange-in");
		std::vector<double>::iterator max_ = std::max_element(std::begin(vData), std::end(vData));
		maxData = *max_;
		auto min_ = std::min_element(std::begin(vData), std::end(vData));
		minData = *min_;
	}

	void CAnalysis::SendAnalysisData()
	{
		g_logger.TraceInfo("CAnalysis::SendAnalysisData-in");
		theApp->m_pCommunicator->SendAnalysisData2UI(m_vAnalysisData);
		//if(!PostThreadMessage(theApp->m_dwMainThreadID, cmd_ANALYSIS_DATA, 1, (LPARAM)&stData))
		//{
		//	g_logger.TraceError("CAnalysis::AnalyseResult - PostThreadMessage failed");
		//}

	}
	//1 - 分析中
	//2 - 分析成功
	void CAnalysis::PostAnalysisStateMsg(const int nState)
	{
		PostThreadMessage(theApp->m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,nState,NULL);//分析中
		g_logger.TraceInfo("CAnalysis::PostAnalysisStateMsg:%d",nState);
	}

}

