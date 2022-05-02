#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../generic/ScenarioGenerator.h"
#include "../generic/countries/Country.h"
#include "Hoi4Country.h"
#include "Hoi4GameRegion.h"
#include "NationalFocus.h"
#include <array>
#include <set>

struct strategicRegion {
  std::set<int> gameRegionIDs;
  // weather: month{averageTemp, standard deviation, average precipitation,
  // tempLow, tempHigh, tempNightly, snowChance, lightRainChance,
  // heavyRainChance, blizzardChance,mudChance, sandstormChance}
  std::vector<std::vector<double>> weatherMonths;
  std::string name;
};

class Hoi4ScenarioGenerator {
  // vars
  int landStates = 0;
  int focusID = 0;
  // vars - track industry statistics
  int totalWorldIndustry = 0;
  int militaryIndustry = 0;
  int navalIndustry = 0;
  int civilianIndustry = 0;
  // vars - track civil statistics
  long long worldPop = 0;
  // vars - track resource statistics
  int totalAluminium = 0;
  int totalChromium = 0;
  int totalRubber = 0;
  int totalOil = 0;
  int totalSteel = 0;
  int totalTungsten = 0;
  std::map<std::string, int> totalResources;
  std::vector<Hoi4GameRegion> hoi4Regions;
  // containers
  std::set<std::string> majorPowers;
  std::set<std::string> regionalPowers;
  std::set<std::string> weakPowers;
  std::vector<std::string> wargoalsAttack;
  std::vector<std::string> goalsDefence;
  enum doctrineType {
    blitz,
    infantry,
    milita,
    artillery,
    armored,
    mass,
    support,
    defensive
  };
  std::map<int, std::string> doctrineMap{
      {0, "blitz"},   {1, "infantry"}, {2, "milita"},  {3, "artillery"},
      {4, "armored"}, {5, "mass"},     {6, "support"}, {7, "defensive"}};

public:
  // vars - config options
  double worldPopulationFactor = 1.0;
  double industryFactor = 1.0;
  double resourceFactor = 1.0;

  // containers
  std::vector<NationalFocus> foci;
  std::vector<NationalFocus> warFoci;
  std::map<int, std::vector<std::string>> strengthScores;
  std::map<std::string, Hoi4Country> countryMap;
  // a list of connections: {sourceHub, destHub, provinces the rails go through}
  std::vector<std::vector<int>> supplyNodeConnections;
  // container holding the resource configurations
  std::map<std::string, std::vector<double>> resources;
  std::vector<strategicRegion> strategicRegions;

  // member functions
  // constructors/destructors
  Hoi4ScenarioGenerator();
  ~Hoi4ScenarioGenerator();
  // give resources to states
  void generateStateResources();
  // industry, development, population, state category
  void generateStateSpecifics();
  // politics: ideology, strength, major
  void generateCountrySpecifics(ScenarioGenerator &scenGen,
                                std::map<std::string, Country> &countries);
  // build strategic regions from gameregions
  void generateStrategicRegions(ScenarioGenerator &scenGen);
  // generate weather per strategic region, from baseprovinces
  void generateWeather(ScenarioGenerator &scenGen);
  // supply hubs and railroads
  void generateLogistics(ScenarioGenerator &scenGen);
  // calculate how strong each country is
  void evaluateCountries(ScenarioGenerator &scenGen);

  bool unitFulfillsRequirements(std::vector<std::string> unitRequirements,
                                Hoi4Country &country);
  // determine unit composition, templates
  void generateCountryUnits();
  // build a focus from the chain
  NationalFocus buildFocus(const std::vector<std::string> chainStep,
                           const Hoi4Country &source,
                           const Hoi4Country &target);
  // make a tree out of all focus chains and single foci
  void buildFocusTree(Hoi4Country &source);
  // check if a national focus fulfills requirements
  bool stepFulfillsRequirements(
      std::vector<std::string> stepRequirements,
      const std::vector<std::set<Hoi4Country>> stepTargets);
  // check if a national focus fulfills requirements
  bool targetFulfillsRequirements(
      std::vector<std::string> targetRequirements, Hoi4Country &source,
      Hoi4Country &target,
      const std::vector<std::set<std::string>> levelTargets, const int level);
  // evaluate the focus chains for each country
  void evaluateCountryGoals();
  // 
  void generateHoi4RegionList();
  // see which countries are in need of unification
  void evaluateBrotherlyWars();
  // see which country needs to see some action
  void evaluateCivilWars();
  // create a strategy for this country
  void evaluateCountryStrategy();
  // print world info to console
  void printStatistics(ScenarioGenerator &scenGen);
};
