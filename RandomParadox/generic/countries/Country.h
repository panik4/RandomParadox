#pragma once
#include <string>
#include <vector>
#include "../../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../GameRegion.h"
#include "../Flag.h"
class Country
{
	std::string tag;
	// Flag flag
	int capitalRegionID;

public:
	int ID;
	Country();
	Country(std::string tag);
	~Country();
	std::vector<GameRegion> ownedRegions;
	std::set<std::string> neighbours;

	std::string name;
	std::string adjective;
	double developmentFactor;
	std::map<std::string, std::string> attributeStrings;
	std::map<std::string, int> attributeDoubles;
	std::map<std::string, vector<int>> attributeVectors;
	Flag flag;
	Colour colour;
	void addRegion(GameRegion& region, vector<GameRegion>& gameRegions);
	void assignRegions(int maxRegions, vector<GameRegion>& gameRegions, GameRegion& startRegion);
	bool operator<(const Country& right) const
	{
		return ID < right.ID;
	};
};

