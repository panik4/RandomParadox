#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
class GameProvince
{
public:
	std::string name;

	std::string owner;
	int ID;
	const Province * baseProvince;
	GameProvince();
	~GameProvince();
};

