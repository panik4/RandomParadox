#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "FormatConverter.h"
#include "generic/ScenarioGenerator.h"
#include "hoi4/Hoi4Module.h"
using namespace std;


void readConfig() {

}


int main() {
	// Read the basic settings
	ifstream f("basic_settings.json");
	std::stringstream buffer;
	if (!f.good())
	{
		std::cout << "Config could not be loaded" << std::endl;
	}
	buffer << f.rdbuf();
	// Short alias for this namespace
	namespace pt = boost::property_tree;
	// Create a root
	pt::ptree root;
	pt::read_json(buffer, root);

	// if debug is enabled in the config, a directory subtree containing visualisation of many maps will be created
	bool debug = root.get<bool>("randomScenario.debug");
	if (debug) {
		std::experimental::filesystem::create_directory("Maps");
		std::experimental::filesystem::create_directory("Maps\\world");
		std::experimental::filesystem::create_directory("Maps\\resources");
		std::experimental::filesystem::create_directory("Maps\\layers");
	}
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

	if (!Data::getInstance().getConfig("config.json"))
	{
		system("pause");
		return -1;
	}
	FastWorldGenerator fastWorldGen;
	Hoi4Module hoi4Mod;
	hoi4Mod.readConfig();
	hoi4Mod.findHoi4();
	if (!useDefaultMap) {
		if (useGlobalExistingHeightmap) {
			// overwrite settings of fastworldgen
			Data::getInstance().heightmapIn = globalHeightMapPath;
			Data::getInstance().genHeight = false;
			Data::getInstance().latLow = latLow;
			Data::getInstance().latHigh = latHigh;
		}
		fastWorldGen.generateWorld();
	}

	ScenarioGenerator sG(fastWorldGen);
	if (genHoi4Scenario)
	{
		hoi4Mod.genHoi(useDefaultMap, useDefaultStates, useDefaultProvinces, sG);
	}
	return 0;

}

