#pragma once
#ifndef _COMMONVARIABLES
#define _COMMONVARIABLES

#define FRAMES_PER_SECOND 60
#define GRAVITATIONAL_CONSTANT 9.81 // [N/Kg]
#define MAX_EXERTABLE_TRANSLATIONAL_FORCE 37.5 // [N]

#define SENSOR_READING_TYPE double

#define DEFAULT_HD_NAME "HighForceGimbal"
#define DEFAULT_DAQ_NAME "Dev1"

#include <vector>

#include "customtimer.h"
#include "HDU\hduVector.h"
#include "HDU\hduMatrix.h"
#include "ExperimentInfo.h"

namespace ExperimentCommon {

	//Experiment info
	extern std::shared_ptr<ExperimentInfo> expInfo;

	//Possible experiment states
	enum class State {
		TRIAL_SETUP,
		TRIAL,
		TRIAL_END,
		EXPERIMENT_END
	};

	//General variables
	extern State experiment_state;
	extern int current_trial_id;
	extern CustomTimer timer;

	//Haptic related variables
	extern hduVector3Dd haptic_position, haptic_velocity, haptic_current_force, haptic_current_gimbal_torque;
	extern HANDLE hapticLogEvent;

	//Visual information buffers
	extern std::vector<double> z_angle_buffer;
	extern std::vector<hduMatrix> transform_buffer;

	//Force sensor related variables
	extern HANDLE requestSensorReadEvent;
	extern HANDLE newSensorDataEvent;
	extern HANDLE forceQuitTouchActivatedEvent;
	extern HANDLE writingTimeEvent;
	extern SENSOR_READING_TYPE readings_sL[6];
	extern SENSOR_READING_TYPE readings_sR[6];
}

extern void ErrorExit(LPCTSTR errormsg);

extern const std::string state_to_string(const ExperimentCommon::State& s);


#endif