#pragma once
#ifndef _FILEHANDLER
#define _FILEHANDLER

#include <Windows.h>
#include <vector>
#include <string>
#include "FileStream.h"

DWORD WINAPI trialDataWriteThread(LPVOID lpParam);

//IO class for logging purposes
class FileHandler
{
	friend DWORD WINAPI trialDataWriteThread(LPVOID lpParam);
public:
	FileHandler(const unsigned int subjectnr, const DWORD& sampling_rate_hz, const std::string& outputDataFilePath, const std::string& outputAnswersFilePath, const std::string& outputExperimentFilePath);

	~FileHandler();
	void openTrialFile(int trial_order);
	void openSubjectFile();
	void openExperimentFile();

	

	void startDataLog();
	void stopDataLog();

	void addTrialData(float current_time);
	void addExperimentTrialData(int trial_order);
	void addSubjectAnswer(const System::Double& answer);

private:
	HANDLE hTrialThread, stopDataLogEvent;
	DWORD dwTrialThreadID;

	const std::string outputDataFilePath, outputAnswersFilePath, outputExperimentFilePath;
	
	std::string day_string, month_string;

	void openFileStream(FileStream& fileStream, const std::string& path, std::ios_base::openmode mode);

	FileStream subject_answers, trial_data, experiment_data;

	const unsigned int subject_number;

	int current_open_trial;

	float last_read_time;

	DWORD sampling_rate_interval;
};

#endif