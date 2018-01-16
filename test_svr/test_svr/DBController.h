#pragma once


#include <stdlib.h>
#include "sqlite3.h"

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <vector>
using std::vector;

#include <iterator>
using std::iterator;

namespace DBCONTROLLER
{

#define DB_GETALLUSERS 100
#define DB_VERRIFYUSERPWD 101
#define DB_ADDUSER 102
#define DB_MODIFYPWD 103
#define DB_DELETEUSER 104


	//typedef int (*sqlite3_callback)(
	//	void*,    /* Data provided in the 4th argument of sqlite3_exec() */
	//	int,      /* The number of columns in row */
	//	char**,   /* An array of strings representing fields in the row */
	//	char**    /* An array of strings representing column names */
	//	);
	unsigned int g_uCmd = 0;
	vector <string> g_vUserName;
	string g_sPwd("");

	static int callback(void *NotUsed, int argc, char **argv, char **azColName){
		//TRACE("argc=%d,Notused=%s\n",argc,NotUsed);
		string strInput((char*)NotUsed);
		//TRACE("callback\n");
		switch (g_uCmd){

		case DB_GETALLUSERS:
			{
				for(int i=0; i<argc; i++){
					//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
					string strColName(azColName[i]);
					if (strColName == strInput)
					{
						//TRACE("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
						//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
						g_vUserName.push_back(argv[i]);
					}
				}
			}

			break;

		case DB_VERRIFYUSERPWD:
			{
				g_sPwd = *argv;
			}
			break;

		default:
			break;

		}


		printf("\n");
		return 0;
	}

	class CDBController
	{
	public:
		CDBController(void);
		~CDBController(void);
	private:
		string m_sDBName;
		sqlite3 *m_pstdb;


	private:
		bool OpenDB();
		bool CloseDB();

	public:
		bool SetDBName(const string& csName);
		//注册
		bool UserRegister(int& nErr, const string& sUserName, const string& sPwd, const int level = 1);
		//登录
		bool UserLogin(int& nErr, const string& sUserName, const string& sPwd);	
		//删除用户
		bool DeleteUser(const string& sUserName, const string& sPwd="");
		//改密码
		bool ModifyPwd(const string& sUserName, const string& csPwd,const int level = 1);

		//private, call by adduser and verifypwd
		bool GetAllUserNames(vector<string>& vUsers,const string& sTargetUserName, int& nTargetState);

	};


}
