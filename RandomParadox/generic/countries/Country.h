#pragma once
#include <string>
#include <vector>
#include "../../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../GameRegion.h"
#include "../Flag.h"
#include "../../hoi4/NationalFocus.h"
class Country
{
public:
	// member variables
	std::string tag;
	int ID;
	std::string name;
	std::string adjective;
	int capitalRegionID;
	double developmentFactor;
	Flag flag;
	Colour colour;
	// constructors/destructors
	Country();
	Country(std::string tag);
	~Country();
	// containers
	std::map<std::string, std::string> attributeStrings;
	std::map<std::string, int> attributeDoubles;
	std::map<std::string, std::vector<int>> attributeVectors;
	std::vector<GameRegion> ownedRegions;
	std::vector<std::vector<NationalFocus>> foci;
	std::set<std::string> neighbours;
	// member functions
	void addRegion(GameRegion& region, std::vector<GameRegion>& gameRegions, std::vector<GameProvince>& gameProvinces);
	void assignRegions(int maxRegions, std::vector<GameRegion>& gameRegions, GameRegion& startRegion, std::vector<GameProvince>& gameProvinces);
	// operators
	bool operator<(const Country& right) const	{
		return ID < right.ID;
	};
};

