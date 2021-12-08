#pragma once
#include "Country.h"
#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "ScenarioGenerator.h"

class Hoi4ScenarioGenerator
{
	FastWorldGenerator f;
	ranlux24 random;
	ScenarioGenerator scenGen;

public:
	Hoi4ScenarioGenerator(FastWorldGenerator& f, ScenarioGenerator sG);
	~Hoi4ScenarioGenerator();

	void generateWorld();

};

