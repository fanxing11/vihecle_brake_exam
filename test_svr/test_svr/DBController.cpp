//#include "StdAfx.h"
#include "DBController.h"

#include "const.h"


//只有函数调用出错的时候返回false
//其他状态，都返回true
namespace DBCONTROLLER
{

	CDBController::CDBController(void)
		:m_sDBName("carexamine.db")
		,m_pstdb(NULL)
	{
		g_logger.TraceInfo("CDBController::CDBController");
	}

	CDBController::~CDBController(void)
	{
		g_logger.TraceWarning("CDBController::~CDBController");
	}

	bool CDBController::SetDBName(const string& csName)
	{
		m_sDBName = csName;
		return true;
	}
	//@out: nErr
	//level=0:管理员用户 level=1:普通用户
	//nErr=0:good;
	//nErr=1,user name repeated
	//nErr=-1,other error
	bool CDBController::UserRegister(int& nErr, const string& sUserName, const string& sPwd, const int level)
	{
		printf("AddUser\n");
		nErr=-1;
		int nTargetState=0;
		vector <string> vNames;
		GetAllUserNames(vNames,sUserName,nTargetState);
		if (nTargetState == 1)//existed
		{
			nErr = 1;
			return true;
		}

		g_uCmd = DB_ADDUSER;
		bool rt = true;

		if (!OpenDB())
		{
			rt = false;
		}
		stringstream stream;
		string sLevel;
		stream<<level;
		stream>>sLevel;

		string s_sql("insert into userInfo (userName,pwd,level) values(\"");
		s_sql.append(sUserName);
		s_sql.append("\",\"");
		s_sql.append(sPwd);
		s_sql.append("\",");
		s_sql.append(sLevel);
		s_sql.append(")");

		int rc;
		char *zErrMsg = 0;
		rc = sqlite3_exec(m_pstdb, s_sql.c_str(), callback, NULL/*"userName"*/, &zErrMsg);
		if( rc != SQLITE_OK ){
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			rt = false;
		}else{
			nErr=0;
			printf("SQL execute successfully\n");
		}

		CloseDB();

		return rt;
	}
	bool CDBController::DeleteUser(const string& sUserName, const string& sPwd)
	{
		printf("DeleteUser\n");

		// if the user and pwd right
		int nErr=-1;
		UserLogin(nErr,sUserName,sPwd);
		if (nErr !=0)
		{
			return false;//user name or pwd wrong
		}

		//delete from userInfo where userName='qq'
		g_uCmd = DB_DELETEUSER;
		bool rt = true;

		if (!OpenDB())
		{
			rt = false;
		}
		stringstream ss;

		string s_sql("delete from userInfo where userName=\"");
		s_sql.append(sUserName);
		s_sql.append("\" and level=\"");
		s_sql.append("1");
		s_sql.append("\"");

		int rc;
		char *zErrMsg = 0;
		rc = sqlite3_exec(m_pstdb, s_sql.c_str(), callback, "", &zErrMsg);
		if( rc != SQLITE_OK ){
			fprintf(stderr, "DeleteUser:SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			rt = false;
		}else{
			printf("DeleteUser:SQL execute successfully\n");
		}
		CloseDB();

		return rt;

	}
	bool CDBController::ModifyPwd(const string& sUserName, const string& csPwd,const int level)
	{
		printf("ModifyPwd\n");

		//update userInfo set pwd=2222 where userName='qq'
		g_uCmd = DB_MODIFYPWD;
		bool rt = true;

		if (!OpenDB())
		{
			rt = false;
		}
		stringstream ss;

		string s_sql("update userInfo set pwd=");
		s_sql.append(csPwd);
		s_sql.append(" where userName=\"");
		s_sql.append(sUserName);
		s_sql.append("\"");

		int rc;
		char *zErrMsg = 0;
		rc = sqlite3_exec(m_pstdb, s_sql.c_str(), callback, "", &zErrMsg);
		if( rc != SQLITE_OK ){
			fprintf(stderr, "ModifyPwd:SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			rt = false;
		}else{
			printf("ModifyPwd:SQL execute successfully\n");
		}
		CloseDB();

		return rt;
	}

	/*
	vUsers - user namelist
	sTargetUserName - targetusername
	nTargetState=2 - target user not exist
	nTargetState=1 - target user existed
	*/
	bool CDBController::GetAllUserNames(vector<string>& vUsers,const string& sTargetUserName, int& nTargetState)
	{
		g_vUserName.clear();
		g_uCmd = DB_GETALLUSERS;
		bool rt = true;

		if (!OpenDB())
		{
			rt = false;
		}
		char* sql;
		sql = "SELECT * from nameview";
		int rc;
		char *zErrMsg = 0;
		rc = sqlite3_exec(m_pstdb, sql, callback, "userName", &zErrMsg);
		if( rc != SQLITE_OK ){
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			rt = false;
		}else{
			printf("GetAllUsers:SQL execute successfully\n");
		}
		vUsers = g_vUserName;
		CloseDB();

		// for the target User name state
		nTargetState=2;
		if (sTargetUserName != "")
		{
			vector<string>::iterator it;
			for (it=g_vUserName.begin();it!=g_vUserName.end();it++)
			{
				if (*it == sTargetUserName)
				{
					nTargetState=1;
					//printf("this name existed");
				}
			}
		}

		return rt;
	}
	//in@
	//nErr=0,good;
	//nErr=1,pwd error
	//nErr=2,target user not exist
	bool CDBController::UserLogin(int& nErr, const string& sUserName, const string& sPwd)
	{
		printf("VerifyUserPwd\n");
		nErr=-1;
		int nTargetState=0;
		vector <string> vNames;
		GetAllUserNames(vNames,sUserName,nTargetState);
		if (nTargetState == 2)//not exist
		{
			nErr = 2;
			return true;
		}

		g_uCmd = DB_VERRIFYUSERPWD;
		bool rt = true;

		if (!OpenDB())
		{
			rt = false;
		}
		string s_sql("SELECT pwd from userInfo where userName=\"");
		s_sql.append(sUserName);
		s_sql.append("\"");

		int rc;
		char *zErrMsg = 0;
		rc = sqlite3_exec(m_pstdb, s_sql.c_str(), callback, "userName", &zErrMsg);
		if( rc != SQLITE_OK ){
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			rt = false;
		}else{
			printf("SQL execute successfully\n");
		}
		if (g_sPwd == sPwd)
			nErr = 0;
		else
			nErr = 1;

		CloseDB();

		g_sPwd = "";

		return rt;
	}

	//in@
	//nErr=0,good;
	//nErr=1,pwd error
	//nErr=2,target user not exist
	bool CDBController::AdminUserVerify(int& nErr, const string& sUserName, const string& sPwd)
	{
		printf("AdminUserVerify\n");
		nErr=-1;
		int nTargetState=0;
		vector <string> vNames;
		GetAllUserNames(vNames,sUserName,nTargetState);
		if (nTargetState == 2)//not exist
		{
			nErr = 2;
			return true;
		}

		g_uCmd = DB_VERRIFYUSERPWD;
		bool rt = true;

		if (!OpenDB())
		{
			rt = false;
		}
		string s_sql("SELECT pwd from userInfo where userName=\"");
		s_sql.append(sUserName);
		s_sql.append("\" and level=\"");
		s_sql.append("0");
		s_sql.append("\"");

		int rc;
		char *zErrMsg = 0;
		rc = sqlite3_exec(m_pstdb, s_sql.c_str(), callback, "userName", &zErrMsg);
		if( rc != SQLITE_OK ){
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			rt = false;
		}else{
			printf("SQL execute successfully\n");
		}
		if (g_sPwd == sPwd)
			nErr = 0;
		else
			nErr = 1;
		
		CloseDB();
		g_sPwd = "";

		return rt;
	}


	bool CDBController::OpenDB()
	{
		int rc;
		if (m_pstdb)
		{
			return false;
		}

		/* Open database */
		rc = sqlite3_open(m_sDBName.c_str(), &m_pstdb);
		if (rc != SQLITE_OK)
		{
			printf("open FAILED:%s", sqlite3_errmsg(m_pstdb));
			return false;
		}
		else
		{
			printf("open GOOD\n");
			return true;
		}
	}

	bool CDBController::CloseDB()
	{
		if (m_pstdb)
		{
			sqlite3_close(m_pstdb);
			printf("close GOOD\n");

			m_pstdb = NULL;
			return true;
		}
		printf("close FAILED\n");
		return false;
	}

}