#pragma once

#include <Winsock2.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")

#include "const.h"


namespace COMMUNICATOR
{
	class CCommunicator
	{
	public:
		CCommunicator(void);
		~CCommunicator(void);

		bool Initialize();

	private:
		// 通讯线程句柄
		DWORD m_dwMainThreadId;
		HANDLE m_hRevThread;
		SOCKET m_SockSrv;
		SOCKADDR_IN m_addrClient;


	public:
		SOCKET GetSocket();
		//解析UDP接收的数据
		bool RecvData();
		bool ParseData(const char* pData);
		bool SendDatatoUI(const UINT Cmd, const int nParam=0,/*o->good*/ const string strData2Send = "");
		bool SendAnalysisResult2UI(const int nResult, const ANALYSISRESULT& stResult);
		bool SendAnalysisData2UI( vector<ANALYSISDATA>& stData );

	private:
		bool cmdASD(const char* pData	);
		bool cmdUserRegister( const char* pData);
		bool cmdUserLogin(const char* pData	);
		bool cmdUserDelete(const char* pData );
		bool cmdAdminUser(const char* pData );
		bool cmdModifyPwd(const char* pData );
		bool cmdParseUserInfo(const char* pData, char *&pUserName, char *&pPwd );

		bool cmdNewProject(const char* pData );
		bool cmdTerminateProject( );
		bool cmdInitGradientBegin(const char* pData );
		bool cmdInitGradientEnd(const char* pData );
		bool cmdStillDetectionBegin(const char* pData );
		bool cmdStillDetectionEnd(const char* pData );
		bool cmdMoveDetectionBegin(const char* pData );
		bool cmdMoveDetectionEnd(const char* pData );
		bool cmdSetReportPath(const char* pData );
		bool cmdAnalysisBegin(const char* pData );
		bool cmdHeartBeatSignal(const char* pData);
		bool cmdQuit();

		UINT_PTR nTimer;//temp
	};

}
