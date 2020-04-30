#pragma once
#ifndef SENSORCONTAINER_H
#define SENSORCONTAINER_H

//Container class for FTSystem variables
ref class SensorContainer {
public:
	static ATICombinedDAQFT::FTSystem sL, sR;
};
#endif