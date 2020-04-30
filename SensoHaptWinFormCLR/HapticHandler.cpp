#include "stdafx.h"

#include "HapticHandler.h"
#include "common_variables.h"

#define OVER_TEMPERATURE 0.98
#define GRAVITY_COMPENSATION_FORCE 2.7
#define FORCE_OFFSET_X 0.0
#define FORCE_OFFSET_Y GRAVITY_COMPENSATION_FORCE
#define FORCE_OFFSET_Z 0.0
#define GIMBAL_TORQUE_OFFSET_X 0.0
#define GIMBAL_TORQUE_OFFSET_Y 0.0
#define	GIMBAL_TORQUE_OFFSET_Z 0.0
#define MAX_RAMPUP 1.0 //values bigger than 1.0 would lead to multiplication

using namespace System;
using namespace System::Diagnostics;

////////////////////////////////////////////////////////////////////////////////////////////////////

//Initialization of static variables

HDdouble HapticHandler::device_offsetx = 0.0;
HDdouble HapticHandler::device_offsety = 0.0;
HDdouble HapticHandler::device_offsetz = 0.0;

////////////////////////////////////////////////////////////////////////////////////////////////////

//Initialization of global delay buffers

std::vector<double> ExperimentCommon::z_angle_buffer;
std::vector<hduMatrix> ExperimentCommon::transform_buffer;
hduVector3Dd ExperimentCommon::haptic_position, ExperimentCommon::haptic_velocity, ExperimentCommon::haptic_current_force, ExperimentCommon::haptic_current_gimbal_torque;
HANDLE ExperimentCommon::hapticLogEvent;

////////////////////////////////////////////////////////////////////////////////////////////////////


HapticHandler::HapticHandler(const HDstring& device_name, const hduVector3Dd& max_torque, HDdouble safety_temperature_margin, double rampup_rate, double rampup_step, HDdouble max_force_ratio) 
	:
	current_trial_force{ 0.0,GRAVITY_COMPENSATION_FORCE,0.0 }, max_force_ratio{ max_force_ratio }, max_torque{ max_torque },
	rampup_rate{ rampup_rate }, rampup_step{ rampup_step }, safety_temperature_margin{ safety_temperature_margin },
	enable_force_feedback{ true }, new_force_data{ false }, new_torque_data{ false },
	buffered_new_perturbation{hduVector3Dd{0,0,0},0,0}, buffered_remove_perturbation{ hduVector3Dd{ 0,0,0 },0,0 },
	last_read_time{ 0 }
{
#ifndef NO_HARDWARE_DEBUG
	device_handle = hdInitDevice(device_name);

	if (device_handle != HD_INVALID_HANDLE)
	{
		//Log event
		Console::WriteLine(L"Device '" + gcnew System::String(device_name) + L"' has been found", L"HapticHandler");

		//Initial setup haptic device
		hdMakeCurrentDevice(device_handle);
		hdEnable(HD_FORCE_OUTPUT);
		hdEnable(HD_MAX_FORCE_CLAMPING); //is onze eigen force clamping hierdoor redundant?
		hdEnable(HD_FORCE_RAMPING); //is onze eigen force ramping hierdoor redundant?

		hdGetDoublev(HD_NOMINAL_MAX_FORCE, &max_force);
		max_force *= max_force_ratio;

		HDdouble torque_temp[3];
		current_trial_torque = hduVector3Dd{ torque_temp[0], torque_temp[1], torque_temp[2] };

		//Register the scheduler callback function and start the scheduler
		scheduler_handle = hdScheduleAsynchronous(HDSimulationCallback, this, HD_MAX_SCHEDULER_PRIORITY); //Max priority zorgt ervoor dat deze callback als eerste wordt opgeroepen gedurende scheduler tick
		hdStartScheduler(); //Starts servo-loop thread
#ifdef LOG_DEBUG
		Debug::WriteLine(L"Started servo-loop thread", L"HapticHandler");
#endif
	}
	else ErrorExit(L"Haptic device not found");
#endif

	//Creating system events
	stopPositionBufferEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"stopPositionBufferEvent"
	);
	if (stopPositionBufferEvent == NULL) ErrorExit(L"Failed to create stopPositionBufferEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created stopPositionBufferEvent", L"HapticHandler");
#endif

	positionUpdateEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"positionUpdateEvent"
	);
	if (positionUpdateEvent == NULL) ErrorExit(L"Failed to create positionUpdateEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created positionUpdateEvent", L"HapticHandler");
