#pragma once
#include "ArdaGen.h"
#include "FastWorldGenerator.h"
#include "eu5/Eu5ImageExporter.h"
#include "eu5/Eu5Parsing.h"
#include <cmath>
#include <generic/ModGenerator.h>
#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <vector>
namespace Rpx::Eu5 {

struct Eu5Config {
  using CTI = Fwg::Climate::Detail::ClimateTypeIndex;
  std::vector<Arda::Utils::ResConfig> resConfigs{};
};
struct Eu5Data {
  // std::vector<std::shared_ptr<Arda::Region>> eu5Regions;
  // std::map<std::string, std::shared_ptr<Country>> eu5Countries;
};
struct Eu5GameData {
  // std::map<std::string, Technology> techs;
  // std::map<std::string, TechnologyLevel> techLevels;
  // std::map<std::string, ProductionmethodGroup> productionmethodGroups;
  // std::map<std::string, Productionmethod> productionmethods;
  // std::vector<BuildingType> buildingsTypes;
  // std::map<std::string, Good> goods;
  // std::map<std::string, Buypackage> buypackages;
  // std::map<std::string, PopNeed> popNeeds;
  //// to search for productionmethods that produce this good
  // std::map<std::string, std::vector<Productionmethod>>
  // goodToProdMethodsOutput;
  //// to search for productionmethods that require this good
  // std::map<std::string, std::vector<Productionmethod>>
  // goodToProdMethodsInput;

  //// to search for buildings that use this productionMethod
  // std::map<std::string, std::vector<BuildingType>>
  //     productionMethodToBuildingTypes;
  //// to search for buildings that produce this good
  // std::map<std::string, std::vector<BuildingType>> goodToBuildingTypes;
};
struct Eu5Stats {};

class Generator : public Rpx::ModGenerator {
  Eu5Config eu5Config;
  Eu5Data modData;
  Eu5GameData eu5GameData;
  Eu5Stats eu5Stats;

  Rpx::Gfx::Eu5::ImageExporter imageExporter;

public:
  // std::vector<Arda::Utils::ResConfig> &getResConfigs() {
  //   //return this->vic3Config.resConfigs;
  // }
  Generator(const std::string &configSubFolder,
            const boost::property_tree::ptree &rpdConf);
  // clear and create all the mod paths at each run
  bool createPaths();

  void configureModGen(const std::string &configSubFolder,
                       const std::string &username,
                       const boost::property_tree::ptree &rpdConf) override;
  virtual Fwg::Gfx::Bitmap mapTerrain();
  virtual void mapRegions();
  void totalArableLand(const std::vector<float> &arableLand);
  void distributeResources();
  // map scenario countries to vic3 countries
  virtual void mapCountries();
  // details of countries
  virtual void generateCountrySpecifics();
  // read buildings, techs, etc from the game files
  bool importData(const std::string &path);

  // figure out how countries stand towards each other - colonies,
  // protectorates, vassals, allies, customs union etc
  void diplomaticRelations();

  // combine countries into markets according to their diplomatic stances
  void createMarkets();

  void calculateNeeds();

  void distributeBuildings();

  // iterates over all states and tries to place locators for all 5 types
  void createLocators();

  // calculate naval exits
  void calculateNavalExits();

  virtual void generate();
  virtual void initImageExporter();
  virtual void writeTextFiles();
  virtual void writeImages();
  void writeSplnet();
};
} // namespace Rpx::Eu5