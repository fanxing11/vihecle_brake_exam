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
		void BeginAnalysis(const string strProjectPath);

		bool _BeginaAnalysis(string &strErrInfo);

	private:
		HANDLE m_hAnalysisThread;
		string m_strProjectPath;
		HANDLE m_hBeginEvent;
	};

}
