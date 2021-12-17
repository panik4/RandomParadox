#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
class GameProvince
{
public:
	std::string name;
	double popFactor;
	double devFactor;
	double cityShare;
	std::string terrainType;
	std::string owner;
	int ID;
	const Province * baseProvince;
	std::vector<GameProvince> neighbours;
	GameProvince(Province* province);
	~GameProvince();
};

