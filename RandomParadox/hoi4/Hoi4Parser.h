#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../generic/countries/Country.h"
#include "../generic/ParserUtils.h"
#include "../generic/TextureWriter.h"
#include "../generic/NameGenerator.h"
#include "Hoi4ScenarioGenerator.h"
#include "NationalFocus.h"
#include <array>

class Hoi4Parser
{
	typedef ParserUtils pU;
	static std::vector<std::string> defaultTags;
public:
	static std::string getBuildingLine(const std::string type, const Region& region, const bool coastal, const Bitmap& heightmap);
	// map
	static void dumpAdj(std::string path);
	static void dumpAdjacencyRules(std::string path);
	static void dumpAirports(std::string path, const std::vector<Region>& regions);
	static void dumpBuildings(std::string path, const std::vector<Region>& regions);
	static void dumpContinents(std::string path, const std::vector<Continent>& continents);
	static void dumpDefinition(std::string path, std::vector<GameProvince>& provinces);
	static void dumpRocketSites(std::string path, const std::vector<Region>& regions);
	static void dumpStrategicRegions(std::string path, const std::vector<Region>& regions, const std::vector<strategicRegion> strategicRegions);
	static void dumpSupply(std::string path, const std::vector<std::vector<int>> supplyNodeConnections);
	static void dumpUnitStacks(std::string path, const std::vector<Province*> provinces);
	static void dumpWeatherPositions(std::string path, const std::vector<Region>& regions, const std::vector<strategicRegion> strategicRegions);
	//gfx
	static void dumpFlags(std::string path, const std::map<std::string, Country>& countries);

	// history
	static void dumpStates(std::string path, std::map<std::string, Country>& countries);
	static void writeHistoryCountries(std::string path, const std::map<std::string, Country>& countries);
	static void writeHistoryUnits(std::string path, const std::map<std::string, Country>& countries);
	// history - National Focus
	static std::vector<std::string> readTypeMap();
	static std::map<std::string, std::string> readRewardMap(std::string path);
	static void writeFoci(std::string path, const std::map<std::string, Country>& countries);

	// common
	static void dumpCommonBookmarks(std::string path, const std::map<std::string, Country>& countries, std::map<int, std::vector<std::string>> strengthScores);
	static void dumpCommonCountries(std::string path, std::string hoiPath, const std::map<std::string, Country>& countries);
	static void dumpCommonCountryTags(std::string path, const std::map<std::string, Country>& countries);

	// localisation
	static void writeCountryNames(std::string path, const std::map<std::string, Country>& countries);
	static void writeStateNames(std::string path, const std::map<std::string, Country>& countries);
	static void writeStrategicRegionNames(std::string path, const std::vector<strategicRegion> strategicRegions);

	// copy base game countries and remove certain lines to reduce crashes
	static void writeCompatibilityHistory(std::string path, std::string hoiPath, const std::vector<Region>& regions);

	// copy over mod descriptor file
	static void copyDescriptorFile(const std::string sourcePath, const std::string destPath, const std::string modsDirectory, const std::string modName);
};

