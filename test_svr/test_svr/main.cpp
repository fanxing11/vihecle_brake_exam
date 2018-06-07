#include "main.h"

CLogger g_logger(LOGGER::LogLevel_Info,CLogger::GetAppPathA().append("log\\"));

CtheApp::CtheApp(void)
{
	m_pCommunicator = new CCommunicator;
	m_pDBC = new CDBController;
	m_pDataController = new CDataControler;
	m_pDAQController = new CDAQControler;
	m_pAnalysis = new CAnalysis;
}


CtheApp::~CtheApp(void)
{
	delete m_pCommunicator;
	delete m_pDBC;
	delete m_pDataController;
	delete m_pDAQController;
}

CtheApp* theApp = NULL;

//int main()
int APIENTRY WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd )
{
	g_logger.TraceWarning("_func_in_main");

	theApp = new CtheApp;
	if (!theApp->m_pCommunicator->Initialize())
	{
		MessageBox(NULL,TEXT("server startup failed."),NULL,MB_OK);
		return 0;
	}

	theApp->m_dwMainThreadID = GetCurrentThreadId();


	g_logger.ChangeLogLevel(LOGGER::LogLevel_Info);

	cout<<"this is first line"<<endl;


	MSG msg;
	bool bRet = false;
	while(GetMessage(&msg,NULL,0,0))
	{
		cout<<"get one msg"<<endl;
		if (msg.message == msg_MAIN_QIUT)
		{
			break;
		}
		switch (msg.message)
		{
		case msg_DAQ_ASD:
			{
				int nErr = NUM_NEGONE;
				if ( theApp->m_pDAQController->CheckDAQStarted() )
				{
					nErr = NUM_ZERO;
				}
				else
				{
					nErr = NUM_ONE;
				}
				theApp->m_pCommunicator->SendDatatoUI(msg.message,nErr);

				break;
			}
		case msg_DB_USERLOGIN:
			{
				char* pUserName = (char*)msg.wParam;
				char* pPwd = (char*)msg.lParam;
				g_logger.TraceWarning("in main::msg_DB_USERLOGIN,UserName=%s,Pwd=%s",pUserName,"pPwd");

				int nErr = NUM_NEGONE;
				if( !theApp->m_pDBC->UserLogin(nErr,string(pUserName),string(pPwd)) )
				{
					g_logger.TraceError("DBC.UserLogin error");
				}
				else
				{
					theApp->m_pCommunicator->SendDatatoUI(msg.message,nErr);
				}
				delete [] pUserName;pUserName = NULL;
				delete [] pPwd;pPwd = NULL;
				break;
			}

		case msg_DB_USERREGISTER:
			{
				char* pUserName = (char*)msg.wParam;
				char* pPwd = (char*)msg.lParam;
				g_logger.TraceWarning("in main::msg_DB_USERLOGIN,UserName=%s,Pwd=%s",pUserName,"pPwd");

				int nErr = NUM_NEGONE;
				if( !theApp->m_pDBC->UserRegister(nErr,string(pUserName),string(pPwd)) )
				{
					g_logger.TraceError("DBC.UserRegister error");
				}
				else
				{
					theApp->m_pCommunicator->SendDatatoUI(msg.message,nErr);
				}
				delete [] pUserName;
				pUserName = NULL;
				delete [] pPwd;
				pPwd = NULL;
				break;
			}
		case msg_DB_MODIFYPASSWORD:
			{
				char* pUserName = (char*)msg.wParam;
				char* pPwd = (char*)msg.lParam;
				g_logger.TraceWarning("in main::msg_DB_CHANGEPASSWORD,UserName=%s,Pwd=%s",pUserName,"pPwd");

				if( !theApp->m_pDBC->ModifyPwd(string(pUserName),string(pPwd)) )
				{
					g_logger.TraceError("DBC.ModifyPwd error");
					theApp->m_pCommunicator->SendDatatoUI(msg.message,NUM_ONE);
				}
				else
				{
					theApp->m_pCommunicator->SendDatatoUI(msg.message);
				}
				delete [] pUserName;pUserName = NULL;
				delete [] pPwd;pPwd = NULL;
				break;
			}
		case msg_DB_DELETEUSER:
			{
				char* pUserName = (char*)msg.wParam;
				char* pPwd = (char*)msg.lParam;
				g_logger.TraceWarning("in main::msg_DB_DELETEUSER,UserName=%s,Pwd=%s",pUserName,"pPwd");

				if( !theApp->m_pDBC->DeleteUser(string(pUserName),string(pPwd)) )
				{
					g_logger.TraceError("DBC.DeleteUser failed");
					theApp->m_pCommunicator->SendDatatoUI(msg.message,NUM_ONE);
				}
				else
				{
					theApp->m_pCommunicator->SendDatatoUI(msg.message);
				}
				delete [] pUserName;pUserName = NULL;
				delete [] pPwd;pPwd = NULL;
				break;
			}
		case msg_DB_ADMINUSER:
			{
				char* pUserName = (char*)msg.wParam;
				char* pPwd = (char*)msg.lParam;
				g_logger.TraceWarning("in main::msg_DB_USERLOGIN,UserName=%s,Pwd=%s",pUserName,"pPwd");

				int nErr = NUM_NEGONE;
				if( !theApp->m_pDBC->AdminUserVerify(nErr,string(pUserName),string(pPwd)) )
				{
					g_logger.TraceError("DBC.AdminUserVerify error");
				}
				else
				{
					theApp->m_pCommunicator->SendDatatoUI(msg.message,nErr);
				}
				delete [] pUserName;pUserName = NULL;
				delete [] pPwd;pPwd = NULL;
				break;
			}
		case msg_DAQ_DATAONE:
			{
				g_logger.TraceInfo("in main::msg_DAQ_DATAONE");
				theApp->m_pCommunicator->SendDatatoUI(msg.message);

				break;
			}
		case msg_DAQ_NEWPROJECT:
			{
				string strInfo("");
				bool bRet = theApp->m_pDataController->NewProject( strInfo ) ;

				if ( bRet )//success
				{
					theApp->m_pCommunicator->SendDatatoUI(msg.message, NUM_ZERO, strInfo );
				}
				else
				{
					theApp->m_pCommunicator->SendDatatoUI(msg.message, NUM_ONE, strInfo);
				}
				break;
			}
		case msg_DAQ_TERMINATEPROJECT:
			{
				theApp->m_pDataController->TerminateCurrentProject();
				theApp->m_pCommunicator->SendDatatoUI(msg.message,NUM_ZERO,"");

				break;
			}
		case msg_DATA_SETREPORTPATH:
			{
				char* pPath = (char*)msg.wParam;
				string strPath(pPath);
				theApp->m_pDataController->SetReportPath(strPath);
				delete[] pPath;
				pPath = NULL;

				break;
			}
		case msg_ANA_ANALYSIS_BEGIN:
			{
				char cCurrentTest = (char)msg.wParam;
				char* pFilePath = (char*)msg.lParam;
				string strProjPath("");
				if (pFilePath != NULL)
				{
					strProjPath = pFilePath;
					delete[] pFilePath;
					pFilePath = NULL;
				}

				string strRetInfo("");

				if (cCurrentTest == 0x01)//current test
				{
					if ( NUM_TWO == theApp->m_pDataController->GetCurrentProjectState() )
					{
						theApp->m_pDataController->GetProjectPath(strProjPath);
						theApp->m_pAnalysis->BeginAnalysis(strProjPath);
					}
					else
					{

						if ( NUM_ONE == theApp->m_pDataController->GetCurrentProjectState() )
						{
							g_logger.TraceWarning("msg_ANA_ANALYSIS_BEGIN:CurrentProject is onGoing");
							strRetInfo = ("当前检测正在进行");
						}
						else if ( NUM_ZERO == theApp->m_pDataController->GetCurrentProjectState() )
						{
							g_logger.TraceWarning("msg_ANA_ANALYSIS_BEGIN:CurrentProject has not started");
							strRetInfo = ("当前检测尚未开始");
						}
						char* pBuf = NULL;
						int nLen = strRetInfo.length();
						pBuf = new char[nLen+1];
						memset(pBuf,0,nLen+1);
						memcpy(pBuf,strRetInfo.c_str(),nLen);
						if(!PostThreadMessage(theApp->m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_THREE,(LPARAM)pBuf))
						{
							g_logger.TraceError("main::msg_ANA_ANALYSIS_BEGIN - PostThreadMessage failed");
							delete[] pBuf;
							pBuf = NULL;
						}
					}
				}
				else if (cCurrentTest == 0x02)//history test
				{
					if ( NUM_ONE == theApp->m_pDataController->GetCurrentProjectState() )
					{
						g_logger.TraceWarning("msg_ANA_ANALYSIS_BEGIN:CurrentProject is onGoing");
						strRetInfo = ("检测正在进行,请等待检测结束后进行分析");

						char* pBuf = NULL;
						int nLen = strRetInfo.length();
						pBuf = new char[nLen+1];
						memset(pBuf,0,nLen+1);
						memcpy(pBuf,strRetInfo.c_str(),nLen);
						if(!PostThreadMessage(theApp->m_dwMainThreadID,msg_ANA_ANALYSIS_STATE,NUM_THREE,(LPARAM)pBuf))
						{
							g_logger.TraceError("main::msg_ANA_ANALYSIS_BEGIN - PostThreadMessage failed");
							delete[] pBuf;
							pBuf = NULL;
						}
					}
					else
					{
						theApp->m_pAnalysis->BeginAnalysis(strProjPath);
					}
				}
				else
				{
					g_logger.TraceError("msg_ANA_ANALYSIS_BEGIN:cCurrentTest != 0x01|0x02");
				}
				break;
			}
		case msg_ANA_ANALYSIS_STATE:
			{
				char* pp = (char*)msg.lParam;
				string strInfo("");
				if (NULL != pp)
				{
					strInfo = pp;
					delete[] pp;
					pp = NULL;
				}
				theApp->m_pCommunicator->SendDatatoUI(msg.message,msg.wParam,strInfo);
				break;
			}
		case msg_ANA_ANALYSIS_RESULT:
			{
				ANALYSISRESULT stResult;
				stResult = *(ANALYSISRESULT*)msg.lParam;
				theApp->m_pCommunicator->SendAnalysisResult2UI((int)msg.wParam, stResult);
				break;
			}
		case  cmd_ANALYSIS_DATA:
			{
				//ANALYSISDATA stData;
				//stData = *(ANALYSISDATA*)msg.lParam;
				//theApp->m_pCommunicator->SendAnalysisData2UI((int)msg.wParam, stData);
				break;
			}
		}

	}
	delete theApp;
	theApp = NULL;

	cout<<"this is last line"<<endl;

	g_logger.TraceWarning("_fun_cout_main");
	return 0;
}