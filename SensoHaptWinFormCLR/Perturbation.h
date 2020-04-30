#pragma once
#ifndef _PERTURBATION
#define _PERTURBATION

#include "HDU\hduVector.h"

//Class modelling a force perturbation
class Perturbation{
	friend bool operator==(const Perturbation&, const Perturbation&);
public:
	Perturbation(const hduVector3Dd& peak_intensity, int frequency, int phase_shift);

	void setStartTime(double new_start_time) { start_time = new_start_time; }
	
	hduVector3Dd getPerturbationForce(double current_time) const;
private:
	hduVector3Dd peak_intensity;

	double start_time;
	
	int frequency, phase_shift;
};

inline bool operator==(const Perturbation& lhs, const Perturbation& rhs)
{
	if (lhs.peak_intensity == rhs.peak_intensity && lhs.frequency == rhs.frequency && lhs.phase_shift == rhs.phase_shift
		&& lhs.start_time == rhs.start_time) return true;
	else return false;
}

#endif