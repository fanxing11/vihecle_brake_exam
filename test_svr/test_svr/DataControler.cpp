#include "main.h"

#include "DataControler.h"
#include <math.h>
#include "time.h"

#include <Dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")

extern CtheApp theApp;


namespace DATACONTROLER
{

	CDataControler::CDataControler(void)
	{
		m_nCurrentProjectState = 0;
	}

	CDataControler::~CDataControler(void)
	{
	}

	int CDataControler::GetCurrentProjectState()const
	{
		return m_nCurrentProjectState;
	}


	void CDataControler::SetStartChannel(const char cStart){m_cStartChannel = cStart;}
	void CDataControler::SetEndChannel(const char cEnd){m_cEndChannel = cEnd;}
	void CDataControler::SetSampleFrequency(const char cSampleFrequency){m_cSampleFrequency = cSampleFrequency;}
	void CDataControler::SetMode(const char cMode){m_cMode = cMode;}
	void CDataControler::SetArchiveFromat(char cFormat){m_cArchiveFormat = cFormat;}
	void CDataControler::SetUserName(const string strUserName){m_strUserName = strUserName;}
	void CDataControler::SetMaxAcceleratedVel(const float ff){m_fMaxAcceleratedVel = ff;}
	void CDataControler::SetBrakingLength(const float ff){m_fBrakingLength = ff;}
	void CDataControler::SetAverageVel(const float ff){m_fAverageVel = ff;}
	void CDataControler::SetGradient(const float ff){m_fGradient = ff;}
	void CDataControler::SetPedalDistance(const float ff){m_fPedalDistance = ff;}
	void CDataControler::SetMaxHandBrakeForce(const float ff){m_fMaxHandBrakeForce = ff;}
	void CDataControler::SetMaxFootBrakeForce(const float ff){m_fMaxFootBrakeForce = ff;}
	void CDataControler::SetResult(const char cResult){m_cResult = cResult;}
	void CDataControler::SetReportPath(const string strPath)
	{
		m_strReportPath = strPath;
	}

	void CDataControler::SetNewProjectPara(const char* pData)
	{
		//解析所有信息，保存到内存
		int nLoc = 2;
		char cTemp;
		memcpy(&cTemp,pData+nLoc,1);
		this->SetStartChannel(cTemp);
		++nLoc;
		memcpy(&cTemp,pData+nLoc,1);
		this->SetEndChannel(cTemp);
		++nLoc;
		memcpy(&cTemp,pData+nLoc,1);
		this->SetSampleFrequency(cTemp);
		++nLoc;
		memcpy(&cTemp,pData+nLoc,1);
		this->SetMode(cTemp);
		++nLoc;
		memcpy(&cTemp,pData+nLoc,1);
		this->SetArchiveFromat(cTemp);
		++nLoc;
		int nInfo = 0;
		memcpy(&nInfo,pData+nLoc,1);

		string strPath = "";
		if (NUM_ZERO != nInfo)
		{
			++nLoc;
			char* pbuf = new char[nInfo+1];
			memset(pbuf,0,nInfo+1);
			memcpy(pbuf,pData+nLoc,nInfo);
			strPath = pbuf;
			delete[] pbuf;
			pbuf = NULL;
			if( !CheckFolderExist(strPath) )
			{
				g_logger.TraceWarning("Passed in New Project Folder not right,created new one.");
				strPath = "";
			}
		}
		if (strPath == "")
		{
			this->CreateDefaultProjectPath(strPath);
		}
		this->SetProjectPath(strPath);
	}

	//return = true,strInfo是工作目录
	//return = false, strInfo是错误信息
	bool CDataControler::NewProject(string& strInfo)
	{
		//检查是否开始，如果开始，返回新建错误信息；如果没有开始，设置标志位，
		if (NUM_ONE == m_nCurrentProjectState)
		{
			strInfo = string("检测正在进行中。");
			return false;
		}
		//取得检测模式，设置DAQ线程中的标志位
		char cMode = theApp.m_pDataC->GetMode();

		//解析工作目录，新建工作目录，保存检测信息到文件
		SaveProjectInfo2File();

		theApp.m_pDAQC->NewProject(m_cMode);

		//返回新建成功与否信息
		strInfo = m_strProjectPath;

		m_nCurrentProjectState = NUM_ONE;
		return true;
	}
	bool CDataControler::TerminateCurrentProject()
	{
		if(NUM_ZERO == m_nCurrentProjectState)
		{
			g_logger.TraceError("CDataControler::TerminateCurrentProject - current project have not started.");
			return false;
		}
		if(NUM_TWO == m_nCurrentProjectState)
		{
			g_logger.TraceError("CDataControler::TerminateCurrentProject - current project already terminated.");
			return false;
		}
		//设置DAQ标志结束
		theApp.m_pDAQC->TerminateProject();

		m_nCurrentProjectState = NUM_TWO;
		return true;
	}


	char CDataControler::GetMode() const
	{
		return m_cMode;
	}

