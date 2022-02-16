#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
class GameProvince
{
public:
	int ID;
	std::string name;
	std::string owner;
	std::string terrainType;
	double popFactor;
	double devFactor;
	double cityShare;
	const Province * baseProvince;
	// containers
	std::vector<GameProvince> neighbours;
	std::map<std::string, double> attributeDoubles;
	std::map<std::string, std::string> attributeStrings;
	// constructors/destructor
	GameProvince(Province* province);
	GameProvince() {};
	~GameProvince();
	// operators
	bool operator==(const GameProvince& right) const {
		return ID == right.ID;
	};
	bool operator<(const GameProvince& right) const {
		return ID < right.ID;
	};
};

