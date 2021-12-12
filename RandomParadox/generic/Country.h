#pragma once
#include <string>
#include <vector>
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "GameRegion.h"
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

	Colour colour;
	void addRegion(GameRegion& region);
	void assignRegions(int maxRegions, vector<GameRegion>& gameRegions, GameRegion& startRegion);
};

