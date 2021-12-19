#pragma once
#include "../generic/countries/Country.h"
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../generic/ScenarioGenerator.h"
#include "NationalFocus.h"
#include <set>

class Hoi4ScenarioGenerator
{
	FastWorldGenerator f;
	ranlux24 random;
	NameGenerator nG;

	vector<std::string> majorPowers;
	vector<std::string> regionalPowers;
	vector<std::string> weakPowers;
	vector<std::string> wargoalsAttack;
	vector<std::string> goalsDefence;
	int focusID = 0;

public:
	vector<NationalFocus> foci;
	vector<NationalFocus> warFoci;
	Hoi4ScenarioGenerator(FastWorldGenerator& f, ScenarioGenerator sG);
	~Hoi4ScenarioGenerator();

	void generateStateSpecifics(ScenarioGenerator& scenGen);
	void generateCountrySpecifics(ScenarioGenerator& scenGen, std::map<std::string, Country>& countries);
	// calculate how strong each country is
	void evaluateCountries(ScenarioGenerator & scenGen);

	void generateCountryUnits(ScenarioGenerator & scenGen);

	void evaluateCountryGoals(ScenarioGenerator & scenGen);
	// see which countries are in need of unification
	void evaluateBrotherlyWars();
	// see which country needs to see some action
	void evaluateCivilWars();
	// create a strategy for this country
	void evaluateCountryStrategy();
	void buildFocusTree();
};

