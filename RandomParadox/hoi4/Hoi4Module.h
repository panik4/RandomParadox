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
	// member variables
	Hoi4ScenarioGenerator hoi4Gen;
	int numCountries;
	std::string hoi4Path;
	std::string hoi4ModPath;
public:
	Hoi4Module();
	~Hoi4Module();
	// member functions
	// clear and create all the mod paths at each run
	void createPaths();
	void genHoi(bool useDefaultMap, bool useDefaultStates, bool useDefaultProvinces, ScenarioGenerator& scenGen);
	// try to locate hoi4 at configured path, if not found, try other standard locations
	bool findHoi4();
	// read the config specific to hoi4
	void readConfig();
};

