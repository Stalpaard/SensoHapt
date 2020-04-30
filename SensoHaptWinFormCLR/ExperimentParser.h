#pragma once
#ifndef _EXPERIMENTPARSER
#define _EXPERIMENTPARSER

#include <string>
#include "ExperimentInfo.h"

//Abstract parser class for processing experiment configuration input files
class ExperimentParser
{
	public:	
		ExperimentParser() = default;


		virtual ExperimentInfo operator() (const std::string& filePath) const = 0;

		class ParseException
		{
			public:
				ParseException(const std::wstring& message) : message{ message } {}

				virtual const std::wstring description() const throw ()
				{
					return L"Parsing Error: " + message;
				}
			protected:
				std::wstring message;
		};
};



#endif