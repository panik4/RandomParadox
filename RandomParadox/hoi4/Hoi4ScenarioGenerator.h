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
	// container holding the resource configurations
	std::map<std::string, std::vector<double>> resources;
	Hoi4ScenarioGenerator();
	~Hoi4ScenarioGenerator();
	// give resources to states
	void generateStateResources(ScenarioGenerator& scenGen);
	// industry, development, population, state category
	void generateStateSpecifics(ScenarioGenerator& scenGen);
	// politics: ideology, strength, major
	void generateCountrySpecifics(ScenarioGenerator& scenGen, std::map<std::string, Country>& countries);
	// supply hubs and railroads
	void generateLogistics(ScenarioGenerator& scenGen);
	// calculate how strong each country is
	void evaluateCountries(ScenarioGenerator & scenGen);
	// determine unit composition, templates
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
	// make a tree out of all focus chains and single foci
	void buildFocusTree();
};

