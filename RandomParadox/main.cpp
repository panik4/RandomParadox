#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "FormatConverter.h"
#include "generic/ScenarioGenerator.h"
#include "hoi4/Hoi4Module.h"


void readConfig() {

}


int main() {
	// Read the basic settings
	std::ifstream f("RandomParadox.json");
	std::stringstream buffer;
	if (!f.good())
		std::cout << "Config could not be loaded" << std::endl;
	buffer << f.rdbuf();
	// Short alias for this namespace
	namespace pt = boost::property_tree;
	// Create a root
	pt::ptree root;
	try {
		pt::read_json(buffer, root);
	}
	catch (std::exception e) {
		logLine("Incorrect config \"RandomParadox.json\"");
		logLine("You can try fixing it yourself. Error is: ", e.what());
		logLine("Otherwise try running it through a json validator, e.g. \"https://jsonlint.com/\" or search for \"json validator\"");
		system("pause");
		return -1;
	}

	// if debug is enabled in the config, a directory subtree containing visualisation of many maps will be created
	bool debug = root.get<bool>("randomScenario.debug");

	// generate hoi4 scenario or not
	bool genHoi4Scenario = root.get<bool>("randomScenario.genhoi4");
	// use the same input heightmap for every scenario/map generation
	bool useGlobalExistingHeightmap = root.get<bool>("randomScenario.inputheightmap");
	// get the path
	std::string globalHeightMapPath = root.get<std::string>("randomScenario.heightmapPath");
	// read the configured latitude range. 0.0 = 90 degrees south, 2.0 = 90 degrees north
	auto latLow = root.get<double>("randomScenario.latitudeLow");
	auto latHigh = root.get<double>("randomScenario.latitudeHigh");

	bool useDefaultMap = false;
	bool useDefaultStates = false;
	bool useDefaultProvinces = false;

	// check if we can read the config
	try {
		if (!Data::getInstance().getConfig("FastWorldGenerator.json")) {
			system("pause");
			return -1;
		}
	}
	catch (std::exception e) {
		logLine("Incorrect config \"FastWorldGenerator.json\"");
		logLine("You can try fixing it yourself. Error is: ", e.what());
		logLine("Otherwise try running it through a json validator, e.g. \"https://jsonlint.com/\" or \"search for json validator\"");
		system("pause");
		return -1;
	}
	// if we don't want the FastWorldGenerator output in MapsPath, debug = 0 turns this off
	if (!debug) {
		Data::getInstance().writeMaps = false;
	}
	NameGenerator::prepare();
	FastWorldGenerator fastWorldGen;
	Hoi4Module hoi4Mod;
	hoi4Mod.readConfig();
	if (!useDefaultMap) {
		// if we configured to use an existing heightmap
		if (useGlobalExistingHeightmap) {
			// overwrite settings of fastworldgen
			Data::getInstance().heightmapIn = globalHeightMapPath;
			Data::getInstance().genHeight = false;
			Data::getInstance().latLow = latLow;
			Data::getInstance().latHigh = latHigh;
		}
		// now run the world generation
		fastWorldGen.generateWorld();
	}
	// now start the generation of the scenario with the generated map files
	ScenarioGenerator sG(fastWorldGen);
	// and now check if we need to generate game specific files
	if (genHoi4Scenario)
		// generate hoi4 scenario
		hoi4Mod.genHoi(useDefaultMap, useDefaultStates, useDefaultProvinces, sG);
	return 0;

}

