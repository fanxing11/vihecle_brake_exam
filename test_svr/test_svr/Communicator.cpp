
#include "main.h"

#include "Communicator.h"
#include "DataControler.h"

extern CtheApp theApp;

namespace COMMUNICATOR
{

	unsigned int WINAPI UDPRevThreadFunc(LPVOID lp)
	{
		g_logger.TraceInfo("UDPRevThreadFunc_funcin");

		//STIN_COMTHREAD stTep;
		//stTep = *( STIN_COMTHREAD* )lp;
		CCommunicator* pCCommunicator=(CCommunicator*)lp;


		while(1)
		{
			if( !pCCommunicator->RecvData())
			{
				g_logger.TraceInfo("UDPRevThreadFunc_break while");
				break;
			}
		}

		_endthreadex(0);

		g_logger.TraceInfo("UDPRevThreadFunc_funcout");

		return 0;
	}

	CCommunicator::CCommunicator(void)
		:m_hRevThread(NULL)
		,m_SockSrv(NULL)
		,m_dwMainThreadId(0)
	{
		g_logger.TraceInfo("CCommunicator::CCommunicator");
		this->Initialize();
	}

	CCommunicator::~CCommunicator(void)
	{
		DWORD dw =WaitForSingleObject(m_hRevThread,1000);
		if( dw == WAIT_TIMEOUT )
		{
			//cout<<"wait for thread timeout"<<endl;
			g_logger.TraceInfo("wait for thread timeout");
		}
		if (m_SockSrv != NULL)
		{
			closesocket(m_SockSrv);
		}
		WSACleanup();
	}

	SOCKET CCommunicator::GetSocket()
	{
		return m_SockSrv;
	}

	bool CCommunicator::Initialize()
	{
		WORD wVersionRequested; 
		WSADATA wsaData; 
		int err; 

		wVersionRequested=MAKEWORD(1,1); 

		err = WSAStartup(wVersionRequested, &wsaData); 
		if (err != 0) 
		{ 
			return false; 
		} 
		g_logger.TraceInfo("max IP package size:iMaxUdpDg=%d",wsaData.iMaxUdpDg);

		if (LOBYTE(wsaData.wVersion)!=1 || 
			HIBYTE(wsaData.wVersion)!=1) 
		{ 
			WSACleanup(); 
			return false; 
		} 

		m_SockSrv=socket(AF_INET,SOCK_DGRAM,0);
		SOCKADDR_IN addrSrv;
		addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
		addrSrv.sin_family=AF_INET;
		addrSrv.sin_port=htons(10000);

		bind(m_SockSrv,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));

		m_dwMainThreadId = GetCurrentThreadId();
		//STIN_COMTHREAD stTep;
		//stTep.skSever = m_SockSrv;
		//stTep.dwMainThreadID = m_dwMainThreadId;

		m_hRevThread = (HANDLE)_beginthreadex(NULL, 0, UDPRevThreadFunc, (LPVOID)this, 0, NULL);  
		//WaitForSingleObject(handle, INFINITE); 


