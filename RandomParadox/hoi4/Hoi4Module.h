#pragma once
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../FormatConverter.h"
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "Hoi4Parser.h"
#include "Hoi4ScenarioGenerator.h"
class Hoi4Module
{
	// member variables
	Hoi4ScenarioGenerator hoi4Gen;
	int numCountries;
	std::string modName;
	std::string hoi4Path;
	std::string hoi4ModPath;
	std::string hoi4ModsDirectory;
public:
	Hoi4Module();
	~Hoi4Module();
	// member functions
	// clear and create all the mod paths at each run
	bool createPaths();
	void genHoi(bool useDefaultMap, bool useDefaultStates, bool useDefaultProvinces, ScenarioGenerator& scenGen);
	// try to locate hoi4 at configured path, if not found, try other standard locations
	bool findHoi4();
	// read the config specific to hoi4
	void readConfig();
};

