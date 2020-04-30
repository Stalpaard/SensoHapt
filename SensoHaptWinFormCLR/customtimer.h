#ifndef _CUSTOMTIMER
#define _CUSTOMTIMER

#include <Windows.h>

//Custom timer class using the QueryPerformanceCounter (high-precision)
//Note: depends on hardware whether this function is available

class CustomTimer
{
public:
	CustomTimer(float beginTime) : time{ beginTime } { reset(); }
	
	void reset();

	void update();

	float time;

private:
	float beforeQuadpart, freqQuadPart;
	LARGE_INTEGER nFreq;
	LARGE_INTEGER nBefore;
	LARGE_INTEGER nAfter;
};


#endif



