#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include <experimental/filesystem>
#include "../generic/countries/Country.h"
#include "../generic/ParserUtils.h"
#include "../generic/TextureWriter.h"

class Hoi4Parser
{
	typedef ParserUtils pU;
public:
	Hoi4Parser();
	~Hoi4Parser();
	vector<std::string> defaultTags;


	//void insertAtKeyAndReplace(std::string& content, std::string key, std::string value);

	void dumpAdj(std::string path);
	void dumpAirports(std::string path, const vector<Region>& regions);
	std::string getBuildingLine(std::string type, Region& region, bool coastal);
	void dumpBuildings(std::string path, const vector<Region>& regions);
	void dumpContinents(std::string path, const vector<Continent>& continents);
	void dumpDefinition(std::string path, const vector<Province*>& provinces);
	void dumpRocketSites(std::string path, const vector<Region>& regions);
	void dumpUnitStacks(std::string path, const vector<Province*> provinces);
	void dumpWeatherPositions(std::string path, const vector<Region>& regions);
	void dumpAdjacencyRules(std::string path);
	void dumpStrategicRegions(std::string path, const vector<Region>& regions);
	void dumpSupplyAreas(std::string path, const vector<Region>& regions);
	void dumpStates(std::string path, std::map<std::string, Country>& countries);
	void dumpFlags(std::string path, const std::map<std::string, Country>& countries);

	void copyDefaultOverwrites(std::string pathToHoi4);

	void calcAIAreas();
	void dumpCommonAIAreas();
	void dumpCommonBookmark();


	void writeCompatibilityHistory(std::string path, std::string hoiPath, const vector<Region>& regions);
	void writeHistoryCountries(std::string path, const std::map<std::string, Country>& countries);
	void writeHistoryUnits(std::string path, const std::map<std::string, Country>& countries);
	void writeHistory();
	void dumpCommonCountries(std::string path, std::string hoiPath, const std::map<std::string, Country>& countries);
	void dumpCommonCountryTags(std::string path, const std::map<std::string, Country>& countries);

};

