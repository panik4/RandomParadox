#pragma once
#include "FastWorldGenerator.h"
#include "generic/Country.h"
#include "generic/GenericParsing.h"
#include "generic/ScenarioGenerator.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4FocusGen.h"
#include "hoi4/Hoi4Region.h"
#include "hoi4/NationalFocus.h"
#include <array>
#include <set>

namespace Scenario::Hoi4 {

class Generator : public Scenario::Generator {
  // vars
  int focusID = 0;
  std::map<std::string, int> totalResources;
  // containers
  std::vector<std::shared_ptr<Country>> majorPowers;
  std::vector<std::shared_ptr<Country>> regionalPowers;
  std::vector<std::shared_ptr<Country>> weakPowers;
  std::vector<std::string> wargoalsAttack;
  std::vector<std::string> goalsDefence;
  std::map<int, std::string> doctrineMap{
      {0, "blitz"},   {1, "infantry"}, {2, "milita"},  {3, "artillery"},
      {4, "armored"}, {5, "mass"},     {6, "support"}, {7, "defensive"}};

public:
  // containers
  std::vector<std::shared_ptr<Region>> hoi4States;
  std::vector<std::shared_ptr<Hoi4Country>> hoi4Countries;
  // a list of connections: {sourceHub, destHub, provinces the rails go through}
  std::vector<std::vector<int>> supplyNodeConnections;
  // container holding the resource configurations
  std::map<std::string, std::vector<double>> resources;
  std::map<std::string, double> weatherChances;
  // vars - track industry statistics
  int totalWorldIndustry = 0;
  int militaryIndustry = 0;
  int navalIndustry = 0;
  int civilianIndustry = 0;
  bool statesInitialised = false;

  // member functions
  // constructors/destructors
  Generator();
  Generator(const std::string &configSubFolder);
  ~Generator();
  void mapRegions();
  virtual Fwg::Gfx::Bitmap mapTerrain();
  void cutFromFiles(const std::string &gamePath);
  // initialize states
  // void initializeStates();
  // initialize states
  void mapCountries();
  // give resources to states
  void generateStateResources();
  // industry, development, population, state category
  void generateStateSpecifics();
  // politics: ideology, strength, major
  void generateCountrySpecifics();
  // generate weather per strategic region, from baseprovinces
  void generateWeather();
  // supply hubs and railroads
  void generateLogistics();
  // generate tech levels
  void generateTechLevels();

  // calculate how strong each country is
  void evaluateCountries();
  // determine the total amount of VPs per country, and distribute them in a
  // country
  void distributeVictoryPoints();
  // determine urbanisation
  void generateUrbanisation();
  bool unitFulfillsRequirements(std::vector<std::string> unitRequirements,
                                std::shared_ptr<Hoi4Country> &country);
  void generateArmorVariants();
  // determine unit composition, templates
  void generateCountryUnits();
  // determine unit composition, templates
  void generateCountryNavies();

  void generateFocusTrees();

  // see which countries are in need of unification
  void evaluateBrotherlyWars();
  // see which country needs to see some action
  void evaluateCivilWars();
  // create a strategy for this country
  void evaluateCountryStrategy();
  // print world info to console
  void printStatistics();

  void loadStates();
  virtual bool loadRivers(Fwg::Cfg &config,
                          Fwg::Gfx::Bitmap &riverInput) override;
};
} // namespace Scenario::Hoi4