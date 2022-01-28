#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../FormatConverter.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Hoi4Parser.h"
//#include <filesystem>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include "Hoi4ScenarioGenerator.h"
class Hoi4Module
{
	std::string hoi4Path;
	std::string hoi4ModPath;
	int numCountries;
	Hoi4ScenarioGenerator hoi4Gen;
public:
	Hoi4Module();
	~Hoi4Module();
	void createPaths();
	bool findHoi4();
	void genHoi(bool useDefaultMap, bool useDefaultStates, bool useDefaultProvinces, ScenarioGenerator& scenGen);
	void readConfig();
};

