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

	bool genHoi4Scenario = root.get<bool>("randomScenario.genhoi4");
	// use the same input heightmap for every scenario/map generation
	bool useGlobalExistingHeightmap = root.get<bool>("randomScenario.inputheightmap");
	// get the path
	std::string globalHeightMapPath = root.get<std::string>("randomScenario.heightmapPath");

	bool useDefaultMap = false;
	bool useDefaultStates = false;
	bool useDefaultProvinces = false;

	bool genHoi4 = true;

	if (!Data::getInstance().getConfig("config.json"))
	{
		system("pause");
		return -1;
	}
	FastWorldGenerator fastWorldGen;
	Hoi4Module hoi4Mod;
	if (!useDefaultMap)
	{
		if (useGlobalExistingHeightmap)
		{
			// overwrite settings of fastworldgen
			Data::getInstance().heightmapIn = globalHeightMapPath;
			Data::getInstance().genHeight = false;
		}
		fastWorldGen.generateWorld();
	}
	ScenarioGenerator sG(fastWorldGen);
	if (genHoi4)
	{
		hoi4Mod.genHoi(useDefaultMap, useDefaultStates, useDefaultProvinces, sG);
	}
	return 0;

}

