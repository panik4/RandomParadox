#pragma once
#include "Country.h"
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "GameRegion.h"
#include "ResourceLoader.h"
#include <map>
class ScenarioGenerator
{
	vector<Region> baseRegions;
	// used to gather data we don't want to generate ourselves
	ResourceLoader rLoader;
	std::map<std::string, Bitmap> bitmaps;
public:
	FastWorldGenerator f;
	vector<Province*> provinces;
	vector<GameRegion> gameRegions;
	map<std::string, Country> countryMap;
	ScenarioGenerator(FastWorldGenerator& f);
	~ScenarioGenerator();
	void loadStuff();
	void calcRegions();
	// map base regions to generic paradox compatible game regions
	void mapRegions();

	GameRegion& findStartRegion();
	void generateCountries();
	void dumpDebugCountrymap(std::string path);
};

