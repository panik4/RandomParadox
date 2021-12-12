#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
class GameRegion
{
	vector<std::string> cores;

public:
	std::string owner;
	vector<uint32_t> neighbours;
	int ID;
	bool sea;
	const Region baseRegion;
	Position position;
	bool assigned;
	GameRegion();
	GameRegion(const Region& baseRegion);
	~GameRegion();
};

