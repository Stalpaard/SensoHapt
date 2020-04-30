#include "stdafx.h"

#include "ExperimentParserXML.h"
#include "event_headers.h"

#define INTENSITY_DELIMITER ","
#define COLOUR_DELIMITER ","

#define INTENSITY_DIMENSION 3
#define COLOUR_DIMENSION 4

#define DEFAULT_COLOUR_RGBA 1.0f,1.0f,0.0f,1.0f

using namespace pugi;
using namespace System::Diagnostics;


/* NOTES ON CONVERSION

as_bool(): returns false in case first char isn't: '1', 't', 'T', 'y', 'Y'

as_int(), as_double(): undefined behaviour in case of non-numeric strings

*/


ExperimentInfo ExperimentParserXML::operator() (const std::string& filePath) const
{
	ExperimentInfo expInfo;
	xml_document doc;
	if (!(doc.load_file(filePath.c_str()))) throw ParseException(L"Failed to load XML file");

	xml_node experiment_node = doc.child("Experiment");

	//Parsing experiment attributes
	bool parsed_answers_enabled = experiment_node.attribute("answers_enabled").as_bool();
	
	bool parsed_random_enabled = experiment_node.attribute("random_enabled").as_bool();

	int parsed_randomize_factor = experiment_node.attribute("randomize_factor").as_int();
	if (parsed_random_enabled) 
		if (parsed_randomize_factor <= 0) throw ParseException(L"Invalid randomize factor");

	int parsed_sampling_rate_datalog = experiment_node.attribute("sampling_rate_datalog").as_int();
	if (parsed_sampling_rate_datalog <= 0) throw ParseException(L"Invalid  experiment datalog sampling rate");

	int amount_of_trials = experiment_node.select_nodes("Trial").size();
	std::vector<std::shared_ptr<Trial>> parsed_trials;
	
	//Populate trial vector
	if (!(parsed_random_enabled)) parsed_trials = std::vector<std::shared_ptr<Trial>>{ unsigned(amount_of_trials), std::make_shared<Trial>( 0,0,0,0,0,std::vector<std::shared_ptr<Event>>{},std::array<GLfloat,4>{0,0,0,0} ) };

	//Parse trial nodes
	for (xml_node trial : experiment_node.children("Trial"))
	{
		//Default trial values
		int parsed_duration_s;
		int parsed_touch_activated_s = 0;
		int parsed_visual_delay_ms = 0;
		HDdouble parsed_cube_weight_kg = 0.0;
		std::vector<std::shared_ptr<Event>> parsed_events;
		std::array<GLfloat, COLOUR_DIMENSION> parsed_cube_colour{ DEFAULT_COLOUR_RGBA }; 
		
		//Parse trial id
		int parsed_id = trial.attribute("id").as_int();
		std::wstring trial_order_wstring = std::to_wstring(parsed_id);
		if (parsed_id < 0 || parsed_id >= amount_of_trials) throw ParseException(L"Invalid trial id: " + trial_order_wstring);
		

		//Parse trial duration
		parsed_duration_s = trial.attribute("duration").as_int();
		if (parsed_duration_s <= 0) throw ParseException(L"Invalid trial duration in trial: " + trial_order_wstring);

		//Parse trial touch_activated
		xml_node touch_activated_node = trial.child("touch_activated");
		if (!(touch_activated_node.empty())) 
		{
			parsed_touch_activated_s = touch_activated_node.attribute("threshold").as_int();
			if (parsed_touch_activated_s < 0 || parsed_touch_activated_s >= parsed_duration_s) throw ParseException(L"Invalid touch_activated value in trial: " + trial_order_wstring);
		}

		//Parse cube colour
		xml_node cube_colour = trial.child("cube_colour");
		if (!(cube_colour.empty()))
		{
			std::string colour_string{ cube_colour.attribute("rgba").value() };
			std::string rgba_delimiter = COLOUR_DELIMITER;
			std::size_t pos = 0;
			int i = 0;
			while ((pos = colour_string.find(rgba_delimiter)) != std::string::npos)
			{
				try {
					parsed_cube_colour.at(i++) = std::stof(colour_string.substr(0, pos));
					colour_string.erase(0, pos + rgba_delimiter.length());
				}
				catch (std::out_of_range e)
				{
					throw ParseException(L"Invalid cube_colour rgba value in trial: " + trial_order_wstring);
				}
			}
			if(i < COLOUR_DIMENSION) throw ParseException(L"Invalid cube_colour rgba value in trial: " + trial_order_wstring);
		}
		
		//Parse trial cube_weight
		xml_node weight_node = trial.child("weight");
		if (!(weight_node.empty())) 
		{
			parsed_cube_weight_kg = weight_node.attribute("kg").as_double();
			if (parsed_cube_weight_kg < 0) throw ParseException(L"Invalid weight value in trial: " + trial_order_wstring);
		}

		//Parse trial visual_delay
		xml_node visual_delay_node = trial.child("visual_delay");
		if (!(visual_delay_node.empty())) 
		{
			parsed_visual_delay_ms = visual_delay_node.attribute("delay").as_int();
			if (parsed_visual_delay_ms < 0 || parsed_visual_delay_ms > 983) throw ParseException(L"Invalid visual delay value in trial: " + trial_order_wstring);
		}
			
		//Parse event nodes
		for (xml_node event : trial.children("event"))
		{
			int start_timestamp = event.attribute("start_timestamp").as_int();
			if (start_timestamp < 0 || start_timestamp >= parsed_duration_s) throw ParseException(L"Invalid event start_timestamp value in trial: " + trial_order_wstring);

			int end_timestamp = event.attribute("end_timestamp").as_int();
			if (end_timestamp < 0 || end_timestamp > parsed_duration_s) throw ParseException(L"Invalid event end_timestamp value in trial: " + trial_order_wstring);

			for (xml_node event_comp : event.children("perturbation"))
			{
				int frequency = event_comp.attribute("frequency").as_int();
				if (frequency <= 0) throw ParseException(L"Invalid perturbation frequency in trial: " + trial_order_wstring);
					
				int phase_shift = event_comp.attribute("phase_shift").as_int();
				if(phase_shift < 0) throw ParseException(L"Invalid perturbation phase_shift in trial: " + trial_order_wstring);

				std::array<HDdouble, INTENSITY_DIMENSION> parsed_peak_intensity;
				std::string peak_intensity_string{ event_comp.attribute("peak_intensity").value() };
				std::string delimiter = INTENSITY_DELIMITER;
				std::size_t pos = 0;
				int i = 0;
				while ((pos = peak_intensity_string.find(delimiter)) != std::string::npos)
				{
					try {
						parsed_peak_intensity.at(i++) = std::stod(peak_intensity_string.substr(0, pos));
						peak_intensity_string.erase(0, pos + delimiter.length());
					}
					catch (std::out_of_range e)
					{
						throw ParseException(L"Invalid perturbation peak_intensity in trial: " + trial_order_wstring);
					}
				}
				if (i < INTENSITY_DIMENSION) throw ParseException(L"Invalid perturbation peak_intensity in trial: " + trial_order_wstring);

				parsed_events.push_back(std::make_shared<PerturbationEvent>(start_timestamp, end_timestamp, 
					hduVector3Dd{ parsed_peak_intensity.at(0), parsed_peak_intensity.at(1), parsed_peak_intensity.at(2) }, frequency, phase_shift));
			}
		}

		if (parsed_events.size() > 0) std::sort(parsed_events.begin(), parsed_events.end(), eventTimeSort);

		//If random isn't enabled, use ascending order according to trial IDs
		if (parsed_random_enabled)
		{
			std::shared_ptr<Trial> new_trial = std::make_shared<Trial>(parsed_id, parsed_duration_s, parsed_visual_delay_ms, parsed_touch_activated_s, parsed_cube_weight_kg, parsed_events, parsed_cube_colour);
			for (int i = 0; i < parsed_randomize_factor; i++) parsed_trials.push_back(new_trial);
		}
		else parsed_trials.at(parsed_id) = std::make_shared<Trial>( parsed_id, parsed_duration_s, parsed_visual_delay_ms, parsed_touch_activated_s, parsed_cube_weight_kg, parsed_events, parsed_cube_colour );
	}


	//Randomize trial order if random_enabled
	if (parsed_random_enabled)
	{
		std::srand(unsigned(std::time(0)));
		std::random_shuffle(parsed_trials.begin(), parsed_trials.end());
	}


	expInfo.trials = parsed_trials;
	expInfo.answers_enabled = parsed_answers_enabled;
	expInfo.sampling_rate_datalog = parsed_sampling_rate_datalog;

	return expInfo;
}

