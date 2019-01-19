#include "main.h"

#include "DataControler.h"
#include <math.h>
#include "time.h"

#include <Dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")

#include <sstream>

#include "Filter.h"


extern CtheApp* theApp;


namespace DATACONTROLER
{

	CDataControler::CDataControler(void)
		:m_hEvtMoveDetectionInfo(NULL)
		,m_hEvtStillDetectionInfo(NULL)
		,m_hEvtInitGradientInfo(NULL)
		,m_nCurrentProjectState(NUM_ZERO)
		,m_dInitYAngle(0.0)
		,m_dInitXAngle(0.0)
		,m_dInitCarXAngle(0.0)
		,m_dInitCarYAngle(0.0)
		,m_strConfigFullName("")
		,m_nCurrentType(NONTYPE)
		,m_bInitAngleFlag(false)
		,m_bInitHandForceFlag(false)
		,m_dIsWireless(-1)
		,m_dValidFootBrakeForce(-1)
		,m_dFootBrakePara1(0.0)
		,m_dFootBrakePara2(0.0)
		,m_dHandBrakePara1(0.0)
		,m_dHandBrakePara2(0.0)
		,m_dAnglePara1(0.0)
		,m_dAnglePara2(0.0)
		,m_dPedalDistance1(0.0)
		//,m_dPedalDistance2(0.0)
		//,m_dPedalDistance3(0.0)
		//,m_dPedalDistance4(0.0)
		//,m_dPedalDistance5(0.0)
		,m_dAccelaration1(0.0)
		,m_dInitHandForce(0.0)
		,m_dInitFootForce(0.0)
		,m_dInitAccA(0.0)
		,m_dInitAccB(0.0)
		,m_dInitAccC(0.0)
		,m_dMaxHandBrakeForce(0.0)
		,m_dMaxPedalDistance(0.0)
		,m_bGetInitPedalDist(false)
		,m_dInitPedalDist(0.0)
		,m_bGetInitFootBrakeForce(false)
	{
		g_logger.TraceInfo("CDataControler::CDataControler");
		m_hEvtMoveDetectionInfo = CreateEvent(NULL,TRUE,FALSE,L"");
		m_hEvtStillDetectionInfo = CreateEvent(NULL,TRUE,FALSE,L"");
		m_hEvtInitGradientInfo = CreateEvent(NULL,TRUE,FALSE,L"");

		if(!ReadSensorConfigFromINI())
		{
			g_logger.TraceError("CDataControler::CDataControler-error");
		}
	}

	CDataControler::~CDataControler(void)
	{
		g_logger.TraceWarning("CDataControler::~CDataControler");
	}

	void CDataControler::SetGetInitPedalDist()
	{
		m_bGetInitPedalDist = true;
	}

	void CDataControler::SetGetInitFootBrakeForce()
	{
		m_bGetInitFootBrakeForce = true;
	}

	int CDataControler::GetCurrentProjectState()const
	{
		return m_nCurrentProjectState;
	}
	bool CDataControler::DAQIsWirelessType()
	{
		g_logger.TraceInfo("CDataControler::DAQIsWirelessType:-in.");

		bool ret = false;
		if (m_dIsWireless == 1.0)
		{
			ret = true;
		}
		else if (m_dIsWireless == 0.0)
		{
			ret = false;
		}
		else
		{
			g_logger.TraceError("CDataControler::DAQIsWirelessType - DAQ type error");
		}
		return ret;
	}
	double CDataControler::GetValidFootBrakeForce() const
	{
		g_logger.TraceInfo("CDataControler::GetValidFootBrakeForce- %.1f",m_dValidFootBrakeForce);

		//dForce = (dForce + m_dFootBrakePara1)*m_dFootBrakePara2;
		double dForce = 0.0;
		if (m_dFootBrakePara2>0.0001)
		{
			dForce = m_dValidFootBrakeForce/m_dFootBrakePara2-m_dFootBrakePara1;
		}
		else
		{
			g_logger.TraceError("CDataControler::GetValidFootBrakeForce- invalid parameter-ValidFootBrakeForce");
		}

		return dForce;
	}

