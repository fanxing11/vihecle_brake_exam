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
	HANDLE m_gEvtStress;
	HANDLE m_gEvtVelocity;
	HANDLE m_gEvtSaveFile;
	HANDLE m_gEvtInitAngleFlag;//First need save InitAngle

	class CDAQControler
	{
	public:
		CDAQControler(void);
		~CDAQControler(void);

	public:
		void Initialize();
		void VelocityBegin();
		void VelocityEnd();
		void StressBegin();
		void StressEnd();

		void NewProject(char cMode);
		void TerminateProject();



	private:

		void SetInitAngleFlag();
		void SampleBegin();
		void SampleEnd();
		void CreateSyncEvent();
		void CloseEvtHandle();
		void DisInitialize();


		WaveformAiCtrl * m_wfAiCtrl;

		HANDLE m_hDAQThread;

	};


}
