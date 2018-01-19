#pragma once 
#include <Windows.h>
#include <io.h>  //struct _finddata_t
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
using namespace std;
#include <stdio.h>

#include "Logger.h"
using namespace LOGGER;
extern CLogger g_logger;

#include <process.h>//_beginthreadex_endthreadex
#pragma comment(lib,"user32.lib")//GetMessage

const int NUM_ZERO = 0;
const int NUM_NEGONE = -1;
const int NUM_ONE = 1;
const int NUM_TWO = 2;
const int NUM_THREE = 3;
const int NUM_FOUR = 4;
const int NUM_FIVE = 5;
const int NUM_SIX = 6;
const int NUM_SEVEN = 7;
const int NUM_EIGHT = 8;
const int NUM_NINE = 9;
const int NUM_TEN = 10;
const int NUM_ELENEN = 11;


//----- UDP_rev_thread to main thread msg  
// && main thread to UDP thread SendDatatoUI
const UINT msg_CONTROL_INITIL = WM_USER+100;
const UINT msg_CONTROL_QUIT = WM_USER+101;

const UINT msg_DB_USERREGISTER = WM_USER+200;
const UINT msg_DB_USERLOGIN = WM_USER+201;
const UINT msg_DB_MODIFYPASSWORD = WM_USER+202;
const UINT msg_DB_DELETEUSER = WM_USER+203;
const UINT msg_DAQ_NEWPROJECT = WM_USER+204;
const UINT msg_DAQ_TERMINATEPROJECT = WM_USER+205;
const UINT msg_DAQ_VELOCITY_BEGIN = WM_USER+206;
const UINT msg_DAQ_VELOCITY_END = WM_USER+207;
const UINT msg_DAQ_STRESS_BEGIN = WM_USER+208;
const UINT msg_DAQ_STRESS_END = WM_USER+209;
const UINT msg_DATA_SETREPORTPATH = WM_USER+210;
const UINT msg_ANA_ANALYSIS_BEGIN = WM_USER+211;
const UINT msg_ANA_ANALYSIS_STATE = WM_USER+212;
const UINT msg_ANA_ANALYSIS_RESULT = WM_USER+213;

const UINT msg_MAIN_QIUT = WM_USER+444;


//----- UDP cmd from/to UI
const BYTE cmd_HEADER=0x7E;
const BYTE cmd_TAIL=0x7F;

const BYTE cmd_USERLOGIN=0x01;//in out
const BYTE cmd_USERREGISTER=0x02;//in out
const BYTE cmd_MODIFYPWD=0x03;//in out
const BYTE cmd_USERDELETE=0x04;//in out

const BYTE cmd_NEWPROJECT=0x11;//in out
const BYTE cmd_TERMINATEPROJECT=0x12;//in out

const BYTE cmd_VELOCITY_BEGIN=0x21;//in 
const BYTE cmd_VELOCITY_END=0x22;//in 
const BYTE cmd_VELOCITY=0x21;//out

const BYTE cmd_STRESS_BEGIN=0x31;//in 
const BYTE cmd_STRESS_END=0x32;//in
const BYTE cmd_STRESS=0x31;//out

const BYTE cmd_REPORTPATH=0x41;//in 
const BYTE cmd_ANALYSIS_BEGIN=0x42;//in 
const BYTE cmd_ANALYSIS_STATE=0x41;//out
const BYTE cmd_ANALYSIS_RESULT=0x42;//out
//分析没有终止命令。

const BYTE cmd_HEARTBEAT=0x51;//in out


//DAQ controler Msg to main thread msg  
const UINT msg_DAQ_VELOCITY = WM_USER+301;
const UINT msg_DAQ_STRESS = WM_USER+302;
const UINT msg_DAQ_DATAONE = WM_USER+303;



typedef struct StressInfo
{
	double MaxFootBrakeForce;
	double Gradient;
	double MaxHandBrakeForce;
	double PedalDistance;
}STRESSINFO;
typedef struct VelocityInfo
{
	double LastAccelaration;
	double LastVelocity;//只传一组数据里面的最后一个速度到UI
}VELOCITYINFO;

typedef struct AnalysisResult
{
	double MaxAccelaration;
	double BrakeDistance;
	double AverageVelocity;
	double Gradient;//max value
	double PedalDistance;//max value
	double MaxHandBrakeForce;
	double MaxFootBrakeForce;

}ANALYSISRESULT;

const string gc_strProjectParaINI_FileName("projectparameter.ini");
const string gc_strProjectInfo("ProjectInfo");
const string gc_strInitialAngle("InitialAngle");
const string gc_strInitXAngle("InitialX");
const string gc_strInitYAngle("InitialY");
