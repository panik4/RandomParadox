#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
class GameProvince
{
public:
	std::string name;
	double popFactor;
	double devFactor;
	double cityShare;
	std::map<std::string, std::string> attributeStrings;
	std::map<std::string, double> attributeDoubles;
	std::string terrainType;
	std::string owner;
	int ID;
	const Province * baseProvince;
	std::vector<GameProvince> neighbours;
	GameProvince(Province* province);
	~GameProvince();
	bool operator==(const GameProvince& right) const {
		return ID == right.ID;
	};
	bool operator<(const GameProvince& right) const {
		return ID < right.ID;
	};
};

