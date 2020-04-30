#pragma once
#ifndef _PERTURBATION_EVENT
#define _PERTURBATION_EVENT

#include <memory>
#include "Event.h"
#include "Perturbation.h"

//Trial event for adding/removing perturbations
class PerturbationEvent : public Event
{
	public:
		PerturbationEvent(int start_timestamp, int end_timestamp, const hduVector3Dd& peak_intensity, int frequency, int phase_shift);
		void eventActions(float start_timer_time) override;
		void stopAction(float stop_timer_time) override;
	private:
		Perturbation perturbation;
};

#endif