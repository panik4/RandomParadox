#pragma once
#include "FastWorldGenerator.h"
#include "generic/PdoxCountry.h"
#include "generic/ScenarioGenerator.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4Region.h"
#include "hoi4/NationalFocus.h"
#include <array>
#include <set>

namespace Scenario::Hoi4 {

class Generator : public Scenario::Generator {
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
  std::map<std::string, int> totalResources;
  // containers
  std::set<std::string> majorPowers;
  std::set<std::string> regionalPowers;
  std::set<std::string> weakPowers;
  std::vector<std::string> wargoalsAttack;
  std::vector<std::string> goalsDefence;
  std::map<int, std::string> doctrineMap{
      {0, "blitz"},   {1, "infantry"}, {2, "milita"},  {3, "artillery"},
      {4, "armored"}, {5, "mass"},     {6, "support"}, {7, "defensive"}};

public:
  // vars - config options
  double worldPopulationFactor = 1.0;
  double industryFactor = 1.0;
  double resourceFactor = 1.0;
  double sizeFactor = 1.0;

  // containers
  std::vector<NationalFocus> foci;
  std::vector<NationalFocus> warFoci;
  std::vector<std::shared_ptr<Region>> hoi4States;
  std::map<int, std::vector<std::string>> strengthScores;
  std::map<std::string, Hoi4Country> hoi4Countries;
  // a list of connections: {sourceHub, destHub, provinces the rails go through}
  std::vector<std::vector<int>> supplyNodeConnections;
  // container holding the resource configurations
  std::map<std::string, std::vector<double>> resources;

  // member functions
  // constructors/destructors
  Generator();
  Generator(Fwg::FastWorldGenerator &fwg);
  ~Generator();
  // initialize states
  void initializeStates();
  // initialize states
  void initializeCountries();
  // give resources to states
  void generateStateResources();
  // industry, development, population, state category
  void generateStateSpecifics(const int regionAmount);
  // politics: ideology, strength, major
  void generateCountrySpecifics();
  // generate weather per strategic region, from baseprovinces
  void generateWeather();
  // supply hubs and railroads
  void generateLogistics(Fwg::Gfx::Bitmap countries);
  // calculate how strong each country is
  void evaluateCountries();
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
      const std::string stepRequirements,
      const std::vector<std::set<Hoi4Country>> &stepTargets);
  // check if a national focus fulfills requirements
  bool targetFulfillsRequirements(
      const std::string &targetRequirements, const Hoi4Country &source,
      const Hoi4Country &target,
      const std::vector<std::set<std::string>> &levelTargets, const int level);
  // evaluate the focus chains for each country
  void evaluateCountryGoals();
  // see which countries are in need of unification
  void evaluateBrotherlyWars();
  // see which country needs to see some action
  void evaluateCivilWars();
  // create a strategy for this country
  void evaluateCountryStrategy();
  // print world info to console
  void printStatistics();
};
} // namespace Scenario::Hoi4