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
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\gfx\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\history\\states\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\history\\states\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\map\\strategicregions\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\map\\strategicregions\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\map\\supplyareas\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\map\\supplyareas\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\gfx\\flags\\medium\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\gfx\\flags\\small\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\gfx\\flags\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\gfx\\flags\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\gfx\\flags\\small\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\gfx\\flags\\medium\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\common\\countries\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\common\\country_tags\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\common\\countries\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\common\\country_tags\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\history\\countries\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\history\\countries\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\history\\units\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\history\\units\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\localisation\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\common\\national_focus\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\common\\national_focus\\");
	if (useDefaultMap)
	{
		scenGen.hoi4Preparations(useDefaultStates, useDefaultProvinces); // load files, read states/create states
		scenGen.mapRegions(); // create gameRegions
		scenGen.generateCountries();
		scenGen.dumpDebugCountrymap(Data::getInstance().debugMapsPath + "countries.bmp");
		Hoi4ScenarioGenerator hoi4Gen(f, scenGen);

		hoiParse.dumpStates(hoi4ModPath + "\\history\\states", scenGen.countryMap);
		hoiParse.dumpUnitStacks(hoi4ModPath + "\\map\\unitstacks.txt", scenGen.f.provinceGenerator.provinces);
		hoiParse.dumpRocketSites(hoi4ModPath + "\\map\\rocketsites.txt", scenGen.f.provinceGenerator.regions);
		hoiParse.dumpStrategicRegions(hoi4ModPath + "\\map\\strategicregions", scenGen.f.provinceGenerator.regions);
		hoiParse.dumpSupplyAreas(hoi4ModPath + "\\map\\supplyareas", scenGen.f.provinceGenerator.regions);
		hoiParse.dumpWeatherPositions(hoi4ModPath + "\\map\\weatherpositions.txt", scenGen.f.provinceGenerator.regions);
		hoiParse.dumpAdjacencyRules(hoi4ModPath + "\\map\\adjacency_rules.txt");
		//hoiParse.dumpAdj(hoi4ModPath + "\\map\\adjacencies.csv");
		hoiParse.dumpAirports(hoi4ModPath + "\\map\\airports.txt", scenGen.f.provinceGenerator.regions);
		hoiParse.dumpBuildings(hoi4ModPath + "\\map\\buildings.txt", scenGen.f.provinceGenerator.regions);
		hoiParse.writeCompatibilityHistory(hoi4ModPath + "\\history\\countries\\", hoi4Path + "\\history\\countries\\", scenGen.f.provinceGenerator.regions);
		if (!useDefaultStates)
		{
			//hoiParse.dumpAdj(hoi4ModPath + "\\map\\adjacencies.csv");
			hoiParse.dumpDefinition(hoi4ModPath + "\\map\\definition.csv", scenGen.gameProvinces);
		}
		Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("provinces"), (hoi4ModPath + ("\\map\\provinces.bmp")).c_str());
	}
	else {
		scenGen.mapRegions();
		scenGen.mapContinents();
		scenGen.generateCountries();
		scenGen.evaluateNeighbours();
		scenGen.generateWorld();
		
		// countries
		Hoi4ScenarioGenerator hoi4Gen(f, scenGen);
		hoi4Gen.generateCountrySpecifics(scenGen, scenGen.countryMap);
		hoi4Gen.generateStateSpecifics(scenGen);
		hoi4Gen.evaluateCountries(scenGen);
		hoi4Gen.evaluateCountryGoals(scenGen);
		hoiParse.writeHistoryCountries(hoi4ModPath + "\\history\\countries\\", scenGen.countryMap);
		hoiParse.writeHistoryUnits(hoi4ModPath + "\\history\\units\\", scenGen.countryMap);
		hoiParse.dumpCommonCountryTags(hoi4ModPath + "\\common\\country_tags\\02_countries.txt", scenGen.countryMap);
		hoiParse.dumpCommonCountries(hoi4ModPath + "\\common\\countries\\", hoi4Path + "\\common\\countries\\colors.txt", scenGen.countryMap);
		Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("provinces"), (hoi4ModPath + ("\\map\\provinces.bmp")).c_str());

		//hoiParse.readDefaultCountries();
		hoiParse.dumpAdj(hoi4ModPath + "\\map\\adjacencies.csv");
		hoiParse.dumpAirports(hoi4ModPath + "\\map\\airports.txt", f.provinceGenerator.regions);
		hoiParse.dumpBuildings(hoi4ModPath + "\\map\\buildings.txt", f.provinceGenerator.regions);
		hoiParse.dumpContinents(hoi4ModPath + "\\map\\continents.txt", f.provinceGenerator.continents);
		hoiParse.dumpDefinition(hoi4ModPath + "\\map\\definition.csv", scenGen.gameProvinces);
		hoiParse.dumpUnitStacks(hoi4ModPath + "\\map\\unitstacks.txt", f.provinceGenerator.provinces);
		hoiParse.dumpRocketSites(hoi4ModPath + "\\map\\rocketsites.txt", f.provinceGenerator.regions);
		hoiParse.dumpStrategicRegions(hoi4ModPath + "\\map\\strategicregions", f.provinceGenerator.regions);
		hoiParse.dumpSupplyAreas(hoi4ModPath + "\\map\\supplyareas", f.provinceGenerator.regions);
		hoiParse.dumpStates(hoi4ModPath + "\\history\\states", scenGen.countryMap);
		hoiParse.dumpFlags(hoi4ModPath + "\\gfx\\flags\\", scenGen.countryMap);
		hoiParse.dumpWeatherPositions(hoi4ModPath + "\\map\\weatherpositions.txt", f.provinceGenerator.regions);
		hoiParse.dumpAdjacencyRules(hoi4ModPath + "\\map\\adjacency_rules.txt");
		hoiParse.writeStateNames(hoi4ModPath + "\\localisation\\", scenGen.countryMap);
		hoiParse.writeCountryNames(hoi4ModPath + "\\localisation\\", scenGen.countryMap);
		hoiParse.writeFoci(hoi4ModPath + "\\common\\national_focus\\", hoi4Gen.foci, scenGen.countryMap);

		// history
		// HOI 4:
		FormatConverter formatConverter;
		formatConverter.dump8BitTerrain(hoi4ModPath + "\\map\\terrain.bmp", "terrainHoi4");
		formatConverter.dump8BitCities(hoi4ModPath + "\\map\\cities.bmp", "citiesHoi4");
		formatConverter.dump8BitRivers(hoi4ModPath + "\\map\\rivers.bmp", "riversHoi4");
		formatConverter.dump8BitTrees(hoi4ModPath + "\\map\\trees.bmp", "treesHoi4");
		formatConverter.dump8BitHeightmap(hoi4ModPath + "\\map\\heightmap.bmp", "heightmapHoi4");
		formatConverter.dumpTerrainColourmap(hoi4ModPath + "\\map\\terrain\\colormap_rgb_cityemissivemask_a.dds");
		formatConverter.dumpDDSFiles(hoi4ModPath + "\\map\\terrain\\colormap_water_");
		formatConverter.dumpWorldNormal(hoi4ModPath + "\\map\\world_normal.bmp");
		//hoiParse.writeCompatibilityHistory(hoi4ModPath + "\\history\\countries\\", hoi4Path + "\\history\\countries\\", f.provinceGenerator.regions);
	}
	scenGen.dumpDebugCountrymap(Data::getInstance().debugMapsPath + "countries.bmp");
}

