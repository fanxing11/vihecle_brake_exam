#pragma once
#include "const.h"

namespace DATACONTROLER
{
	class CDataControler
	{
	public:
		CDataControler(void);
		~CDataControler(void);
	private:
		//0 - 未开始
		//1 - 正在进行
		//2 - 已经完成
		short m_nCurrentProjectState;
		enDETECTION_TYPE m_nCurrentType;
		//是否需要存储0.5s内的初始踏板距离
		bool m_bGetInitPedalDist;
		bool m_bGetInitFootBrakeForce;

		char m_cStartChannel;
		char m_cEndChannel;
		//0x01:1K
		//0x02:2K
		//0x03:3K
		char m_cSampleFrequency;
		//0x01:测试模式
		//0x02:完整检测
		char m_cMode;//检测模式
		//0x01:word
		//0x02:excel
		//0x03:txt
		char m_cArchiveFormat;
		string m_strProjectPath;

		string m_strUserName;

		float m_fMaxAcceleratedVel;
		float m_fBrakingLength;
		float m_fAverageVel;
		float m_fGradient;//podu
		float m_fPedalDistance;
		float m_fMaxHandBrakeForce;
		float m_fMaxFootBrakeForce;
		char m_cResult;

		string m_strReportPath;

		void SetStartChannel(const char cStart);
		void SetEndChannel(const char cEnd);
		void SetSampleFrequency(const char cSampleFrequency);
		void SetMode(const char cMode);
		void SetArchiveFromat(char cFormat);
		void SetProjectPath(string& strPath);
		void CreateDefaultProjectPath(string &strPath);
		bool SaveProjectInfo2INIFile();

		bool ReadSensorConfigFromINI();
		bool ReadOneParaFromConfigINI(const string SectionName, 
			const string ParaName,
			double & dPara);
		double m_dIsWireless;//Initial sensor para
		double m_dValidFootBrakeForce;
		double m_dFootBrakePara1;//Initial sensor para
		double m_dFootBrakePara2;
		double m_dHandBrakePara1;
		double m_dHandBrakePara2;
		double m_dAnglePara1;
		double m_dAnglePara2;
		double m_dPedalDistance1;
		//double m_dPedalDistance2;
		//double m_dPedalDistance3;
		//double m_dPedalDistance4;
		//double m_dPedalDistance5;
		double m_dAccelaration1;

	public:
		//wired / wireless
		bool DAQIsWirelessType();
		double GetValidFootBrakeForce() const;
		//called in main
		int GetCurrentProjectState()const;
		bool GetProjectPath(string& strPath) const;
		
		//called by communicator
		void SetNewProjectPara(const char* pData);
		bool NewProject(string& strInfo);
		bool TerminateCurrentProject();
		char GetMode()const;
		void SetCurrentType(enDETECTION_TYPE dt);

		//-----useless
		void SetUserName(const string strUserName);

		void SetMaxAcceleratedVel(const float ff);
		void SetBrakingLength(const float ff);
		void SetAverageVel(const float ff);
		void SetGradient(const float ff);//坡度.地面倾角
		void SetPedalDistance(const float ff);
		void SetMaxHandBrakeForce(const float ff);
		void SetMaxFootBrakeForce(const float ff);
		void SetResult(const char cResult);
		//----useless

		void SetReportPath(const string strPath);
		 
		// called by DAQ
		void HandleStillDetectionData(const double* pData, const int channelCount, const int sectionLength);
		void HandleMoveDetectionData(const double* pData, const int channelCount, const int sectionLength, const double deltat);
		void HandleInitGradientData(const double* pData, const int channelCount, const int sectionLength);
		void GetInitValue(const double* pData, const int channelCount, const int sectionLength);
		
		// called by DAQ- wireless
		void HandleStillDetectionDataW(const double* pData, const int channelCount, const int sectionLength);
		void HandleMoveDetectionDataW(const double* pData, const int channelCount, const int sectionLength, const double deltat);
		void HandleInitGradientDataW(const double* pData, const int channelCount, const int sectionLength);
		void GetInitValueW(const double* pData, const int channelCount, const int sectionLength);
		void HandleGradientData(const double* pData, const int channelCount, const int sectionLength);

		// called by send2UI(main)
		void GetGradientInfo(double& dX);
		void GetInitGradientInfo(double& dX, double& dY);
		void GetMoveDetectionInfo(MOVEDETECTIONINFO& stStressInfo);
		void GetStillDetectionInfo(STILLDETECTIONINFO& stStillDetectionInfo);
		void SetInitHandForceFlag();
		void SetInitAngleFlag();
		bool SaveMaxHandBrakeForce2INI();
		bool SaveMaxPedalDistance2INI();
		bool SaveInitValue2INI();

		void SetGetInitPedalDist();
		void SetGetInitFootBrakeForce();

		//set the gradient file path.
		void SetGradientPath(const string strPath);
		//initialize gradient, call 5 times per test.
		void GradientInitialize();
		//sampling, call many times(<256)
		void GradientSampling();
		//get current sampling result.
		void GetGradientCurrentResult();
		//get history sampling result.
		void GetGradientFromFile(const string strPath);


	private:
		MOVEDETECTIONINFO m_stMoveDetectionInfo;
		STILLDETECTIONINFO m_stStillDetectionInfo;
		double m_dGradient;
		HANDLE m_hEvtMoveDetectionInfo;
		HANDLE m_hEvtStillDetectionInfo;
		HANDLE m_hEvtInitGradientInfo;
		HANDLE m_hEvtGradientInfo;
		string m_strConfigFullName;

		double m_dInitXAngle;//初始地面倾角
		double m_dInitYAngle;
		double m_dInitCarXAngle;//车辆倾角
		double m_dInitCarYAngle;
		bool m_bInitAngleFlag;
		bool m_bInitHandForceFlag;
		void SaveCarAngle();
		double m_dMaxHandBrakeForce;
		double m_dMaxPedalDistance;

		double m_dInitHandForce;
		double m_dInitFootForce;
		double m_dInitAccA;
		double m_dInitAccB;
		double m_dInitAccC;

		//初始脚踏板力
		double m_dInitPedalDist;

		string m_strGradientFileName;
		int m_nGradientSamplingCount;
		vector<double> m_vtGradientInitData;
		//地面与车之间的相对倾角
		double m_dRelativeGradient;


	public:
		bool TransformBrakeDistance(double & dVel);
		bool TransformVelocity(double & dVel);
		bool TransformAcceleration(double & dAcc);
		bool TransformFootBrakeForce(double &dForce);
		bool TransformHandBrakeForce(double &dForce);
		bool TransformGradient(double &dGradient);
		bool TransformPedalDistance(double &dDist);
	};

}

