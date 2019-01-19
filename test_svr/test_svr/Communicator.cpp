
#include "main.h"

#include "Communicator.h"
#include "DataControler.h"

#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")

extern CtheApp* theApp;

//如果需要发送模拟数据到client，打开此开关
#define TESTDATA 0

namespace COMMUNICATOR
{

	unsigned int WINAPI UDPRevThreadFunc(LPVOID lp)
	{
		g_logger.TraceInfo("UDPRevThreadFunc_funcin");

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
		,nTimer(0)
	{
		g_logger.TraceInfo("CCommunicator::CCommunicator");
	}

	CCommunicator::~CCommunicator(void)
	{
		g_logger.TraceWarning("CCommunicator::~CCommunicator-in");
		DWORD dw =WaitForSingleObject(m_hRevThread,1);
		if( dw == WAIT_TIMEOUT )
		{
			g_logger.TraceInfo("CCommunicator::~CCommunicator - wait for thread timeout");
		}
		if (m_SockSrv != NULL)
		{
			closesocket(m_SockSrv);
		}
		WSACleanup();
		g_logger.TraceWarning("CCommunicator::~CCommunicator-out");
	}

	SOCKET CCommunicator::GetSocket()
	{
		return m_SockSrv;
	}

	bool CCommunicator::Initialize()
	{
		try
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

			int nRet=bind(m_SockSrv,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
			if (NUM_ZERO != nRet)
			{
				return false;
			}

			m_dwMainThreadId = GetCurrentThreadId();

			m_hRevThread = (HANDLE)_beginthreadex(NULL, 0, UDPRevThreadFunc, (LPVOID)this, 0, NULL);  

			return true;

		}
		catch (exception &e)
		{
			g_logger.TraceError("CCommunicator::Initialize:(in catch)Initial Communicator failed.");
			return false;
		}
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
		case msg_DAQ_ASD:
			{
				char Ret[4] = {cmd_HEADER,cmd_ASD,0x00,cmd_TAIL};
				switch (nParam)
				{
				case NUM_ZERO://pass
					Ret[2] = 0x00;
					break;
				case NUM_ONE://error
					Ret[2] = 0x01;
					break;
				}
				sendto(m_SockSrv, Ret, 4, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				break;
			}
		case msg_DB_ADMINUSER:
			{
				char Ret[4] = {cmd_HEADER,cmd_ADMINUSER,0x00,cmd_TAIL};
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
				//test
				if (TESTDATA)
				{
					//char Ret[27] = {cmd_HEADER,cmd_MOVE_DETECT,
					//	0x00,0x00,0x00,0x00,
					//	0x00,0x00,0x00,0x00,
					//	0x00,0x00,0x00,0x00,
					//	0x00,0x00,0x00,0x00,
					//	0x00,0x00,0x00,0x00,
					//	0x00,0x00,0x00,0x00,
					//	cmd_TAIL};	

					//MOVEDETECTIONINFO stStressInfo;
					//theApp->m_pDataController->GetMoveDetectionInfo(stStressInfo);

					//int nPedalDist = (int)(100*( stStressInfo.PedalDistance ));
					//int nGradientX = (int)(100*( stStressInfo.GradientX ));
					//int nGradientY = (int)(100*( stStressInfo.GradientY ));
					//int nFootBrakeForce = (int)(100*(stStressInfo.MaxFootBrakeForce));
					//int nLastVelocity = (int)(100*(stStressInfo.LastVelocity));
					//int nLastAccelaration = (int)(100*(stStressInfo.LastAccelaration));

					static int n=0;
					static bool bTransform = false;
					if (n == 50 || n ==-50)
					{
						bTransform = !bTransform;
					}
					if (bTransform)
					{
						n++;
					}
					else
					{
						n--;
					}
					////nLastAccelaration -= 100*n;
					//nLastAccelaration = 100*n;
					//nGradientY = abs(100*n);
					//nGradientX = 2000;

					//memcpy(Ret+2, &nFootBrakeForce, sizeof(int));//4bit
					//memcpy(Ret+6, &nPedalDist, sizeof(int));//4bit
					//memcpy(Ret+10, &nGradientX, sizeof(int));//4bit
					//memcpy(Ret+14, &nGradientY, sizeof(int));//4bit
					//memcpy(Ret+18, &nLastVelocity, sizeof(int));//4bit
					//memcpy(Ret+22, &nLastAccelaration, sizeof(int));//4bit

					//sendto(m_SockSrv, Ret, 27, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));

					char Ret[15] = {cmd_HEADER,cmd_STILL_DETECT,
						0x00,0x00,0x00,0x00,//handbrakeforce
						0x00,0x00,0x00,0x00,//X
						0x00,0x00,0x00,0x00,//Y
						cmd_TAIL};	

					STILLDETECTIONINFO stStillDetectionInfo;
					theApp->m_pDataController->GetStillDetectionInfo(stStillDetectionInfo);

					int nMaxHandBrakeForce = (int)(100*( stStillDetectionInfo.MaxHandBrakeForce ));
					nMaxHandBrakeForce = 100*n;
					int nGradientX = (int)(100*( stStillDetectionInfo.GradientX ));
					int nGradientY = (int)(100*( stStillDetectionInfo.GradientY ));

					g_logger.TraceWarning("sendData2UI-HandBrakeForce = %f",
						nMaxHandBrakeForce / 100.0);

					memcpy(Ret+2, &nMaxHandBrakeForce, sizeof(int));//4bit
					memcpy(Ret+6, &nGradientX, sizeof(int));//4bit
					memcpy(Ret+10, &nGradientY, sizeof(int));//4bit

					sendto(m_SockSrv, Ret, 15, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));

				}
				if (WAIT_OBJECT_0 == WaitForSingleObject(DAQCONTROLER::m_gEvtInitGradient,0))
				{
					char Ret[11] = {cmd_HEADER,cmd_INITGRADIENT,
						0x00,0x00,0x00,0x00,//X
						0x00,0x00,0x00,0x00,//Y
						cmd_TAIL};	
					double dX=0,dY=0;
					theApp->m_pDataController->GetInitGradientInfo(dX,dY);

					int nX = (int)(100*( dX ));
					int nY = (int)(100*( dY ));
					
					memcpy(Ret+2, &nX, sizeof(int));//4bit
					memcpy(Ret+6, &nY, sizeof(int));//4bit

					sendto(m_SockSrv, Ret, 11, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				}
				if (WAIT_OBJECT_0 == WaitForSingleObject(DAQCONTROLER::m_gEvtStillDetection,0))
				{
					char Ret[19] = {cmd_HEADER,cmd_STILL_DETECT,
						0x00,0x00,0x00,0x00,//handbrakeforce
						0x00,0x00,0x00,0x00,//X
						0x00,0x00,0x00,0x00,//Y
						0x00,0x00,0x00,0x00,
						cmd_TAIL};	

					STILLDETECTIONINFO stStillDetectionInfo;
					theApp->m_pDataController->GetStillDetectionInfo(stStillDetectionInfo);

					int nMaxHandBrakeForce = (int)(100*( stStillDetectionInfo.MaxHandBrakeForce ));
					int nGradientX = (int)(100*( stStillDetectionInfo.GradientX ));
					int nGradientY = (int)(100*( stStillDetectionInfo.GradientY ));
					int nPedalDist = (int)(100*( stStillDetectionInfo.PedalDistance ));

					g_logger.TraceWarning("sendData2UI-HandBrakeForce = %f,nPedalDist= %f",
						nMaxHandBrakeForce / 100.0,
						nPedalDist / 100.0);

					memcpy(Ret+2, &nMaxHandBrakeForce, sizeof(int));//4bit
					memcpy(Ret+6, &nGradientX, sizeof(int));//4bit
					memcpy(Ret+10, &nGradientY, sizeof(int));//4bit
					memcpy(Ret+14, &nPedalDist, sizeof(int));//4bit

					sendto(m_SockSrv, Ret, 19, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				}
				if (WAIT_OBJECT_0 == WaitForSingleObject(DAQCONTROLER::m_gEvtMoveDetection,0))
				{
					char Ret[23] = {cmd_HEADER,cmd_MOVE_DETECT,
						0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,
						cmd_TAIL};	

					MOVEDETECTIONINFO stStressInfo;
					theApp->m_pDataController->GetMoveDetectionInfo(stStressInfo);

					int nGradientX = (int)(100*( stStressInfo.GradientX ));
					int nGradientY = (int)(100*( stStressInfo.GradientY ));
					int nFootBrakeForce = (int)(100*(stStressInfo.MaxFootBrakeForce));
					int nLastVelocity = (int)(100*(stStressInfo.LastVelocity));
					int nLastAccelaration = (int)(100*(stStressInfo.LastAccelaration));

					g_logger.TraceWarning("sendData2UI-GradientX = %f,GradientY = %f,nLastVelocity=%f",
						nGradientX/100.0,
						nGradientY/100.0,
						nLastVelocity/100.0);

					memcpy(Ret+2, &nFootBrakeForce, sizeof(int));//4bit
					memcpy(Ret+6, &nGradientX, sizeof(int));//4bit
					memcpy(Ret+10, &nGradientY, sizeof(int));//4bit
					memcpy(Ret+14, &nLastVelocity, sizeof(int));//4bit
					memcpy(Ret+18, &nLastAccelaration, sizeof(int));//4bit
			
					sendto(m_SockSrv, Ret, 23, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				}

				break;
			}
		case msg_DAQ_NEWPROJECT:
			{
				char cMode = theApp->m_pDataController->GetMode();
				char cSucceed;
				switch (nParam)
				{
				case NUM_ZERO://success
					cSucceed = 0x00;
					break;
				case NUM_ONE:
					cSucceed = 0x01;
					break;
				}
				unsigned char cDataLen = strData2Send.length();//需要其长度不大于256，如果大于溢出了
				int nCmdLen = 6+cDataLen;
				char* pBuf = new char[nCmdLen];
				memset( pBuf, 0, nCmdLen );
				int nLoc=0;
				memcpy(pBuf+nLoc,&cmd_HEADER,1);
				nLoc+=1;
				memcpy(pBuf+nLoc,&cmd_NEWPROJECT, 1);
				nLoc+=1;
				memcpy(pBuf+nLoc,&cMode, 1);
				nLoc+=1;
				memcpy(pBuf+nLoc,&cSucceed, 1);
				nLoc+=1;
				memcpy(pBuf+nLoc,&cDataLen, 1);
				nLoc+=1;
				memcpy(pBuf+nLoc,strData2Send.c_str(),cDataLen);
				nLoc+=cDataLen;
				memcpy(pBuf+nLoc,&cmd_TAIL,1);

				sendto(m_SockSrv, pBuf, nCmdLen, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));

				delete[] pBuf;
				pBuf = NULL;

				break;
			}
		case msg_DAQ_TERMINATEPROJECT:
			{
				char Ret[3] = {cmd_HEADER,cmd_TERMINATEPROJECT,cmd_TAIL};

				sendto(m_SockSrv, Ret, 3, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));

				break;
			}
		case msg_ANA_ANALYSIS_STATE:
			{
				char Ret[5] = {cmd_HEADER,cmd_ANALYSIS_STATE,0x00,0x00,cmd_TAIL};


				switch (nParam)
				{
				case NUM_ONE://going
					Ret[2] = 0x01;
					break;
				case NUM_TWO://success
					Ret[2] = 0x02;
					break;
				case NUM_THREE://failed
					{
						int nInfoLen = strData2Send.length();
						g_logger.TraceWarning("CCommunicator::SendDatatoUI - %s",strData2Send.c_str());
						char cInfoLen = (char)nInfoLen;
						int nCmdLen = nInfoLen+5;
						char* pBuf= new char[nCmdLen];
						memset(pBuf,0,nInfoLen+5);
						Ret[2] = 0x03;
						Ret[3] = cInfoLen;
						memcpy(pBuf, Ret, 4);
						memcpy(pBuf+4,strData2Send.c_str(),nInfoLen);
						memcpy(pBuf+4+nInfoLen,&cmd_TAIL,1);
						sendto(m_SockSrv, pBuf, nCmdLen, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
						delete[] pBuf;
						pBuf = NULL;

						break;
					}
				}
				if (Cmd != NUM_THREE)
				{
					sendto(m_SockSrv, Ret, 5, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
				}
				break;
			}

		}

		return true;
	}

	bool CCommunicator::SendAnalysisResult2UI(const int nResult, const ANALYSISRESULT& stResult)
	{
		g_logger.TraceInfo("CCommunicator::SendAnalysisResult2UI-in");
		char Ret[36] = {cmd_HEADER,cmd_ANALYSIS_RESULT,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,cmd_TAIL};

		//int nAcceleration = (int)(100*( stVelocityInfo.LastAccelaration ));

		//ANALYSISRESULT stResult;
		//memcpy(&stResult, strData2Send.c_str(), sizeof(ANALYSISRESULT));

		ANALYSISRESULT_INT stResultInt;
		stResultInt.MeanDragAccelaration = abs( (int)(100*stResult.MeanDragAccelaration) );
		stResultInt.BrakeLength = abs( (int)(100*stResult.BrakeLength) );
		stResultInt.InitBrakeVelocity = abs( (int)(100*stResult.InitBrakeVelocity) );
		stResultInt.GradientX = abs( (int)(100*stResult.GradientX) );
		stResultInt.GradientY = abs( (int)(100*stResult.GradientY) );
		//int nTmp = stResult.GradientY;
		//stResultInt.GradientY = (int)(100*stResult.GradientX);
		//stResultInt.GradientX = (int)(100*nTmp);
		stResultInt.PedalDistance = abs( (int)(100*stResult.PedalDistance) );
		stResultInt.MaxHandBrakeForce = abs( (int)(100*stResult.MaxHandBrakeForce) );
		stResultInt.MaxFootBrakeForce = abs( (int)(100*stResult.MaxFootBrakeForce) );

		memcpy( (Ret+2), &(stResultInt.MeanDragAccelaration), sizeof(int) );
		memcpy( (Ret+6), &(stResultInt.BrakeLength), sizeof(int) );
		memcpy( (Ret+10), &(stResultInt.InitBrakeVelocity), sizeof(int) );
		memcpy( (Ret+14), &(stResultInt.GradientX), sizeof(int) );
		memcpy( (Ret+18), &(stResultInt.GradientY), sizeof(int) );
		memcpy( (Ret+22), &(stResultInt.PedalDistance), sizeof(int) );
		memcpy( (Ret+26), &(stResultInt.MaxHandBrakeForce), sizeof(int) );
		memcpy( (Ret+30), &(stResultInt.MaxFootBrakeForce), sizeof(int) );

		//0x01 完全合格
		//0x02 局部损坏
		//0x03 损坏严重
		//0x04 不合格
		switch (nResult)
		{
		case NUM_ONE:
			Ret[34] = 0x01;
			break;
		case NUM_TWO:
			Ret[34] = 0x02;
			break;
		case NUM_THREE:
			Ret[34] = 0x03;
			break;
		case NUM_FOUR:
			Ret[34] = 0x04;
			break;
		default:
			Ret[34] = 0x05;
			g_logger.TraceError("CCommunicator::SendDatatoUI - msg_ANA_ANALYSIS_RESULT error");
		}

		sendto(m_SockSrv, Ret, 36, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));

		g_logger.TraceInfo("CCommunicator::SendAnalysisResult2UI-out");
		return true;
	}

	bool CCommunicator::SendAnalysisData2UI(vector<ANALYSISDATA>& stData)
	{
		g_logger.TraceInfo("CCommunicator::SendAnalysisData2UI-in");
		//一个点数包括一个加速度+一个速度+一个脚刹位置+一个脚刹力
		WORD wPointNumber = stData.size();//2byte
		int nPointPerPack = 250;
		char cPackNumber = 0x00;
		cPackNumber = wPointNumber / nPointPerPack;
		if (0 != wPointNumber % nPointPerPack)
		{
			cPackNumber ++;
		}
		if (cPackNumber > 256)
		{
			g_logger.TraceError("CCommunicator::SendAnalysisData2UI - cPackNumber = %d > 256, wPoitNumber=%d ",cPackNumber,wPointNumber);
			cPackNumber = 256;
			wPointNumber = cPackNumber*nPointPerPack;
		}
		int nSizeofSend = 7 + 4*nPointPerPack + 1;
		char* pBuf = new char[nSizeofSend];
		int nSendTime = 0;
		for (char cCurPack=1;cCurPack<=cPackNumber;cCurPack++)
		{
			char cAcc = 0x00;
			char cVel = 0x00;
			char cPedalLoc = 0x00;
			char cForce = 0x00;

			memset(pBuf,'\0',nSizeofSend);

			int nLoc = 0;
			memcpy(pBuf+nLoc, &cmd_HEADER, 1);
			nLoc ++;
			memcpy(pBuf+nLoc, &cmd_ANALYSIS_DATA, 1);
			nLoc ++;
			memcpy(pBuf+nLoc, &wPointNumber, 2);
			nLoc += 2;
			memcpy(pBuf+nLoc, &cPackNumber, 1);
			nLoc++;
			memcpy(pBuf+nLoc, &cCurPack, 1);
			nLoc++;
			char cPointCurPack = 0x00;
			memcpy(pBuf+nLoc, &cPointCurPack, 1);//点数，在最后一包的时候是不准的，解包时需要根据实际总点数计算出来
			nLoc++;
			int nPoint = -1;
			while( ++nPoint < nPointPerPack )
			{
				int nCurLocInVerctor = (cCurPack-1)*nPointPerPack +  nPoint;
				if ( nCurLocInVerctor > wPointNumber -1 )
				{
					break;//导致地址最后几个数为0
				}
				ANALYSISDATA stGrop = stData.at( nCurLocInVerctor );
				cAcc = (int)stGrop.Accelaration;
				cVel = (int)stGrop.Velocity;
				cPedalLoc = (int)stGrop.PedalDistance;
				cForce = (int)stGrop.FootBrakeForce;
				//g_logger.TraceInfo("CCommunicator::SendAnalysisData2UI - data to send:%d\t%d\t%d\t%d",
				//	cAcc,cVel,cPedalLoc,cForce);

				memcpy(pBuf+nLoc, &cAcc, 1);
				nLoc++;
				memcpy(pBuf+nLoc, &cVel, 1);
				nLoc++;
				memcpy(pBuf+nLoc, &cPedalLoc, 1);
				nLoc++;
				memcpy(pBuf+nLoc, &cForce, 1);
				nLoc++;

				++cPointCurPack;
			}
			memcpy(pBuf+6, &cPointCurPack, 1 );//实际点数
			memcpy(pBuf+nSizeofSend-1,&cmd_TAIL, 1);

			//send
			nSendTime++;
			int nLenSend = sendto(m_SockSrv, pBuf, nSizeofSend, 0, (SOCKADDR*)&m_addrClient,sizeof(SOCKADDR));
			//Sleep(5);
			if ( nLenSend == SOCKET_ERROR)
			{
				g_logger.TraceError("CCommunicator::SendAnalysisData2UI - sendto return SOCKET_ERROR,error code:%d ",WSAGetLastError());
			}
			else if( nLenSend != nSizeofSend )
			{
				g_logger.TraceError("CCommunicator::SendAnalysisData2UI - sendto length not match:send=%d,sent=%d",nSizeofSend,nLenSend);
			}
			else
			{
				g_logger.TraceWarning("CCommunicator::SendAnalysisData2UI - send success,sentLen=%d,sendTime=%d",nLenSend,nSendTime);
				g_logger.TraceWarning("CCommunicator::SendAnalysisData2UI - send success,curPack=%d",cPointCurPack);
			}
			Sleep(50);
		}

		
		delete [] pBuf;
		pBuf = NULL;

		//发送
		g_logger.TraceInfo("CCommunicator::SendAnalysisData2UI-out");
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

		char pBuf[500]={0};//max byte no. 100
		SOCKADDR_IN addrClient;

		int nLenRev = recvfrom(m_SockSrv,pBuf,100,0,(SOCKADDR*)&addrClient,&nAddrBufLen);
		if (nLenRev == 0)
		{
			g_logger.TraceError("CCommunicator::ParseRevData - connection has teminated");
			return false;
		}
		else if ( SOCKET_ERROR == nLenRev )
		{
			g_logger.TraceError("CCommunicator::ParseRevData - recvfrom return SOCKET_ERROR,error code:%d ",WSAGetLastError());
			return false;
		}
		else
		{
			//g_logger.TraceInfo("CCommunicator::ParseRevData - recvfrom %d length data",nLenRev);
			if( !ParseData(pBuf) )
			{
				g_logger.TraceError("CCommunicator::ParseRevData - recv data parse error");
			}
		}

		if (snFirstTime)
		{
			m_addrClient = addrClient;
			snFirstTime = false;
		}

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
		//g_logger.TraceInfo("CCommunicator::ParseData:cmd=0x%x",cb);
		switch (cb)
		{
		case cmd_ASD:
			this->cmdASD(pData);
			break;
		case cmd_USERLOGIN:
			this->cmdUserLogin(pData);
			break;
		case cmd_USERREGISTER:
			this->cmdUserRegister(pData);
			break;
		case cmd_MODIFYPWD:
			//MessageBeep(MB_OK);

			this->cmdModifyPwd(pData);
			break;
		case cmd_USERDELETE:
			this->cmdUserDelete(pData);
			break;
		case cmd_ADMINUSER:
			this->cmdAdminUser(pData);
			break;
		case cmd_NEWPROJECT:
			this->cmdNewProject(pData);
			break;
		case cmd_TERMINATEPROJECT:
			this->cmdTerminateProject( );
			break;
		case cmd_INITGRADIENT_BEGIN:
			this->cmdInitGradientBegin(pData);
			this->cmdStillDetectionBegin(pData);
			break;
		case cmd_INITGRADIENT_END:
			this->cmdInitGradientEnd(pData);
			this->cmdStillDetectionEnd(pData);
			break;
		//case cmd_STILL_DETECT_BEGIN:
		//	this->cmdStillDetectionBegin(pData);
		//	break;
		//case cmd_STILL_DETECT_END:
		//	this->cmdStillDetectionEnd(pData);
		//	break;
		//case cmd_MOVE_DETECT_BEGIN:
		//	this->cmdMoveDetectionBegin(pData);
		//	break;
		//case cmd_MOVE_DETECT_END:
		//	this->cmdMoveDetectionEnd(pData);
		//	break;
		case cmd_BEGIN_DETECT:
			this->cmdMoveDetectionBegin(pData);
			break;
		case cmd_END_DETECT:
			this->cmdMoveDetectionEnd(pData);
			break;
		case cmd_REPORTPATH:
			this->cmdSetReportPath(pData);
			break;
		case cmd_ANALYSIS_BEGIN:
			this->cmdAnalysisBegin(pData);
			break;
		case cmd_HEARTBEAT:
			this->cmdHeartBeatSignal(pData);
			break;
		case cmd_QUIT:
			this->cmdQuit();
		default:
			g_logger.TraceError("CCommunicator::ParseData:cmd false,cmd=%x",cb);
			break;
		}

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

	bool CCommunicator::cmdASD(const char* pData)
	{
		g_logger.TraceWarning("CCommunicator::cmdASD");
		if ( !PostThreadMessage(m_dwMainThreadId,msg_DAQ_ASD,NULL,NULL ))
		{
			g_logger.TraceError("CCommunicator::cmdASD");
			return false;
		}
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
	bool CCommunicator::cmdAdminUser(const char* pData )
	{
		g_logger.TraceWarning("CCommunicator::cmdAdminUser");
		char* pUserName = NULL;
		char* pPwd = NULL;
		cmdParseUserInfo(pData,pUserName,pPwd);

		if ( !PostThreadMessage(m_dwMainThreadId,msg_DB_ADMINUSER,(WPARAM)pUserName,(LPARAM)pPwd) )
		{
			delete [] pUserName;
			delete [] pPwd;
			pUserName = pPwd = NULL;
		}

		return true;
	}

	bool CCommunicator::cmdNewProject(const char* pData )
	{
		theApp->m_pDataController->SetNewProjectPara(pData);
		PostThreadMessage(m_dwMainThreadId, msg_DAQ_NEWPROJECT, NULL, NULL); 

		return true;
	}
	bool CCommunicator::cmdTerminateProject( )
	{
		PostThreadMessage(m_dwMainThreadId, msg_DAQ_TERMINATEPROJECT, NULL, NULL); 
		return true;
	}
	bool CCommunicator::cmdInitGradientBegin(const char* pData )
	{
		g_logger.TraceInfo("CCommunicator::cmdInitGradientBegin");

		theApp->m_pDataController->SetCurrentType(INITGRADIENT);
		theApp->m_pDAQController->InitGradientBegin();
		return true;
	}
	bool CCommunicator::cmdInitGradientEnd(const char* pData )
	{
		g_logger.TraceInfo("CCommunicator::cmdInitGradientEnd");

		//Sleep(5);//为了防止init angle第一次运行不能存文件
		theApp->m_pDAQController->InitGradientEnd();
		theApp->m_pDataController->SaveInitValue2INI();
		theApp->m_pDataController->SetInitHandForceFlag();
		theApp->m_pDataController->SetInitAngleFlag();
		return true;
	}
	bool CCommunicator::cmdStillDetectionBegin(const char* pData )
	{
		g_logger.TraceInfo("CCommunicator::cmdStillDetectionBegin");

		theApp->m_pDataController->SetCurrentType(STILLDETECTION);
		theApp->m_pDAQController->StillDetectionBegin();
		theApp->m_pDataController->SetGetInitPedalDist();
		return true;
	}
	bool CCommunicator::cmdStillDetectionEnd(const char* pData )
	{
		g_logger.TraceInfo("CCommunicator::cmdStillDetectionEnd");

		theApp->m_pDAQController->StillDetectionEnd();
		theApp->m_pDataController->SaveMaxHandBrakeForce2INI();//静止检测结束时保存到INI
		return true;
	}

	////temp by fx for test20180314
	//void CALLBACK TimerProc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime)
	void CALLBACK TimerProc(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
	{
		//MessageBox(NULL, L"", L"",MB_OK);
		theApp->m_pCommunicator->SendDatatoUI(msg_DAQ_DATAONE);
	}
	//动态测试，需要时也担任发送模拟数据的作用。
	bool CCommunicator::cmdMoveDetectionBegin(const char* pData )
	{
		g_logger.TraceInfo("CCommunicator::cmdMoveDetectionBegin");

		if (TESTDATA)
		{
			//temp by fx for test20180314;for send simulate data to client
			if (nTimer == 0)
			{
				nTimer = timeSetEvent( 100,0, TimerProc, 0, (UINT)TIME_PERIODIC);
				if (nTimer == 0)
				{
					g_logger.TraceError("set timer failed");
					return false;
				}			
			}

		}
		else
		{
			theApp->m_pDataController->SetCurrentType(MOVEDETECTION);
			theApp->m_pDataController->SetGetInitFootBrakeForce();
			theApp->m_pDAQController->MoveDetectionBegin();
		}

		return true;
	}

	bool CCommunicator::cmdMoveDetectionEnd(const char* pData )
	{
		g_logger.TraceInfo("CCommunicator::cmdMoveDetectionEnd");

		if (TESTDATA)
		{
			//temp by fx for test20180314
			timeKillEvent(nTimer);
			nTimer =0;	
		}
		else
		{
			theApp->m_pDAQController->MoveDetectionEnd();
			theApp->m_pDataController->SaveMaxPedalDistance2INI();//结束检测时保存到INI
		}

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

		if ( !PostThreadMessage(m_dwMainThreadId,msg_DATA_SETREPORTPATH,(WPARAM)pPath,NULL ) )
		{
			delete[] pPath;
			pPath = NULL;
		}


		return true;
	}
	bool CCommunicator::cmdAnalysisBegin(const char* pData )
	{
		int nLoc = 2;
		char cCurrentTest = 0x00;
		memcpy(&cCurrentTest,pData+nLoc,1);
		string strMsgInfo("");
		char *pFP = NULL;
		if (0x02 == cCurrentTest)//指定历史检测
		{
			++nLoc;
			int nFilePathLen=0;
			memcpy(&nFilePathLen,pData+nLoc,1);
			++nLoc;
			pFP = new char[nFilePathLen+1];
			memset(pFP,0,nFilePathLen+1);
			memcpy(pFP, pData+nLoc,nFilePathLen);
			//strMsgInfo = pFP;

		}

		if ( !PostThreadMessage(m_dwMainThreadId, msg_ANA_ANALYSIS_BEGIN, (WPARAM)cCurrentTest, (LPARAM)pFP ) )
		{
			delete[] pFP;
			pFP = NULL;	
			g_logger.TraceError("CCommunicator::cmdAnalysisBegin - PostThreadMessage failed");
		}

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
	bool CCommunicator::cmdQuit()
	{
		PostThreadMessage(m_dwMainThreadId, msg_MAIN_QIUT, NULL, NULL );
		return true;
	}

}
