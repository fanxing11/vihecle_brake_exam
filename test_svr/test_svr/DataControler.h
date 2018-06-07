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


	public:
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

		// called by DAQ- wireless
		void HandleStillDetectionDataW(const double* pData, const int channelCount, const int sectionLength);
		void HandleMoveDetectionDataW(const double* pData, const int channelCount, const int sectionLength, const double deltat);
		void HandleInitGradientDataW(const double* pData, const int channelCount, const int sectionLength);

		// called by send2UI(main)
		void GetInitGradientInfo(double& dX, double& dY);
		void GetMoveDetectionInfo(MOVEDETECTIONINFO& stStressInfo);
		void GetStillDetectionInfo(STILLDETECTIONINFO& stStillDetectionInfo);
		void SetUpdateCarAngleFlag();
		bool SaveMaxHandBrakeForce2INI();

	private:
		MOVEDETECTIONINFO m_stMoveDetectionInfo;
		STILLDETECTIONINFO m_stStillDetectionInfo;
		HANDLE m_hEvtMoveDetectionInfo;
		HANDLE m_hEvtStillDetectionInfo;
		HANDLE m_hEvtInitGradientInfo;
		string m_strConfigFullName;

		double m_dInitXAngle;//初始地面倾角
		double m_dInitYAngle;
		double m_dInitCarXAngle;//车辆倾角
		double m_dInitCarYAngle;
		bool m_bUpdateCarAngleFlag;
		void SaveCarAngle();
		double m_dMaxHandBrakeForce;

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

