#pragma once
#ifndef _EXPERIMENTCONTROLLER
#define _EXPERIMENTCONTROLLER

#include <vector>
#include <memory>
#include <string>
#include <Windows.h>

#include "common_variables.h"

class HapticHandler;
class GraphicsHandler;
class FileHandler;
class Perturbation;
class Trial;
class Event;

//Main controller class of the experiment, keeps track of all handlers and processes experiment events
class ExperimentController {
	friend void keyboardFunc(unsigned char key, int x, int y);
	friend DWORD WINAPI experimentMain(LPVOID lpParam);
public:
	ExperimentController(unsigned int subject_nr, const HDstring& hapticDeviceName, const std::string&forceSensorDeviceName,
		const std::string& leftCalibrationPath, const std::string& rightCalibrationPath,
		const std::string& outputDataFilePath, const std::string& outputAnswersFilePath, const std::string& outputExperimentFilePath);
	
	~ExperimentController();

	void requestAddPerturbation(const Perturbation& pert) const;
	void requestRemovePerturbation(const Perturbation& pert) const;

	HANDLE stopExperimentEvent;

private:
	void experimentLoop();

	void checkEvents();
	
	std::shared_ptr<HapticHandler> hapticHandler;
	std::shared_ptr<GraphicsHandler> graphicsHandler;
	std::shared_ptr<FileHandler> fileHandler;

	HANDLE hTouchActivatedThread;

	std::shared_ptr<Trial> current_trial;

	unsigned int current_trial_nr, 
		current_amount_of_events;

	bool touch_activated_enabled;

	std::vector<std::shared_ptr<Event>>::const_iterator upcoming_event;

	std::vector<std::shared_ptr<Event>> running_events;

	const std::vector<std::shared_ptr<Event>>* current_trial_events;

	ExperimentCommon::State last_state;

	DSBeeper beeper;
};

#endif