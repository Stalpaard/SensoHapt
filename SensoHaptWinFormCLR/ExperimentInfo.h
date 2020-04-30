#pragma once
#ifndef _EXPERIMENTINFO
#define _EXPERIMENTINFO

#include <vector>
#include <Windows.h>

#include "Trial.h"

//Class encapsulating experiment-related info
class ExperimentInfo
{
	public:
		ExperimentInfo() = default;

		std::vector<std::shared_ptr<Trial>> trials;
		bool answers_enabled;
		DWORD sampling_rate_datalog;
};

#endif