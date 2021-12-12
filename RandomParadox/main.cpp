#include "FastWorldGen/FastWorldGen/entities/Colour.h"
#include "FastWorldGen/FastWorldGen/utils/Bitmap.h"
#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "FormatConverter.h"
#include "hoi4/Hoi4Parser.h"
#include <filesystem>
#include <experimental/filesystem>
#include "hoi4/Hoi4ScenarioGenerator.h"
using namespace std;
int main() {
	string modPath = "pdoxMod\\";
	string hoiPath = "D:\\Steam\\steamapps\\common\\Hearts of Iron IV\\";

	FastWorldGenerator f;
	if (true)
	{

	ScenarioGenerator scenGen(f);
	scenGen.loadStuff();
	scenGen.mapRegions();
	scenGen.generateCountries();
	scenGen.dumpDebugCountrymap(Data::getInstance().debugMapsPath + "countries.bmp");
	Hoi4ScenarioGenerator hoi4Gen(f, scenGen);

	Hoi4Parser hoiParse;
	std::experimental::filesystem::create_directory(modPath + "\\map\\");
	std::experimental::filesystem::create_directory(modPath + "\\common\\");
	std::experimental::filesystem::create_directory(modPath + "\\history\\");
	std::experimental::filesystem::remove_all(modPath + "\\history\\states\\");
	std::experimental::filesystem::create_directory(modPath + "\\history\\states\\");
	std::experimental::filesystem::remove_all(modPath + "\\map\\strategicregions\\");
	std::experimental::filesystem::create_directory(modPath + "\\map\\strategicregions\\");
	std::experimental::filesystem::remove_all(modPath + "\\map\\supplyareas\\");
	std::experimental::filesystem::create_directory(modPath + "\\map\\supplyareas\\");
	hoiParse.dumpStates(modPath + "\\history\\states", scenGen.countryMap);


	hoiParse.dumpUnitStacks(modPath + "\\map\\unitstacks.txt", scenGen.f.provinceGenerator.provinces);
	hoiParse.dumpRocketSites(modPath + "\\map\\rocketsites.txt", scenGen.f.provinceGenerator.regions);

	hoiParse.dumpStrategicRegions(modPath + "\\map\\strategicregions", scenGen.f.provinceGenerator.regions);
	hoiParse.dumpSupplyAreas(modPath + "\\map\\supplyareas", scenGen.f.provinceGenerator.regions);
	hoiParse.dumpWeatherPositions(modPath + "\\map\\weatherpositions.txt", scenGen.f.provinceGenerator.regions);

	hoiParse.dumpAdjacencyRules(modPath + "\\map\\adjacency_rules.txt");
	//hoiParse.dumpAdj(modPath + "\\map\\adjacencies.csv");
	hoiParse.dumpAirports(modPath + "\\map\\airports.txt", scenGen.f.provinceGenerator.regions);
	hoiParse.dumpBuildings(modPath + "\\map\\buildings.txt", scenGen.f.provinceGenerator.regions);
	hoiParse.writeCompatibilityHistory(modPath + "\\history\\countries\\", hoiPath + "\\history\\countries\\", scenGen.f.provinceGenerator.regions);


	Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("provinces"), (modPath + ("\\map\\provinces.bmp")).c_str());
	return 0;
	}
	else {
		f.generateWorld();
		ScenarioGenerator scenGen(f);
		scenGen.mapRegions();
		scenGen.generateCountries();
		scenGen.dumpDebugCountrymap(Data::getInstance().debugMapsPath + "countries.bmp");

		//Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("heightmap"), (modPath + ("\\map\\heightmap.bmp")).c_str());
		Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("provinces"), (modPath + ("\\map\\provinces.bmp")).c_str());


		// prepare folder structure
		std::experimental::filesystem::create_directory(modPath + "\\map\\");
		std::experimental::filesystem::create_directory(modPath + "\\common\\");
		std::experimental::filesystem::create_directory(modPath + "\\history\\");
		std::experimental::filesystem::remove_all(modPath + "\\map\\strategicregions\\");
		std::experimental::filesystem::create_directory(modPath + "\\map\\strategicregions\\");
		std::experimental::filesystem::remove_all(modPath + "\\map\\supplyareas\\");
		std::experimental::filesystem::create_directory(modPath + "\\map\\supplyareas\\");
		std::experimental::filesystem::remove_all(modPath + "\\history\\states\\");
		std::experimental::filesystem::create_directory(modPath + "\\history\\states\\");
		//std::experimental::filesystem::remove_all(modPath + "\\history\\countries\\");
		//std::experimental::filesystem::create_directory(modPath + "\\history\\countries\\");
		// HOI 4:
		FormatConverter formatConverter;
		formatConverter.dump8BitTerrain(modPath + "\\map\\terrain.bmp", "terrainHoi4");
		formatConverter.dump8BitCities(modPath + "\\map\\cities.bmp", "citiesHoi4");
		formatConverter.dump8BitRivers(modPath + "\\map\\rivers.bmp", "riversHoi4");
		formatConverter.dump8BitTrees(modPath + "\\map\\trees.bmp", "treesHoi4");
		formatConverter.dump8BitHeightmap(modPath + "\\map\\heightmap.bmp", "heightmapHoi4");
		//formatConverter.dumpDDSFiles(modPath + "\\map\\terrain\\colormap_water_");
		formatConverter.dumpWorldNormal(modPath + "\\map\\world_normal.bmp");






		Hoi4ScenarioGenerator hoi4Gen(f, scenGen);

		Hoi4Parser hoiParse;
		//hoiParse.readDefaultCountries();
		hoiParse.dumpAdj(modPath + "\\map\\adjacencies.csv");
		hoiParse.dumpAirports(modPath + "\\map\\airports.txt", f.provinceGenerator.regions);
		hoiParse.dumpBuildings(modPath + "\\map\\buildings.txt", f.provinceGenerator.regions);
		hoiParse.dumpContinents(modPath + "\\map\\continents.txt", f.provinceGenerator.continents);
		hoiParse.dumpDefinition(modPath + "\\map\\definition.csv", f.provinceGenerator.provinces);
		hoiParse.dumpUnitStacks(modPath + "\\map\\unitstacks.txt", f.provinceGenerator.provinces);
		hoiParse.dumpRocketSites(modPath + "\\map\\rocketsites.txt", f.provinceGenerator.regions);
		hoiParse.dumpStrategicRegions(modPath + "\\map\\strategicregions", f.provinceGenerator.regions);
		hoiParse.dumpSupplyAreas(modPath + "\\map\\supplyareas", f.provinceGenerator.regions);
		hoiParse.dumpStates(modPath + "\\history\\states", scenGen.countryMap);
		hoiParse.dumpWeatherPositions(modPath + "\\map\\weatherpositions.txt", f.provinceGenerator.regions);
		hoiParse.dumpAdjacencyRules(modPath + "\\map\\adjacency_rules.txt");

		// history

		//hoiParse.writeCompatibilityHistory(modPath + "\\history\\countries\\", hoiPath + "\\history\\countries\\", f.provinceGenerator.regions);

		return 0;
	}
}