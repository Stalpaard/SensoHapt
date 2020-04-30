#include "stdafx.h"

#include "FileHandler.h"
#include "common_variables.h"

#define OUTPUT_PRECISION 3


#define CSV_DELIMITER ","

using namespace System::Diagnostics;

FileHandler::FileHandler(const unsigned int subjectnr, const DWORD& sampling_rate_hz, const std::string& outputDataFilePath, const std::string& outputAnswersFilePath, const std::string& outputExperimentFilePath) : 
	subject_number{ subjectnr }, sampling_rate_interval{1000/sampling_rate_hz}, subject_answers{L"Subject Answers"}, trial_data{L"Trial Data"}, experiment_data{L"Experiment Data"},
	outputDataFilePath{ outputDataFilePath }, outputAnswersFilePath{outputAnswersFilePath}, outputExperimentFilePath{outputExperimentFilePath}, current_open_trial{1}, last_read_time{0}
{
	//Set precision for floating point output
	std::setprecision(OUTPUT_PRECISION);

	//Get current date for output file name
	SYSTEMTIME systemtime;
	GetLocalTime(&systemtime);
	WORD current_day = systemtime.wDay;
	WORD current_month = systemtime.wMonth;
	if (current_day < 10) day_string = "0" + std::to_string(current_day);
	else day_string = std::to_string(current_day);
	if (current_month < 10) month_string = "0" + std::to_string(current_month);
	else month_string = std::to_string(current_month);

	//Create system events
	stopDataLogEvent = CreateEvent(
		NULL,
		TRUE,
		TRUE,
		L"stopDataLogEvent"
	);
	if (stopDataLogEvent == NULL) ErrorExit(L"Failed to create stopDataLogEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created stopDataLogEvent", L"FileHandler");
#endif
}