#endif

	forceUpdateEvent = CreateEvent(
		NULL,
		TRUE,
		TRUE,
		L"forceUpdateEvent"
	);
	if (forceUpdateEvent == NULL) ErrorExit(L"Failed to create forceUpdateEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created forceUpdateEvent", L"HapticHandler");
#endif

	torqueUpdateEvent = CreateEvent(
		NULL,
		TRUE,
		TRUE,
		L"torqueUpdateEvent"
	);
	if (torqueUpdateEvent == NULL) ErrorExit(L"Failed to create torqueUpdateEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created torqueUpdateEvent", L"HapticHandler");
#endif

	newPerturbationEvent = CreateEvent(
		NULL,
		TRUE,
		TRUE,
		L"newPerturbationEvent"
	);
	if (newPerturbationEvent == NULL) ErrorExit(L"Failed to create newPerturbationEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created newPerturbationEvent", L"HapticHandler");
#endif

	removePerturbationEvent = CreateEvent(
		NULL,
		TRUE,
		TRUE,
		L"removePerturbationEvent"
	);
	if (newPerturbationEvent == NULL) ErrorExit(L"Failed to create removePerturbationEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created removePerturbationEvent", L"HapticHandler");
#endif

	ExperimentCommon::hapticLogEvent = CreateEvent(
		NULL,
		TRUE,
		TRUE,
		L"hapticLogEvent"
	);
	if (ExperimentCommon::hapticLogEvent == NULL) ErrorExit(L"Failed to create hapticLogEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created hapticLogEvent", L"HapticHandler");
#endif
	
	//Start position buffer update thread
	hPosBufferThread = CreateThread(
		NULL,
		0,
		posBufferThread,
		this,
		0,
		&dwPosBufferThread);
	if (hPosBufferThread == NULL) ErrorExit(L"Failed to create position buffer thread");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created position buffer thread with ID: " + dwPosBufferThread.ToString(), L"HapticHandler");
#endif
	
}

HapticHandler::~HapticHandler()
{
	//Stop buffer update thread
	if (!(SetEvent(stopPositionBufferEvent))) ErrorExit(L"Failed to set stopPositionBufferEvent to signaled state");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed stopPositionBufferEvent", L"HapticHandler");
#endif

	if (WaitForSingleObject(hPosBufferThread, INFINITE) == WAIT_OBJECT_0)
	{
#ifdef LOG_DEBUG
		Debug::WriteLine(L"Position buffer thread signaled", L"HapticHandler");
#endif
	}

	//Close system handles
	if (!CloseHandle(stopPositionBufferEvent)) ErrorExit(L"Failed to close stopPositionBufferEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed stopPositionBufferEvent", L"HapticHandler");
#endif

	if (!CloseHandle(positionUpdateEvent)) ErrorExit(L"Failed to close positionUpdateEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed positionUpdateEvent", L"HapticHandler");
#endif

	if (!CloseHandle(forceUpdateEvent)) ErrorExit(L"Failed to close forceUpdateEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed forceUpdateEvent", L"HapticHandler");
#endif

	if (!CloseHandle(torqueUpdateEvent)) ErrorExit(L"Failed to close torqueUpdateEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed torqueUpdateEvent", L"HapticHandler");
#endif

	if (!CloseHandle(newPerturbationEvent)) ErrorExit(L"Failed to close newPerturbationEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed newPerturbationEvent", L"HapticHandler");
#endif

	if (!CloseHandle(removePerturbationEvent)) ErrorExit(L"Failed to close removePerturbationEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed removePerturbationEvent", L"HapticHandler");
#endif

	if (!CloseHandle(ExperimentCommon::hapticLogEvent)) ErrorExit(L"Failed to close ExperimentCommon::hapticLogEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed ExperimentCommon::hapticLogEvent", L"HapticHandler");
#endif

	if (!CloseHandle(hPosBufferThread)) ErrorExit(L"Failed to close hapticPositionBuffer thread handle");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Closed hapticPositionBuffer thread handle", L"HapticHandler");
#endif

#ifndef NO_HARDWARE_DEBUG
	if (scheduler_handle != HD_INVALID_HANDLE)
	{
		//Stop the scheduler
		hdStopScheduler();
		hdUnschedule(scheduler_handle);
#ifdef LOG_DEBUG
		Debug::WriteLine(L"Closed servo-loop thread", L"HapticHandler");
#endif
	}

	//Close haptic device handle
	if (device_handle != HD_INVALID_HANDLE)
	{
		hdDisableDevice(device_handle);
#ifdef LOG_DEBUG
		Debug::WriteLine(L"Closed haptic device", L"HapticHandler");
#endif
		device_handle = HD_INVALID_HANDLE;
	}
#endif
}



