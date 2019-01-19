#pragma once
#include "const.h"


#define _MY_VERSION_2_0_3_ 1



namespace ANALYSISSPACE
{
	//每多少个点发送一个到UI
	const int COUNTBETWEENSEND = 5120;
	class CAnalysis
	{
	public:
		CAnalysis(void);
		~CAnalysis(void);

	public:
		void BeginAnalysis(const string &strProjectPath);

		bool _BeginaAnalysis(string &strErrInfo);
		void PostAnalysisStateMsg(const int nState=1);

	private:
		void InitData();
		HANDLE m_hAnalysisThread;
		string m_strProjectPath;
		string m_strConfigFile;
		bool GetDataFile(string& strFileName);

		double m_dCarInitXAngle;
		double m_dCarInitYAngle;
		double m_dMaxHandBrakeForce;
		double m_dInitAccA;
		double m_dInitAccB;
		double m_dInitAccC;
		double m_dInitFootBrakeForce;
		double m_dInitHandBrakeForce;
		double m_dMaxPedalDistance;
		bool ReadParaFromINI(string &strErrInfo);
		bool ReadDataFromFile(string &strErrInfo);

		void HandleData(const double* pData, const int channelCount, const DWORD dwDataSize/*Byte*/, const double deltat);
		// for wireless DAQ file
		void HandleDataW(const double* pData, const int channelCount, const DWORD dwDataSize/*Byte*/, const double deltat);
		bool AnalyseResult();
		void NormalizData();
		//dData - in out,
		//nOriginRange - in
		//nNewRange - out (0-nRange)
		void NormalizDataOne(double& dData, const double dOriginMin, const double dOriginMax, const int nNewRange=100);
		void FindMaxMinRange(vector<double> &vData/*in*/, double& minData, double& maxData);
		void SendAnalysisData();
		ANALYSISRESULT m_stResult;
		vector <ANALYSISDATA> m_vAnalysisData;
		vector <double> m_vAccelaration;
		vector <double> m_vVelocity;
		vector <double> m_vFootBrakeForce;
		//vector <double> m_vPedalDistance ;


		HANDLE m_hFileReader;;
	};

}
