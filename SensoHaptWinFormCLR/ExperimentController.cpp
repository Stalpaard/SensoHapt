#include "stdafx.h"


#using <Microsoft.VisualBasic.dll>

#include "ExperimentController.h"
#include "ForceSensorHandler.h"
#include "HapticHandler.h"
#include "GraphicsHandler.h"
#include "FileHandler.h"

using namespace System;
using namespace System::Diagnostics;

#define HF_TORQUE_MAX_ROLL 170 //According to documentation of 3DSystems
#define HF_TORQUE_MAX_PITCH 515 //Unit: mNm
#define HF_TORQUE_MAX_YAW 515

#define BEEP_FREQUENCY 440 //Frequency of audible beep at trial_setup and trial_end [Hz]
#define BEEP_DURATION 150 //Duration of the beep [ms]

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr double DELAY_RESOLUTION = 1000 / FRAMES_PER_SECOND; //Min. interval of delay milliseconds

//Global experiment variables initialization
ExperimentCommon::State ExperimentCommon::experiment_state;
int ExperimentCommon::current_trial_id;
HANDLE ExperimentCommon::writingTimeEvent;

////////////////////////////////////////////////////////////////////////////////////////////////////


/************************************
		EXPERIMENT PROGRAM STATES
************************************/

ExperimentController::ExperimentController(unsigned int subject_nr, const HDstring& hapticDeviceName, const std::string& forceSensorDeviceName, 
	const std::string& leftCalibrationPath, const std::string& rightCalibrationPath,
	const std::string& outputDataFilePath, const std::string& outputAnswersFilePath, const std::string& outputExperimentFilePath ) :
	current_trial_nr{ 1 }, last_state{ ExperimentCommon::State::TRIAL_SETUP }
{
	ExperimentCommon::experiment_state = ExperimentCommon::State::TRIAL_SETUP;

	//Setup experiment handlers
	hapticHandler = std::make_shared<HapticHandler>(hapticDeviceName, hduVector3Dd{ HF_TORQUE_MAX_ROLL, HF_TORQUE_MAX_PITCH, HF_TORQUE_MAX_YAW });
	graphicsHandler = std::make_shared<GraphicsHandler>();
	fileHandler = std::make_shared<FileHandler>(subject_nr, ExperimentCommon::expInfo->sampling_rate_datalog, outputDataFilePath, outputAnswersFilePath, outputExperimentFilePath);
	ForceSensorHandler::startDataAcquisition(leftCalibrationPath, rightCalibrationPath, forceSensorDeviceName);
	
	//Log experiment start to debug console
#ifdef LOG_DEBUG
	Debug::WriteLine(L"EXPERIMENT START");
	Debug::WriteLine(ExperimentCommon::expInfo->trials.size().ToString(), L"Amount of trials");
#endif
	
	//Set event experiment controller reference (expInfo already initialized due to XML parsing in UI)
	for (auto& trial : ExperimentCommon::expInfo->trials) 
		for (auto& trial_event : trial->events) trial_event->setController(this);
		

	if (ExperimentCommon::expInfo->answers_enabled) fileHandler->openSubjectFile();
	fileHandler->openExperimentFile();

	//Create needed system events (thread synchronization)
	stopExperimentEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"stopExperimentEvent"
	);
	if (stopExperimentEvent == NULL) ErrorExit(L"Failed to create stopExperimentEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created stopExperimentEvent", L"ExperimentController");
#endif

	ExperimentCommon::writingTimeEvent = CreateEvent(
		NULL,
		TRUE,
		TRUE,
		L"writingTimeEvent"
	);
	if (stopExperimentEvent == NULL) ErrorExit(L"Failed to create writingTimeEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created writingTimeEvent", L"ExperimentController");
#endif
}

ExperimentController::~ExperimentController()
{
	//Stop DAQ
	ForceSensorHandler::stopDataAcquisition();

	//Close event handles
	if (!(CloseHandle(stopExperimentEvent))) ErrorExit(L"Failed to close stopExperimentEvent");
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Closed stopExperimentEvent", L"ExperimentController");
#endif
	if (!(CloseHandle(ExperimentCommon::writingTimeEvent))) ErrorExit(L"Failed to close writingTimeEvent");
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Stopped writingTimeEvent", L"ExperimentController");
#endif
}

