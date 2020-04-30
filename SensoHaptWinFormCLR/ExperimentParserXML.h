#pragma once
#ifndef _EXPERIMENTPARSERXML
#define _EXPERIMENTPARSERXML

#include "ExperimentParser.h"

//XML experiment configuration parser
class ExperimentParserXML : public ExperimentParser
{
	public:
		ExperimentParserXML() : ExperimentParser{} {}
		ExperimentInfo operator() (const std::string& filePath) const override;
};

#endif