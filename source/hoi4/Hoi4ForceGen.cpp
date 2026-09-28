#include "hoi4/Hoi4ForceGen.h"

namespace Rpx::Hoi4 {

void generateArmorVariants(Hoi4Config modConfig, Hoi4Data &modData,
                           Hoi4Stats &stats) {
  struct TankType {
    ArmorType type;
    ArmorRole subType;
  };
  Fwg::Utils::Logging::logLine("HOI4: Generating Armor Variants");
  for (auto &country : modData.hoi4Countries) {
    if (country->hoi4Regions.empty()) {
      continue;
    }
    // first check if we have any armor techs
    if (hasTechnology(country->armorTechs, "gwtank_chassis")) {
      auto combinedTech = country->armorTechs;
      // add all landTechs for the different weapon types
      for (auto &techEra : country->infantryTechs) {
        for (auto &tech : techEra.second) {
          combinedTech.at(techEra.first).push_back(tech);
        }
      }
      std::map<std::string, TankType> chassisToGenerate;
      chassisToGenerate["light_tank_chassis_0"] = {ArmorType::LightArmor,
                                                   ArmorRole::Tank};
      chassisToGenerate["medium_tank_chassis_0"] = {ArmorType::MediumArmor,
                                                    ArmorRole::Tank};
      if (hasTechnology(country->armorTechs, "interwar_antitank")) {
        chassisToGenerate["light_tank_chassis_0"] = {ArmorType::LightArmor,
                                                     ArmorRole::TankDestroyer};
        chassisToGenerate["medium_tank_chassis_0"] = {ArmorType::MediumArmor,
                                                      ArmorRole::TankDestroyer};
      }
      if (hasTechnology(country->armorTechs, "interwar_artillery")) {
        chassisToGenerate["light_tank_chassis_0"] = {ArmorType::LightArmor,
                                                     ArmorRole::Artillery};
        chassisToGenerate["medium_tank_chassis_0"] = {ArmorType::MediumArmor,
                                                      ArmorRole::Artillery};
      }
      chassisToGenerate["heavy_tank_chassis_0"] = {ArmorType::HeavyArmor,
                                                   ArmorRole::Tank};
      if (hasTechnology(country->armorTechs, "basic_light_tank_chassis")) {
        chassisToGenerate["light_tank_chassis_1"] = {ArmorType::LightArmor,
                                                     ArmorRole::Tank};
        if (hasTechnology(country->armorTechs, "interwar_antitank")) {
          chassisToGenerate["light_tank_chassis_1"] = {
              ArmorType::LightArmor, ArmorRole::TankDestroyer};
        }
        if (hasTechnology(country->armorTechs, "interwar_artillery")) {
          chassisToGenerate["light_tank_chassis_1"] = {ArmorType::LightArmor,
                                                       ArmorRole::Artillery};
        }
      }
      if (hasTechnology(country->armorTechs, "improved_light_tank_chassis")) {
        chassisToGenerate["light_tank_chassis_2"] = {ArmorType::LightArmor,
                                                     ArmorRole::Tank};

        if (hasTechnology(country->armorTechs, "interwar_antitank")) {
          chassisToGenerate["light_tank_chassis_2"] = {
              ArmorType::LightArmor, ArmorRole::TankDestroyer};
        }
        if (hasTechnology(country->armorTechs, "interwar_artillery")) {
          chassisToGenerate["light_tank_chassis_2"] = {ArmorType::LightArmor,
                                                       ArmorRole::Artillery};
        }
      }

      if (hasTechnology(country->armorTechs, "basic_heavy_tank_chassis")) {
        chassisToGenerate["heavy_tank_chassis_1"] = {ArmorType::HeavyArmor,
                                                     ArmorRole::Tank};
        if (hasTechnology(country->armorTechs, "interwar_antitank")) {
          chassisToGenerate["heavy_tank_chassis_1"] = {
              ArmorType::HeavyArmor, ArmorRole::TankDestroyer};
        }
      }

      for (auto &chassis : chassisToGenerate) {
        // we can create a tank variant
        TankVariant tankVariant;
        tankVariant.type = chassis.second.type;
        tankVariant.subType = chassis.second.subType;
        tankVariant.bbaArmorName = chassis.first;
        tankVariant.era = TechEra::Interwar;
        tankVariant.name = country->getPrimaryCulture()
                               ->language->generateGenericCapitalizedWord() +
                           " Mk " + std::to_string(RandNum::getRandom(0, 3));

        addArmorModules(tankVariant, combinedTech);
        country->tankVariants.push_back(tankVariant);
      }
    }
  }
}
void generateAirVariants(Hoi4Config modConfig, Hoi4Data &modData,
                         Hoi4Stats &stats) {
  struct AirType {
    PlaneType type;
    PlaneRole subType;
    std::string frame;
    TechEra era;
  };
  Fwg::Utils::Logging::logLine("HOI4: Generating Air Variants");

  for (auto &country : modData.hoi4Countries) {
    country->planeVariants.clear();
    country->airWings.clear();
    // clear all wings from the countries airbases
    for (auto &airbase : country->airBases) {
      airbase->wings.clear();
    }
    country->airBases.clear();

    // if we don't have any regions, skip this country
    if (country->hoi4Regions.empty()) {
      continue;
    }
    bool hasCarrier = false;
    for (auto &ship : country->ships) {
      if (ship->shipClass.type == ShipClassType::Carrier) {
        hasCarrier = true;
        break;
      }
    }
    std::map<std::string, AirType> frameToGenerate;
    // first check if we have techs for small airframes
    if (hasTechnology(country->airTechs, "iw_small_airframe")) {
      frameToGenerate["iw_fighter"] = {PlaneType::SmallFrame,
                                       PlaneRole::Fighter, "iw_small_airframe",
                                       TechEra::Interwar};
      if (hasCarrier) {
        frameToGenerate["iw_carrier_fighter"] = {
            PlaneType::SmallFrame, PlaneRole::CarrierFighter,
            "iw_small_airframe", TechEra::Interwar};
      }
      if (hasTechnology(country->airTechs, "air_torpedoe_1")) {
        frameToGenerate["iw_nav_bomb"] = {
            PlaneType::SmallFrame, PlaneRole::NavalBomber, "iw_small_airframe",
            TechEra::Interwar};
        if (hasCarrier) {
          frameToGenerate["iw_carrier_nav_bomb"] = {
              PlaneType::SmallFrame, PlaneRole::CarrierNavalBomber,
              "iw_small_airframe", TechEra::Interwar};
        }
      }
      // check if we have everything for gw CAS
      if (hasTechnology(country->airTechs, "early_bombs")) {
        frameToGenerate["gw_cas"] = {PlaneType::SmallFrame, PlaneRole::Cas,
                                     "gw_small_airframe", TechEra::Interwar};
        if (hasCarrier) {
          frameToGenerate["iw_carrier_cas"] = {
              PlaneType::SmallFrame, PlaneRole::CarrierCas, "iw_small_airframe",
              TechEra::Interwar};
        }
      }
      // do the same for basic small airframes
      if (hasTechnology(country->airTechs, "basic_small_airframe")) {
        frameToGenerate["basic_fighter"] = {
            PlaneType::SmallFrame, PlaneRole::Fighter, "basic_small_airframe",
            TechEra::Buildup};
        if (hasCarrier) {
          frameToGenerate["basic_carrier_fighters"] = {
              PlaneType::SmallFrame, PlaneRole::CarrierFighter,
              "basic_small_airframe", TechEra::Buildup};
        }
        if (hasTechnology(country->airTechs, "air_torpedoe_1")) {
          frameToGenerate["basic_nav_bomb"] = {
              PlaneType::SmallFrame, PlaneRole::NavalBomber,
              "basic_small_airframe", TechEra::Buildup};
          if (hasCarrier) {
            frameToGenerate["basic_carrier_nav_bomb"] = {
                PlaneType::SmallFrame, PlaneRole::CarrierNavalBomber,
                "basic_small_airframe", TechEra::Buildup};
          }
        }
        // check if we have everything for basic CAS
        if (hasTechnology(country->airTechs, "early_bombs")) {
          frameToGenerate["basic_cas"] = {PlaneType::SmallFrame, PlaneRole::Cas,
                                          "basic_small_airframe",
                                          TechEra::Buildup};
          if (hasCarrier) {
            frameToGenerate["basic_carrier_cas"] = {
                PlaneType::SmallFrame, PlaneRole::CarrierCas,
                "basic_small_airframe", TechEra::Buildup};
          }
        }
      }
    }
    // tact bombers, strat bombers
    if (hasTechnology(country->airTechs, "early_bombs")) {
      if (hasTechnology(country->airTechs, "iw_medium_airframe")) {
        frameToGenerate["iw_tac_bomb"] = {
            PlaneType::MediumFrame, PlaneRole::TacticalBomber,
            "iw_medium_airframe", TechEra::Interwar};
        if (hasTechnology(country->airTechs, "basic_medium_airframe")) {
          frameToGenerate["basic_tac_bomb"] = {
              PlaneType::MediumFrame, PlaneRole::TacticalBomber,
              "basic_medium_airframe", TechEra::Buildup};
        }
      }
      if (hasTechnology(country->airTechs, "iw_large_airframe")) {
        frameToGenerate["iw_strat_bomb"] = {
            PlaneType::LargeFrame, PlaneRole::StrategicBomber,
            "iw_large_airframe", TechEra::Interwar};
        if (hasTechnology(country->airTechs, "basic_large_airframe")) {
          frameToGenerate["basic_strat_bomb"] = {
              PlaneType::LargeFrame, PlaneRole::StrategicBomber,
              "basic_large_airframe", TechEra::Buildup};
        }
      }
    }
    for (auto &frame : frameToGenerate) {
      // we can create a plane variant
      PlaneVariant airVariant;
      airVariant.type = frame.second.type;
      airVariant.subType = frame.second.subType;
      if (airVariant.type == PlaneType::SmallFrame) {
        airVariant.bbaFrameName = "small_plane_airframe_0";
        airVariant.vanillaFrameName = "fighter_equipment_0";
        if (airVariant.subType == PlaneRole::CarrierCas) {
          airVariant.bbaFrameName = "cv_small_plane_cas_airframe_0";
          airVariant.vanillaFrameName = "cv_CAS_equipment_0";
        } else if (airVariant.subType == PlaneRole::CarrierFighter) {
          airVariant.bbaFrameName = "cv_small_plane_airframe_0";
          airVariant.vanillaFrameName = "cv_fighter_equipment_0";
        } else if (airVariant.subType == PlaneRole::CarrierNavalBomber) {
          airVariant.bbaFrameName = "cv_small_plane_naval_bomber_airframe_0";
          airVariant.vanillaFrameName = "cv_nav_bomber_equipment_0";
        } else if (airVariant.subType == PlaneRole::Cas) {
          airVariant.bbaFrameName = "small_plane_cas_airframe_0";
          airVariant.vanillaFrameName = "CAS_equipment_0";
        } else if (airVariant.subType == PlaneRole::Fighter) {
          airVariant.bbaFrameName = "small_plane_airframe_0";
          airVariant.vanillaFrameName = "fighter_equipment_0";
        } else if (airVariant.subType == PlaneRole::NavalBomber) {
          airVariant.bbaFrameName = "small_plane_naval_bomber_airframe_0";
          airVariant.vanillaFrameName = "nav_bomber_equipment_0";
        }

      } else if (airVariant.type == PlaneType::MediumFrame) {
        airVariant.bbaFrameName = "medium_plane_airframe_0";
        airVariant.vanillaFrameName = "tac_bomber_equipment_0";
      } else if (airVariant.type == PlaneType::LargeFrame) {
        airVariant.bbaFrameName = "large_plane_airframe_0";
        airVariant.vanillaFrameName = "strat_bomber_equipment_0";
      }
      // if we have a basic variant, we replace the 0 with 1
      if (frame.second.era == TechEra::Buildup) {
        airVariant.bbaFrameName[airVariant.bbaFrameName.size() - 1] = '1';
      }

      airVariant.name = country->getPrimaryCulture()
                            ->language->generateGenericCapitalizedWord() +
                        " Mk " + std::to_string(RandNum::getRandom(0, 3));

      addPlaneModules(airVariant, country->airTechs);
      country->planeVariants.push_back(airVariant);
    }
    // lets distribute at least ONE airbase throughout every country, even those
    // without plane tech. This is due to hoi4 ai not building airforces
    // properly otherwise after researching the techs
    country->addAirBase(1);
    double airforceStrength = country->airFocus * country->armsFactories *
                              modConfig.startingAirforceStrengthFactor;
    int airBaseAmount = 1 + airforceStrength / 10.0;
    if (country->planeVariants.size() && airforceStrength > 0) {
      // lets distribute airbases throughout the country
      for (int i = 0; i < airBaseAmount; i++) {
        country->addAirBase(1);
      }
      // first gather the amount of planes per variant
      while (airforceStrength > 0) {
        auto &variant =
            Fwg::Utils::Random::selectRandom(country->planeVariants);
        variant.amount++;
        airforceStrength -= variant.cost;
      }

      // now we generate the air wings
      for (auto i = 0; i < country->planeVariants.size(); i++) {
        for (auto j = 0; j < country->planeVariants[i].amount; j += 50) {
          AirWing wing;
          wing.variant = country->planeVariants[i];
          wing.name = std::to_string(i) + ". " + country->planeVariants[i].name;
          wing.amount = std::min<int>(country->planeVariants[i].amount, 50);
          auto &randomAirbase =
              Fwg::Utils::Random::selectRandom(country->airBases);
          randomAirbase->wings.push_back(wing);
          country->airWings.push_back(wing);
        }
      }
    }
  }
}
void generateCountryUnits(Hoi4Config modConfig, Hoi4Data &modData,
                          Hoi4Stats &stats) {
  Fwg::Utils::Logging::logLine("HOI4: Generating Country Unit Files");

  const std::vector<DivisionType> divisionTypes = {
      DivisionType::Irregulars,
      DivisionType::Infantry,
      DivisionType::SupportedInfantry,
      DivisionType::HeavyArtilleryInfantry,
      DivisionType::Cavalry,
      DivisionType::Motorized,
      DivisionType::Armor};
  stats.resetDivisionStats();
  for (auto &country : modData.hoi4Countries) {
    // clear existing divisions
    country->divisions.clear();
    country->divisionTemplates.clear();

    // first determine total army strength based on arms industry
    // TODO add a factor to settings
    country->totalArmyStrength = country->armsFactories * 10 *
                                 modConfig.startingArmyStrengthFactor;

    // basic idea: we create unit templates first. We start with irregulars,
    // then infantry only, then infantry with support, then infantry with
    // artillery, then infantry with armor, then motorised infantry, then
    // motorised infantry with support, then motorised infantry with armor.
    // for each of those, we depend on certain techs.
    // each of these will vary a bit per country, depending on their techs and
    // some randomness in regiments per column (we vary between 2-4 regiments
    // of the same type per column)
    std::vector<CombatRegimentType> allowedRegimentTypes;
    std::vector<SupportRegimentType> allowedSupportRegimentTypes;
    std::set<DivisionType> desiredDivisionTemplates;
    // we also vary the amount of columns per division, between 2 and 4
    if (country->hasTech("infantry_weapons")) {
      allowedRegimentTypes.push_back(CombatRegimentType::Infantry);
      allowedRegimentTypes.push_back(CombatRegimentType::Irregulars);
      desiredDivisionTemplates.insert(DivisionType::Militia);
      desiredDivisionTemplates.insert(DivisionType::Infantry);
      desiredDivisionTemplates.insert(DivisionType::Cavalry);
    }
    if (country->hasTech("tech_recon")) {
      allowedSupportRegimentTypes.push_back(SupportRegimentType::Recon);
      desiredDivisionTemplates.insert(DivisionType::SupportedInfantry);
    }
    if (country->hasTech("tech_maintenance_company")) {
      allowedSupportRegimentTypes.push_back(SupportRegimentType::Maintenance);
      desiredDivisionTemplates.insert(DivisionType::SupportedInfantry);
    }
    if (country->hasTech("tech_engineers")) {
      allowedSupportRegimentTypes.push_back(SupportRegimentType::Engineer);
      desiredDivisionTemplates.insert(DivisionType::SupportedInfantry);
    }
    if (country->hasTech("gw_artillery")) {
      allowedRegimentTypes.push_back(CombatRegimentType::Artillery);
      allowedSupportRegimentTypes.push_back(SupportRegimentType::Artillery);
      desiredDivisionTemplates.insert(DivisionType::HeavyArtilleryInfantry);
      if (country->hasTech("tech_trucks")) {
        allowedRegimentTypes.push_back(CombatRegimentType::MotorizedArtillery);
      }
    }
    if (country->hasTech("interwar_antiair")) {
      allowedRegimentTypes.push_back(CombatRegimentType::AntiAir);
      allowedSupportRegimentTypes.push_back(SupportRegimentType::AntiAir);
      if (country->hasTech("tech_trucks")) {
        allowedRegimentTypes.push_back(CombatRegimentType::MotorizedAntiAir);
      }
    }
    if (country->hasTech("interwar_antitank")) {
      allowedRegimentTypes.push_back(CombatRegimentType::AntiTank);
      allowedSupportRegimentTypes.push_back(SupportRegimentType::AntiTank);
      if (country->hasTech("tech_trucks")) {
        allowedRegimentTypes.push_back(CombatRegimentType::MotorizedAntiTank);
      }
    }
    if (country->hasTech("motorised_infantry")) {
      allowedRegimentTypes.push_back(CombatRegimentType::Motorized);
      desiredDivisionTemplates.insert(DivisionType::Motorized);
      // if we have CombatRegimentType::MotorizedAntiTank or AntiAir or
      // Artillery we want a supportedMotorized
      if (country->hasTech("tech_recon") ||
          country->hasTech("tech_engineers") ||
          country->hasTech("gw_artillery")) {
        desiredDivisionTemplates.insert(DivisionType::SupportedMotorized);
      }
      // with artillery available, lets get a motorized artillery division
      if (country->hasTech("gw_artillery")) {
        desiredDivisionTemplates.insert(DivisionType::HeavyArtilleryMotorized);
      }
    }
    if (country->hasTech("basic_light_tank_chassis")) {
      allowedRegimentTypes.push_back(CombatRegimentType::LightArmor);
    }

    // now we generate the division templates
    country->divisionTemplates =
        createDivisionTemplates(desiredDivisionTemplates, allowedRegimentTypes,
                                allowedSupportRegimentTypes);

    // at the end, we evaluate which of these templates is used with which
    // share, as a developed country for example will NOT use irregular
    // infantry in its army, but a minor power might. the more developed we
    // are, the more likely we are to use the more expensive divisions
    auto &development = country->technologyLevel;
    for (auto &division : country->divisionTemplates) {
      if (division.type == DivisionType::Militia) {
        division.armyShare = 0.35 - development;
      } else if (division.type == DivisionType::Cavalry) {
        division.armyShare = 0.2 - (development - 0.3);
      } else if (division.type == DivisionType::Infantry) {
        division.armyShare = 0.2 - (development - 0.3);
      } else if (division.type == DivisionType::SupportedInfantry) {
        division.armyShare = 0.2 - (development - 0.4);
      } else if (division.type == DivisionType::HeavyArtilleryInfantry) {
        division.armyShare = 0.2 - (development - 0.4);
      } else if (division.type == DivisionType::Motorized) {
        division.armyShare = 0.1 - (development - 0.5);
      } else if (division.type == DivisionType::SupportedMotorized) {
        division.armyShare = 0.1 - (development - 0.6);
      } else if (division.type == DivisionType::HeavyArtilleryMotorized) {
        division.armyShare = 0.1 - (development - 0.6);
      } else if (division.type == DivisionType::Armor) {
        division.armyShare = 0.1 - (development - 0.7);
      }
      // clamp to non-negative to prevent negative shares
      division.armyShare = std::max(0.0, division.armyShare);
    }
    // now normalise the shares so we get a sum of 1
    double sum = 0.0;
    for (auto &division : country->divisionTemplates) {
      sum += division.armyShare;
    }
    // prevent division by zero or near-zero which would create huge shares
    if (sum > 0.0) {
      for (auto &division : country->divisionTemplates) {
        division.armyShare /= sum;
      }
    }
    // now we can generate the divisions. Each typeshare is multiplied with
    // the totalArmyStrength, and then we generate the divisions until their
    // cost reaches the typeshare
    if (country->ownedRegions.size()) {

      // lets gather eligible provinces for division placement
      std::vector<std::shared_ptr<Arda::ArdaProvince>> eligibleProvinces;
      for (auto &region : country->hoi4Regions) {
        if (region->isLand() &&
            !region->topographyTypes.count(
                Arda::Civilization::TopographyType::WASTELAND)) {
          for (auto &province : region->ardaProvinces) {
            eligibleProvinces.push_back(province);
          }
        }
      }

      for (auto &divisionTemplate : country->divisionTemplates) {
        auto divisionMaxCost =
            divisionTemplate.armyShare * country->totalArmyStrength;
        int count = 1;
        while ((divisionMaxCost -= divisionTemplate.cost) > 0) {
          if (eligibleProvinces.size()) {
            Division division;
            division.divisionTemplate = divisionTemplate;
            division.name = std::to_string(count);

            // Special-case 11, 12, 13
            int lastTwo = count % 100;
            if (lastTwo >= 11 && lastTwo <= 13) {
              division.name += "th";
            } else {
              switch (count % 10) {
              case 1:
                division.name += "st";
                break;
              case 2:
                division.name += "nd";
                break;
              case 3:
                division.name += "rd";
                break;
              default:
                division.name += "th";
                break;
              }
            }
            division.location =
                Fwg::Utils::Random::selectRandom(eligibleProvinces);
            division.name += " '" + division.location->name + "' " +
                             division.divisionTemplate.name;
            division.startingEquipmentFactor =
                std::min<double>(0.7 + country->technologyLevel * 0.3 +
                                     RandNum::getRandom(0.0, 0.2),
                                 1.0);
            division.startingExperienceFactor = RandNum::getRandom(0.0, 1.0);
            stats.divisionsByType[division.divisionTemplate.type]++;
            country->divisions.push_back(division);
            count++;
          }
        }
      }
    }
  }
}

void generateCountryNavies(Hoi4Config modConfig, Hoi4Data &modData,
                           Hoi4Stats &stats, std::vector<std::shared_ptr<Arda::ArdaProvince>>& eligibleProvinces) {

  for (auto &country : modData.hoi4Countries) {
    country->fleets.clear();
    country->ships.clear();
    country->shipClasses.clear();

    if (!country->ownedRegions.size())
      continue;
    // first generate the different ship classes, in each ShipclassType, we
    // have three: Interwar, Buildup
    for (const auto &shipclassType : shipClassTypes) {
      country->shipClasses.insert({shipclassType, {}});
      auto availableHullTypeEras =
          country->hullTech[shipClassToHullType[shipclassType]];

      for (const auto &shipera : shipEras) {
        // check if we have the required tech level for this ship class
        if (std::find(availableHullTypeEras.begin(),
                      availableHullTypeEras.end(),
                      shipera) == availableHullTypeEras.end()) {
          continue;
        }

        ShipClass shipClass;
        shipClass.type = shipclassType;
        shipClass.era = shipera;
        auto primaryCulture = country->getPrimaryCulture();
        if (!primaryCulture) {
          shipClass.name =
              std::to_string(country->shipClasses.size()) + " Class";
          Fwg::Utils::Logging::logLine(
              "Warning: Country " + country->name +
              " has no primary culture, cannot generate ship names");
        } else {
          shipClass.name = Fwg::Utils::Random::selectRandom(
                               primaryCulture->language->shipNames) +
                           " Class";
        }
        shipClass.vanillaShipType =
            ShipClassTypeDefinitions[shipclassType] +
            (shipClass.era == TechEra::Interwar ? "_1" : "_2");

        shipClass.mtgHullname =
            shipHullDefinitions[shipclassType] +
            (shipClass.era == TechEra::Interwar ? "_1" : "_2");

        // carriers are special just for mtg, they have a different interwar
        // level, namely deck conversions from ca and bb.
        if (shipclassType == ShipClassType::Carrier) {
          if (shipClass.era == TechEra::Interwar) {
            // randomly decide on ca or bb deck conversion
            if (RandNum::getRandom(0.0, 1.0) < 0.5) {
              shipClass.mtgHullname = "ship_hull_carrier_conversion_bb";
            } else {
              shipClass.mtgHullname = "ship_hull_carrier_conversion_ca";
            }
          } else {
            // _1 is the second level, early carriers, different from all other
            // ship classes
            shipClass.mtgHullname = shipHullDefinitions[shipclassType] + "_1";
          }
        }

        shipClass.tonnage = tonnages[shipclassType];

        addShipClassModules(shipClass, country->navyTechs,
                            country->infantryTechs);
        country->shipClasses.at(shipClass.type).push_back(shipClass);
      }
    }
    // we only set the designs if we're landlocked
    if (country->landlocked) {
      continue;
    }

    // determine the total tonnage by taking the naval focus times the
    // countries naval industry
    auto totalTonnage = country->navalFocus * country->dockyards * 400.0 *
                        modConfig.startingNavyStrengthFactor;

    // calculate amount of convoys based on tonnage
    country->convoyAmount = totalTonnage / 500;

    // now determine the composition of the navy, first the share of carriers,
    // battleships and screens
    auto carrierShare = 0.0;
    auto battleshipShare = 0.0;
    auto screenShare = 0.0;
    // carriers are only built by major powers
    if (country->rank == Arda::Rank::GreatPower) {
      carrierShare = 0.15;
      battleshipShare = 0.2;
      screenShare = 0.65;
    } else if (country->rank == Arda::Rank::SecondaryPower) {
      carrierShare = 0.1;
      battleshipShare = 0.3;
      screenShare = 0.6;
    } else if (country->rank == Arda::Rank::RegionalPower) {
      carrierShare = 0.00;
      battleshipShare = 0.3;
      screenShare = 0.7;
    } else if (country->rank == Arda::Rank::LocalPower) {
      carrierShare = 0.00;
      battleshipShare = 0.2;
      screenShare = 0.8;
    } else {
      carrierShare = 0.0;
      battleshipShare = 0.1;
      screenShare = 0.9;
    }

    // let's evaluate if the carrier tonnage is enough to spawn one carrier
    int carrierTargetTonnage = totalTonnage * carrierShare;
    const std::vector<ShipClass> &carrierClasses =
        country->shipClasses.at(ShipClassType::Carrier);
    bool canAffordCarrier = false;
    if (carrierClasses.size()) {
      auto randomCarrierShipClass =
          Fwg::Utils::Random::selectRandom(carrierClasses);
      // check if we can afford at least one carrier
      if (carrierTargetTonnage > randomCarrierShipClass.tonnage) {
        canAffordCarrier = true;
        // as long as we have enough tonnage for a carrier, spawn one
        while (carrierTargetTonnage > randomCarrierShipClass.tonnage) {
          // create a carrier ship
          Ship carrier;
          carrier.shipClass = randomCarrierShipClass;
          // push shared pointer to new ship
          country->ships.push_back(std::make_shared<Ship>(carrier));
          carrierTargetTonnage -= randomCarrierShipClass.tonnage;
        }
      }
    }
    // if we can't afford a carrier, redistribute the tonnage to battleship
    // share
    if (!canAffordCarrier) {
      battleshipShare += carrierShare;
    }
    int heavyShipTargetTonnage = totalTonnage * battleshipShare;
    // we randomly select Ship Classes Battleship and Heavy Cruiser
    const std::vector<ShipClass> &battleshipClasses =
        country->shipClasses.at(ShipClassType::BattleShip);
    const std::vector<ShipClass> &battleCruiserClasses =
        country->shipClasses.at(ShipClassType::BattleCruiser);
    const std::vector<ShipClass> &heavyCruiserClasses =
        country->shipClasses.at(ShipClassType::HeavyCruiser);
    bool canAffordHeavyShip = false;
    if (battleshipClasses.size() || battleCruiserClasses.size() ||
        heavyCruiserClasses.size()) {
      // determine minimum tonnage required for any heavy ship
      int minHeavyShipTonnage = std::numeric_limits<int>::max();
      if (heavyCruiserClasses.size()) {
        for (const auto &shipClass : heavyCruiserClasses) {
          minHeavyShipTonnage =
              std::min(minHeavyShipTonnage, shipClass.tonnage);
        }
      }
      if (battleCruiserClasses.size()) {
        for (const auto &shipClass : battleCruiserClasses) {
          minHeavyShipTonnage =
              std::min(minHeavyShipTonnage, shipClass.tonnage);
        }
      }
      if (battleshipClasses.size()) {
        for (const auto &shipClass : battleshipClasses) {
          minHeavyShipTonnage =
              std::min(minHeavyShipTonnage, shipClass.tonnage);
        }
      }

      // check if we can afford at least one heavy ship
      if (heavyShipTargetTonnage > minHeavyShipTonnage) {
        canAffordHeavyShip = true;
        // as long as we have enough tonnage for a heavy ship, spawn one
        while (heavyShipTargetTonnage > 0) {
          // create a heavy ship
          Ship heavyShip;
          bool shipSelected = false;

          // try to select a ship that fits
          int attempts = 0;
          while (!shipSelected && attempts < 20) {
            if (RandNum::getRandom(0, 2)) {
              if (heavyCruiserClasses.size()) {
                heavyShip.shipClass =
                    Fwg::Utils::Random::selectRandom(heavyCruiserClasses);
                shipSelected = true;
              }
            } else if (RandNum::getRandom(0, 2)) {
              if (battleCruiserClasses.size()) {
                heavyShip.shipClass =
                    Fwg::Utils::Random::selectRandom(battleCruiserClasses);
                shipSelected = true;
              }
            } else {
              if (battleshipClasses.size()) {
                heavyShip.shipClass =
                    Fwg::Utils::Random::selectRandom(battleshipClasses);
                shipSelected = true;
              }
            }
            attempts++;
          }

          if (!shipSelected) {
            break;
          }

          // check if the selected ship fits in the remaining tonnage
          if (heavyShip.shipClass.tonnage > heavyShipTargetTonnage) {
            // try to find a smaller ship that fits
            bool foundSmallerShip = false;
            if (heavyCruiserClasses.size()) {
              for (const auto &shipClass : heavyCruiserClasses) {
                if (shipClass.tonnage <= heavyShipTargetTonnage) {
                  heavyShip.shipClass = shipClass;
                  foundSmallerShip = true;
                  break;
                }
              }
            }
            if (!foundSmallerShip && battleCruiserClasses.size()) {
              for (const auto &shipClass : battleCruiserClasses) {
                if (shipClass.tonnage <= heavyShipTargetTonnage) {
                  heavyShip.shipClass = shipClass;
                  foundSmallerShip = true;
                  break;
                }
              }
            }
            if (!foundSmallerShip && battleshipClasses.size()) {
              for (const auto &shipClass : battleshipClasses) {
                if (shipClass.tonnage <= heavyShipTargetTonnage) {
                  heavyShip.shipClass = shipClass;
                  foundSmallerShip = true;
                  break;
                }
              }
            }
            if (!foundSmallerShip) {
              // no ship fits, break out
              break;
            }
          }

          // push shared pointer to new ship
          country->ships.push_back(std::make_shared<Ship>(heavyShip));
          heavyShipTargetTonnage -= heavyShip.shipClass.tonnage;
        }
      }
    }
    // if we can't afford a heavy ship, redistribute the tonnage to screen
    // share
    if (!canAffordHeavyShip) {
      screenShare += battleshipShare;
    }

    // now we have to distribute the remaining tonnage to screens
    int screenTargetTonnage = totalTonnage * screenShare;
    const std::vector<ShipClass> &destroyerClasses =
        country->shipClasses.at(ShipClassType::Destroyer);
    const std::vector<ShipClass> &lightCruiserClasses =
        country->shipClasses.at(ShipClassType::LightCruiser);
    if (destroyerClasses.size() || lightCruiserClasses.size()) {
      // as long as we have enough tonnage for a screen, spawn one
      while (screenTargetTonnage > 0) {
        // create a screen ship
        Ship screenShip;
        bool shipSelected = false;

        // try to select a ship that fits
        int attempts = 0;
        while (!shipSelected && attempts < 20) {
          if (RandNum::getRandom(0, 2)) {
            if (destroyerClasses.size()) {
              screenShip.shipClass =
                  Fwg::Utils::Random::selectRandom(destroyerClasses);
              shipSelected = true;
            }
          } else {
            if (lightCruiserClasses.size()) {
              screenShip.shipClass =
                  Fwg::Utils::Random::selectRandom(lightCruiserClasses);
              shipSelected = true;
            }
          }
          attempts++;
        }

        if (!shipSelected) {
          break;
        }

        // check if the selected ship fits in the remaining tonnage
        if (screenShip.shipClass.tonnage > screenTargetTonnage) {
          // try to find a smaller ship that fits
          bool foundSmallerShip = false;
          if (destroyerClasses.size()) {
            for (const auto &shipClass : destroyerClasses) {
              if (shipClass.tonnage <= screenTargetTonnage) {
                screenShip.shipClass = shipClass;
                foundSmallerShip = true;
                break;
              }
            }
          }
          if (!foundSmallerShip && lightCruiserClasses.size()) {
            for (const auto &shipClass : lightCruiserClasses) {
              if (shipClass.tonnage <= screenTargetTonnage) {
                screenShip.shipClass = shipClass;
                foundSmallerShip = true;
                break;
              }
            }
          }
          if (!foundSmallerShip) {
            // no ship fits, break out
            break;
          }
        }

        // push shared pointer to new ship
        country->ships.push_back(std::make_shared<Ship>(screenShip));
        screenTargetTonnage -= screenShip.shipClass.tonnage;
      }
    }
  }
  stats.resetShipStats();
  // put all ships in one fleet
  for (auto &country : modData.hoi4Countries) {
    // we only set the designs if we're landlocked
    if (country->landlocked) {
      continue;
    }
    std::map<std::string, int> utilisedShipNames;
    Fleet fleet;
    fleet.name = country->name + " Fleet";
    for (auto &ship : country->ships) {
      auto primaryCulture = country->getPrimaryCulture();
      if (!primaryCulture) {
        ship->name = "Unnamed";
        Fwg::Utils::Logging::logLine(
            "Warning: Country " + country->name +
            " has no primary culture, cannot generate ship names");
      } else {
        ship->name = Fwg::Utils::Random::selectRandom(
            primaryCulture->language->shipNames);
      }
      if (utilisedShipNames.find(ship->name) != utilisedShipNames.end()) {
        utilisedShipNames[ship->name]++;
        ship->name += " " + std::to_string(utilisedShipNames[ship->name]);
      } else {
        utilisedShipNames[ship->name] = 1;
      }
      stats.shipsByClass[ship->shipClass.type]++;
      fleet.ships.push_back(ship);
    }
    // find some random port location
    for (auto &region : country->hoi4Regions) {
      for (auto &navalbase : region->navalBases) {
        if (navalbase.second > 0) {
          fleet.startingPort = eligibleProvinces.at(navalbase.first);
          break;
        }
      }
    }
    // check if no port was found
    if (fleet.startingPort == nullptr || fleet.ships.empty()) {
      Fwg::Utils::Logging::logLine(
          "Warning: Country " + country->name +
          " has no naval base or no ships, cannot assign fleet port");
    } else {
      country->fleets.push_back(fleet);
    }
  }
}

} // namespace Rpx::Hoi4