#pragma once
#include "const.h"
namespace ANALYSISSPACE
{

	class CAnalysis
	{
	public:
		CAnalysis(void);
		~CAnalysis(void);

	public:
		void BeginAnalysis(const string &strProjectPath);

		bool _BeginaAnalysis(string &strErrInfo);

	private:
		HANDLE m_hAnalysisThread;
		string m_strProjectPath;
		string m_strConfigFile;
		bool GetDataFile(string& strFileName);

		double m_dInitXAngle;
		double m_dInitYAngle;
		bool ReadParaFromINI(string &strErrInfo);
		bool ReadDataFromFile(string &strErrInfo);

		void HandleData(const double* pData, const int channelCount, const DWORD dwDataSize/*Byte*/, const double deltat);
		bool AnalyseResult();
		ANALYSISRESULT m_stResult;

		HANDLE m_hFileReader;;
	};

}