DWORD WINAPI trialDataWriteThread(LPVOID lpParam)
{
	FileHandler* fileHandler = static_cast<FileHandler*>(lpParam);


	//WaitForSingleObject(ExperimentCommon::writingTimeEvent, INFINITE);
	float current_time = 0;

	DWORD sampling_interval = fileHandler->sampling_rate_interval;

	//Reset stopDatalogEvent to its unsignaled state
	if (!(ResetEvent(fileHandler->stopDataLogEvent))) ErrorExit(L"Failed to reset stopDataLogEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"stopDataLogEvent resetted", L"FileHandler");
#endif

	while (WaitForSingleObject(fileHandler->stopDataLogEvent, 0) == WAIT_TIMEOUT)
	{
		fileHandler->addTrialData(current_time);
		Sleep(sampling_interval);
		current_time += sampling_interval;
	}
	return 0;
}

FileHandler::~FileHandler()
{
	//Close system handles
	if (!CloseHandle(stopDataLogEvent)) ErrorExit(L"Failed to close stopDataLogEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed stopDataLogEvent", L"FileHandler");
#endif

	if (!CloseHandle(hTrialThread)) ErrorExit(L"Failed to close trial thread handle");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed dataWriteThread handle", L"FileHandler");
#endif
}

void FileHandler::startDataLog()
{
	//Start datalog writing thread
	hTrialThread = CreateThread(
		NULL,
		0,
		trialDataWriteThread,
		this,
		0,
		&dwTrialThreadID
	);

	if (hTrialThread == NULL) ErrorExit(L"Failed to create trial thread");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created dataWriteThread with ID: " + dwTrialThreadID.ToString(), L"FileHandler");
#endif
}

void FileHandler::stopDataLog()
{
	//Set stopDataLogEvent to signaled state (close datalogthread), Events are thread-safe so no SRWLOCK/CS needed
	if(!(SetEvent(stopDataLogEvent))) ErrorExit(L"Failed to signal stopDataLogEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Signaled stopDataLogEvent", L"FileHandler");
#endif
	if (WaitForSingleObject(hTrialThread, INFINITE) == WAIT_OBJECT_0)
	{
#ifdef LOG_DEBUG
		Debug::WriteLine(L"dataWriteThread signaled", L"FileHandler");
#endif
	}
}

void FileHandler::openTrialFile(int trial)
{
	//Open new trial data file
	current_open_trial = trial;
	openFileStream(trial_data, outputDataFilePath + "\\S" + std::to_string(subject_number) + "_T" + std::to_string(trial) + "_" + day_string + month_string + ".csv", std::ios_base::out);
	try {
		trial_data << "TrialID" << CSV_DELIMITER << "Time" << CSV_DELIMITER << "PosX" << CSV_DELIMITER << "PosY" << CSV_DELIMITER << "PosZ" << CSV_DELIMITER
			<< "VeloX" << CSV_DELIMITER << "VeloY" << CSV_DELIMITER << "VeloZ" << CSV_DELIMITER
			<< "HapticForceX" << CSV_DELIMITER << "HapticForceY" << CSV_DELIMITER << "HapticForceZ" << CSV_DELIMITER
			<< "HapticTorqueX" << CSV_DELIMITER << "HapticTorqueY" << CSV_DELIMITER << "HapticTorqueZ" << CSV_DELIMITER
			<< "LeftSensorForceX" << CSV_DELIMITER << "LeftSensorForceY" << CSV_DELIMITER << "LeftSensorForceZ" << CSV_DELIMITER
			<< "LeftSensorTorqueX" << CSV_DELIMITER << "LeftSensorTorqueY" << CSV_DELIMITER << "LeftSensorTorqueZ" << CSV_DELIMITER
			<< "RightSensorForceX" << CSV_DELIMITER << "RightSensorForceY" << CSV_DELIMITER << "RightSensorForceZ" << CSV_DELIMITER
			<< "RightSensorTorqueX" << CSV_DELIMITER << "RightSensorTorqueY" << CSV_DELIMITER << "RightSensorTorqueZ" 
			<< "\n";
	}
	catch (std::fstream::failure e) {
		ErrorExit(L"Exception writing to trial data file");
	}
}

void FileHandler::openFileStream(FileStream& fileStream, const std::string& path, std::ios_base::openmode mode)
{
	//Open FileStream
	try {
		if (fileStream.is_open()) fileStream.close();
		fileStream.exceptions(std::fstream::failbit | std::fstream::badbit);
		fileStream.open(path, mode);
	}
	catch (std::fstream::failure e) {
		std::wstring errormsg = L"Failed to open " + fileStream.getName() + L" file";
		ErrorExit(errormsg.c_str());
	}
}

void FileHandler::openSubjectFile()
{
	//Open Answers file
	openFileStream(subject_answers, outputAnswersFilePath + "\\Answers_S" + std::to_string(subject_number) + ".csv", std::ios_base::out);
	try {
		subject_answers << "TrialID" << CSV_DELIMITER << "SubjectNumber" << CSV_DELIMITER << "Answer" << CSV_DELIMITER << "TrialOrder" 
			<< "\n";
	}
	catch (std::fstream::failure e) {
		ErrorExit(L"Exception writing to subject answers file");
	}
}

void FileHandler::openExperimentFile()
{
	//Open Experiment Details file
	openFileStream(experiment_data, outputExperimentFilePath + "\\Experiment_S" + std::to_string(subject_number) + ".csv", std::ios_base::out);
	experiment_data.exceptions(std::fstream::failbit | std::fstream::badbit);
	try {
		experiment_data << "TrialOrder" << CSV_DELIMITER << "TrialID"
			<< "\n";
	}
	catch (std::fstream::failure e) {
		ErrorExit(L"Exception writing to subject answers file");
	}
}

void FileHandler::addExperimentTrialData(int trial_order)
{
	//Log the details of the past trial
	try {
		experiment_data << trial_order << CSV_DELIMITER << ExperimentCommon::current_trial_id
			<< "\n";
	}
	catch (std::fstream::failure e) {
		ErrorExit(L"Exception writing to experiment data file");
	}
}

void FileHandler::addTrialData(float current_time)
{
	//Log trial data entry
#ifndef NO_HARDWARE_DEBUG
	if(!(SetEvent(ExperimentCommon::requestSensorReadEvent))) ErrorExit(L"Failed to signal requestSensorReadEvent");
	WaitForSingleObject(ExperimentCommon::newSensorDataEvent, INFINITE);
	if (!(ResetEvent(ExperimentCommon::newSensorDataEvent))) ErrorExit(L"Failed to reset newSensorDataEvent");

	if(!(ResetEvent(ExperimentCommon::hapticLogEvent))) ErrorExit(L"Failed to reset hapticLogEvent");
	WaitForSingleObject(ExperimentCommon::hapticLogEvent, INFINITE);
#endif
	try {
		trial_data
			<< ExperimentCommon::current_trial_id
			<< CSV_DELIMITER << current_time
			<< CSV_DELIMITER << ExperimentCommon::haptic_position[0] << CSV_DELIMITER << ExperimentCommon::haptic_position[1] << CSV_DELIMITER << ExperimentCommon::haptic_position[2]
			<< CSV_DELIMITER << ExperimentCommon::haptic_velocity[0] << CSV_DELIMITER << ExperimentCommon::haptic_velocity[1] << CSV_DELIMITER << ExperimentCommon::haptic_velocity[2]
			<< CSV_DELIMITER << ExperimentCommon::haptic_current_force[0] << CSV_DELIMITER << ExperimentCommon::haptic_current_force[1] << CSV_DELIMITER << ExperimentCommon::haptic_current_force[2]
			<< CSV_DELIMITER << ExperimentCommon::haptic_current_gimbal_torque[0] << CSV_DELIMITER << ExperimentCommon::haptic_current_gimbal_torque[1] << CSV_DELIMITER << ExperimentCommon::haptic_current_gimbal_torque[2]
			<< CSV_DELIMITER << ExperimentCommon::readings_sL[0] << CSV_DELIMITER << ExperimentCommon::readings_sL[1] << CSV_DELIMITER << ExperimentCommon::readings_sL[2]
			<< CSV_DELIMITER << ExperimentCommon::readings_sL[3] << CSV_DELIMITER << ExperimentCommon::readings_sL[4] << CSV_DELIMITER << ExperimentCommon::readings_sL[5]
			<< CSV_DELIMITER << ExperimentCommon::readings_sR[0] << CSV_DELIMITER << ExperimentCommon::readings_sR[1] << CSV_DELIMITER << ExperimentCommon::readings_sR[2]
			<< CSV_DELIMITER << ExperimentCommon::readings_sR[3] << CSV_DELIMITER << ExperimentCommon::readings_sR[4] << CSV_DELIMITER << ExperimentCommon::readings_sR[5]
			<< "\n";
	}
	catch (std::fstream::failure e)
	{
		ErrorExit(L"Exception writing to trial data file");
	}
}

void FileHandler::addSubjectAnswer(const System::Double& answer)
{
	//Log subject answer
	try {
		subject_answers << ExperimentCommon::current_trial_id << CSV_DELIMITER << subject_number << CSV_DELIMITER << answer << CSV_DELIMITER << current_open_trial
			<< "\n";
	}
	catch (std::fstream::failure e) {
		ErrorExit(L"Exception writing to subject answers file");
	}
}