	bool CDataControler::ReadSensorConfigFromINI()
	{
		ReadOneParaFromConfigINI(gc_strDAQType,gc_strIsWireless,m_dIsWireless);
		//v1.9.6
		ReadOneParaFromConfigINI(gc_strANALYSIS,gc_strValidFootBrakeForce,m_dValidFootBrakeForce);

		if (DAQIsWirelessType())
		{
			ReadOneParaFromConfigINI(gc_strParaFootBrakeForceW,gc_strPara1,m_dFootBrakePara1);
			ReadOneParaFromConfigINI(gc_strParaFootBrakeForceW,gc_strPara2,m_dFootBrakePara2);
			ReadOneParaFromConfigINI(gc_strParaHandBrakeForceW,gc_strPara1,m_dHandBrakePara1);
			ReadOneParaFromConfigINI(gc_strParaHandBrakeForceW,gc_strPara2,m_dHandBrakePara2);
			ReadOneParaFromConfigINI(gc_strParaXYAngleW,gc_strPara1,m_dAnglePara1);
			ReadOneParaFromConfigINI(gc_strParaXYAngleW,gc_strPara2,m_dAnglePara2);
			ReadOneParaFromConfigINI(gc_strParaPedalDistanceW,gc_strPara1,m_dPedalDistance1);
			//ReadOneParaFromConfigINI(gc_strParaPedalDistanceW,gc_strPara2,m_dPedalDistance2);
			//ReadOneParaFromConfigINI(gc_strParaPedalDistanceW,gc_strPara3,m_dPedalDistance3);
			//ReadOneParaFromConfigINI(gc_strParaPedalDistanceW,gc_strPara4,m_dPedalDistance4);
			//ReadOneParaFromConfigINI(gc_strParaPedalDistanceW,gc_strPara5,m_dPedalDistance5);
			ReadOneParaFromConfigINI(gc_strParaAccelarationW,gc_strPara1,m_dAccelaration1);
		}
		else
		{
			ReadOneParaFromConfigINI(gc_strParaFootBrakeForce,gc_strPara1,m_dFootBrakePara1);
			ReadOneParaFromConfigINI(gc_strParaFootBrakeForce,gc_strPara2,m_dFootBrakePara2);
			ReadOneParaFromConfigINI(gc_strParaHandBrakeForce,gc_strPara1,m_dHandBrakePara1);
			ReadOneParaFromConfigINI(gc_strParaHandBrakeForce,gc_strPara2,m_dHandBrakePara2);
			ReadOneParaFromConfigINI(gc_strParaXYAngle,gc_strPara1,m_dAnglePara1);
			ReadOneParaFromConfigINI(gc_strParaXYAngle,gc_strPara2,m_dAnglePara2);
			ReadOneParaFromConfigINI(gc_strParaPedalDistance,gc_strPara1,m_dPedalDistance1);
			//ReadOneParaFromConfigINI(gc_strParaPedalDistance,gc_strPara2,m_dPedalDistance2);
			//ReadOneParaFromConfigINI(gc_strParaPedalDistance,gc_strPara3,m_dPedalDistance3);
			//ReadOneParaFromConfigINI(gc_strParaPedalDistance,gc_strPara4,m_dPedalDistance4);
			//ReadOneParaFromConfigINI(gc_strParaPedalDistance,gc_strPara5,m_dPedalDistance5);
			ReadOneParaFromConfigINI(gc_strParaAccelaration,gc_strPara1,m_dAccelaration1);
		}
		return true;
	}
	bool CDataControler::ReadOneParaFromConfigINI(const string SectionName, 
											const string ParaName,
											double & dPara)
	{
		char stPara[50]={0};  
		DWORD dwRet3;
		dwRet3 = GetPrivateProfileStringA(SectionName.c_str(), ParaName.c_str(), "", stPara, 50, gc_strSensorConfig_FileName.c_str());  
		if ( dwRet3 <= 0 )
		{
			g_logger.TraceError("CDataControler::ReadOneParaFromINI failed- %s,%s",SectionName.c_str(),ParaName.c_str() );
			return false;
		}
		std::stringstream stream;
		stream.clear();
		stream<<stPara;
		stream>>dPara;

		//dPara = atof(stPara);

		return true;
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
	void CDataControler::SetCurrentType(enDETECTION_TYPE dt)
	{
		m_nCurrentType = dt;
	}
	void CDataControler::SetInitHandForceFlag()
	{
		g_logger.TraceInfo("CDataControler::SetInitHandForceFlag");
		m_bInitHandForceFlag = false;//恢复状态，以便下次测试
	}
	void CDataControler::SetInitAngleFlag()
	{
		g_logger.TraceInfo("CDataControler::SetInitAngleFlag");
		m_bInitAngleFlag = false;//恢复状态，以便下次测试
	}
	bool CDataControler::SaveMaxHandBrakeForce2INI()
	{
		//这里只保存了最大手刹力，所以结果分析中除此之外的参数都是运动中得到的
		//包括地面倾角,亦即,静止检测中的地面倾角被没有作用，只在测量时展示了一下
		char buf[40] = {0};
		sprintf_s(buf,"%f",m_dMaxHandBrakeForce);
		if (NUM_ZERO == WritePrivateProfileStringA(gc_strResult.c_str(),gc_strMaxHandBrakeForce.c_str(),buf,m_strConfigFullName.c_str()))
		{
			g_logger.TraceError("CDataControler::SaveMaxHandBrakeForce2INI: WritePrivateProfileStringA,GetLastError=%d",
				GetLastError());
			return false;
		}
		return true;
	}
	bool CDataControler::SaveMaxPedalDistance2INI()
	{
		//这里只保存了最大手刹力，所以结果分析中除此之外的参数都是运动中得到的
		//包括地面倾角,亦即,静止检测中的地面倾角被没有作用，只在测量时展示了一下
		char buf[40] = {0};
		sprintf_s(buf,"%f",m_dMaxPedalDistance);
		if (NUM_ZERO == WritePrivateProfileStringA(gc_strResult.c_str(),gc_strMaxPedalDistance.c_str(),buf,m_strConfigFullName.c_str()))
		{
			g_logger.TraceError("CDataControler::SaveMaxPedalDistance2INI: WritePrivateProfileStringA,GetLastError=%d",
				GetLastError());
			return false;
		}
		return true;
	}
	bool CDataControler::SaveInitValue2INI()
	{
		//需求增加：需要保存一下量初始值。这些值在实时检测、历史分析中都要用到。
		char buf[40] = {0};
		sprintf_s(buf,"%.6f",m_dInitAccA);
		bool bSuccess = true;
		if (NUM_ZERO == WritePrivateProfileStringA(gc_strInitValue.c_str(),gc_strInitAccA.c_str(),buf,m_strConfigFullName.c_str()))
		{ bSuccess = false;}
		memset(buf,0,40);
		sprintf_s(buf,"%.6f",m_dInitAccB);
		if (NUM_ZERO == WritePrivateProfileStringA(gc_strInitValue.c_str(),gc_strInitAccB.c_str(),buf,m_strConfigFullName.c_str()))
		{ bSuccess = false;}
		memset(buf,0,40);
		sprintf_s(buf,"%.6f",m_dInitAccC);
		if (NUM_ZERO == WritePrivateProfileStringA(gc_strInitValue.c_str(),gc_strInitAccC.c_str(),buf,m_strConfigFullName.c_str()))
		{ bSuccess = false;}
		memset(buf,0,40);
		sprintf_s(buf,"%.6f",m_dInitFootForce);
		if (NUM_ZERO == WritePrivateProfileStringA(gc_strInitValue.c_str(),gc_strInitFootBrakeForce.c_str(),buf,m_strConfigFullName.c_str()))
		{ bSuccess = false;}
		memset(buf,0,40);
		sprintf_s(buf,"%.6f",m_dInitHandForce);
		if (NUM_ZERO == WritePrivateProfileStringA(gc_strInitValue.c_str(),gc_strInitHandBrakeForce.c_str(),buf,m_strConfigFullName.c_str()))
		{ bSuccess = false;}
		memset(buf,0,40);//这个已经不需要存了.
		sprintf_s(buf,"%.6f",m_dInitPedalDist);
		if (NUM_ZERO == WritePrivateProfileStringA(gc_strInitValue.c_str(),gc_strInitPedalDistance.c_str(),buf,m_strConfigFullName.c_str()))
		{ bSuccess = false;}

		SaveCarAngle();

		if (!bSuccess)
		{
			g_logger.TraceError("CDataControler::SaveInitValue2INI: WritePrivateProfileStringA,GetLastError=%d",
				GetLastError());
		}
		else
		{
			g_logger.TraceInfo("CDataControler::SaveInitValue2INI success");
		}

		return bSuccess;
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
		char cMode = theApp->m_pDataController->GetMode();
		//设置新建标志到DAQ
		if(false == theApp->m_pDAQController->NewProject(m_cMode))
		{
			strInfo = string("采集卡初始化失败。");
			return false;
		}

		if(false == SaveProjectInfo2INIFile())
		{
			strInfo = string("检测参数保存失败。");
			return false;
		}

		//新建成功，返回新建成功与否信息
		strInfo = m_strProjectPath;

		m_nCurrentProjectState = NUM_ONE;
		//init state
		m_stMoveDetectionInfo.GradientX = DOUBLE_ZERO;
		m_stMoveDetectionInfo.GradientY = DOUBLE_ZERO;
		m_stMoveDetectionInfo.LastAccelaration = DOUBLE_ZERO;
		m_stMoveDetectionInfo.LastVelocity=0.0;
		m_stMoveDetectionInfo.MaxFootBrakeForce=0.0;
		m_stStillDetectionInfo.PedalDistance=0.0;
		m_stStillDetectionInfo.GradientX = DOUBLE_ZERO;
		m_stStillDetectionInfo.GradientY = DOUBLE_ZERO;
		m_stStillDetectionInfo.MaxHandBrakeForce=0.0;
		m_dMaxHandBrakeForce = DOUBLE_ZERO;
		m_dMaxPedalDistance = DOUBLE_ZERO;

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
		theApp->m_pDAQController->TerminateProject();

		m_nCurrentProjectState = NUM_TWO;
		return true;
	}


	char CDataControler::GetMode() const
	{
		return m_cMode;
	}

	//解析工作目录，新建工作目录，保存检测信息到文件
	bool CDataControler::SaveProjectInfo2INIFile()
	{
		bool bRet=true;
		m_strConfigFullName = m_strProjectPath + gc_strProjectParaINI_FileName;

		char buf[40] = {0};
		sprintf_s(buf,"%d",m_cStartChannel);
		if (NUM_ZERO == WritePrivateProfileStringA(gc_strProjectInfo.c_str(),"StartChannel",buf,m_strConfigFullName.c_str()))
		{
			g_logger.TraceError("CDataControler::SaveProjectInfo2INIFile: WritePrivateProfileStringA,GetLastError=%d",
				GetLastError());
			bRet = false;
		}

		memset(buf,0,40);
		sprintf_s(buf,"%d",m_cEndChannel);
		if( NUM_ZERO == WritePrivateProfileStringA(gc_strProjectInfo.c_str(),"EndChannel",buf,m_strConfigFullName.c_str()) )
		{
			g_logger.TraceError("CDataControler::SaveProjectInfo2INIFile: WritePrivateProfileStringA,GetLastError=%d",
				GetLastError());
			bRet = false;
		}

		memset(buf,0,40);
		sprintf_s(buf,"%d KHz",m_cSampleFrequency);
		if( NUM_ZERO == WritePrivateProfileStringA(gc_strProjectInfo.c_str(),"SampleFrequency",buf,m_strConfigFullName.c_str()) )
		{
			g_logger.TraceError("CDataControler::SaveProjectInfo2INIFile: WritePrivateProfileStringA,GetLastError=%d",
				GetLastError());
			bRet = false;
		}

		memset(buf,0,40);
		switch(m_cMode)
		{
		case 0x01:
			sprintf_s(buf,"测试模式");
			break;
		case 0x02:
			sprintf_s(buf,"完整检测模式");
			break;
		default:
			{
				g_logger.TraceError("CDataControler::SaveProjectInfo2INIFile error: Mode=%d",(int)m_cMode);
				bRet = false;
				break;
			}
		}
		if( NUM_ZERO == WritePrivateProfileStringA(gc_strProjectInfo.c_str(),"Mode",buf,m_strConfigFullName.c_str()))
		{
			g_logger.TraceError("CDataControler::SaveProjectInfo2INIFile: WritePrivateProfileStringA,GetLastError=%d",
				GetLastError());
			bRet = false;
		}

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
		if( NUM_ZERO == WritePrivateProfileStringA(gc_strProjectInfo.c_str(),"ArchiveFormat",buf,m_strConfigFullName.c_str()))
		{
			g_logger.TraceError("CDataControler::SaveProjectInfo2INIFile: WritePrivateProfileStringA,GetLastError=%d",
				GetLastError());
			bRet = false;
		}

		return bRet;
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
		if (IsDiskExist('D'))//v1.9.6
		{
			sprintf_s(stFilePath,"D:\\CJZD_Data\\%04d%02d%02d_%02d%02d%02d\\",
				local->tm_year+1900,
				local->tm_mon+1,
				local->tm_mday,
				local->tm_hour,
				local->tm_min,
				local->tm_sec
				);
		}
		else
		{
			sprintf_s(stFilePath,"C:\\CJZD_Data\\%04d%02d%02d_%02d%02d%02d\\",
				local->tm_year+1900,
				local->tm_mon+1,
				local->tm_mday,
				local->tm_hour,
				local->tm_min,
				local->tm_sec
				);
		}

		MakeSureDirectoryPathExists(stFilePath);
		strPath = stFilePath;
	}
	void CDataControler::SaveCarAngle()
	{

		if (!m_bInitAngleFlag)//v1.9.6.3
		{
			m_dInitCarXAngle = m_stStillDetectionInfo.GradientX ;
			m_dInitCarYAngle = m_stStillDetectionInfo.GradientY ;
			m_bInitAngleFlag = true;
		}
		////第一次静止检测时，把倾角减去地面倾角作为车辆的倾角，并保存到ini中
		//m_dInitCarXAngle = m_stStillDetectionInfo.GradientX - m_dInitXAngle;
		//m_dInitCarYAngle = m_stStillDetectionInfo.GradientY - m_dInitYAngle;

		char bufx[40] = {0};
		sprintf_s(bufx,"%f",m_dInitCarXAngle);
		g_logger.TraceInfo("CDataControler::SaveCarAngle m_dInitCarXAngle=%.2f",m_dInitCarXAngle);
		if(!WritePrivateProfileStringA(gc_strInitialCarAngle.c_str(),gc_strInitXAngle.c_str(),bufx,m_strConfigFullName.c_str()) )
		{
			g_logger.TraceError("CDataControler::SaveCarAngleX - %d",GetLastError());
		}

		char bufy[40] = {0};
		sprintf_s(bufy,"%f",m_dInitCarYAngle);
		if( !WritePrivateProfileStringA(gc_strInitialCarAngle.c_str(),gc_strInitYAngle.c_str(),bufy,m_strConfigFullName.c_str()) )
		{
			g_logger.TraceError("CDataControler::SaveCarAngleY - %d",GetLastError());
		}

	}

	//在获取初始倾角时获得加速度、力的初始值；
	//后面采集时所有的值减去对应值
	//以去掉重力加速度的及其他的影响
	void CDataControler::GetInitValue(const double* pData, const int channelCount, const int sectionLength)
	{
		Filter FilterAccA;
		Filter FilterAccB;
		Filter FilterAccC;
		Filter FilterHandForce;
		//Filter FilterFootForce;

		for (int i=0;i<sectionLength;++i)
		{
			FilterAccA.AddData(*(pData+(i*channelCount)+7));
			FilterAccA.AddData(*(pData+(i*channelCount)+8));
			FilterAccA.AddData(*(pData+(i*channelCount)+9));
			FilterHandForce.AddData(*(pData+(i*channelCount)+4) - *(pData+(i*channelCount)+5));
			//FilterFootForce.AddData(*(pData+(i*channelCount)) - *(pData+(i*channelCount)+1));
		}
		m_dInitAccA = FilterAccA.GetMeanData();
		m_dInitAccB = FilterAccB.GetMeanData();
		m_dInitAccC = FilterAccC.GetMeanData();
		m_dInitHandForce = FilterHandForce.GetMeanData();
		//m_dInitFootForce = FilterFootForce.GetMeanData();
	}
	//处理其他初始值
	void CDataControler::GetInitValueW(const double* pData, const int channelCount, const int sectionLength)
	{
		Filter FilterAccA;
		Filter FilterAccB;
		Filter FilterAccC;
		Filter FilterHandForce;


		static int nMid = stnMidCountInit;
		for (int i=0;i<sectionLength;++i)
		{
			FilterAccA.AddData(*(pData+i));
			FilterAccB.AddData(*(pData+2*sectionLength+i));
			FilterAccC.AddData(*(pData+3*sectionLength+i));
			if (--nMid == 0)
			{
				nMid = stnMidCountInit;
				FilterHandForce.AddData1(*(pData+6*sectionLength+i));
			}
		}
		m_dInitAccA = FilterAccA.GetMeanData();
		m_dInitAccB = FilterAccB.GetMeanData();
		m_dInitAccC = FilterAccC.GetMeanData();
		if (!m_bInitHandForceFlag)
		{
			m_dInitHandForce = FilterHandForce.GetMidValue();
			m_bInitHandForceFlag = true;
		}
	}

	//最大手刹力、XY倾角
	//V = sum(ai*t) 
	void CDataControler::HandleStillDetectionData(const double* pData, const int channelCount, const int sectionLength)
	{
		ResetEvent(m_hEvtStillDetectionInfo);
		//m_stStillDetectionInfo.MaxHandBrakeForce = *(pData+4) - *(pData+5);
		//m_stStillDetectionInfo.GradientX = *(pData+2);
		//m_stStillDetectionInfo.GradientY = *(pData+3);
		//TransformGradient(m_stStillDetectionInfo.GradientX);
		//TransformGradient(m_stStillDetectionInfo.GradientY);

		//STILLDETECTIONINFO stStillDetectionInfo;
		//for (int i=0;i<sectionLength;++i)
		//{
		//	stStillDetectionInfo.MaxHandBrakeForce = *(pData+(i*channelCount)+4) - *(pData+(i*channelCount)+5);
		//	stStillDetectionInfo.GradientX = *(pData+((i*channelCount))+2);
		//	stStillDetectionInfo.GradientY = *(pData+((i*channelCount))+3);
		//	TransformGradient(stStillDetectionInfo.GradientX);
		//	TransformGradient(stStillDetectionInfo.GradientY);
		//	if (m_stStillDetectionInfo.MaxHandBrakeForce < stStillDetectionInfo.MaxHandBrakeForce)
		//	{
		//		m_stStillDetectionInfo.MaxHandBrakeForce = stStillDetectionInfo.MaxHandBrakeForce;
		//	}
		//	if ( abs(m_stStillDetectionInfo.GradientX) < abs(stStillDetectionInfo.GradientX) )
		//	{
		//		m_stStillDetectionInfo.GradientX = stStillDetectionInfo.GradientX;
		//	}
		//	if ( abs(m_stStillDetectionInfo.GradientY) < abs(stStillDetectionInfo.GradientY) )
		//	{
		//		m_stStillDetectionInfo.GradientY = stStillDetectionInfo.GradientY;
		//	}

		//}
		//m_stStillDetectionInfo.MaxHandBrakeForce -= m_dInitHandForce;
		//TransformHandBrakeForce(m_stStillDetectionInfo.MaxHandBrakeForce);

		//if( m_dMaxHandBrakeForce<m_stStillDetectionInfo.MaxHandBrakeForce)//保存下静止时的最大手刹力
		//{
		//	m_dMaxHandBrakeForce = m_stStillDetectionInfo.MaxHandBrakeForce;
		//}

		////if(m_bUpdateCarAngleFlag)
		////{
		////	SaveCarAngle();
		////	m_bUpdateCarAngleFlag = false;
		////}
		SetEvent(m_hEvtStillDetectionInfo);
	}
	//最大脚刹力、脚刹位置、XY倾角、加速度近似取最后一个，速度取本组数据最后点处的速度
	//得到最大手刹力和脚刹力，坡度和脚刹位置近似取第一个值
	void CDataControler::HandleMoveDetectionData(const double* pData, const int channelCount, const int sectionLength, const double deltat)
	{
		ResetEvent(m_hEvtMoveDetectionInfo);
		//double dSumA = DOUBLE_ZERO;
		//double dCompoundA = DOUBLE_ZERO;

		//m_stMoveDetectionInfo.MaxFootBrakeForce = *(pData+0) - *(pData+1);
		//m_stMoveDetectionInfo.GradientX = *(pData+2);
		//m_stMoveDetectionInfo.GradientY = *(pData+3);
		//TransformGradient(m_stMoveDetectionInfo.GradientX);
		//TransformGradient(m_stMoveDetectionInfo.GradientY);
		//m_stMoveDetectionInfo.PedalDistance = *(pData+6);
		//Filter FilterFootBrakeForce;
		//Filter FilterGradientX ;
		//Filter FilterGradientY ;
		//Filter FilterPedalDistance ;
		//MOVEDETECTIONINFO stMoveDetectionInfo;
		//for (int i=0;i<sectionLength;++i)
		//{
		//	double dAccA = *(pData+(i*channelCount)+7) - m_dInitAccA;
		//	//v1.9 使用一个方向的加速度。不再进行合成。
		//	//double dAccB = *(pData+(i*channelCount)+8) - m_dInitAccB;
		//	//double dAccC = *(pData+(i*channelCount)+9) - m_dInitAccC;

		//	//dCompoundA = sqrt((dAccA)*(dAccA)+(dAccB)*(dAccB)+(dAccC)*(dAccC));
		//	dCompoundA = dAccA;
		//	dSumA += dCompoundA;

		//	stMoveDetectionInfo.GradientX = *(pData+((i*channelCount))+2);
		//	stMoveDetectionInfo.GradientY = *(pData+((i*channelCount))+3);
		//	TransformGradient(stMoveDetectionInfo.GradientX);
		//	TransformGradient(stMoveDetectionInfo.GradientY);
		//	////取一组数据中的max值代表这组数据
		//	//if ( abs(m_stMoveDetectionInfo.GradientX) < abs(stMoveDetectionInfo.GradientX) )
		//	//{
		//	//	m_stMoveDetectionInfo.GradientX = stMoveDetectionInfo.GradientX;
		//	//}
		//	//if ( abs(m_stMoveDetectionInfo.GradientY) < abs(stMoveDetectionInfo.GradientY) )
		//	//{
		//	//	m_stMoveDetectionInfo.GradientY = stMoveDetectionInfo.GradientY;
		//	//}
		//	stMoveDetectionInfo.MaxFootBrakeForce = *(pData+(i*channelCount)) - *(pData+(i*channelCount)+1);
		//	//if (m_stMoveDetectionInfo.MaxFootBrakeForce < stMoveDetectionInfo.MaxFootBrakeForce)
		//	//{
		//	//	m_stMoveDetectionInfo.MaxFootBrakeForce = stMoveDetectionInfo.MaxFootBrakeForce;
		//	//}
		//	stMoveDetectionInfo.PedalDistance = *(pData+(i*channelCount)+6);
		//	//if (m_stMoveDetectionInfo.PedalDistance < stMoveDetectionInfo.PedalDistance)
		//	//{
		//	//	m_stMoveDetectionInfo.PedalDistance = stMoveDetectionInfo.PedalDistance;
		//	//}
		//	//均值滤波
		//	FilterGradientX.AddData(stMoveDetectionInfo.GradientX);
		//	FilterGradientY.AddData(stMoveDetectionInfo.GradientY);
		//	FilterFootBrakeForce.AddData1(stMoveDetectionInfo.MaxFootBrakeForce);
		//	FilterPedalDistance.AddData(stMoveDetectionInfo.PedalDistance);
		//}
		//m_stMoveDetectionInfo.GradientX = FilterGradientX.GetMeanData();
		//m_stMoveDetectionInfo.GradientY = FilterGradientY.GetMeanData();
		//m_stMoveDetectionInfo.MaxFootBrakeForce = FilterFootBrakeForce.GetMidValue();
		//m_stMoveDetectionInfo.PedalDistance = FilterPedalDistance.GetMeanData();

		//m_stMoveDetectionInfo.LastAccelaration = dCompoundA;
		//m_stMoveDetectionInfo.LastVelocity = dSumA * deltat * sectionLength;
		//TransformAcceleration(m_stMoveDetectionInfo.LastAccelaration);
		//TransformVelocity(m_stMoveDetectionInfo.LastVelocity);

		//m_stMoveDetectionInfo.MaxFootBrakeForce -= m_dInitFootForce;
		//TransformFootBrakeForce(m_stMoveDetectionInfo.MaxFootBrakeForce);
		//TransformPedalDistance(m_stMoveDetectionInfo.PedalDistance);

		////GetInitXAngle 需要减去初始车辆倾角
		//m_stMoveDetectionInfo.GradientX = m_stMoveDetectionInfo.GradientX-m_dInitCarXAngle;
		//m_stMoveDetectionInfo.GradientY = m_stMoveDetectionInfo.GradientY-m_dInitCarYAngle;

		SetEvent(m_hEvtMoveDetectionInfo);
	}


	void CDataControler::HandleInitGradientData(const double* pData, const int channelCount, const int sectionLength)
	{
		ResetEvent(m_hEvtInitGradientInfo);
		double dMaxX = *(pData+2);
		double dMaxY = *(pData+3);
		TransformGradient(dMaxX);
		TransformGradient(dMaxY);
		double dX=0,dY=0;

		for (int i=0;i<sectionLength;++i)
		{
			dX = *(pData+(i*channelCount)) - *(pData+(i*channelCount)+2);
			dY = *(pData+(i*channelCount)+4) - *(pData+(i*channelCount)+3);
			TransformGradient(dX);
			TransformGradient(dY);//由于有abs，转换中有减法，可能改变正负号，所以需要变换后再比较
			if (abs(dMaxX)<abs(dX))
			{
				dMaxX = dX;
			}
			if ( abs(dMaxY)<abs(dY) )
			{
				dMaxY = dY;
			}
		}

		m_dInitXAngle = dMaxX;
		m_dInitYAngle = dMaxY;

		SetEvent(m_hEvtInitGradientInfo);
	}

	//最大脚刹力、脚刹位置、XY倾角、加速度近似取最后一个，速度取本组数据最后点处的速度
	//得到最大手刹力和脚刹力，坡度近似取第一个值
	void CDataControler::HandleMoveDetectionDataW(const double* pData, const int channelCount, const int sectionLength, const double deltat)
	{
		g_logger.TraceInfo("CDataControler::HandleMoveDetectionDataW");
		//static int snMinCount = stnMidCount;

		ResetEvent(m_hEvtMoveDetectionInfo);
		double dCurrentSectionAddV = DOUBLE_ZERO;
		double dCompoundA = DOUBLE_ZERO;
		double dAddVel = DOUBLE_ZERO;

		m_stMoveDetectionInfo.MaxFootBrakeForce = *(pData+7*sectionLength);
		m_stMoveDetectionInfo.GradientX = *(pData+1*sectionLength);
		m_stMoveDetectionInfo.GradientY = *(pData+4*sectionLength);
		TransformGradient(m_stMoveDetectionInfo.GradientX);
		TransformGradient(m_stMoveDetectionInfo.GradientY);
		Filter FilterFootBrakeForce;
		Filter FilterGradientX ;
		Filter FilterGradientY ;
		MOVEDETECTIONINFO stMoveDetectionInfo;
		for (int i=0;i<sectionLength;++i)
		{
			dCompoundA = *(pData+i);
			//v1.9 使用一个方向的加速度。不再进行合成。
			//double dAccB = *(pData+2*1024+i) - m_dInitAccB;
			//double dAccC = *(pData+3*1024+i) - m_dInitAccC;
			//dCompoundA = sqrt( dAccA*dAccA + dAccB*dAccB + dAccC*dAccC );
			dCompoundA -= m_dInitAccA;

			dAddVel = dCompoundA*deltat;
			dCurrentSectionAddV += dAddVel;


			stMoveDetectionInfo.GradientX = *(pData+1*sectionLength+i);
			stMoveDetectionInfo.GradientY = *(pData+4*sectionLength+i);
			TransformGradient(stMoveDetectionInfo.GradientX);
			TransformGradient(stMoveDetectionInfo.GradientY);
			stMoveDetectionInfo.MaxFootBrakeForce = *(pData+7*sectionLength+i);
			////取一组数据中的max值代表这组数据
			//if ( abs(m_stMoveDetectionInfo.GradientX) < abs(stMoveDetectionInfo.GradientX) )
			//{
			//	m_stMoveDetectionInfo.GradientX = stMoveDetectionInfo.GradientX;
			//}
			//if ( abs(m_stMoveDetectionInfo.GradientY) < abs(stMoveDetectionInfo.GradientY) )
			//{
			//	m_stMoveDetectionInfo.GradientY = stMoveDetectionInfo.GradientY;
			//}
			//stMoveDetectionInfo.MaxFootBrakeForce = *(pData+7*1024+i);
			//if (m_stMoveDetectionInfo.MaxFootBrakeForce < stMoveDetectionInfo.MaxFootBrakeForce)
			//{
			//	m_stMoveDetectionInfo.MaxFootBrakeForce = stMoveDetectionInfo.MaxFootBrakeForce;
			//}
			//stMoveDetectionInfo.PedalDistance = *(pData+5*1024+i);
			//if (m_stMoveDetectionInfo.PedalDistance < stMoveDetectionInfo.PedalDistance)
			//{
			//	m_stMoveDetectionInfo.PedalDistance = stMoveDetectionInfo.PedalDistance;
			//}

			////均值滤波
			//FilterGradientX.AddData(stMoveDetectionInfo.GradientX);
			//FilterGradientY.AddData(stMoveDetectionInfo.GradientY);
			if ( abs(m_stMoveDetectionInfo.GradientX) < abs(stMoveDetectionInfo.GradientX) )
			{
				m_stMoveDetectionInfo.GradientX = stMoveDetectionInfo.GradientX;
			}
			if ( abs(m_stMoveDetectionInfo.GradientY) < abs(stMoveDetectionInfo.GradientY) )
			{
				m_stMoveDetectionInfo.GradientY = stMoveDetectionInfo.GradientY;
			}
			//if (--snMinCount == 0)
			//{
			//	snMinCount = stnMidCount;
			if(i<300)//取前M个数，而不再隔N个数取一个，希望连续的能减少误差
			{
				FilterFootBrakeForce.AddData1(stMoveDetectionInfo.MaxFootBrakeForce);//显示时不再使用中值滤波，而是取均值
			}
		}
		//m_stMoveDetectionInfo.GradientX = FilterGradientX.GetMeanData();
		//m_stMoveDetectionInfo.GradientY = FilterGradientY.GetMeanData();
		//if (snMinCount == stnMidCount)
		{
			m_stMoveDetectionInfo.MaxFootBrakeForce = FilterFootBrakeForce.GetMidValue();
			if (m_bGetInitFootBrakeForce)
			{
				m_dInitFootForce = m_stMoveDetectionInfo.MaxFootBrakeForce;
				m_bGetInitFootBrakeForce = false;
			}
			FilterFootBrakeForce.ResetMid();
		}

		m_stMoveDetectionInfo.LastAccelaration = dCompoundA;
		//m_stMoveDetectionInfo.LastVelocity += dSumA * deltat * sectionLength;
		TransformAcceleration(m_stMoveDetectionInfo.LastAccelaration);

		TransformAcceleration(dCurrentSectionAddV);
		TransformVelocity(dCurrentSectionAddV);
		dCurrentSectionAddV /= 10; //tmp1.9.2.1
		m_stMoveDetectionInfo.LastVelocity += dCurrentSectionAddV;
		//if (m_stMoveDetectionInfo.LastVelocity > 10)
		//{
		//	MessageBeep(MB_OK);
		//}

		//g_logger.TraceWarning("CDataControler::HandleMoveDetectionDataW MaxFootBrakeForce- %f,%f",
		//	m_stMoveDetectionInfo.MaxFootBrakeForce,
		//	m_dInitFootForce);
		m_stMoveDetectionInfo.MaxFootBrakeForce -= m_dInitFootForce;
		TransformFootBrakeForce(m_stMoveDetectionInfo.MaxFootBrakeForce);
		g_logger.TraceWarning("CDataControler::HandleMoveDetectionDataW MaxFootBrakeForce- %f,%f",
			m_stMoveDetectionInfo.MaxFootBrakeForce,
			m_dInitFootForce);


		////GetInitXAngle 需要减去初始车辆倾角
		////g_logger.TraceWarning("CDataControler::HandleMoveDetectionDataW - %f,%f-%f,%f",
		////	m_stMoveDetectionInfo.GradientX,
		////	m_dInitCarXAngle,
		////	m_stMoveDetectionInfo.GradientY,
		////	m_dInitCarYAngle);
		m_stMoveDetectionInfo.GradientX = m_stMoveDetectionInfo.GradientX-m_dInitCarXAngle;
		m_stMoveDetectionInfo.GradientY = m_stMoveDetectionInfo.GradientY-m_dInitCarYAngle;
		////g_logger.TraceWarning("CDataControler::HandleMoveDetectionDataW - %f,%f-%f,%f",
		////	m_stMoveDetectionInfo.GradientX,
		////	m_dInitCarXAngle,
		////	m_stMoveDetectionInfo.GradientY,
		////	m_dInitCarYAngle);

		////tmp 由于加速度对倾角有大影响。所以暂时以初始地面倾角，亦即静止态地面倾角代替动听地面倾角。
		//double dXangle = m_dInitXAngle;
		//double dYangle = m_dInitYAngle;
		//m_stMoveDetectionInfo.GradientX = dXangle*(0.9 + (rand()%20 / 100.0));//%2的波动
		//m_stMoveDetectionInfo.GradientY = dYangle*(0.9 + (rand()%20 / 100.0));

		SetEvent(m_hEvtMoveDetectionInfo);
	}
	//处理坡度初始值
	void CDataControler::HandleInitGradientDataW(const double* pData, const int channelCount, const int sectionLength)
	{
		g_logger.TraceInfo("CDataControler::HandleInitGradientDataW");
		
		ResetEvent(m_hEvtInitGradientInfo);
		double dMaxX = *(pData+1*sectionLength);
		double dMaxY = *(pData+4*sectionLength);
		TransformGradient(dMaxX);
		TransformGradient(dMaxY);
		double dX=0,dY=0;

		for (int i=0;i<sectionLength;++i)
		{
			dX = *(pData+1*sectionLength+i);
			dY = *(pData+4*sectionLength+i);
			TransformGradient(dX);
			TransformGradient(dY);//由于有abs，转换中有减法，可能改变正负号，所以需要变换后再比较
			if (abs(dMaxX)<abs(dX))
			{
				dMaxX = dX;
			}
			if ( abs(dMaxY)<abs(dY) )
			{
				dMaxY = dY;
			}
		}

		m_dInitXAngle = dMaxX;
		m_dInitYAngle = dMaxY;

		SetEvent(m_hEvtInitGradientInfo);
	}

	//最大手刹力、XY倾角/脚刹位置
	void CDataControler::HandleStillDetectionDataW(const double* pData, const int channelCount, const int sectionLength)
	{
		g_logger.TraceInfo("CDataControler::HandleStillDetectionDataW");
		static int nMidCount = stnMidCount;
		//V = sum(ai*t) 

		ResetEvent(m_hEvtStillDetectionInfo);
		m_stStillDetectionInfo.MaxHandBrakeForce = *(pData+6*sectionLength);
		m_stStillDetectionInfo.GradientX = *(pData+1*sectionLength);
		m_stStillDetectionInfo.GradientY = *(pData+4*sectionLength);
		TransformGradient(m_stStillDetectionInfo.GradientX);
		TransformGradient(m_stStillDetectionInfo.GradientY);
		m_stStillDetectionInfo.PedalDistance = *(pData+5*sectionLength);

		Filter FilterHandBrakeForce;
		//Filter FilterGradientX ;
		//Filter FilterGradientY ;
		Filter FilterPedalDistance ;
		STILLDETECTIONINFO stStillDetectionInfo;
		for (int i=0;i<sectionLength;++i)
		{
			stStillDetectionInfo.MaxHandBrakeForce = *(pData+6*sectionLength+i);
			stStillDetectionInfo.GradientX = *(pData+1*sectionLength+i);
			stStillDetectionInfo.GradientY = *(pData+4*sectionLength+i);
			TransformGradient(stStillDetectionInfo.GradientX);
			TransformGradient(stStillDetectionInfo.GradientY);
			stStillDetectionInfo.PedalDistance = *(pData+5*sectionLength+i);
			//if (m_stStillDetectionInfo.MaxHandBrakeForce < stStillDetectionInfo.MaxHandBrakeForce)
			//{
			//	m_stStillDetectionInfo.MaxHandBrakeForce = stStillDetectionInfo.MaxHandBrakeForce;
			//}
			if (--nMidCount == 0)
			{
				nMidCount = stnMidCount;
				FilterHandBrakeForce.AddData1(stStillDetectionInfo.MaxHandBrakeForce);
			}
			if ( abs(m_stStillDetectionInfo.GradientX) < abs(stStillDetectionInfo.GradientX) )
			{
				m_stStillDetectionInfo.GradientX = stStillDetectionInfo.GradientX;
			}
			if ( abs(m_stStillDetectionInfo.GradientY) < abs(stStillDetectionInfo.GradientY) )
			{
				m_stStillDetectionInfo.GradientY = stStillDetectionInfo.GradientY;
			}
			//FilterGradientX.AddData(stStillDetectionInfo.GradientX);
			//FilterGradientY.AddData(stStillDetectionInfo.GradientY); 
			FilterPedalDistance.AddData(stStillDetectionInfo.PedalDistance);
		}
		//if (nMidCount == stnMidCount)
		{
			m_stStillDetectionInfo.MaxHandBrakeForce = FilterHandBrakeForce.GetMidValue();
		}
		m_stStillDetectionInfo.PedalDistance = FilterPedalDistance.GetMeanData();
		if (m_bGetInitPedalDist)
		{
			m_dInitPedalDist = m_stStillDetectionInfo.PedalDistance;
			m_bGetInitPedalDist = false;
		}
		//m_stStillDetectionInfo.MaxHandBrakeForce -= m_dInitHandForce;//180928 现在不再使用初始手刹力
		m_stStillDetectionInfo.MaxHandBrakeForce -= m_dInitHandForce;//181006 再次使用初始手刹力
		TransformPedalDistance(m_stStillDetectionInfo.PedalDistance);

		TransformHandBrakeForce(m_stStillDetectionInfo.MaxHandBrakeForce);

		//m_stStillDetectionInfo.GradientX = FilterGradientX.GetMeanData();
		//m_stStillDetectionInfo.GradientY = FilterGradientY.GetMeanData();
		//GetInitXAngle 需要减去初始车辆倾角
		//m_stStillDetectionInfo.GradientX = m_stStillDetectionInfo.GradientX-m_dInitCarXAngle;//v1.9.6.3
		//m_stStillDetectionInfo.GradientY = m_stStillDetectionInfo.GradientY-m_dInitCarYAngle;

		//保存下静止时的最大手刹力:有正负，所以是绝对值比大小
		if( abs(m_dMaxHandBrakeForce)<abs(m_stStillDetectionInfo.MaxHandBrakeForce) )
		{
			m_dMaxHandBrakeForce = m_stStillDetectionInfo.MaxHandBrakeForce;
		}
		if( abs(m_dMaxPedalDistance)<abs(m_stStillDetectionInfo.PedalDistance) )
		{
			m_dMaxPedalDistance = m_stStillDetectionInfo.PedalDistance;
		}

		//if(m_bUpdateCarAngleFlag)
		//{
		//	SaveCarAngle();
		//	m_bUpdateCarAngleFlag = false;
		//}
		SetEvent(m_hEvtStillDetectionInfo);
	}


	void CDataControler::GetInitGradientInfo(double& dX, double& dY)
	{
		DWORD dwRet = WaitForSingleObject(m_hEvtInitGradientInfo,1);
		if (WAIT_OBJECT_0 == dwRet)
		{
			dX = m_dInitXAngle;
			dY = m_dInitYAngle;
		}
		else
		{
			g_logger.TraceError("CDataControler::GetInitGradientInfo - %d",dwRet);
		}
	}
	void CDataControler::GetMoveDetectionInfo(MOVEDETECTIONINFO& stStressInfo)
	{
		DWORD dwRet = WaitForSingleObject(m_hEvtMoveDetectionInfo,1);
		if (WAIT_OBJECT_0 == dwRet)
		{
			stStressInfo = m_stMoveDetectionInfo;
		}
		else
		{
			g_logger.TraceError("CDataControler::GetStressInfo - %d",dwRet);
		}
	}
	void CDataControler::GetStillDetectionInfo(STILLDETECTIONINFO& stStillDetectionInfo)
	{
		DWORD dwRet = WaitForSingleObject(m_hEvtStillDetectionInfo,1);
		if (WAIT_OBJECT_0 == dwRet)
		{
			stStillDetectionInfo = m_stStillDetectionInfo;
		}
		else
		{
			g_logger.TraceError("CDataControler::GetVelocityInfo - %d",dwRet);
		}
	}

	//bool TransformBrakeDistance(double & dDist)//discard
	//{
	//	dDist = dDist/0.04;
	//	return true;
	//}
	bool CDataControler::TransformVelocity(double & dVel)
	{//nothing - DO NOT calc repeatedly!
//<<<<<<< Updated upstream
//		dVel = dVel/4.1;
//=======
//		dVel = dVel/m_dAccelaration1;
//>>>>>>> Stashed changes
//		//dVel = dVel/0.04;
		dVel = dVel*3.6;// m/s--Km/h
		return true;
	}
	bool CDataControler::TransformAcceleration(double & dAcc)
	{
//<<<<<<< Updated upstream
//		dAcc  = dAcc/4.1;
//=======
		dAcc  = dAcc/m_dAccelaration1;
		return true;
	}
	bool CDataControler::TransformFootBrakeForce(double &dForce)
	{
//<<<<<<< Updated upstream
//		dForce = (dForce-15.2) * 0.87048384;
//=======
		dForce = (dForce + m_dFootBrakePara1)*m_dFootBrakePara2;

		return true;
	}
	bool CDataControler::TransformHandBrakeForce(double &dForce)
	{
//<<<<<<< Updated upstream
//		dForce = (dForce - 4) * 0.27674141;
//=======
		dForce = (dForce + m_dHandBrakePara1) *m_dHandBrakePara2;
//>>>>>>> Stashed changes
//		//dForce = dForce * 4166.6666;
		return true;
	}

	bool CDataControler::TransformGradient(double &dGradient)
	{
//<<<<<<< Updated upstream
//		dGradient = tan( (dGradient-2500)/83.333 ) * 100;//%坡度单位用100%表示，所以*100
//=======
		//v1.7.2 采集到是弧度
		//dGradient = tan((dGradient + m_dAnglePara1)/m_dAnglePara2) *100;//%坡度单位用100%表示，所以*100
		//tan函数默认参数应该是弧度制
		dGradient = tan( (dGradient + m_dAnglePara1)/m_dAnglePara2 / 180.0 * NUM_PI ) *100;//%坡度单位用100%表示，所以*100
//>>>>>>> Stashed changes
//		//dGradient = (dGradient-2.59)*0.08333;
		return true;
	}

	bool CDataControler::TransformPedalDistance(double &dDist)
	{
//<<<<<<< Updated upstream
//		dDist = 1 / (0.12592593*dDist-0.01703704);
//=======
		//static int nCount=20;
		//nCount--;
		//if (nCount<0 )
		//{
		//	g_logger.TraceWarning("TransformPedalDistance- dist=%f",dDist);
		//}
		//g_logger.TraceWarning("TransformPedalDistance=%f - %f - %f",m_dPedalDistance1,m_dPedalDistance2,m_dPedalDistance3);
		
		//dDist = m_dPedalDistance1 / (dDist*m_dPedalDistance2+m_dPedalDistance3);
		// x=（V-V0）*0.4
		dDist = (dDist-m_dInitPedalDist)*m_dPedalDistance1;

		//if (nCount<0)
		//{
		//	g_logger.TraceWarning("TransformPedalDistance- dist1=%f",dDist);
		//	//nCount = 20;
		//}
//>>>>>>> Stashed changes
//		//dDist = 1 / (dDist-0.44) * 0.1026856240126 + 1/30;
		return true;
	}

}
