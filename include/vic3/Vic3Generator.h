#pragma once
#include "Vic3Country.h"
#include "Vic3Region.h"
#include "vic3/Vic3Importer.h"
#include "vic3/Vic3Utils.h"
#include <cmath>
#include <generic/ModGenerator.h>
#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <vector>
namespace Rpx::Vic3 {

struct Vic3StratRegion {
  std::set<int> areaIDs;
  std::string name;
};

class Generator : public Rpx::ModGenerator {
  using CTI = Fwg::Climate::Detail::ClimateTypeIndex;
  std::vector<Arda::Utils::ResConfig> resConfigs{
      {"bg_coal_mining", true, 5.0, true, Arda::Utils::defaultNoise},
      {"bg_gold_mining", true, 0.2, true, Arda::Utils::rareNoise},
      {"bg_gold_fields", true, 0.2, true, Arda::Utils::rareNoise},
      {"bg_iron_mining", true, 5.0, true, Arda::Utils::defaultNoise},
      {"bg_lead_mining", true, 2.0, true, Arda::Utils::semiRareNoise},
      {"bg_oil_extraction", true, 1.0, true, Arda::Utils::rareLargePatch},
      {"bg_sulfur_mining", true, 2.0, true, Arda::Utils::semiRareNoise},
      {"bg_logging",
       true,
       10.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 0.8},
        {CTI::COLDSEMIARID, 0.1},
        {CTI::HOTSEMIARID, 0.1},
        {CTI::CONTINENTALCOLD, 1.0},
        {CTI::CONTINENTALHOT, 1.0},
        {CTI::CONTINENTALWARM, 0.8},
        {CTI::TEMPERATECOLD, 1.0},
        {CTI::TEMPERATEWARM, 0.8},
        {CTI::TEMPERATEHOT, 0.6}}},
      {"bg_livestock_ranches",
       false,
       10.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::POLARTUNDRA, 0.2},
        {CTI::TROPICSMONSOON, 0.7},
        {CTI::TROPICSRAINFOREST, 0.6},
        {CTI::TROPICSSAVANNA, 0.8},
        {CTI::COLDSEMIARID, 0.5},
        {CTI::HOTSEMIARID, 0.4},
        {CTI::CONTINENTALCOLD, 0.9},
        {CTI::CONTINENTALHOT, 0.9},
        {CTI::CONTINENTALWARM, 0.9},
        {CTI::TEMPERATECOLD, 1.0},
        {CTI::TEMPERATEWARM, 1.0},
        {CTI::TEMPERATEHOT, 1.0}}},
      {"bg_rubber",
       true,
       5.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 0.5}}},
      {"bg_banana_plantations",
       false,
       10.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 1.0},
        {CTI::TROPICSSAVANNA, 0.9}}},
      {"bg_coffee_plantations",
       false,
       5.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 1.0},
        {CTI::TROPICSSAVANNA, 1.0},
        {CTI::HOTSEMIARID, 1.0}}},
      {"bg_cotton_plantations",
       false,
       10.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TEMPERATEHOT, 0.8},
        {CTI::HOTSEMIARID, 1.0},
        {CTI::TROPICSMONSOON, 0.8},
        {CTI::TROPICSRAINFOREST, 0.4},
        {CTI::TROPICSSAVANNA, 0.4}}},
      {"bg_dye_plantations",
       false,
       5.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::HOTSEMIARID, 1.0},
        {CTI::TROPICSMONSOON, 0.8},
        {CTI::TROPICSRAINFOREST, 0.4},
        {CTI::TROPICSSAVANNA, 0.4}}},
      {"bg_opium_plantations",
       false,
       4.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::HOTSEMIARID, 0.8}}},
      {"bg_silk_plantations",
       false,
       5.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TEMPERATEHOT, 1.0}}},
      {"bg_sugar_plantations",
       false,
       1.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TEMPERATEHOT, 0.8},
        {CTI::HOTSEMIARID, 1.0},
        {CTI::TROPICSMONSOON, 0.8},
        {CTI::TROPICSRAINFOREST, 0.4},
        {CTI::TROPICSSAVANNA, 0.4}}},
      {"bg_tea_plantations",
       false,
       1.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::HOTSEMIARID, 1.0},
        {CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.4},
        {CTI::TROPICSSAVANNA, 0.6}}},
      {"bg_tobacco_plantations",
       false,
       1.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::HOTSEMIARID, 1.0},
        {CTI::TROPICSMONSOON, 0.8},
        {CTI::TROPICSRAINFOREST, 0.4},
        {CTI::TROPICSSAVANNA, 0.4}}},
      {"bg_vineyard_plantations",
       false,
       1.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TEMPERATEHOT, 1.0}, {CTI::TEMPERATEWARM, 0.8}}},
      {"bg_wheat_farms",
       false,
       1.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::CONTINENTALHOT, 1.0},
        {CTI::CONTINENTALWARM, 0.8},
        {CTI::TEMPERATEHOT, 1.0},
        {CTI::TEMPERATEWARM, 0.8},
        {CTI::HOTSEMIARID, 0.8}}},
      {"bg_maize_farms",
       false,
       1.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 1.0},
        {CTI::CONTINENTALHOT, 1.0},
        {CTI::CONTINENTALWARM, 0.8},
        {CTI::TEMPERATEHOT, 1.0},
        {CTI::TEMPERATEWARM, 0.8},
        {CTI::HOTSEMIARID, 0.8}}},
      {"bg_millet_farms",
       false,
       1.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 1.0},
        {CTI::HOTSEMIARID, 0.8}}},
      {"bg_rice_farms",
       false,
       1.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TEMPERATEWARM, 0.8},
        {CTI::HOTSEMIARID, 0.8}}},
      {"bg_rye_farms",
       false,
       1.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::CONTINENTALCOLD, 1.0},
        {CTI::CONTINENTALWARM, 0.8},
        {CTI::TEMPERATECOLD, 1.0},
        {CTI::COLDSEMIARID, 0.8}}},
      {"bg_fishing",
       true,
       1.0,
       false,
       Arda::Utils::defaultNoise,
       false,
       {},
       false,
       {},
       true,
       1.0,
       1.0},
      {"bg_whaling",
       true,
       1.0,
       false,
       Arda::Utils::defaultNoise,
       false,
       {},
       false,
       {},
       true,
       1.0,
       0.0}};

  std::map<std::string, Technology> techs;
  std::map<std::string, TechnologyLevel> techLevels;
  std::map<std::string, ProductionmethodGroup> productionmethodGroups;
  std::map<std::string, Productionmethod> productionmethods;
  std::vector<BuildingType> buildingsTypes;
  std::map<std::string, Good> goods;
  std::map<std::string, Buypackage> buypackages;
  std::map<std::string, PopNeed> popNeeds;
  // to search for productionmethods that produce this good
  std::map<std::string, std::vector<Productionmethod>> goodToProdMethodsOutput;
  // to search for productionmethods that require this good
  std::map<std::string, std::vector<Productionmethod>> goodToProdMethodsInput;

  // to search for buildings that use this productionMethod
  std::map<std::string, std::vector<BuildingType>>
      productionMethodToBuildingTypes;
  // to search for buildings that produce this good
  std::map<std::string, std::vector<BuildingType>> goodToBuildingTypes;

public:
  std::vector<Arda::Utils::ResConfig> &getResConfigs() {
    return this->resConfigs;
  }
  std::vector<std::shared_ptr<Region>> vic3Regions;
  std::map<std::string, std::shared_ptr<Country>> vic3Countries;
  Generator();
  Generator(const std::string &configSubFolder);
  virtual Fwg::Gfx::Bitmap mapTerrain();
  virtual void mapRegions();
  void distributePops();
  void totalArableLand(const std::vector<float> &arableLand);
  void distributeResources();
  // initialize states
  virtual void initializeStates();
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
};
} // namespace Rpx::Vic3