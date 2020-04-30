#include "stdafx.h"

#include "customtimer.h"
#include "common_variables.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

CustomTimer ExperimentCommon::timer{ 0.0 }; //Timer to keep time to write to Data_file / perform perturbations

////////////////////////////////////////////////////////////////////////////////////////////////////


void CustomTimer::reset()
{
	time = 0;
	nFreq.QuadPart = 0;
	nBefore.QuadPart = 0;
	nAfter.QuadPart = 0;
	if (!(QueryPerformanceFrequency(&nFreq))) ErrorExit(L"Hardware system doesn't support a high-resolution performance counter");
	freqQuadPart = float(nFreq.QuadPart);
	QueryPerformanceCounter(&nBefore);
	beforeQuadpart = float(nBefore.QuadPart);
}

void CustomTimer::update()
{
	if (!ResetEvent(ExperimentCommon::writingTimeEvent)) ErrorExit(L"Failed to reset writingTimeEvent");
	QueryPerformanceCounter(&nAfter);
	time = ((float(nAfter.QuadPart) - beforeQuadpart) / freqQuadPart) * 1000; //Get elapsed milliseconds since start
	if (!SetEvent(ExperimentCommon::writingTimeEvent)) ErrorExit(L"Failed to signal writingTimeEvent");
}