HDCallbackCode HDCALLBACK HDSimulationCallback(void * hapticHandler_voidptr)
{
	HapticHandler* hapticHandler = static_cast<HapticHandler*>(hapticHandler_voidptr);
	
	HHD* device_handle = &(hapticHandler->device_handle);
	
	if (*device_handle != HD_INVALID_HANDLE)
	{
		
		hdMakeCurrentDevice(*device_handle);
		// Start haptic frame
		hdBeginFrame(*device_handle);
		
		//Update log variables
		if (WaitForSingleObject(ExperimentCommon::hapticLogEvent, 0) == WAIT_TIMEOUT) //try om haperingen te voorkomen
		{
			hdGetDoublev(HD_CURRENT_POSITION, ExperimentCommon::haptic_position);
			hdGetDoublev(HD_CURRENT_VELOCITY, ExperimentCommon::haptic_velocity);
			hdGetDoublev(HD_CURRENT_FORCE, ExperimentCommon::haptic_current_force);
			hdGetDoublev(HD_CURRENT_GIMBAL_TORQUE, ExperimentCommon::haptic_current_gimbal_torque);
			if (!(SetEvent(ExperimentCommon::hapticLogEvent))) ErrorExit(L"Failed to signal hapticLogEvent");
		}
		
		//Check motor temperatures
		hdGetDoublev(HD_MOTOR_TEMPERATURE, hapticHandler->motor_temperatures);
		for (auto&& motor_temp : hapticHandler->motor_temperatures)
			if (motor_temp > OVER_TEMPERATURE - hapticHandler->safety_temperature_margin)
			{
				hapticHandler->setForceEnabled(false);
				ErrorExit(L"Motor safety temperature reached! Please try again later");
			}

		//Update positional variables
		if (WaitForSingleObject(hapticHandler->positionUpdateEvent, 0) == WAIT_TIMEOUT)
		{
			HDdouble gimbal_angles[3];
			hduMatrix transform_matrix;
			hdGetDoublev(HD_CURRENT_TRANSFORM, transform_matrix);
			hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, gimbal_angles);
			
			transform_matrix[3][0] = transform_matrix[3][0] * 0.001 + hapticHandler->device_offsetx;
			transform_matrix[3][1] = transform_matrix[3][1] * 0.001 + hapticHandler->device_offsety;
			transform_matrix[3][2] = transform_matrix[3][2] * 0.001 + hapticHandler->device_offsetz;
			
			hapticHandler->current_z_angle = gimbal_angles[2];
			hapticHandler->current_transform_matrix = transform_matrix;
			if (!(SetEvent(hapticHandler->positionUpdateEvent))) ErrorExit(L"Failed to set positionUpdateEvent");
		}
		

		//Update force/torque feedback
		if (hapticHandler->enable_force_feedback == true)
		{
			//Check for updated force values
			if (WaitForSingleObject(hapticHandler->forceUpdateEvent, 0) == WAIT_TIMEOUT)
			{
				hapticHandler->current_trial_force = hduVector3Dd{ FORCE_OFFSET_X, FORCE_OFFSET_Y, FORCE_OFFSET_Z };
				for (hduVector3Dd force : hapticHandler->force_buffer) hapticHandler->current_trial_force += force;
				hapticHandler->clampForce();
				if (!(SetEvent(hapticHandler->forceUpdateEvent))) ErrorExit(L"Failed to signal forceUpdateEvent");
			}
			//Check for updated torque values
			if (WaitForSingleObject(hapticHandler->torqueUpdateEvent, 0) == WAIT_TIMEOUT)
			{
				hapticHandler->current_trial_torque = hduVector3Dd{ GIMBAL_TORQUE_OFFSET_X, GIMBAL_TORQUE_OFFSET_Y, GIMBAL_TORQUE_OFFSET_Z };
				for (hduVector3Dd torque : hapticHandler->gimbal_torque_buffer) hapticHandler->current_trial_torque += torque;
				hapticHandler->clampTorque();
				if (!(SetEvent(hapticHandler->torqueUpdateEvent))) ErrorExit(L"Failed to signal torqueUpdateEvent");
			}

			hduVector3Dd output_force = hapticHandler->current_trial_force;
			hduVector3Dd output_torque = hapticHandler->current_trial_torque;

			//Check for perturbation events
			if (WaitForSingleObject(hapticHandler->newPerturbationEvent, 0) == WAIT_TIMEOUT)
			{
				hapticHandler->perturbation_vec.push_back(hapticHandler->buffered_new_perturbation);
				if (!(SetEvent(hapticHandler->newPerturbationEvent))) ErrorExit(L"Failed to signal newPerturbationEvent");
			}

			if (WaitForSingleObject(hapticHandler->removePerturbationEvent, 0) == WAIT_TIMEOUT)
			{
				std::vector<Perturbation>::iterator it = std::find(hapticHandler->perturbation_vec.begin(), hapticHandler->perturbation_vec.end(), hapticHandler->buffered_remove_perturbation);
				if (it != hapticHandler->perturbation_vec.end()) hapticHandler->perturbation_vec.erase(it);
#ifdef LOG_DEBUG
				else Debug::WriteLine(L"Perturbation not found when trying to remove from perturbation_vec", L"HapticHandler");
#endif
				if (!(SetEvent(hapticHandler->removePerturbationEvent))) ErrorExit(L"Failed to signal removePerturbationEvent");
			}

			//Process perturbations
			if (hapticHandler->perturbation_vec.size() > 0)
			{
				if (WaitForSingleObject(ExperimentCommon::writingTimeEvent, 0) == WAIT_OBJECT_0) hapticHandler->last_read_time = ExperimentCommon::timer.time;
				for (auto&& pert : hapticHandler->perturbation_vec) output_force += pert.getPerturbationForce(hapticHandler->last_read_time);
				hapticHandler->clampForce();
			}

			//Rampup of forces/torques at initial stage
			if (hapticHandler->rampup_rate < MAX_RAMPUP)
			{
				hapticHandler->current_trial_force *= hapticHandler->rampup_rate;
				hapticHandler->current_trial_torque *= hapticHandler->rampup_rate;
				hapticHandler->rampup_rate += hapticHandler->rampup_step;
			}

			// Output force to device
			hdSetDoublev(HD_CURRENT_FORCE, hapticHandler->current_trial_force);
			hdSetDoublev(HD_CURRENT_GIMBAL_TORQUE, hapticHandler->current_trial_torque);
		}
		else {
			hdSetDoublev(HD_CURRENT_FORCE, hduVector3Dd{ 0,0,0 });
		}
		

		// Close haptic frame (commit changes made during this haptic tick to device)
		hdEndFrame(hapticHandler->device_handle);
		
		HDErrorInfo error;
		if (HD_DEVICE_ERROR(error = hdGetError()))
		{
			if (hduIsForceError(&error))
			{
				std::wstring string = L"Force Error, HD Error Code: ";
				string += error.errorCode;
#ifdef LOG_DEBUG
				Debug::WriteLine(gcnew System::String(string.c_str()), L"HDSimulationCallback");
#endif
			}
			else if (hduIsSchedulerError(&error))
			{
				std::wstring string = L"Scheduler Error, HD Error Code: ";
				string += error.errorCode;
#ifdef LOG_DEBUG
				Debug::WriteLine(gcnew System::String(string.c_str()), L"HDSimulationCallback");
#endif
				return HD_CALLBACK_DONE;
			}
		}
		return HD_CALLBACK_CONTINUE;
	}
	else
	{
		ErrorExit(L"Haptic device handle not found in callback function");
		return HD_CALLBACK_DONE;
	}
}

