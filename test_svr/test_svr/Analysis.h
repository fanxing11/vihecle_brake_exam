#pragma once
#include "const.h"
namespace ANALYSISSPACE
{
const int COUNTBETWEENSEND = 500;
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
		HANDLE m_hAnalysisThread;
		string m_strProjectPath;
		string m_strConfigFile;
		bool GetDataFile(string& strFileName);

		double m_dCarInitXAngle;
		double m_dCarInitYAngle;
		double m_dMaxHandBrakeForce;
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
		vector <double> m_vPedalDistance ;


		HANDLE m_hFileReader;;
	};

}
