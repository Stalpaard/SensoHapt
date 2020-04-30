#include "stdafx.h"

#include "ExperimentController.h"
#include "ExperimentParserXML.h"

using namespace System::Diagnostics;
using namespace pugi;

DWORD WINAPI experimentMain(LPVOID lpParam)
{
	//Get passed subject_number from UI thread
	uint16_t subject_number = *(static_cast<uint16_t*>(lpParam));
	delete lpParam; //free passed memory
	msclr::interop::marshal_context context;

	//Load path configuration and hardware device names from XML configuration
	xml_document doc;
	System::String^ defaultXmlDir = gcnew System::String(System::Windows::Forms::Application::StartupPath + L"\\UserSettings\\experimentXmlPath.xml");
	if (!(doc.load_file(context.marshal_as<std::string>(defaultXmlDir).c_str()))) ErrorExit(L"Corrupted settings file, please restart application");

	HDstring hapticDeviceName{ doc.child("HapticDeviceName").attribute("name").as_string() };
	std::string forceSensorDeviceName{ doc.child("DaqName").attribute("name").as_string() };
	std::string leftCalibrationPath{ doc.child("LeftSensorCalibration").attribute("path").as_string() };
	std::string rightCalibrationPath{ doc.child("RightSensorCalibration").attribute("path").as_string() };
	
	std::string outputTrialDataPath{ doc.child("TrialDataPath").attribute("path").as_string() };
	std::string outputAnswersPath{ doc.child("AnswersPath").attribute("path").as_string() }; //zelf lezen uit config xml
	std::string outputExperimentDetailsPath{ doc.child("ExperimentDetailPath").attribute("path").as_string() }; //zelf lezen uit config xml

	//Initialize ExperimentController object
	ExperimentController experimentController{ subject_number, hapticDeviceName , forceSensorDeviceName,
		leftCalibrationPath, rightCalibrationPath, outputTrialDataPath, outputAnswersPath, outputExperimentDetailsPath };

	//Main experiment loop
	while (WaitForSingleObject(experimentController.stopExperimentEvent,0) == WAIT_TIMEOUT)
	{
		ExperimentCommon::timer.update(); //Once per loop: expensive function
		experimentController.experimentLoop();
	}
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Experiment main finished", L"ExperimentMain");
#endif
	return 0;
}

//Global function used for error exits
void ErrorExit(LPCTSTR lpszFunction)

{
	DWORD dw = GetLastError();
	System::String^ errormsg = gcnew System::String(lpszFunction);
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Error code " + dw, errormsg);
#endif
	MessageBox(NULL, lpszFunction, TEXT("Error"), MB_OK);
	ExitProcess(1);
}


