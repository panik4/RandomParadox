#pragma once
#include "FastWorldGenerator.h"
#include "countries/Country.h"
#include "generic/ModGenerator.h"
#include "hoi4/Hoi4Army.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4FocusGen.h"
#include "hoi4/Hoi4ImageExporter.h"
#include "hoi4/Hoi4Parsing.h"
#include "hoi4/Hoi4Region.h"
#include "io/GenericParsing.h"
#include "utils/RpxUtils.h"
#include <array>
#include <set>

namespace Rpx::Hoi4 {

struct Hoi4Config {
  // modifiers for prevalence of certain weather types
  std::map<std::string, double> weatherChances;
  // container holding the resource configurations
  std::map<std::string, std::vector<double>> resources;
  using CTI = Fwg::Climate::Detail::ClimateTypeIndex;
  std::vector<Arda::Utils::ResConfig> resConfigs{
      {"chromium", true, 1250.0, true, Arda::Utils::rareNoise},
      {"steel", true, 2562.0, true, Arda::Utils::defaultNoise},
      {"tungsten", true, 1188.0, true, Arda::Utils::semiRareNoise},
      {"aluminium", true, 1169, true, Arda::Utils::semiRareNoise},
      {"oil", true, 1220.0, true, Arda::Utils::rareLargePatch},
      {"rubber",
       true,
       1029.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 0.5}}}};
};

struct Hoi4Data {
  // containers
  std::vector<std::shared_ptr<Region>> hoi4States;
  std::vector<std::shared_ptr<Hoi4Country>> hoi4Countries;
  // a list of connections: {sourceHub, destHub, provinces the rails go through}
  std::vector<std::vector<int>> supplyNodeConnections;
  bool statesInitialised = false;
};

struct Hoi4Stats {
  // vars - track industry statistics
  int totalWorldIndustry = 0;
  int militaryIndustry = 0;
  int navalIndustry = 0;
  int civilianIndustry = 0;
};

class Generator : public Rpx::ModGenerator {
  // a hoi4 specific image exporter
  Gfx::Hoi4::ImageExporter imageExporter;
  Hoi4Stats stats;

public:
  Hoi4Config modConfig;
  Hoi4Data modData;

  // constructors/destructors
  Generator(const std::string &configSubFolder,
            const boost::property_tree::ptree &rpdConf);
  ~Generator();
  // member functions
  bool createPaths();
  void configureModGen(const std::string &configSubFolder,
                       const std::string &username,
                       const boost::property_tree::ptree &rpdConf) override;

  void mapRegions();
  virtual Fwg::Gfx::Bitmap mapTerrain();
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

  // print world info to console
  void printStatistics();

  void loadStates();
  virtual bool loadRivers(Fwg::Cfg &config,
                          const Fwg::Gfx::Bitmap &riverInput) override;

  virtual void generate();
  virtual void initImageExporter();
  void writeLocalisation();
  virtual void writeTextFiles();
  virtual void writeImages();
  const Gfx::Hoi4::ImageExporter &getImageExporter() const {
    return imageExporter;
  }

  void readHoi(std::string &gamePath);
};
} // namespace Rpx::Hoi4