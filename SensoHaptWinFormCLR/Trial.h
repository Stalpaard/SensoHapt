#pragma once
#ifndef _TRIAL
#define _TRIAL

#include <vector>
#include <array>
#include <HD/hdDefines.h>
#include "Event.h"

//Class modelling a trial in the experiment
class Trial
{
public:
	Trial(int id, int duration_s, int visual_delay_ms, int touch_activated_s, HDdouble cube_weight_kg, const std::vector<std::shared_ptr<Event>>& events_vec, const std::array<GLfloat,4>& cube_colour_rgba)
		: id{id}, duration{ duration_s }, visual_delay{ visual_delay_ms }, touch_activated{ touch_activated_s }, cube_weight{ cube_weight_kg }, events{ events_vec }
	{
		cube_colour = cube_colour_rgba;
	}

	int duration, touch_activated, visual_delay, id;
	HDdouble cube_weight;
	std::array<GLfloat, 4> cube_colour;
	std::vector<std::shared_ptr<Event>> events;
};

#endif