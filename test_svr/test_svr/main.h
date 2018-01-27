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
	CDataControler* m_pDataController;
	CDAQControler* m_pDAQController;
	CAnalysis* m_pAnalysis;
	DWORD m_dwMainThreadID;


};

inline	bool CheckFolderExist(const string& strInPath)
{//注意，最后不是"\\",return false
	string strPath = strInPath;
	if ( strInPath.at(strPath.length()-1) == '\\' )
	{
		strPath.erase(strPath.length()-1,1);
	}

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

//获取所有的文件名  
void GetAllFiles( string path, vector<string>& files, string format )    
{    
	long   hFile   =   0;    
	//文件信息    
	struct _finddata_t fileinfo;//用来存储文件信息的结构体   
	unsigned int len = format.length();
	string p,temp;    
	if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1)  //第一次查找  
	{
		do    
		{     
			if((fileinfo.attrib &  _A_SUBDIR))  //如果查找到的是文件夹  
			{    
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)  //进入文件夹查找  
				{  
					//files.push_back(p.assign(path).append("\\").append(fileinfo.name) );  
					GetAllFiles( p.assign(path).append("\\").append(fileinfo.name), files,format );   
				}  
			}    
			else //如果查找到的不是是文件夹   
			{    
				//files.push_back(p.assign(fileinfo.name) );  //将文件路径保存，也可以只保存文件名:    p.assign(path).append("\\").append(fileinfo.name)  
				temp = fileinfo.name;
				//判断字符串是否以format格式结尾
				if(temp.length()>len && temp.compare(temp.length()-len,len,format)==0)
					files.push_back(p.assign(path).append("\\").append(fileinfo.name) ); 
			}   

		}while(_findnext(hFile, &fileinfo)  == 0);    

		_findclose(hFile); //结束查找  
	}   

}   