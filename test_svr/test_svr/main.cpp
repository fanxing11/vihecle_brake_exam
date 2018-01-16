#include "main.h"

CLogger g_logger(LOGGER::LogLevel_Info,CLogger::GetAppPathA().append("log\\"));

CtheApp::CtheApp(void)
{
	m_pCommunicator = new CCommunicator;
	m_pDBC = new CDBController;
	m_pDataC = new CDataControler;
	m_pDAQC = new CDAQControler;
	
}


CtheApp::~CtheApp(void)
{
	delete m_pCommunicator;
	delete m_pDBC;
	delete m_pDataC;
	delete m_pDAQC;
}

CtheApp theApp;

int main()
{

	theApp.m_dwMainThreadID = GetCurrentThreadId();

	g_logger.TraceWarning("_func_in_main");

	g_logger.ChangeLogLevel(LOGGER::LogLevel_Info);

	cout<<"this is first line"<<endl;


	MSG msg;
	bool bRet = false;
	while(GetMessage(&msg,NULL,0,0))
	{
		cout<<"get one msg"<<endl;

		switch (msg.message)
		{
		case msg_DB_USERLOGIN:
			{
				char* pUserName = (char*)msg.wParam;
				char* pPwd = (char*)msg.lParam;
				g_logger.TraceWarning("in main::msg_DB_USERLOGIN,UserName=%s,Pwd=%s",pUserName,pPwd);

				int nErr = NUM_NEGONE;
				if( !theApp.m_pDBC->UserLogin(nErr,string(pUserName),string(pPwd)) )
				{
					g_logger.TraceError("DBC.UserLogin error");
				}
				else
				{
					theApp.m_pCommunicator->SendDatatoUI(msg.message,nErr);
				}
				delete [] pUserName;pUserName = NULL;
				delete [] pPwd;pPwd = NULL;
				break;
			}

		case msg_DB_USERREGISTER:
			{
				char* pUserName = (char*)msg.wParam;
				char* pPwd = (char*)msg.lParam;
				g_logger.TraceWarning("in main::msg_DB_USERLOGIN,UserName=%s,Pwd=%s",pUserName,pPwd);

				int nErr = NUM_NEGONE;
				if( !theApp.m_pDBC->UserRegister(nErr,string(pUserName),string(pPwd)) )
				{
					g_logger.TraceError("DBC.UserRegister error");
				}
				else
				{
					theApp.m_pCommunicator->SendDatatoUI(msg.message,nErr);
				}
				delete [] pUserName;pUserName = NULL;
				delete [] pPwd;pPwd = NULL;
				break;
			}
		case msg_DB_MODIFYPASSWORD:
			{
				char* pUserName = (char*)msg.wParam;
				char* pPwd = (char*)msg.lParam;
				g_logger.TraceWarning("in main::msg_DB_CHANGEPASSWORD,UserName=%s,Pwd=%s",pUserName,pPwd);

				if( !theApp.m_pDBC->ModifyPwd(string(pUserName),string(pPwd)) )
				{
					g_logger.TraceError("DBC.ModifyPwd error");
					theApp.m_pCommunicator->SendDatatoUI(msg.message,NUM_ONE);
				}
				else
				{
					theApp.m_pCommunicator->SendDatatoUI(msg.message);
				}
				delete [] pUserName;pUserName = NULL;
				delete [] pPwd;pPwd = NULL;
				break;
			}
		case msg_DB_DELETEUSER:
			{
				char* pUserName = (char*)msg.wParam;
				char* pPwd = (char*)msg.lParam;
				g_logger.TraceWarning("in main::msg_DB_DELETEUSER,UserName=%s,Pwd=%s",pUserName,pPwd);

				if( !theApp.m_pDBC->DeleteUser(string(pUserName),string(pPwd)) )
				{
					g_logger.TraceError("DBC.DeleteUser failed");
					theApp.m_pCommunicator->SendDatatoUI(msg.message,NUM_ONE);
				}
				else
				{
					theApp.m_pCommunicator->SendDatatoUI(msg.message);
				}
				delete [] pUserName;pUserName = NULL;
				delete [] pPwd;pPwd = NULL;
				break;
			}
		case msg_DAQ_DATAONE:
			{
				g_logger.TraceInfo("in main::msg_DAQ_DATAONE");
				theApp.m_pCommunicator->SendDatatoUI(msg.message);

				break;
			}
		}

	}


	cout<<"this is last line"<<endl;

	g_logger.TraceWarning("_fun_cout_main");
	return 0;
}