void HapticHandler::clampForce()
{
	//Clamp force to max_force, keeping relative magnitude of the forces (no change of direction)
	if (current_trial_force.magnitude() > max_force) 
	{
		current_trial_force.normalize();
		current_trial_force *= max_force;
	}
}

void HapticHandler::clampTorque()
{
	//Clamp torque to max_torque, keeping relative magnitude of the forces (no change of direction)
	if (current_trial_torque[0] > max_torque[0] || current_trial_torque[1] > max_torque[1] || current_trial_torque[2] > max_torque[2])
	{
		current_trial_torque.normalize();
		for (int i = 0; i < 3; i++) current_trial_torque[i] *= max_torque[i];
	}
}



void HapticHandler::addPerturbation(const Perturbation& perturbation)
{
	//Buffer a new perturbation and wait for the haptic thread to incorporate it
#ifndef NO_HARDWARE_DEBUG
	WaitForSingleObject(newPerturbationEvent, INFINITE);
	buffered_new_perturbation = perturbation;
	if (!(ResetEvent(newPerturbationEvent))) ErrorExit(L"Failed to reset newPerturbationEvent");
#else
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Can't add perturbations in no hardware mode", L"HapticHandler");
#endif
#endif
}

void HapticHandler::removePerturbation(const Perturbation& perturbation)
{
	//Buffer the perturbation to be removed and wait for the haptic thread to remove it
#ifndef NO_HARDWARE_DEBUG
	WaitForSingleObject(removePerturbationEvent, INFINITE);
	buffered_remove_perturbation = perturbation;
	if (!(ResetEvent(newPerturbationEvent))) ErrorExit(L"Failed to reset removePerturbationEvent");
#else
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Can't remove perturbations in no hardware mode", L"HapticHandler");
#endif
#endif
}

