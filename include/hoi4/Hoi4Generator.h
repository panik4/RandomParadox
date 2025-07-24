#pragma once
#include "FastWorldGenerator.h"
#include "generic/Country.h"
#include "generic/GenericParsing.h"
#include "generic/ScenarioGenerator.h"
#include "generic/ScenarioUtils.h"
#include "hoi4/Hoi4Army.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4FocusGen.h"
#include "hoi4/Hoi4Region.h"
#include "hoi4/NationalFocus.h"
#include <array>
#include <set>

namespace Scenario::Hoi4 {

class Generator : public Scenario::Generator {
  using CTI = Fwg::Climate::Detail::ClimateTypeIndex;
  std::vector<Scenario::Utils::ResConfig> resConfigs{
      {"chromium", true, 1250.0, true, Scenario::Utils::rareNoise},
      {"steel", true, 2562.0, true, Scenario::Utils::defaultNoise},
      {"tungsten", true, 1188.0, true, Scenario::Utils::semiRareNoise},
      {"aluminium", true, 1169, true, Scenario::Utils::semiRareNoise},
      {"oil", true, 1220.0, true, Scenario::Utils::rareLargePatch},
      {"rubber",
       true,
       1029.0,
       false,
       Scenario::Utils::agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 0.5}}}};
  // vars
  int focusID = 0;
  std::map<std::string, int> totalResources;


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

  // generate characters
  void generateCharacters();

  void generateArmorVariants();
  void generateAirVariants();
  // determine unit composition, templates
  void generateCountryUnits();
  // determine unit composition, templates
  void generateCountryNavies();

  void generateFocusTrees();

  void generatePositions();

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
                          const Fwg::Gfx::Bitmap &riverInput) override;
};
} // namespace Scenario::Hoi4