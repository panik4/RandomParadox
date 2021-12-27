#pragma once
#include <string>
#include <vector>
#include "../../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../GameRegion.h"
#include "../Flag.h"
#include "../../hoi4/NationalFocus.h"
class Country
{
	// Flag flag
	int capitalRegionID;

public:
	std::string tag;
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


	vector<vector<NationalFocus>> foci;
};

