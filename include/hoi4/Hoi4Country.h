#pragma once
#include "Hoi4Region.h"
#include "countries/Country.h"
#include "hoi4/Hoi4Armor.h"
#include "hoi4/Hoi4Army.h"
#include "hoi4/Hoi4Navies.h"
#include <array>
#include <string>
#include <vector>

namespace Rpx::Hoi4 {

class Hoi4Country : public Arda::Country {
public:
  Hoi4Country();
  Hoi4Country(std::string tag, int ID, std::string name, std::string adjective,
              Arda::Gfx::Flag flag);
  Hoi4Country(Arda::Country &country,
              std::vector<std::shared_ptr<Hoi4::Region>> &hoi4Region);
  ~Hoi4Country();
  std::string fullName;
  std::string gfxCulture;
  // politics
  std::string ideology;
  bool allowElections = false;
  std::array<int, 4> parties = {25, 25, 25, 25};
  std::string lastElection = "1.1.1933";
  int warSupport = 0;
  int stability = 0;

  // mapdata
  int victoryPoints = 0;
  std::vector<std::shared_ptr<Region>> hoi4Regions;
  // typedef doctrineType mytype;
  // national focus
  std::string focusTree;
  std::string ideas;
  // navy
  int totalNavyStrength = 0;
  std::map<NavalHullType, std::vector<TechEra>> hullTech;
  std::map<TechEra, std::vector<Technology>> navyTechs;
  std::map<ShipClassType, std::vector<ShipClass>> shipClasses;
  std::vector<std::shared_ptr<Ship>> ships;
  std::vector<Fleet> fleets;
  int convoyAmount = 0;

  // army
  int totalArmyStrength = 0;
  std::vector<int> units;
  std::vector<int> unitCount;
  std::map<TechEra, std::vector<Technology>> infantryTechs;
  std::map<TechEra, std::vector<Technology>> armorTechs;
  std::vector<TankVariant> tankVariants;
  std::vector<DivisionTemplate> divisionTemplates;
  std::vector<Division> divisions;
  // airforce
  int totalAirStrength;
  std::map<TechEra, std::vector<Technology>> airTechs;
  std::vector<PlaneVariant> planeVariants;
  std::vector<AirWing> airWings;
  std::map<std::shared_ptr<Region>, std::shared_ptr<AirBase>> airBases;
  void addAirBase(int level);

  // industry
  std::map<TechEra, std::vector<Technology>> industryElectronicTechs;
  int civilianIndustry = 0;
  int armsFactories = 0;
  int dockyards = 0;
  int researchSlots = 0;

  bool hasTech(const std::string &techname) const;

  std::string exportLine() const;
};
} // namespace Rpx::Hoi4