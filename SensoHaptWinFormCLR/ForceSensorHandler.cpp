#include "stdafx.h"

#include "ForceSensorHandler.h"
#include "SensorContainer.h"
#include "common_variables.h"

#define LEFT_CALIBRATION_INDEX 1 //Calibration index corresponds to the calibration in the .cal that will be used
#define RIGHT_CALIBRATION_INDEX 1

#define FORCE_UNITS "N" //Possible values: "lb","klb","N","kN","g","kg"
#define TORQUE_UNITS "N-m" //Possible values: "in-lb","ft-lb","N-m","N-mm","kg-cm"

#define HOLD_VALUE 1 //Amount of FORCE_UNITS that will be used to determine if the device is held by the subject

#define SAMPLING_RATE 10000
#define AVERAGING_FILTER_SIZE 10

////////////////////////////////////////////////////////////////////////////////////////////////////

//Initializing global variables
HANDLE ExperimentCommon::requestSensorReadEvent;
HANDLE ExperimentCommon::newSensorDataEvent;
HANDLE ExperimentCommon::forceQuitTouchActivatedEvent;
SENSOR_READING_TYPE ExperimentCommon::readings_sL[6];
SENSOR_READING_TYPE ExperimentCommon::readings_sR[6];

////////////////////////////////////////////////////////////////////////////////////////////////////


void ForceSensorHandler::startDataAcquisition(const std::string& left_calibration_path, const std::string& right_calibration_path, const std::string& device_name)
{
#ifndef NO_HARDWARE_DEBUG
	//Loading calibration files
	if (!(SensorContainer::sL.LoadCalibrationFile(gcnew System::String(left_calibration_path.c_str()) , LEFT_CALIBRATION_INDEX))) ErrorExit(L"Failed to load left force sensor calibration file");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Left force sensor calibration file loaded", L"ForceSensorHandler");
#endif
	if (!(SensorContainer::sR.LoadCalibrationFile(gcnew System::String(right_calibration_path.c_str()) , LEFT_CALIBRATION_INDEX))) ErrorExit(L"Failed to load left force sensor calibration file");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Right force sensor calibration file loaded", L"ForceSensorHandler");
#endif

	//Configuring force and torque units
	if(SensorContainer::sL.SetForceUnits(FORCE_UNITS) == 2) ErrorExit(L"Invalid force sensor units");
	SensorContainer::sR.SetForceUnits(FORCE_UNITS);
	if(SensorContainer::sL.SetTorqueUnits(TORQUE_UNITS) == 2) ErrorExit(L"Invalid torque sensor units");
	SensorContainer::sR.SetTorqueUnits(TORQUE_UNITS);

	System::String^ deviceName = gcnew System::String(device_name.c_str());

	//Starting single sample acquisition
	if (!(SensorContainer::sL.StartSingleSampleAcquisition(deviceName, SAMPLING_RATE, AVERAGING_FILTER_SIZE, SensorContainer::sL.GetFirstChannel(), true))) ErrorExit(L"Failed to start sample acquisition of left force sensor");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Left force sensor sample acquisition started", L"ForceSensorHandler");
#endif
	if (!(SensorContainer::sR.StartSingleSampleAcquisition(deviceName, SAMPLING_RATE, AVERAGING_FILTER_SIZE, SensorContainer::sR.GetFirstChannel(), true))) ErrorExit(L"Failed to start sample acquisition of right force sensor");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Right force sensor sample acquisition started", L"ForceSensorHandler");
#endif

	//Bias out the current load on the transducer
	SensorContainer::sL.BiasCurrentLoad();
	SensorContainer::sR.BiasCurrentLoad();
#endif

	//Create system events
	stopDAQEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"StopDaqEvent"
	);
	if (stopDAQEvent == NULL) ErrorExit(L"Failed to create StopDAQEvent");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"StopDAQEvent created", L"ForceSensorHandler");
#endif

	ExperimentCommon::requestSensorReadEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"requestSensorReadEvent"
	);
	if (ExperimentCommon::requestSensorReadEvent == NULL) ErrorExit(L"Failed to create requestSensorReadEvent");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"requestSensorReadEvent created", L"ForceSensorHandler");