void ExperimentController::experimentLoop()
{
#ifdef LOG_DEBUG
	//Debugging state changes
	if (ExperimentCommon::experiment_state != last_state)
	{
		Debug::WriteLine(L"Current experiment_state = " + gcnew System::String(state_to_string(ExperimentCommon::experiment_state).c_str()), L"ExperimentController");
		last_state = ExperimentCommon::experiment_state;
	}
#endif
	//Experiment behaviour according to current experiment state
	switch (ExperimentCommon::experiment_state)
	{
	case ExperimentCommon::State::TRIAL_SETUP:
		{
			current_trial = ExperimentCommon::expInfo->trials.at(current_trial_nr-1);

			//Process trial variables
			ExperimentCommon::current_trial_id = current_trial->id;

			const double trial_delay_double = current_trial->visual_delay;
			GraphicsHandler::current_frame_index = FRAMES_PER_SECOND - std::trunc(trial_delay_double / DELAY_RESOLUTION);
			
			const int colour_size = current_trial->cube_colour.size();
			for(int i = 0; i < colour_size; i++) GraphicsHandler::displayed_cube_colour.at(i) = current_trial->cube_colour.at(i);
#ifdef LOG_DEBUG
			Debug::WriteLine(L"Cube green: " + GraphicsHandler::displayed_cube_colour.at(1).ToString());
#endif

			current_trial_events = &(current_trial->events);
			current_amount_of_events = current_trial_events->size();
			if (current_amount_of_events > 0) upcoming_event = current_trial_events->begin(); //Trial class sorts events collection automatically
			running_events.clear();

			//Clear previous forces
			hapticHandler->clearTrialForce();
			hapticHandler->clearTrialTorque();

#ifdef LOG_DEBUG
			//Print trial overview to debug console
			Debug::WriteLine(current_trial_nr.ToString(), L"TRIAL NUMBER");
			Debug::WriteLine(current_trial->cube_weight.ToString(), L"Mass (kg)");
			Debug::WriteLine(current_trial->visual_delay.ToString(), L"Delay (ms)");
			Debug::WriteLine(current_trial->touch_activated.ToString(), L"Required held time (s)");
			Debug::WriteLine(current_trial->duration.ToString(), L"Trial Duration (ms)");
			Debug::WriteLine(current_amount_of_events.ToString(), L"Amount of events");
			Debug::WriteLine(current_trial->cube_colour.at(0) + L";" + current_trial->cube_colour.at(1) + L";" + current_trial->cube_colour.at(2) + L";" + current_trial->cube_colour.at(3), L"Cube colour (RGBA)");
#endif
			//Open new trial data filestream
			fileHandler->openTrialFile(current_trial_nr);
			
			//Start touch activated thread if needed
			if (current_trial->touch_activated > 0)
			{
				touch_activated_enabled = true;
				hTouchActivatedThread = ForceSensorHandler::startTouchActivated(current_trial->touch_activated); //Returns when object has been held for current_trial_touch_activated seconds
			}
			else touch_activated_enabled = false;

			//Set new experiment state
			ExperimentCommon::experiment_state = ExperimentCommon::State::TRIAL;

			//Play beep sound
			if (!(beeper.Beep(BEEP_FREQUENCY, BEEP_DURATION))) ErrorExit(L"Failed to beep during trial setup");

			//Reset timer
			ExperimentCommon::timer.reset();

			//Add trial force (gravity)
			hapticHandler->addTrialForce(0.0, (current_trial->cube_weight*GRAVITATIONAL_CONSTANT), 0.0);

			//Start logging data
			fileHandler->startDataLog();
		}
		break;

	case ExperimentCommon::State::TRIAL:
		{
			//Process events when needed (depends whether touch_activated is enabled or not)
			if (!(ExperimentCommon::timer.time >  current_trial->duration))
			{
				if (touch_activated_enabled)
				{
					if (WaitForSingleObject(hTouchActivatedThread, 0) == WAIT_OBJECT_0) checkEvents();
				}
				else checkEvents();
			}
			else {
				if (WaitForSingleObject(hTouchActivatedThread, 0) == WAIT_TIMEOUT)
				{
					if (!(SetEvent(ExperimentCommon::forceQuitTouchActivatedEvent))) ErrorExit(L"Failed to signal forceQuitTouchActivatedEvent");
#ifdef LOG_DEBUG
					else Debug::WriteLine(L"Signaled forceQuitTouchActivatedEvent", L"ExperimentController");
#endif
					if (WaitForSingleObject(hTouchActivatedThread, INFINITE) == WAIT_OBJECT_0)
					{
#ifdef LOG_DEBUG
						Debug::WriteLine(L"Touch activated thread force quitted", L"ExperimentController");
#endif
					}
					if (!(CloseHandle(hTouchActivatedThread))) ErrorExit(L"Failed to close touch activated thread handle");
				}
				ExperimentCommon::experiment_state = ExperimentCommon::State::TRIAL_END;
			}
		}
		break;

	case ExperimentCommon::State::TRIAL_END:
		{

			//Play beep sound
			if (!(beeper.Beep(BEEP_FREQUENCY, BEEP_DURATION))) ErrorExit(L"Failed to beep at trial end");

			//Stop trial datalogging
			fileHandler->stopDataLog();
			fileHandler->addExperimentTrialData(current_trial_nr);

			//Prompt for answer if needed
			if (ExperimentCommon::expInfo->answers_enabled)
			{
				System::String^ questionPrompt = gcnew  System::String("Please enter subject answer");
				System::String^ questionTitle = gcnew  System::String("Perceived weight");
				System::String^ questionDefaultAns = gcnew System::String("0");
				System::String^ answer = Microsoft::VisualBasic::Interaction::InputBox(questionPrompt, questionTitle
					, questionDefaultAns, 500, 500);
				try {
#ifdef LOG_DEBUG
					Debug::WriteLine(answer, L"Answer received: ");
#endif
					fileHandler->addSubjectAnswer(Convert::ToDouble(answer));
				}
				catch (FormatException^) {
					ErrorExit(L"Couldn't convert given answer to a number");
				}
			}

			//Check if last trial has finished
			if (current_trial_nr >= ExperimentCommon::expInfo->trials.size())
			{
				ExperimentCommon::experiment_state = ExperimentCommon::State::EXPERIMENT_END;
				Console::WriteLine(L"Trial " + gcnew System::String(std::to_wstring(ExperimentCommon::expInfo->trials.size()).c_str()) + L" finished => END OF EXPERIMENT", L"ExperimentController");
			}
			else
			{
				++current_trial_nr;
				ExperimentCommon::experiment_state = ExperimentCommon::State::TRIAL_SETUP;
			}
		}
		break;

	case ExperimentCommon::State::EXPERIMENT_END :
		{
#ifdef LOG_DEBUG
			//Print to debug console
			Debug::WriteLine(L"EXPERIMENT END");
#endif

			//Notify ExperimentMain thread to quit
			if (!(SetEvent(stopExperimentEvent))) ErrorExit(L"Failed to signal stopExperimentEvent");
#ifdef LOG_DEBUG
			else Debug::WriteLine(L"Signaled stopExperimentEvent", L"ExperimentController");
#endif
		}
		break;
	}
}

