#pragma once
#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
class Hoi4Parser
{
public:
	Hoi4Parser();
	~Hoi4Parser();


	void writeFile(std::string path, std::string content);
	std::string csvFormat(vector<std::string> arguments, char delimiter, bool trailing);

	void dumpAdj(std::string path);
	void dumpAirports(std::string path, vector<Region> regions);
	void dumpBuildings(std::string path, vector<Region> regions);
	void dumpContinents(std::string path, vector<Continent> continents);
	void dumpDefinition(std::string path, vector<Province*> provinces);
	void dumpRocketSites(std::string path, vector<Region> regions);
	void dumpUnitStacks();
	void dumpStrategicRegions(std::string path, vector<Region> regions);
	void dumpSupplyAreas();
};

