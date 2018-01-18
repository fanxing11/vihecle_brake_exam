#pragma once


#include "Communicator.h"
using namespace COMMUNICATOR;
#include "DBController.h"
using namespace DBCONTROLLER;
#include "DataControler.h"
using namespace DATACONTROLER;
#include "DAQControler.h"
using DAQCONTROLER::CDAQControler;
#include "Analysis.h"
using namespace ANALYSISSPACE;

//#include "const.h"

class CtheApp
{
public:
	CtheApp(void);
	~CtheApp(void);

public:
	CCommunicator* m_pCommunicator;
	CDBController* m_pDBC;
	CDataControler* m_pDataC;
	CDAQControler* m_pDAQC;
	CAnalysis* m_pAnalysis;
	DWORD m_dwMainThreadID;


};

inline	bool CheckFolderExist(const string& strPath)
{
	WIN32_FIND_DATAA wfd;
	bool bRet = false;
	HANDLE hFind = FindFirstFileA(strPath.c_str(),&wfd);
	if ( (hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
	{
		bRet = true;
	}
	FindClose(hFind);
	return bRet;
}

inline bool CheckFileExist(const string& strFileFullPath)
{
	WIN32_FIND_DATAA wfd;
	bool bRet = false;
	HANDLE hFind = FindFirstFileA(strFileFullPath.c_str(),&wfd);
	//if ( (hFind != INVALID_HANDLE_VALUE) &&(wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) )
	if ( (hFind != INVALID_HANDLE_VALUE) )
	{
		bRet = true;
	}
	FindClose(hFind);
	return bRet;
}