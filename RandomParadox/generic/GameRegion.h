#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "GameProvince.h"
#include <map>
class GameRegion
{
	std::vector<std::string> cores;

public:
	std::string owner;
	std::vector<int> neighbours;
	std::vector<GameProvince> gameProvinces;
	std::map<std::string, std::string> attributeStrings;
	std::map<std::string, double> attributeDoubles;
	std::string name;
	int ID;
	bool sea;
	const Region baseRegion;
	Position position;
	bool assigned;
	GameRegion();
	GameRegion(const Region& baseRegion);
	~GameRegion();
};

