#include "Hoi4Module.h"



Hoi4Module::Hoi4Module()
{
}

Hoi4Module::~Hoi4Module()
{
}

void Hoi4Module::createPaths()
{
	// prepare folder structure
	std::experimental::filesystem::create_directory(hoi4ModPath);
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
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\map\\terrain\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\map\\terrain\\");
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
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\localisation\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\localisation\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\localisation\\english\\");
	std::experimental::filesystem::remove_all(hoi4ModPath + "\\common\\national_focus\\");
	std::experimental::filesystem::create_directory(hoi4ModPath + "\\common\\national_focus\\");
}

void Hoi4Module::genHoi(bool useDefaultMap, bool useDefaultStates, bool useDefaultProvinces, ScenarioGenerator& scenGen)
{
	createPaths();
	// validate options:
	if (!useDefaultProvinces)	{
		useDefaultStates = false;
	}
	if (!useDefaultMap)	{
		useDefaultProvinces = false;
		useDefaultStates = false;
	}
	if (useDefaultMap)	{
		scenGen.hoi4Preparations(useDefaultStates, useDefaultProvinces); // load files, read states/create states
		scenGen.mapRegions(); // create gameRegions
		scenGen.generateCountries(numCountries);
		scenGen.dumpDebugCountrymap(Data::getInstance().mapsPath + "countries.bmp");

		Hoi4Parser::dumpStates(hoi4ModPath + "\\history\\states", scenGen.countryMap);
		Hoi4Parser::dumpUnitStacks(hoi4ModPath + "\\map\\unitstacks.txt", scenGen.f.provinceGenerator.provinces);
		Hoi4Parser::dumpRocketSites(hoi4ModPath + "\\map\\rocketsites.txt", scenGen.f.provinceGenerator.regions);
		Hoi4Parser::dumpStrategicRegions(hoi4ModPath + "\\map\\strategicregions", scenGen.f.provinceGenerator.regions, hoi4Gen.strategicRegions);
		Hoi4Parser::dumpSupplyAreas(hoi4ModPath + "\\map\\supplyareas", scenGen.f.provinceGenerator.regions);
		Hoi4Parser::dumpWeatherPositions(hoi4ModPath + "\\map\\weatherpositions.txt", scenGen.f.provinceGenerator.regions, hoi4Gen.strategicRegions);
		Hoi4Parser::dumpAdjacencyRules(hoi4ModPath + "\\map\\adjacency_rules.txt");
		//Hoi4Parser::dumpAdj(hoi4ModPath + "\\map\\adjacencies.csv");
		Hoi4Parser::dumpAirports(hoi4ModPath + "\\map\\airports.txt", scenGen.f.provinceGenerator.regions);
		Hoi4Parser::dumpBuildings(hoi4ModPath + "\\map\\buildings.txt", scenGen.f.provinceGenerator.regions);
		if (!useDefaultStates)		{
			//Hoi4Parser::dumpAdj(hoi4ModPath + "\\map\\adjacencies.csv");
			Hoi4Parser::dumpDefinition(hoi4ModPath + "\\map\\definition.csv", scenGen.gameProvinces);
		}
		Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("provinces"), (hoi4ModPath + ("\\map\\provinces.bmp")).c_str());
	}
	else {
		// start with the generic stuff in the Scenario Generator
		scenGen.mapRegions();
		scenGen.mapContinents();
		scenGen.generateCountries(numCountries);
		scenGen.evaluateNeighbours();
		scenGen.generateWorld();

		// now generate hoi4 specific stuff
		hoi4Gen.generateCountrySpecifics(scenGen, scenGen.countryMap);
		hoi4Gen.generateStateSpecifics(scenGen);
		hoi4Gen.generateStateResources(scenGen);
		hoi4Gen.generateStrategicRegions(scenGen);
		hoi4Gen.evaluateCountries(scenGen);
		scenGen.dumpDebugCountrymap(Data::getInstance().mapsPath + "countries.bmp");
		hoi4Gen.generateLogistics(scenGen);
		hoi4Gen.evaluateCountryGoals(scenGen);

		// now start writing game files
		Hoi4Parser::writeCompatibilityHistory(hoi4ModPath + "\\history\\countries\\", hoi4Path, scenGen.f.provinceGenerator.regions);
		Hoi4Parser::writeHistoryCountries(hoi4ModPath + "\\history\\countries\\", scenGen.countryMap);
		Hoi4Parser::writeHistoryUnits(hoi4ModPath + "\\history\\units\\", scenGen.countryMap);
		Hoi4Parser::dumpCommonCountryTags(hoi4ModPath + "\\common\\country_tags\\02_countries.txt", scenGen.countryMap);
		Hoi4Parser::dumpCommonCountries(hoi4ModPath + "\\common\\countries\\", hoi4Path + "\\common\\countries\\colors.txt", scenGen.countryMap);
		Hoi4Parser::dumpAdj(hoi4ModPath + "\\map\\adjacencies.csv");
		Hoi4Parser::dumpAirports(hoi4ModPath + "\\map\\airports.txt", scenGen.f.provinceGenerator.regions);
		Hoi4Parser::dumpBuildings(hoi4ModPath + "\\map\\buildings.txt", scenGen.f.provinceGenerator.regions);
		Hoi4Parser::dumpContinents(hoi4ModPath + "\\map\\continents.txt", scenGen.f.provinceGenerator.continents);
		Hoi4Parser::dumpDefinition(hoi4ModPath + "\\map\\definition.csv", scenGen.gameProvinces);
		Hoi4Parser::dumpUnitStacks(hoi4ModPath + "\\map\\unitstacks.txt", scenGen.f.provinceGenerator.provinces);
		Hoi4Parser::dumpRocketSites(hoi4ModPath + "\\map\\rocketsites.txt", scenGen.f.provinceGenerator.regions);
		Hoi4Parser::dumpStrategicRegions(hoi4ModPath + "\\map\\strategicregions", scenGen.f.provinceGenerator.regions, hoi4Gen.strategicRegions);
		Hoi4Parser::dumpSupplyAreas(hoi4ModPath + "\\map\\supplyareas", scenGen.f.provinceGenerator.regions);
		Hoi4Parser::dumpStates(hoi4ModPath + "\\history\\states", scenGen.countryMap);
		Hoi4Parser::dumpFlags(hoi4ModPath + "\\gfx\\flags\\", scenGen.countryMap);
		Hoi4Parser::dumpWeatherPositions(hoi4ModPath + "\\map\\weatherpositions.txt", scenGen.f.provinceGenerator.regions, hoi4Gen.strategicRegions);
		Hoi4Parser::dumpAdjacencyRules(hoi4ModPath + "\\map\\adjacency_rules.txt");
		Hoi4Parser::dumpSupply(hoi4ModPath + "\\map\\", hoi4Gen.supplyNodeConnections);
		Hoi4Parser::writeStateNames(hoi4ModPath + "\\localisation\\english\\", scenGen.countryMap);
		Hoi4Parser::writeCountryNames(hoi4ModPath + "\\localisation\\english\\", scenGen.countryMap);
		Hoi4Parser::writeFoci(hoi4ModPath + "\\common\\national_focus\\", hoi4Gen.foci, scenGen.countryMap);

		// generate map files. Format must be converted and colours mapped to hoi4 compatbile colours
		FormatConverter formatConverter(hoi4Path);
		formatConverter.dump8BitTerrain(hoi4ModPath + "\\map\\terrain.bmp", "terrainHoi4");
		formatConverter.dump8BitCities(hoi4ModPath + "\\map\\cities.bmp", "citiesHoi4");
		formatConverter.dump8BitRivers(hoi4ModPath + "\\map\\rivers.bmp", "riversHoi4");
		formatConverter.dump8BitTrees(hoi4ModPath + "\\map\\trees.bmp", "treesHoi4");
		formatConverter.dump8BitHeightmap(hoi4ModPath + "\\map\\heightmap.bmp", "heightmapHoi4");
		formatConverter.dumpTerrainColourmap(hoi4ModPath + "\\map\\terrain\\colormap_rgb_cityemissivemask_a.dds");
		formatConverter.dumpDDSFiles(hoi4ModPath + "\\map\\terrain\\colormap_water_");
		formatConverter.dumpWorldNormal(hoi4ModPath + "\\map\\world_normal.bmp");
		// just copy over provinces.bmp, already in a compatible format
		Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("provinces"), (hoi4ModPath + ("\\map\\provinces.bmp")).c_str());

		// now if everything worked, print info about world an pause for user to see
		hoi4Gen.printStatistics();
		system("pause");
	}
}

