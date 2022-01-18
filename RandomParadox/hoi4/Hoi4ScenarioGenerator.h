#pragma once
#include "../generic/countries/Country.h"
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../generic/ScenarioGenerator.h"
#include "NationalFocus.h"
#include <set>

class Hoi4ScenarioGenerator
{
	ranlux24 random;
	NameGenerator nG;
	int landStates = 0;
	vector<std::string> majorPowers;
	vector<std::string> regionalPowers;
	vector<std::string> weakPowers;
	vector<std::string> wargoalsAttack;
	vector<std::string> goalsDefence;
	int focusID = 0;
	std::map<std::string, NationalFocus::FocusType> typeMapping{
		{"attack", NationalFocus::FocusType::attack},
		{ "defense", NationalFocus::FocusType::defense },
		{ "ally", NationalFocus::FocusType::ally }
	};

public:
	vector<NationalFocus> foci;
	vector<NationalFocus> warFoci;
	std::map<std::string, std::vector<double>> resources;
	Hoi4ScenarioGenerator();
	~Hoi4ScenarioGenerator();

	void generateStateResources(ScenarioGenerator& scenGen);
	void generateStateSpecifics(ScenarioGenerator& scenGen);
	void generateCountrySpecifics(ScenarioGenerator& scenGen, std::map<std::string, Country>& countries);
	// calculate how strong each country is
	void evaluateCountries(ScenarioGenerator & scenGen);

	void generateCountryUnits(ScenarioGenerator & scenGen);
	NationalFocus buildFocus(vector<std::string> chainStep, Country& source, Country& target);
	bool fulfillsrequirements(vector<std::string> requirements, Country& source, Country& target);
	void evaluateCountryGoals(ScenarioGenerator & scenGen);
	// see which countries are in need of unification
	void evaluateBrotherlyWars();
	// see which country needs to see some action
	void evaluateCivilWars();
	// create a strategy for this country
	void evaluateCountryStrategy();
	void buildFocusTree();
};