	void CDataControler::SaveProjectInfo2File()
	{
		string strConfigName("config.ini");
		string strAppName("ProjectInfo");
		string strConfigFullName = m_strProjectPath+"\\"+strConfigName;
		string strTemp;
		char buf[40] = {0};
		
		sprintf_s(buf,"%d",m_cStartChannel);
		WritePrivateProfileStringA(strAppName.c_str(),"StartChannel",buf,strConfigFullName.c_str());
		memset(buf,0,40);
		sprintf_s(buf,"%d",m_cEndChannel);
		WritePrivateProfileStringA(strAppName.c_str(),"EndChannel",buf,strConfigFullName.c_str());
		memset(buf,0,40);
		sprintf_s(buf,"%d KHz",m_cSampleFrequency);
		WritePrivateProfileStringA(strAppName.c_str(),"SampleFrequency",buf,strConfigFullName.c_str());
		memset(buf,0,40);
		switch(m_cMode)
		{
		case 0x01:
			sprintf_s(buf,"测试模式");
			break;
		case 0x02:
			sprintf_s(buf,"仅速度模式");
			break;
		case 0x03:
			sprintf_s(buf,"仅力角模式");
			break;
		case 0x04:
			sprintf_s(buf,"完整检测模式");
			break;
		}
		WritePrivateProfileStringA(strAppName.c_str(),"Mode",buf,strConfigFullName.c_str());
		memset(buf,0,40);
		switch(m_cArchiveFormat)
		{
		case 0x01:
			sprintf_s(buf,"word");
			break;
		case 0x02:
			sprintf_s(buf,"excel");
			break;
		case 0x03:
			sprintf_s(buf,"txt");
			break;
		}
		WritePrivateProfileStringA(strAppName.c_str(),"ArchiveFormat",buf,strConfigFullName.c_str());		
	}

	void CDataControler::SetProjectPath(string& strPath)
	{
		if ( strPath.at(strPath.length()-1) != '\\' )
		{
			strPath.append(1,'\\');
		}
		m_strProjectPath = strPath;
	}

	bool CDataControler::GetProjectPath(string& strPath) const
	{
		if (m_strProjectPath == "")
		{
			g_logger.TraceError("CDataControler::GetProjectPath - m_strProjectPath is NULL");
		}
		strPath = m_strProjectPath ;
		return true;
	}

	void CDataControler::CreateDefaultProjectPath(string &strPath)
	{
		//make a file path
		struct tm *local;
		time_t t;
		t = time(NULL);
		local = localtime(&t);
		char stFilePath[MAX_PATH] = {0};
		sprintf_s(stFilePath,"D:\\Data\\%04d%02d%02d_%02d%02d%02d\\",
			local->tm_year+1900,
			local->tm_mon+1,
			local->tm_mday,
			local->tm_hour,
			local->tm_min,
			local->tm_sec
			);
		MakeSureDirectoryPathExists(stFilePath);
		strPath = stFilePath;
	}

	//得到速度和加速度，加速度近似取最后一个，速度取本组数据最后点处的速度
	//V = sum(ai*t) 
	void CDataControler::HandleVelocityData(const double* pData, const int channelCount, const int sectionLength, const double deltat)
	{
		double dSumA = 0.0;
		double dCompoundA = 0.0;
		for (int i=0;i<sectionLength;++i)
		{
			dCompoundA = sqrt((*(pData+(i*channelCount)+7))*(*(pData+(i*channelCount)+7))+(*(pData+(i*channelCount)+8))*(*(pData+(i*channelCount)+8))+(*(pData+(i*channelCount)+9))*(*(pData+(i*channelCount)+9)));
			dSumA += dCompoundA;
		}
		m_stVelocityInfo.LastAccelaration = dCompoundA;
		m_stVelocityInfo.LastVelocity = dSumA * deltat;
	}
	//得到最大手刹力和脚刹力，坡度和脚刹位置近似取第一个值
	void CDataControler::HandleStressData(const double* pData, const int channelCount, const int sectionLength)
	{
		m_stStressInfo.MaxFootBrakeForce = *(pData+0) - *(pData+1);
		m_stStressInfo.Gradient = *(pData+2);
		m_stStressInfo.MaxHandBrakeForce = *(pData+4) - *(pData+5);
		m_stStressInfo.PedalDistance = *(pData+6);
		STRESSINFO stStressInfo;
		for (int i=0;i<sectionLength;++i)
		{
			stStressInfo.MaxFootBrakeForce = *(pData+(i*channelCount)) - *(pData+(i*channelCount)+1);
			stStressInfo.MaxHandBrakeForce = *(pData+(i*channelCount)+4) - *(pData+(i*channelCount)+5);
			if (m_stStressInfo.MaxFootBrakeForce < stStressInfo.MaxFootBrakeForce)
			{
				m_stStressInfo.MaxFootBrakeForce = stStressInfo.MaxFootBrakeForce;
			}
			if (m_stStressInfo.MaxHandBrakeForce< stStressInfo.MaxHandBrakeForce)
			{
				m_stStressInfo.MaxHandBrakeForce = stStressInfo.MaxHandBrakeForce;
			}
		}

	}
	void CDataControler::GetStressInfo(STRESSINFO& stStressInfo)
	{
		stStressInfo = m_stStressInfo;
	}
	void CDataControler::GetVelocityInfo(VELOCITYINFO& stVelocityInfo)
	{
		stVelocityInfo = m_stVelocityInfo;
	}


}
