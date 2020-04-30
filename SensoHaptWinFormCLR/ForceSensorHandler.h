#pragma once
#ifndef _FORCESENSORHANDLER
#define _FORCESENSORHANDLER

#include <string>
#include <Windows.h>


DWORD WINAPI touchActivatedThread(LPVOID lpParam);
DWORD WINAPI readSensorThread(LPVOID lpParam);

//Managed class for DAQ interfacing
ref class ForceSensorHandler {

public:

	static void startDataAcquisition(const std::string& left_calibration_path, const std::string& right_calibration_path, const std::string& device_name);
	static void stopDataAcquisition();
	static HANDLE startTouchActivated(float value);

	static HANDLE stopDAQEvent, hReadSensorThread;

	
};
#endif