		return true;
	}

	bool CCommunicator::SendDatatoUI(const UINT Cmd, const int nParam, const string strData2Send)
	{
		if (m_SockSrv == NULL)
		{
			g_logger.TraceError("CCommunicator::SendDatatoUI - m_SockSrv is NULL");
			return false;
		}

		switch (Cmd)
		{
		case msg_DB_USERLOGIN:
			{
				char Ret[4] = {cmd_HEADER,cmd_USERLOGIN,0x00,cmd_TAIL};
				switch (nParam)
				{
				case NUM_ZERO://good
					Ret[2] = 0x03;
					break;
				case NUM_ONE://pwd error
					Ret[2] = 0x02;
					break;
				case NUM_TWO:
					Ret[2] = 0x01;//user not exist
					break;
				}
				sendto(m_SockSrv, Ret, 4, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				break;
			}
		case msg_DB_USERREGISTER:
			{
				char Ret[4] = {cmd_HEADER,cmd_USERREGISTER,0x00,cmd_TAIL};
				switch (nParam)
				{
				case NUM_ZERO://good
					Ret[2] = 0x02;
					break;
				case NUM_ONE://repeated
					Ret[2] = 0x01;
					break;
				case NUM_NEGONE:
					Ret[2] = 0x00;//other error
					g_logger.TraceError("CCommunicator::SendDatatoUI - msg_DB_USERREGISTER other error");
					break;
				}
				sendto(m_SockSrv, Ret, 4, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				break;
			}
		case msg_DB_MODIFYPASSWORD:
			{
				char Ret[4] = {cmd_HEADER,cmd_MODIFYPWD,0x00,cmd_TAIL};
				switch (nParam)
				{
				case NUM_ZERO://good
					Ret[2] = 0x01;
					break;
				case NUM_ONE://repeated
					Ret[2] = 0x02;
					break;
				}
				sendto(m_SockSrv, Ret, 4, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				break;
			}
		case msg_DB_DELETEUSER:
			{
				char Ret[4] = {cmd_HEADER,cmd_USERDELETE,0x00,cmd_TAIL};
				switch (nParam)
				{
				case NUM_ZERO://good
					Ret[2] = 0x01;
					break;
				case NUM_ONE://repeated
					Ret[2] = 0x02;
					break;
				}
				sendto(m_SockSrv, Ret, 4, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				break;
			}
		case msg_DAQ_DATAONE:
			{
				if (WAIT_OBJECT_0 == WaitForSingleObject(DAQCONTROLER::m_gEvtVelocity,0))
				{
					char Ret[11] = {cmd_HEADER,cmd_VELOCITY,
						0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,
						cmd_TAIL};	

					VELOCITYINFO stVelocityInfo;
					theApp.m_pDataC->GetVelocityInfo(stVelocityInfo);

					int nAcceleration = (int)(100*( stVelocityInfo.LastAccelaration ));
					int nVelocity = (int)(100*( stVelocityInfo.LastVelocity ));

					//nAcceleration = 400;
					//nVelocity = 800;
					
					memcpy(Ret+2, &nVelocity, sizeof(int));//4bit
					memcpy(Ret+6, &nAcceleration, sizeof(int));//4bit

					sendto(m_SockSrv, Ret, 11, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				}
				if (WAIT_OBJECT_0 == WaitForSingleObject(DAQCONTROLER::m_gEvtStress,0))
				{
					char Ret[19] = {cmd_HEADER,cmd_STRESS,
						0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,
						cmd_TAIL};	

					STRESSINFO stStressInfo;
					theApp.m_pDataC->GetStressInfo(stStressInfo);

					int nPedalDist = (int)(100*( stStressInfo.PedalDistance ));
					int nGradient = (int)(100*( stStressInfo.Gradient ));
					int nFootBrakeForce = (int)(100*(stStressInfo.MaxFootBrakeForce * 4166.66));
					int nHandBrakeForce = (int)(100*(stStressInfo.MaxHandBrakeForce * 4166.66));

					cout<<"send------"<<nPedalDist<<" "<<nGradient<<" "<<nFootBrakeForce<<" "<<nHandBrakeForce<<endl;

					memcpy(Ret+2, &nPedalDist, sizeof(int));//4bit
					memcpy(Ret+6, &nGradient, sizeof(int));//4bit
					memcpy(Ret+10, &nFootBrakeForce, sizeof(int));//4bit
					memcpy(Ret+14, &nHandBrakeForce, sizeof(int));//4bit

					sendto(m_SockSrv, Ret, 19, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				}

				break;
			}

		}

		return true;
	}

	bool CCommunicator::RecvData()
	{
		static bool snFirstTime=true;
		if (!GetSocket())
		{
			g_logger.TraceFatal("ThreadFunc:socket is NULL,thread initial failed!");
			return false;
		}

		int nAddrBufLen=sizeof(SOCKADDR);
		//char* pBuf = new char(100);
		//memset(pBuf,0,100);
		char pBuf[100]={0};//max byte no. 100
		SOCKADDR_IN addrClient;

		int nLenRev = recvfrom(m_SockSrv,pBuf,100,0,(SOCKADDR*)&addrClient,&nAddrBufLen);
		if (nLenRev == 0)
		{
			g_logger.TraceError("CCommunicator::ParseRevData - connection has teminated");
		}
		else if ( SOCKET_ERROR == nLenRev )
		{
			g_logger.TraceError("CCommunicator::ParseRevData - recvfrom return SOCKET_ERROR,error code:%d ",WSAGetLastError());
		}
		else
		{
			g_logger.TraceInfo("CCommunicator::ParseRevData - recvfrom %d length data",nLenRev);
			if( !ParseData(pBuf) )
			{
				g_logger.TraceError("CCommunicator::ParseRevData - recv data parse error");
			}
			//printf("rev: %s\n",pBuf);
		}
		//delete[] pBuf;
		//pBuf = NULL;

		if (snFirstTime)
		{
			m_addrClient = addrClient;
			snFirstTime = false;
		}


		//char sendBuf[100]={0};
		//sprintf(sendBuf, "Welcome %s to http://www.meng.org",
		//	inet_ntoa(addrClient.sin_addr));
		////send data
		//int nLenSend = sendto(m_SockSrv,sendBuf,strlen(sendBuf)+1,0,(SOCKADDR*)&addrClient,nAddrBufLen);
		//if ( nLenSend == SOCKET_ERROR)
		//{
		//	g_logger.TraceError("CCommunicator::ParseRevData - sendto return SOCKET_ERROR,error code:%d ",WSAGetLastError());
		//}
		//else if( nLenSend != strlen(sendBuf)+1 )
		//{
		//	g_logger.TraceError("CCommunicator::ParseRevData - sendto length not match:send=%d,sent=%d",strlen(sendBuf)+1,nLenSend);
		//}
		//else
		//{
		//	g_logger.TraceInfo("CCommunicator::ParseRevData - send success,sentLen=%d",nLenSend);
		//}


		return true;
	}

	bool CCommunicator::ParseData( const char* pData)
	{
		BYTE cb=0;
		int nLoc=0;
		memcpy(&cb,pData+nLoc,1);//header
		//printf("header is 0x%x\n",cb);
		if ( cb != cmd_HEADER)//0x7E
		{
			g_logger.TraceInfo("Parse Header bad");
			printf("Parse Header bad");
			return false;
		}

		++nLoc;
		memcpy(&cb,pData+nLoc,1);//cmd
		g_logger.TraceInfo("CCommunicator::ParseData:cmd=0x%x",cb);
		switch (cb)
		{
		case cmd_USERREGISTER:
			this->cmdUserRegister(pData);
			break;
		case cmd_USERLOGIN:
			this->cmdUserLogin(pData);
			break;
		case cmd_USERDELETE:
			this->cmdUserDelete(pData);
			break;
		case cmd_MODIFYPWD:
			this->cmdModifyPwd(pData);
			break;
		case cmd_SYSTEMCONFIG:
			this->cmdSystemConfig(pData);
			break;
		case cmd_VELOCITY_BEGIN:
			this->cmdVelocityBegin(pData);
			break;
		case cmd_VELOCITY_END:
			this->cmdVelocityEnd(pData);
			break;
		case cmd_STRESS_BEGIN:
			this->cmdStressBegin(pData);
			break;
		case cmd_STRESS_END:
			this->cmdStressEnd(pData);
			break;
		//case cmd_REPORTPATH:
		//	this->cmdSetReportPath(pData);
		//	break;
		case cmd_HEARTBEAT:
			this->cmdHeartBeatSignal(pData);
			break;
		default:
			g_logger.TraceError("CCommunicator::ParseData:cmd false,cmd=%x",cb);
			break;
		}




		////if rev 'quit', quit the 
		//string strBuf(pData);
		//string strQuit("quit");
		//if (pData == strQuit)
		//{
		//	g_logger.TraceInfo("post quit msg");
		//	PostThreadMessage(m_dwMainThreadId,2000,3000,4000);
		//	return false;
		//}

		return true;
	}

	bool CCommunicator::cmdParseUserInfo(const char* pData, char *&pUserName, char *&pPwd )
	{
		int nUserNameLen=0;
		int nPwdLen = 0;
		int nLoc = 2;
		memcpy(&nUserNameLen,pData+nLoc,1);
		pUserName = new char[nUserNameLen+1];
		memset(pUserName,'\0',nUserNameLen+1);
		++nLoc;
		memcpy(pUserName,pData+nLoc,nUserNameLen);
		nLoc = nLoc+nUserNameLen;
		memcpy(&nPwdLen,pData+nLoc,1);
		pPwd = new char[nPwdLen+1];
		memset(pPwd,'\0',nPwdLen+1);
		++nLoc;
		memcpy(pPwd,pData+nLoc,nPwdLen);

		return true;
	}

	bool CCommunicator::cmdUserRegister( const char* pData)
	{
		g_logger.TraceWarning("CCommunicator::cmdUserRegister");
		char* pUserName = NULL;
		char* pPwd = NULL;
		cmdParseUserInfo(pData,pUserName,pPwd);

		if ( !PostThreadMessage(m_dwMainThreadId,msg_DB_USERREGISTER,(WPARAM)pUserName,(LPARAM)pPwd) )
		{
			delete [] pUserName;
			delete [] pPwd;
			pUserName = pPwd = NULL;
		}

		return true;
	}

	bool CCommunicator::cmdUserLogin(const char* pData	)
	{
		g_logger.TraceWarning("CCommunicator::cmdUserLogin");
		char* pUserName = NULL;
		char* pPwd = NULL;
		cmdParseUserInfo(pData,pUserName,pPwd);

		if ( !PostThreadMessage(m_dwMainThreadId,msg_DB_USERLOGIN,(WPARAM)pUserName,(LPARAM)pPwd) )
		{
			delete [] pUserName;
			delete [] pPwd;
			pUserName = pPwd = NULL;
		}

		return true;
	}
	bool CCommunicator::cmdUserDelete(const char* pData )
	{
		g_logger.TraceWarning("CCommunicator::cmdUserDelete");
		char* pUserName = NULL;
		char* pPwd = NULL;
		cmdParseUserInfo(pData,pUserName,pPwd);

		if ( !PostThreadMessage(m_dwMainThreadId,msg_DB_DELETEUSER,(WPARAM)pUserName,(LPARAM)pPwd) )
		{
			delete [] pUserName;
			delete [] pPwd;
			pUserName = pPwd = NULL;
		}

		return true;
	}
	bool CCommunicator::cmdModifyPwd(const char* pData )
	{
		g_logger.TraceWarning("CCommunicator::cmdModifyPwd");
		char* pUserName = NULL;
		char* pPwd = NULL;
		cmdParseUserInfo(pData,pUserName,pPwd);

		if ( !PostThreadMessage(m_dwMainThreadId,msg_DB_MODIFYPASSWORD,(WPARAM)pUserName,(LPARAM)pPwd) )
		{
			delete [] pUserName;
			delete [] pPwd;
			pUserName = pPwd = NULL;
		}

		return true;
	}
	bool CCommunicator::cmdSystemConfig(const char* pData )
	{
		int nLoc = 2;
		char cTemp;
		memcpy(&cTemp,pData+nLoc,1);
		theApp.m_pDataC->SetStartChannel(cTemp);
		++nLoc;
		memcpy(&cTemp,pData+nLoc,1);
		theApp.m_pDataC->SetEndChannel(cTemp);
		++nLoc;
		memcpy(&cTemp,pData+nLoc,1);
		theApp.m_pDataC->SetSampleFrequency(cTemp);
		++nLoc;
		memcpy(&cTemp,pData+nLoc,1);
		theApp.m_pDataC->SetMode(cTemp);
		++nLoc;
		memcpy(&cTemp,pData+nLoc,1);
		theApp.m_pDataC->SetArchiveFromat(cTemp);


		return true;
	}
	bool CCommunicator::cmdVelocityBegin(const char* pData )
	{
		theApp.m_pDAQC->VelocityBegin();
		return true;
	}
	bool CCommunicator::cmdVelocityEnd(const char* pData )
	{
		theApp.m_pDAQC->VelocityEnd();
		return true;
	}
	bool CCommunicator::cmdStressBegin(const char* pData )
	{
		theApp.m_pDAQC->StressBegin();
		return true;
	}
	bool CCommunicator::cmdStressEnd(const char* pData )
	{
		theApp.m_pDAQC->StressEnd();
		return true;
	}
	bool CCommunicator::cmdSetReportPath(const char* pData )
	{
		int nPathLen=0;
		int nLoc = 2;
		memcpy(&nPathLen,pData+nLoc,1);
		char* pPath = new char[nPathLen+1];
		memset(pPath,'\0',nPathLen+1);
		++nLoc;
		memcpy(pPath,pData+nLoc,nPathLen);
		string strPath(pPath);
		delete[] pPath;
		pPath = NULL;
		theApp.m_pDataC->SetReportPath(strPath);

		return true;
	}
	bool CCommunicator::cmdHeartBeatSignal(const char* pData)
	{
		char Ret[3] = {0x7E,0x51,0x7F};

		//send data
		int nLenSend = sendto(m_SockSrv,Ret,3,0,(SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
		if ( nLenSend == SOCKET_ERROR)
		{
			g_logger.TraceError("CCommunicator::cmdHeartBeatSignal - sendto return SOCKET_ERROR,error code:%d ",WSAGetLastError());
		}
		else if( nLenSend != 3 )
		{
			g_logger.TraceError("CCommunicator::cmdHeartBeatSignal - sendto length not match:send=%d,sent=%d",3,nLenSend);
		}
		else
		{
			g_logger.TraceInfo("CCommunicator::cmdHeartBeatSignal - send success,sentLen=%d",nLenSend);
		}

		return true;
	}

}
