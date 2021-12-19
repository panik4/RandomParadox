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
	vector<std::string> defaultTags;
	std::string getBuildingLine(std::string type, Region& region, bool coastal);
	// map
	void dumpAdj(std::string path);
	void dumpAirports(std::string path, const vector<Region>& regions);
	void dumpBuildings(std::string path, const vector<Region>& regions);
	void dumpContinents(std::string path, const vector<Continent>& continents);
	void dumpDefinition(std::string path, vector<GameProvince>& provinces);
	void dumpRocketSites(std::string path, const vector<Region>& regions);
	void dumpUnitStacks(std::string path, const vector<Province*> provinces);
	void dumpWeatherPositions(std::string path, const vector<Region>& regions);
	void dumpAdjacencyRules(std::string path);
	void dumpStrategicRegions(std::string path, const vector<Region>& regions);
	void dumpSupplyAreas(std::string path, const vector<Region>& regions);

	//gfx
	void dumpFlags(std::string path, const std::map<std::string, Country>& countries);

	// history
	void dumpStates(std::string path, std::map<std::string, Country>& countries);
	void writeHistoryCountries(std::string path, const std::map<std::string, Country>& countries);
	void writeHistoryUnits(std::string path, const std::map<std::string, Country>& countries);
	// history - National Focus
	void writeFoci(std::string path, vector<NationalFocus> foci, const std::map<std::string, Country>& countries);

	// common
	void dumpCommonCountries(std::string path, std::string hoiPath, const std::map<std::string, Country>& countries);
	void dumpCommonCountryTags(std::string path, const std::map<std::string, Country>& countries);


	// localisation
	void writeCountryNames(std::string path, const std::map<std::string, Country>& countries);
	void writeStateNames(std::string path, const std::map<std::string, Country>& countries);

};

