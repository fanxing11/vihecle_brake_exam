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
		:m_hEvtStressInfo(NULL)
		,m_hEvtVelocityInfo(NULL)
		,m_nCurrentProjectState(NUM_ZERO)
		,m_dInitYAngle(0.0)
		,m_dInitXAngle(0.0)
		,m_strConfigFullName("")
	{
		m_hEvtStressInfo = CreateEvent(NULL,TRUE,FALSE,L"");
		m_hEvtVelocityInfo = CreateEvent(NULL,TRUE,FALSE,L"");
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
		SaveProjectInfo2INIFile();

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

	void CDataControler::SaveProjectInfo2INIFile()
	{
		m_strConfigFullName = m_strProjectPath + gc_strProjectParaINI_FileName;

		char buf[40] = {0};
		sprintf_s(buf,"%d",m_cStartChannel);
		WritePrivateProfileStringA(gc_strProjectInfo.c_str(),"StartChannel",buf,m_strConfigFullName.c_str());
		memset(buf,0,40);
		sprintf_s(buf,"%d",m_cEndChannel);
		WritePrivateProfileStringA(gc_strProjectInfo.c_str(),"EndChannel",buf,m_strConfigFullName.c_str());
		memset(buf,0,40);
		sprintf_s(buf,"%d KHz",m_cSampleFrequency);
		WritePrivateProfileStringA(gc_strProjectInfo.c_str(),"SampleFrequency",buf,m_strConfigFullName.c_str());
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
		WritePrivateProfileStringA(gc_strProjectInfo.c_str(),"Mode",buf,m_strConfigFullName.c_str());
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
		WritePrivateProfileStringA(gc_strProjectInfo.c_str(),"ArchiveFormat",buf,m_strConfigFullName.c_str());		
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
		ResetEvent(m_hEvtVelocityInfo);
		double dSumA = 0.0;
		double dCompoundA = 0.0;
		for (int i=0;i<sectionLength;++i)
		{
			dCompoundA = sqrt((*(pData+(i*channelCount)+7))*(*(pData+(i*channelCount)+7))+(*(pData+(i*channelCount)+8))*(*(pData+(i*channelCount)+8))+(*(pData+(i*channelCount)+9))*(*(pData+(i*channelCount)+9)));
			dSumA += dCompoundA;
		}
		m_stVelocityInfo.LastAccelaration = dCompoundA;
		m_stVelocityInfo.LastVelocity = dSumA * deltat;

		TransformAcceleration(m_stVelocityInfo.LastAccelaration);
		TransformVelocity(m_stVelocityInfo.LastVelocity);

		SetEvent(m_hEvtVelocityInfo);
	}
	//得到最大手刹力和脚刹力，坡度和脚刹位置近似取第一个值
	void CDataControler::HandleStressData(const double* pData, const int channelCount, const int sectionLength)
	{
		ResetEvent(m_hEvtStressInfo);
		m_stStressInfo.MaxFootBrakeForce = *(pData+0) - *(pData+1);
		m_stStressInfo.Gradient = *(pData+2);//暂时使用一个方向的角度
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

		TransformFootBrakeForce(m_stStressInfo.MaxFootBrakeForce);
		TransformHandBrakeForce(m_stStressInfo.MaxHandBrakeForce);
		TransformGradient(m_stStressInfo.Gradient);//暂时使用一个方向的角度
		TransformPedalDistance(m_stStressInfo.PedalDistance);

		SetEvent(m_hEvtStressInfo);
	}
	void CDataControler::SaveInitAngle2INIFile(const double* pData)
	{
		double dX = *(pData+2);
		double dY = *(pData+3);
		this->SetInitXAngle(dX);
		this->SetInitYAngle(dY);
	}

	void CDataControler::GetStressInfo(STRESSINFO& stStressInfo)
	{
		DWORD dwRet = WaitForSingleObject(m_hEvtStressInfo,1);
		if (WAIT_OBJECT_0 == dwRet)
		{
			stStressInfo = m_stStressInfo;
		}
		else
		{
			g_logger.TraceError("CDataControler::GetStressInfo - %d",dwRet);
		}
	}
	void CDataControler::GetVelocityInfo(VELOCITYINFO& stVelocityInfo)
	{
		DWORD dwRet = WaitForSingleObject(m_hEvtVelocityInfo,1);
		if (WAIT_OBJECT_0 == dwRet)
		{
			stVelocityInfo = m_stVelocityInfo;
		}
		else
		{
			g_logger.TraceError("CDataControler::GetVelocityInfo - %d",dwRet);
		}
	}

	void CDataControler::SetInitXAngle(const double dA)
	{
		m_dInitXAngle = dA;
		TransformGradient(m_dInitXAngle);

		char buf[40] = {0};
		sprintf_s(buf,"%f",m_dInitXAngle);
		if(!WritePrivateProfileStringA(gc_strInitialAngle.c_str(),gc_strInitXAngle.c_str(),buf,m_strConfigFullName.c_str()) )
		{
			g_logger.TraceError("CDataControler::SetInitXAngle - %d",GetLastError());
		}
	}
	double CDataControler::GetInitXAngle() const
	{
		return m_dInitXAngle;
	}
	void CDataControler::SetInitYAngle(const double dA)
	{
		m_dInitYAngle = dA;
		TransformGradient(m_dInitYAngle);

		char buf[40] = {0};
		sprintf_s(buf,"%f",m_dInitYAngle);
		if( !WritePrivateProfileStringA(gc_strInitialAngle.c_str(),gc_strInitYAngle.c_str(),buf,m_strConfigFullName.c_str()) )
		{
			g_logger.TraceError("CDataControler::SetInitYAngle - %d",GetLastError());
		}
	}
	double CDataControler::GetInitYAngle() const
	{
		return m_dInitYAngle;
	}

	bool CDataControler::TransformVelocity(double & dVel)
	{//nothing - DO NOT calc repeatedly!
		dVel = dVel/0.04;
		return true;
	}
	bool CDataControler::TransformAcceleration(double & dAcc)
	{
		dAcc  = dAcc/0.04;
		return true;
	}
	bool CDataControler::TransformFootBrakeForce(double &dForce)
	{
		dForce = (dForce-0.095)*245.0259728;
		return true;
	}
	bool CDataControler::TransformHandBrakeForce(double &dForce)
	{
		dForce = dForce * 4166.6666;
		return true;
	}
	bool CDataControler::TransformGradient(double &dGradient)
	{
		dGradient = (dGradient-2.59)*0.08333;
		return true;
	}
	bool CDataControler::TransformPedalDistance(double &dDist)
	{
		dDist = 1 / (dDist-0.44) * 0.1026856240126 + 1/30;
		return true;
	}

}
