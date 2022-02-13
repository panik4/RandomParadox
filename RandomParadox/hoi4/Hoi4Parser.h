#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../generic/countries/Country.h"
#include "../generic/ParserUtils.h"
#include "../generic/TextureWriter.h"
#include "../generic/NameGenerator.h"
#include "NationalFocus.h"

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
	static void dumpStrategicRegions(std::string path, const std::vector<Region>& regions, const std::vector<std::set<int>> strategicRegions);
	static void dumpSupplyAreas(std::string path, const std::vector<Region>& regions);
	static void dumpSupply(std::string path, const std::vector<std::vector<int>> supplyNodeConnections);
	static void dumpUnitStacks(std::string path, const std::vector<Province*> provinces);
	static void dumpWeatherPositions(std::string path, const std::vector<Region>& regions, const std::vector<std::set<int>> strategicRegions);
	//gfx
	static void dumpFlags(std::string path, const std::map<std::string, Country>& countries);

	// history
	static void dumpStates(std::string path, std::map<std::string, Country>& countries);
	static void writeHistoryCountries(std::string path, const std::map<std::string, Country>& countries);
	static void writeHistoryUnits(std::string path, const std::map<std::string, Country>& countries);
	// history - National Focus
	static void writeFoci(std::string path, std::vector<NationalFocus> foci, const std::map<std::string, Country>& countries);

	// common
	static void dumpCommonBookmarks(std::string path, std::string hoiPath, const std::map<std::string, Country>& countries, std::map<int, vector<std::string>> strengthScores);
	static void dumpCommonCountries(std::string path, std::string hoiPath, const std::map<std::string, Country>& countries);
	static void dumpCommonCountryTags(std::string path, const std::map<std::string, Country>& countries);

	// localisation
	static void writeCountryNames(std::string path, const std::map<std::string, Country>& countries);
	static void writeStateNames(std::string path, const std::map<std::string, Country>& countries);

	// copy base game countries and remove certain lines to reduce crashes
	static void writeCompatibilityHistory(std::string path, std::string hoiPath, const vector<Region>& regions);
};

