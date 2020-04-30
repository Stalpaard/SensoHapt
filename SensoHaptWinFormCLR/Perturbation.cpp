#include "stdafx.h"

#include "Perturbation.h"

Perturbation::Perturbation(const hduVector3Dd& peak_intensity, int frequency, int phase_shift) :
	peak_intensity{ peak_intensity }, frequency{ frequency }, phase_shift{ phase_shift }
{

}

hduVector3Dd Perturbation::getPerturbationForce(double current_time) const
{
	//Calculate new perturbation force corresponding to the current trial time
	double elapsed_time = current_time - start_time;

	hduVector3Dd perturbation_force{
		peak_intensity[0] * sin(2 * M_PI*frequency * elapsed_time + phase_shift),
		peak_intensity[1] * sin(2 * M_PI*frequency * elapsed_time + phase_shift),
		peak_intensity[2] * sin(2 * M_PI*frequency * elapsed_time + phase_shift)
	};

	return perturbation_force;
}