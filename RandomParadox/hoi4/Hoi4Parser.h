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
public:
	Hoi4Parser();
	~Hoi4Parser();
	std::vector<std::string> defaultTags;
	std::string getBuildingLine(const std::string type, const Region& region, const bool coastal, const Bitmap& heightmap) const;
	// map
	void dumpAdj(std::string path);
	void dumpAdjacencyRules(std::string path);
	void dumpAirports(std::string path, const std::vector<Region>& regions);
	void dumpBuildings(std::string path, const std::vector<Region>& regions);
	void dumpContinents(std::string path, const std::vector<Continent>& continents);
	void dumpDefinition(std::string path, std::vector<GameProvince>& provinces);
	void dumpRocketSites(std::string path, const std::vector<Region>& regions);
	void dumpStrategicRegions(std::string path, const std::vector<Region>& regions, const std::vector<std::set<int>> strategicRegions);
	void dumpSupplyAreas(std::string path, const std::vector<Region>& regions);
	void dumpSupply(std::string path, const std::vector<std::vector<int>> supplyNodeConnections);
	void dumpUnitStacks(std::string path, const std::vector<Province*> provinces);
	void dumpWeatherPositions(std::string path, const std::vector<Region>& regions, const std::vector<std::set<int>> strategicRegions);
	//gfx
	void dumpFlags(std::string path, const std::map<std::string, Country>& countries);

	// history
	void dumpStates(std::string path, std::map<std::string, Country>& countries);
	void writeHistoryCountries(std::string path, const std::map<std::string, Country>& countries);
	void writeHistoryUnits(std::string path, const std::map<std::string, Country>& countries);
	// history - National Focus
	void writeFoci(std::string path, std::vector<NationalFocus> foci, const std::map<std::string, Country>& countries);

	// common
	void dumpCommonCountries(std::string path, std::string hoiPath, const std::map<std::string, Country>& countries);
	void dumpCommonCountryTags(std::string path, const std::map<std::string, Country>& countries);


	// localisation
	void writeCountryNames(std::string path, const std::map<std::string, Country>& countries);
	void writeStateNames(std::string path, const std::map<std::string, Country>& countries);

	void writeCompatibilityHistory(std::string path, std::string hoiPath, const vector<Region>& regions);
};