void HapticHandler::addTrialForce(HDdouble x, HDdouble y, HDdouble z)
{
	//Add new force data and wait for the haptic thread to update his force values
#ifndef NO_HARDWARE_DEBUG
	WaitForSingleObject(forceUpdateEvent, INFINITE);
	force_buffer.push_back(hduVector3Dd{ x, y, z });
	if (!new_force_data) new_force_data = true;
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Added force with values " + x.ToString() + L", " + y.ToString() + L", " + z.ToString() + L" to the force buffer", L"HapticHandler");
#endif
	if (!ResetEvent(forceUpdateEvent)) ErrorExit(L"Failed to reset forceUpdateEvent");
#else
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Can't add trial forces in no hardware mode", L"HapticHandler");
#endif
#endif
}

void HapticHandler::clearTrialForce()
{
	//Remove forces and wait for haptic thread to commit changes
#ifndef NO_HARDWARE_DEBUG
	WaitForSingleObject(forceUpdateEvent, INFINITE);
	force_buffer.clear();
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Cleared force buffer", L"HapticHandler");
#endif
	if (!ResetEvent(forceUpdateEvent)) ErrorExit(L"Failed to reset forceUpdateEvent");
#else
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Can't clear trial forces in no hardware mode", L"HapticHandler");
#endif
#endif
}

void HapticHandler::addTrialTorque(HDdouble roll, HDdouble pitch, HDdouble yaw)
{
	//Add new torque data and wait for the haptic thread to update his force values
#ifndef NO_HARDWARE_DEBUG
	WaitForSingleObject(torqueUpdateEvent, INFINITE);
	gimbal_torque_buffer.push_back(hduVector3Dd{ roll, pitch, yaw });
	if (!new_torque_data) new_torque_data = true;
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Added torque with values " + roll.ToString() + L", " + pitch.ToString() + L", " + yaw.ToString() + L" to the torque buffer", L"HapticHandler");
#endif
	if (!ResetEvent(torqueUpdateEvent)) ErrorExit(L"Failed to reset torqueUpdateEvent");
#else
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Can't add trial torque in no hardware mode", L"HapticHandler");
#endif
#endif
}

void HapticHandler::clearTrialTorque()
{
	//Remove torque and wait for haptic thread to commit changes
#ifndef NO_HARDWARE_DEBUG
	WaitForSingleObject(torqueUpdateEvent, INFINITE);
	gimbal_torque_buffer.clear();
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Cleared torque buffer", L"HapticHandler");
#endif
	if (!ResetEvent(torqueUpdateEvent)) ErrorExit(L"Failed to reset torqueUpdateEvent");
#else
#ifdef LOG_DEBUG
	Debug::WriteLine(L"Can't clear trial torque in no hardware mode", L"HapticHandler");
#endif
#endif
}

void HapticHandler::setForceEnabled(bool tf)
{
	//Enable/disable haptic force feedback
	enable_force_feedback = tf;
	if (tf) Console::WriteLine(L"Force/Torque feedback enabled", L"HapticHandler");
	else Console::WriteLine(L"Force/Torque feedback disabled", L"HapticHandler");
}

DWORD WINAPI posBufferThread(LPVOID lpParam)
{
	HapticHandler* hapticHandler = static_cast<HapticHandler*>(lpParam);
	
	//Update positional buffers for graphics display
	while (WaitForSingleObject(hapticHandler->stopPositionBufferEvent, 0) == WAIT_TIMEOUT)
	{
#ifndef NO_HARDWARE_DEBUG
		WaitForSingleObject(hapticHandler->positionUpdateEvent, INFINITE);
		ExperimentCommon::transform_buffer.push_back(hapticHandler->current_transform_matrix);
		ExperimentCommon::transform_buffer.erase(ExperimentCommon::transform_buffer.begin());
		ExperimentCommon::z_angle_buffer.push_back(hapticHandler->current_z_angle);
		ExperimentCommon::z_angle_buffer.erase(ExperimentCommon::z_angle_buffer.begin());
		if (!(ResetEvent(hapticHandler->positionUpdateEvent))) ErrorExit(L"Failed to reset positionUpdateEvent");
#endif
		Sleep(1000 / FRAMES_PER_SECOND);
	}
	
	return 0;
}
