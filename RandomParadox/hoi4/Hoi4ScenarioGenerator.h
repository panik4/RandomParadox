#pragma once
#include "../generic/countries/Country.h"
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../generic/ScenarioGenerator.h"

class Hoi4ScenarioGenerator
{
	FastWorldGenerator f;
	ranlux24 random;
	ScenarioGenerator scenGen;
	NameGenerator nG;

public:
	Hoi4ScenarioGenerator(FastWorldGenerator& f, ScenarioGenerator sG);
	~Hoi4ScenarioGenerator();

	void generateWorld();
	void generateStateSpecifics(ScenarioGenerator& scenGen);
	void generateCountrySpecifics(ScenarioGenerator& scenGen, std::map<std::string, Country>& countries);

};

