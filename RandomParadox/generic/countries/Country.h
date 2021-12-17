#pragma once
#include <string>
#include <vector>
#include "../../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../GameRegion.h"
#include "../Flag.h"
class Country
{
	std::string tag;
	static int ID;
	// Flag flag
	int capitalRegionID;

public:
	Country(std::string tag);
	~Country();
	std::vector<GameRegion> ownedRegions;

	std::string name;
	std::string adjective;
	double developmentFactor;
	std::map<std::string, std::string> attributeStrings;
	std::map<std::string, int> attributeDoubles;
	Flag flag;
	Colour colour;
	void addRegion(GameRegion& region, vector<GameRegion>& gameRegions);
	void assignRegions(int maxRegions, vector<GameRegion>& gameRegions, GameRegion& startRegion);
};

