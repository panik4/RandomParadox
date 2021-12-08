#pragma once
#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include <experimental/filesystem>
class Hoi4Parser
{
public:
	Hoi4Parser();
	~Hoi4Parser();


	void writeFile(std::string path, std::string content);
	std::string readFile(std::string path);
	std::string csvFormat(vector<std::string> arguments, char delimiter, bool trailing);
	void replaceOccurences(std::string& content, std::string key, std::string value);
	void replaceLine(std::string& content, std::string key, std::string value);
	//void insertAtKeyAndReplace(std::string& content, std::string key, std::string value);

	void dumpAdj(std::string path);
	void dumpAirports(std::string path, vector<Region> regions);
	std::string getBuildingLine(std::string type, Region& region, bool coastal);
	void dumpBuildings(std::string path, vector<Region> regions);
	void dumpContinents(std::string path, vector<Continent> continents);
	void dumpDefinition(std::string path, vector<Province*> provinces);
	void dumpRocketSites(std::string path, vector<Region> regions);
	void dumpUnitStacks(std::string path, vector<Province*> provinces);
	void dumpWeatherPositions(std::string path, vector<Region> regions);
	void dumpAdjacencyRules(std::string path);
	void dumpStrategicRegions(std::string path, vector<Region> regions);
	void dumpSupplyAreas(std::string path, vector<Region> regions);
	void dumpStates(std::string path, vector<Region> regions);


	void copyDefaultOverwrites(std::string pathToHoi4);

	void calcAIAreas();
	void dumpCommonAIAreas();
	void dumpCommonBookmark();


	void writeCompatibilityHistory(std::string path, std::string hoiPath, vector<Region> regions);
	void writeHistory();
	void dumpCommonCountries();
	void dumpCommonCountryTags();

};