bool Hoi4Module::findHoi4()
{
	std::vector<std::string> drives{ "C:\\", "D:\\", "E:\\", "F:\\", "G:\\", "H:\\" };
	// first try to find hoi4 at the configured location
	if (std::experimental::filesystem::exists(hoi4Path)) {
		hoi4Path = hoi4Path;
		return true;
	}
	for (const auto& drive : drives) {
		if (std::experimental::filesystem::exists(drive + "Program Files (x86)\\Steam\\steamapps\\common\\Hearts of Iron IV")) {
			hoi4Path = drive + "Program Files (x86)\\Steam\\steamapps\\common\\Hearts of Iron IV";
			return true;
		}
		else if (std::experimental::filesystem::exists(drive + "Program Files\\Steam\\steamapps\\common\\Hearts of Iron IV")) {
			hoi4Path = drive + "Program Files\\Steam\\steamapps\\common\\Hearts of Iron IV";
			return true;
		}
		else if (std::experimental::filesystem::exists(drive + "Steam\\steamapps\\common\\Hearts of Iron IV")) {
			hoi4Path = drive + "Steam\\steamapps\\common\\Hearts of Iron IV";
			return true;
		}
	}
	return false;
}

void Hoi4Module::readConfig()
{
	// Short alias for this namespace
	namespace pt = boost::property_tree;
	// Create a root
	pt::ptree root;
	ifstream f("hoiconfig.json");
	std::stringstream buffer;
	if (!f.good()) {
		logLine("Config could not be loaded");
	}
	buffer << f.rdbuf();
	pt::read_json(buffer, root);
	hoi4Path = root.get<string>("module.hoi4Path");
	hoi4ModPath = root.get<string>("module.hoi4ModPath");
	findHoi4();
	hoi4Gen.resources = {
		{ "aluminium",{ root.get<double>("hoi4.aluminiumFactor"), 1169.0, 0.3 }},
		{ "chromium",{ root.get<double>("hoi4.chromiumFactor"), 1250.0, 0.2 } },
		{ "oil",{ root.get<double>("hoi4.oilFactor"), 1220.0, 0.1 }},
		{ "rubber",{ root.get<double>("hoi4.rubberFactor"), 1029.0, 0.1 }},
		{ "steel",{ root.get<double>("hoi4.steelFactor"), 2562.0, 0.5 }},
		{ "tungsten",{ root.get<double>("hoi4.tungstenFactor"), 1188.0, 0.2 }}
		
	};
	numCountries = root.get<int>("scenario.numCountries");
	hoi4Gen.worldPopulationFactor = root.get<double>("scenario.worldPopulationFactor");
	hoi4Gen.industryFactor = root.get<double>("scenario.industryFactor");
	hoi4Gen.resourceFactor = root.get<double>("hoi4.resourceFactor");
}