#endif

	ExperimentCommon::newSensorDataEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"newSensorDataEvent"
	);
	if (ExperimentCommon::newSensorDataEvent == NULL) ErrorExit(L"Failed to create newSensorDataEvent");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"newSensorDataEvent created", L"ForceSensorHandler");
#endif

	ExperimentCommon::forceQuitTouchActivatedEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"forceQuitTouchActivatedEvent"
	);
	if (ExperimentCommon::forceQuitTouchActivatedEvent == NULL) ErrorExit(L"Failed to create forceQuitTouchActivatedEvent");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"forceQuitTouchActivatedEvent created", L"ForceSensorHandler");
#endif

	// Start DAQ read thread
	DWORD dwReadSensorThreadID;
	hReadSensorThread = CreateThread(
		NULL,
		0,
		readSensorThread,
		NULL,
		0,
		&dwReadSensorThreadID
	);
	if (hReadSensorThread == NULL) ErrorExit(L"Failed to create read sensor thread");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Read sensor thread created with ID: " + dwReadSensorThreadID.ToString(), L"ForceSensorHandler");
#endif
}

void ForceSensorHandler::stopDataAcquisition()
{
	//Stop DAQ read thread
	if (SetEvent(stopDAQEvent) == 0) ErrorExit(L"Failed to signal StopDAQEvent");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Signaled stopDAQEvent", L"ForceSensorHandler");
#endif

	//Wakeup DAQ read thread
	if (SetEvent(ExperimentCommon::requestSensorReadEvent) == 0) ErrorExit(L"Failed to signal ExperimentCommon::requestSensorReadEvent for thread exit");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Signaled ExperimentCommon::requestSensorReadEvent to exit thread", L"ForceSensorHandler");
#endif
	
	//Wait until DAQ thread has been signaled (exited)
	if (WaitForSingleObject(hReadSensorThread, INFINITE) == WAIT_OBJECT_0)
	{
#ifdef LOG_DEBUG
		System::Diagnostics::Debug::WriteLine(L"Read sensor thread signaled", L"ForceSensorHandler");
#endif
	}

	//Close system handles (events & threads)
	if (!(CloseHandle(stopDAQEvent))) ErrorExit(L"Failed to close StopDAQEvent handle");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Closed StopDAQEvent handle", L"ForceSensorHandler");
#endif

	if (!(CloseHandle(ExperimentCommon::requestSensorReadEvent))) ErrorExit(L"Failed to close requestSensorReadEvent handle");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Closed requestSensorReadEvent handle", L"ForceSensorHandler");
#endif

	if (!(CloseHandle(ExperimentCommon::newSensorDataEvent))) ErrorExit(L"Failed to close newSensorDataEvent handle");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Closed newSensorDataEvent handle", L"ForceSensorHandler");
#endif

	if (!(CloseHandle(hReadSensorThread))) ErrorExit(L"Failed to close ReadSensorThread handle");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Closed ReadSensorThread handle", L"ForceSensorHandler");
#endif
}

HANDLE ForceSensorHandler::startTouchActivated(float value)
{
	//Start new touch_activated thread which returns when touch_activated is complete
	DWORD dwTouchActivatedThreadId;
	HANDLE hTouchActivatedThread = CreateThread(
		NULL,
		0,
		touchActivatedThread,
		&value,
		0,
		&dwTouchActivatedThreadId
	);
	if (hTouchActivatedThread == NULL) ErrorExit(L"Failed to create touch_activated_thread");
#ifdef LOG_DEBUG
	else System::Diagnostics::Debug::WriteLine(L"Touch_activated thread created with ID: " + dwTouchActivatedThreadId.ToString(), L"ForceSensorHandler");
#endif

	return hTouchActivatedThread;
}

