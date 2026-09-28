#pragma once
#include "FastWorldGenerator.h"
#include "areas/SuperRegion.h"
#include "countries/Country.h"
#include "generic/ModGenerator.h"
#include "generic/StrategicRegion.h"
#include "hoi4/Hoi4Army.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4DecisionGen.h"
#include "hoi4/Hoi4Region.h"
#include "utils/RpxUtils.h"
#include <array>
#include <set>

namespace Rpx::Hoi4 {
struct Hoi4Config {
  // modifiers for prevalence of certain weather types
  std::map<std::string, double> weatherChances;
  // container holding the resource configurations
  std::map<std::string, std::vector<double>> resources;
  using CTI = Fwg::Climate::Detail::ClimateClassId;
  std::vector<Arda::Utils::ResConfig> resConfigs{
      {"chromium", true, 1250.0, true, Arda::Utils::semiRareNoise},
      {"steel", true, 2562.0, true, Arda::Utils::defaultNoise},
      {"tungsten", true, 1188.0, true, Arda::Utils::semiRareNoise},
      {"aluminium", true, 1169, true, Arda::Utils::semiRareNoise},
      {"oil", true, 1220.0, true, Arda::Utils::rareLargePatch},
      {"coal", true, 3000.0, true, Arda::Utils::semiRareNoise},
      {"rubber",
       true,
       1029.0,
       false,
       Arda::Utils::agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 0.5}}}};
  float startingArmyStrengthFactor = 1.0f;
  float startingNavyStrengthFactor = 1.0f;
  float startingAirforceStrengthFactor = 1.0f;
};

struct Hoi4Data {
  // containers
  std::vector<std::shared_ptr<Region>> hoi4States;
  std::vector<std::shared_ptr<Hoi4Country>> hoi4Countries;
  // a list of connections: {sourceHub, destHub, provinces the rails go through}
  std::vector<std::vector<int>> supplyNodeConnections;
  bool statesInitialised = false;
  std::vector<std::shared_ptr<Faction>> factions;
  DecisionData decisionData;
  std::map<Arda::Utils::Ideology,
           std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>>>
      greatPowerIdeologyMap;
};

struct Hoi4Stats {
  // vars - track industry statistics
  int totalWorldIndustry = 0;
  int militaryIndustry = 0;
  int navalIndustry = 0;
  int civilianIndustry = 0;

  std::map<Rpx::Hoi4::DivisionType, int> divisionsByType = {
      {Rpx::Hoi4::DivisionType::Irregulars, 0},
      {Rpx::Hoi4::DivisionType::Militia, 0},
      {Rpx::Hoi4::DivisionType::Infantry, 0},
      {Rpx::Hoi4::DivisionType::SupportedInfantry, 0},
      {Rpx::Hoi4::DivisionType::HeavyArtilleryInfantry, 0},
      {Rpx::Hoi4::DivisionType::Cavalry, 0},
      {Rpx::Hoi4::DivisionType::Motorized, 0},
      {Rpx::Hoi4::DivisionType::SupportedMotorized, 0},
      {Rpx::Hoi4::DivisionType::HeavyArtilleryMotorized, 0},
      {Rpx::Hoi4::DivisionType::Armor, 0}};

  void resetDivisionStats() {
    for (auto &[divisionType, count] : divisionsByType) {
      count = 0;
    }
  }

  std::map<Rpx::Hoi4::ShipClassType, int> shipsByClass = {
      {Rpx::Hoi4::ShipClassType::Destroyer, 0},
      {Rpx::Hoi4::ShipClassType::LightCruiser, 0},
      {Rpx::Hoi4::ShipClassType::HeavyCruiser, 0},
      {Rpx::Hoi4::ShipClassType::BattleCruiser, 0},
      {Rpx::Hoi4::ShipClassType::BattleShip, 0},
      {Rpx::Hoi4::ShipClassType::Carrier, 0},
      {Rpx::Hoi4::ShipClassType::Submarine, 0}};

  void resetShipStats() {
    for (auto &[shipClass, count] : shipsByClass) {
      count = 0;
    }
  }

  int totalFighters = 0;
  int totalCAS = 0;
  int totalMediumPlanes = 0;
  int totalHeavyPlanes = 0;
};
} // namespace Rpx::Hoi4