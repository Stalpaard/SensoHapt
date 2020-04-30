#pragma once
#ifndef _HAPTICHANDLER
#define _HAPTICHANDLER

#include <list>
#include <vector>

#include "HL\hl.h"
#include "HDU\hduVector.h"
#include "HDU\hduMatrix.h"
#include "Perturbation.h"

class Perturbation;

//Class for interfacing with the Phantom HD
class HapticHandler {
	friend HDCallbackCode HDCALLBACK HDSimulationCallback(void*);
	friend DWORD WINAPI posBufferThread(LPVOID lpParam);
	friend void keyboardFunc(unsigned char key, int x, int y);
	friend void drawScene();
public:
	HapticHandler(const HDstring& device_name, const hduVector3Dd& max_torque, HDdouble safety_temperature_margin = 0.18, double rampup_rate = 0.2, double rampup_step = 0.0002, HDdouble max_force_ratio = 1.0);
	
	~HapticHandler();

	void addTrialForce(HDdouble x, HDdouble y, HDdouble z); //add trial force in N

	void addTrialTorque(HDdouble x, HDdouble y, HDdouble z);//add trial force in mNm

	void addPerturbation(const Perturbation& perturbation);

	void removePerturbation(const Perturbation& perturbation);

	void clearTrialForce();

	void clearTrialTorque();

private:

	void clampForce();

	void clampTorque();

	void setForceEnabled(bool tf);
	
	hduVector3Dd current_trial_force, current_trial_torque;

	const HDdouble max_force_ratio //coefficient for further reducing maximum force
		, safety_temperature_margin;
	const hduVector3Dd max_torque; //has to be looked up on documentations
		
	
	static HDdouble device_offsetx, device_offsety, device_offsetz; //(can be bound to keys in KeyboardCallbackFunctions / needs synchronization)

	bool enable_force_feedback, new_force_data, new_torque_data;
	
	

	HDdouble max_force, //nominal max force defined by haptic device
		motor_temperatures[6],
		current_z_angle;

	const double rampup_step; // determines speed of rampup

	double rampup_rate; //Variable to increase power slowly at startup

	float last_read_time; //Needed for thread synchronization
	

	HHD device_handle;
	HDSchedulerHandle scheduler_handle;
	
	std::vector<hduVector3Dd> force_buffer, gimbal_torque_buffer; //Force buffer containing requested forces

	std::vector<Perturbation> perturbation_vec;

	HANDLE hPosBufferThread, stopPositionBufferEvent, 
		positionUpdateEvent, forceUpdateEvent, torqueUpdateEvent, 
		newPerturbationEvent, removePerturbationEvent;

	DWORD dwPosBufferThread;

	Perturbation buffered_new_perturbation, buffered_remove_perturbation;

	hduMatrix current_transform_matrix;
	
};

#endif