DWORD WINAPI readSensorThread(LPVOID lpParam)
{
	//Get new DAQ value if requested (via event)
	while (1)
	{
		WaitForSingleObject(ExperimentCommon::requestSensorReadEvent, INFINITE);
		if (!(ResetEvent(ExperimentCommon::requestSensorReadEvent))) ErrorExit(L"Failed to reset requestSensorReadEvent");
		if (WaitForSingleObject(ForceSensorHandler::stopDAQEvent, 0) == WAIT_TIMEOUT)
		{
#ifndef NO_HARDWARE_DEBUG
			cli::array<SENSOR_READING_TYPE, 1>^ readings_sL_cli = gcnew cli::array<SENSOR_READING_TYPE, 1>(6);
			cli::array<SENSOR_READING_TYPE, 1>^ readings_sR_cli = gcnew cli::array<SENSOR_READING_TYPE, 1>(6);
#ifdef LOG_DEBUG
			if (SensorContainer::sL.ReadSingleFTRecord(readings_sL_cli) == 2) System::Diagnostics::Debug::WriteLine(L"Left sensor strain gauges saturated!", L"ForceSensorHandler");
			if (SensorContainer::sR.ReadSingleFTRecord(readings_sR_cli) == 2) System::Diagnostics::Debug::WriteLine(L"Right sensor strain gauges saturated!", L"ForceSensorHandler");
#endif
			for (int i = 0; i < 6; i++) {
				ExperimentCommon::readings_sL[i] = readings_sL_cli[i];
				ExperimentCommon::readings_sR[i] = readings_sR_cli[i];
			}
#else
#ifdef LOG_DEBUG
			System::Diagnostics::Debug::WriteLine(L"Can't read sensors in no hardware mode", L"ForceSensorHandler");
#endif
#endif
			if (!(SetEvent(ExperimentCommon::newSensorDataEvent))) ErrorExit(L"Failed to signal newSensorDataEvent");
		}
		else break;
	}
	return 0;
}

DWORD WINAPI touchActivatedThread(LPVOID lpParam)
{
#ifndef NO_HARDWARE_DEBUG
	float requested_held_time = *((float*)lpParam);
	requested_held_time *= 1000; //Sampling rate interval is in the order of milliseconds

	float timer_count = -(SAMPLING_RATE / AVERAGING_FILTER_SIZE);

	bool device_held = false;

	while (!(device_held))
	{
		if (WaitForSingleObject(ExperimentCommon::forceQuitTouchActivatedEvent, 0) == WAIT_TIMEOUT)
		{
			if (!(SetEvent(ExperimentCommon::requestSensorReadEvent))) ErrorExit(L"Failed to signal requestSensorReadEvent");
			WaitForSingleObject(ExperimentCommon::newSensorDataEvent, INFINITE);
			if (!(ResetEvent(ExperimentCommon::newSensorDataEvent))) ErrorExit(L"Failed to reset newSensorDataEvent");
			//If device is held, start waiting for the timer_count to reach requested_held_time
			if (ExperimentCommon::readings_sL[2] > HOLD_VALUE && ExperimentCommon::readings_sR[2] > HOLD_VALUE)
			{
				timer_count = -(SAMPLING_RATE / AVERAGING_FILTER_SIZE);
				while (ExperimentCommon::readings_sL[2] > HOLD_VALUE && ExperimentCommon::readings_sR[2] > HOLD_VALUE)
				{
					timer_count += (SAMPLING_RATE / AVERAGING_FILTER_SIZE);
					if (timer_count >= requested_held_time)
					{
						device_held = true;
						break;
					}
					else
					{
						Sleep(SAMPLING_RATE / AVERAGING_FILTER_SIZE);
						if (!(SetEvent(ExperimentCommon::requestSensorReadEvent))) ErrorExit(L"Failed to signal requestSensorReadEvent");
						WaitForSingleObject(ExperimentCommon::newSensorDataEvent, INFINITE);
						if (!(ResetEvent(ExperimentCommon::newSensorDataEvent))) ErrorExit(L"Failed to reset newSensorDataEvent");
					}
				}
			}
		}
		else
		{
			if (!(ResetEvent(ExperimentCommon::forceQuitTouchActivatedEvent))) ErrorExit(L"Failed to reset forceQuitTouchActivatedEvent");
			break;
		}
	}
#ifdef LOG_DEBUG
	System::Diagnostics::Debug::WriteLine(L"Touch_activated thread done with operation", L"ForceSensorHandler");
#endif
#else
#ifdef LOG_DEBUG
	System::Diagnostics::Debug::WriteLine(L"Touch_activated not supported in no hardware mode", L"ForceSensorHandler");
#endif
#endif
	return 0;
}