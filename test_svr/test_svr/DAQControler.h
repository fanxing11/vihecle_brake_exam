#pragma once
#include "const.h"

#include "compatibility.h"
#include "bdaqctrl.h"
//#include <tchar.h>

using namespace Automation::BDaq;


namespace DAQCONTROLER
{

	#define       deviceDescription  L"USB-4716,BID#0"// DemoDevice
	const wchar_t* profilePath = L"profile.xml";
	int32         startChannel = 0;
	const int32   channelCount = 10;
	const int32   sectionLength = 1024;
	const int32   sectionCount = 0;

	DWORD    SingleSavingFileSize = 10240  * sizeof(double);//每次采集(保存)1024*10个点
	DWORD    RequirementFileSize  = 10240 * sizeof(double);//不适用了

	const double deltat = 0.0005;//sample rate = 2kHz,采样周期为1/2k

	HANDLE   hFile;
	double * buffer = NULL;
	DWORD    WrittenBytes;
	DWORD    RealFileSize;

#define       USER_BUFFER_SIZE   channelCount*sectionLength
	double        Data[USER_BUFFER_SIZE]; 
	//double		 DateOne[10];

	//event
	HANDLE m_gEvtSample;//-ing
	HANDLE m_gEvtInitGradient;
	HANDLE m_gEvtMoveDetection;
	HANDLE m_gEvtStillDetection;
	HANDLE m_gEvtSaveFile;

	class CDAQControler
	{
	public:
		CDAQControler(void);
		~CDAQControler(void);

	public:
		void InitGradientBegin();
		void InitGradientEnd();
		void StillDetectionBegin();
		void StillDetectionEnd();
		void MoveDetectionBegin();
		void MoveDetectionEnd();

		bool NewProject(char cMode);
		void TerminateProject();

		void Initialize();


	private:
		void InitializeWiredDAQ();
		void InitializeWirelessDAQ();

		//void SetInitAngleFlag();
		void SampleBegin();
		void SampleEnd();
		void CreateSyncEvent();
		void CloseEvtHandle();
		void DisInitialize();


		WaveformAiCtrl * m_wfAiCtrl;

		HANDLE m_hDAQThread;

		bool m_bDAQInitialSuccessfully;
		//bool CheckDAQStarted()const;

		//---wireless
	public:

		bool CheckDAQStarted()const;
		//void GetData();
	};


//---------------wireless DAQ----
	//*********************************Dll import start*****************************************//
	//-----Lab_ConnectUT89
	typedef bool (*Dll_ConnectUT89)(char* strPath, char* strIP);
	Dll_ConnectUT89 dll_ConnectUT89;
	//-----Lab_SetPar
	typedef bool (*Dll_SetPar)(int freqCode, 
		int* nEnableCh,
		int* sCoupling,
		int* pComInput,
		int* pZoom);
	Dll_SetPar dll_SetPar;
	//-----Lab_SetPar
	typedef int (*Dll_OnStart)();
	Dll_OnStart dll_Start;
	//-----Lab_GetDataLen n_DateSize
	typedef int (*Dll_GetDataLen)();
	Dll_GetDataLen dll_GetDataLen;
	//-----Lab_ReadBuf
	typedef int (*Dll_ReadBuf)(
		int* readBuffer,
		int nLen);
	Dll_ReadBuf dll_ReadBuf;
	//-----Lab_OutLabDate
	typedef int (*Dll_OutLabDate)(
		int iBlockSize,
		int* readBuf,
		int* dFactor,
		short* nEnableCh,
		double* dXTime,
		double* dYAm);
	Dll_OutLabDate dll_OutLabDate;
	//-----Lab_Get2Wave
	typedef double (*Dll_Get2Wave)(
		int iStartCh,
		double* dXDate,
		double* dYDate,
		double* dXTime,
		double* dYAm1,
		double* dYAm2
		);
	Dll_Get2Wave dll_Get2Wave;
	//*********************************Dll import end*****************************************//
	const int WirelessDAQFrq = 2000;//2kHz
	const double deltatW = 0.0005;//sample rate = 2kHz,采样周期为1/2k
	const int channelCountW = 8;
	const int sectionLengthW = 1024;
	DWORD    SingleSavingFileSizeW = 1024 * 8  * sizeof(double);//每次采集(保存)1024*8个点

}
