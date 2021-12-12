#include "Hoi4Module.h"



Hoi4Module::Hoi4Module()
{
}


Hoi4Module::~Hoi4Module()
{
}

void Hoi4Module::genHoi(std::string hoi4ModPath, std::string hoi4Path, FastWorldGenerator f, bool useDefaultMap, bool useDefaultStates, bool useDefaultProvinces, ScenarioGenerator& scenGen)
{
	// validate options:
	if (!useDefaultProvinces)
	{
		useDefaultStates = false;
	}
	if (!useDefaultMap)
	{
		useDefaultProvinces = false;
		useDefaultStates = false;
	}

	Hoi4Parser hoiParse;
	// prepare folder structure
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\map\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\common\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\history\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\history\\states\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\history\\states\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\map\\strategicregions\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\map\\strategicregions\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\map\\supplyareas\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\map\\supplyareas\\");
	//std::experimental::filesystem::remove_all(hoi4ModPath + "\\history\\countries\\");
	//std::experimental::filesystem::create_directory(hoi4ModPath + "\\history\\countries\\");
	if (useDefaultMap)
	{
		scenGen.hoi4Preparations(); // load files, read states/create states
		scenGen.mapRegions(); // create gameRegions
		scenGen.generateCountries();
		scenGen.dumpDebugCountrymap(Data::getInstance().debugMapsPath + "countries.bmp");
		Hoi4ScenarioGenerator hoi4Gen(f, scenGen);

		hoiParse.dumpStates(hoi4ModPath + "\\history\\states", scenGen.countryMap);
		if (useDefaultStates)
		{
			hoiParse.dumpUnitStacks(hoi4ModPath + "\\map\\unitstacks.txt", scenGen.f.provinceGenerator.provinces);
			hoiParse.dumpRocketSites(hoi4ModPath + "\\map\\rocketsites.txt", scenGen.f.provinceGenerator.regions);
			hoiParse.dumpStrategicRegions(hoi4ModPath + "\\map\\strategicregions", scenGen.f.provinceGenerator.regions);
			hoiParse.dumpSupplyAreas(hoi4ModPath + "\\map\\supplyareas", scenGen.f.provinceGenerator.regions);
			hoiParse.dumpWeatherPositions(hoi4ModPath + "\\map\\weatherpositions.txt", scenGen.f.provinceGenerator.regions);
			hoiParse.dumpAdjacencyRules(hoi4ModPath + "\\map\\adjacency_rules.txt");
		}
		//hoiParse.dumpAdj(hoi4ModPath + "\\map\\adjacencies.csv");
		hoiParse.dumpAirports(hoi4ModPath + "\\map\\airports.txt", scenGen.f.provinceGenerator.regions);
		hoiParse.dumpBuildings(hoi4ModPath + "\\map\\buildings.txt", scenGen.f.provinceGenerator.regions);
		hoiParse.writeCompatibilityHistory(hoi4ModPath + "\\history\\countries\\", hoi4Path + "\\history\\countries\\", scenGen.f.provinceGenerator.regions);
		Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("provinces"), (hoi4ModPath + ("\\map\\provinces.bmp")).c_str());
	}
	else {
		scenGen.mapRegions();
		scenGen.generateCountries();
		Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("provinces"), (hoi4ModPath + ("\\map\\provinces.bmp")).c_str());

		Hoi4ScenarioGenerator hoi4Gen(f, scenGen);
		//hoiParse.readDefaultCountries();
		hoiParse.dumpAdj(hoi4ModPath + "\\map\\adjacencies.csv");
		hoiParse.dumpAirports(hoi4ModPath + "\\map\\airports.txt", f.provinceGenerator.regions);
		hoiParse.dumpBuildings(hoi4ModPath + "\\map\\buildings.txt", f.provinceGenerator.regions);
		hoiParse.dumpContinents(hoi4ModPath + "\\map\\continents.txt", f.provinceGenerator.continents);
		hoiParse.dumpDefinition(hoi4ModPath + "\\map\\definition.csv", f.provinceGenerator.provinces);
		hoiParse.dumpUnitStacks(hoi4ModPath + "\\map\\unitstacks.txt", f.provinceGenerator.provinces);
		hoiParse.dumpRocketSites(hoi4ModPath + "\\map\\rocketsites.txt", f.provinceGenerator.regions);
		hoiParse.dumpStrategicRegions(hoi4ModPath + "\\map\\strategicregions", f.provinceGenerator.regions);
		hoiParse.dumpSupplyAreas(hoi4ModPath + "\\map\\supplyareas", f.provinceGenerator.regions);
		hoiParse.dumpStates(hoi4ModPath + "\\history\\states", scenGen.countryMap);
		hoiParse.dumpWeatherPositions(hoi4ModPath + "\\map\\weatherpositions.txt", f.provinceGenerator.regions);
		hoiParse.dumpAdjacencyRules(hoi4ModPath + "\\map\\adjacency_rules.txt");

		// history
		// HOI 4:
		FormatConverter formatConverter;
		formatConverter.dump8BitTerrain(hoi4ModPath + "\\map\\terrain.bmp", "terrainHoi4");
		formatConverter.dump8BitCities(hoi4ModPath + "\\map\\cities.bmp", "citiesHoi4");
		formatConverter.dump8BitRivers(hoi4ModPath + "\\map\\rivers.bmp", "riversHoi4");
		formatConverter.dump8BitTrees(hoi4ModPath + "\\map\\trees.bmp", "treesHoi4");
		formatConverter.dump8BitHeightmap(hoi4ModPath + "\\map\\heightmap.bmp", "heightmapHoi4");
		//formatConverter.dumpDDSFiles(hoi4ModPath + "\\map\\terrain\\colormap_water_");
		formatConverter.dumpWorldNormal(hoi4ModPath + "\\map\\world_normal.bmp");
		hoiParse.writeCompatibilityHistory(hoi4ModPath + "\\history\\countries\\", hoi4Path + "\\history\\countries\\", f.provinceGenerator.regions);
	}
	scenGen.dumpDebugCountrymap(Data::getInstance().debugMapsPath + "countries.bmp");
}