void ExperimentController::checkEvents()
{
	if (current_amount_of_events > 0)
	{
		//Process upcoming events
		if (upcoming_event != current_trial_events->end())
			if ((*upcoming_event)->getStartTimestamp() <= ExperimentCommon::timer.time)
			{
				(*upcoming_event)->action(ExperimentCommon::timer.time);
				running_events.push_back((*upcoming_event));
				++upcoming_event;
			}
		//Check for expired events
		auto& it = running_events.begin();
		while (it != running_events.end())
		{
			if ((*it)->getEndTimestamp() <= ExperimentCommon::timer.time)
			{
				(*it)->stopAction(ExperimentCommon::timer.time);
				it = running_events.erase(it);
			}
			else it++;
		}
	}
}

void ExperimentController::requestAddPerturbation(const Perturbation& perturbation) const
{
	hapticHandler->addPerturbation(perturbation);
}

void ExperimentController::requestRemovePerturbation(const Perturbation& perturbation) const
{
	hapticHandler->removePerturbation(perturbation);
}

//Get string representation of experiment state when needed
const std::string state_to_string(const ExperimentCommon::State& s)
{
	switch (s) {
		case ExperimentCommon::State::TRIAL_SETUP:
			return "TRIAL_SETUP";
		case ExperimentCommon::State::TRIAL:
			return "TRIAL";
		case ExperimentCommon::State::TRIAL_END:
			return "TRIAL_END";
		case ExperimentCommon::State::EXPERIMENT_END:
			return "EXPERIMENT_END";
		default:
			return "";
	}
}









