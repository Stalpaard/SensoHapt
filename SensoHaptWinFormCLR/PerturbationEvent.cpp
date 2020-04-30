#include "stdafx.h"

#include "PerturbationEvent.h"
#include "ExperimentController.h"

using namespace System::Diagnostics;

PerturbationEvent::PerturbationEvent(int start_timestamp, int end_timestamp, const hduVector3Dd& peak_intensity, int frequency, int phase_shift) :
	Event{ start_timestamp, end_timestamp }, perturbation{peak_intensity, frequency, phase_shift}
{

}

void PerturbationEvent::eventActions(float start_timer_time)
{
	//Initialize perturbation and request perturbation addition from ExperimentController
	perturbation.setStartTime(start_timer_time);
	experimentController->requestAddPerturbation(perturbation);
}

void PerturbationEvent::stopAction(float stop_timer_time)
{
	//Request perturbation removal from ExperimentController
	experimentController->requestRemovePerturbation(perturbation);
}

