#pragma once
#ifndef _EVENT
#define _EVENT

#include <memory>

class ExperimentController;

//Abstract Event class containing possible trial event actions
class Event
{
	public:
		Event(int start_timestamp, int end_timestamp) : start_timestamp{ start_timestamp }, end_timestamp{ end_timestamp }, experimentController{nullptr}
		{}

		int getStartTimestamp() const { return start_timestamp; }
		int getEndTimestamp() const { return end_timestamp; }

		void setController(const ExperimentController* experimentController) { this->experimentController = experimentController; }

		bool action(float start_timer_time)
		{
			if (experimentController)
			{
				eventActions(start_timer_time);
				return true;
			}
			else return false;
		}

		virtual void eventActions(float start_timer_time) = 0;

		virtual void stopAction(float stop_timer_time) = 0;

	protected:
		const ExperimentController* experimentController;
	private:
		const int start_timestamp, end_timestamp;
		
};

inline bool eventTimeSort(std::shared_ptr<Event> i, std::shared_ptr<Event> j)
{
	if (i->getStartTimestamp() < j->getStartTimestamp()) return true;
	else return false;
}

#endif
