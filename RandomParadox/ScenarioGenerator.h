#pragma once
#include "Country.h"
#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "GameRegion.h"
#include <map>
class ScenarioGenerator
{
	const FastWorldGenerator f;
public:
	vector<GameRegion> gameRegions;
	map<std::string, Country> countryMap;
	ScenarioGenerator(FastWorldGenerator& f);
	~ScenarioGenerator();

	// map base regions to generic paradox compatible game regions
	void mapRegions();

	GameRegion& findStartRegion();
	void generateCountries();
	void dumpDebugCountrymap(std::string path);
};

