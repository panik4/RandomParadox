#pragma once
#include "Hoi4Region.h"
#include "NationalFocus.h"
#include "generic/Country.h"
#include "hoi4/Hoi4Armor.h"
#include "hoi4/Hoi4Army.h"
#include "hoi4/Hoi4Navies.h"
#include <array>
#include <string>
#include <vector>

namespace Scenario::Hoi4 {

class Hoi4Country : public Scenario::Country {
public:

  Hoi4Country(std::string tag, int ID, std::string name, std::string adjective,
              Gfx::Flag flag);
  Hoi4Country(Scenario::Country &country,
              std::vector<std::shared_ptr<Hoi4::Region>> &hoi4Region);
  ~Hoi4Country();
  std::string fullName;
  std::string gfxCulture;
  // politics
  std::string ideology;
  bool allowElections;
  double bully;
  std::array<int, 4> parties;
  std::string lastElection;
  int warSupport;
  int stability;

  // mapdata
  int victoryPoints;
  std::vector<std::shared_ptr<Region>> hoi4Regions;
  // typedef doctrineType mytype;
  // national focus
  std::vector<FocusBranch> focusBranches;
  std::string focusTree;
  std::string ideas;
  // navy
  double navalFocus;
  int totalNavyStrength;
  std::map<NavalHullType, std::vector<TechEra>> hullTech;
  std::map<TechEra, std::vector<Technology>> navyTechs;
  std::map<ShipClassType, std::vector<ShipClass>> shipClasses;
  std::vector<std::shared_ptr<Ship>> ships;
  std::vector<Fleet> fleets;
  int convoyAmount;

  // army
  double landFocus;
  int totalArmyStrength;
  std::vector<int> units;
  std::vector<int> unitCount;
  std::map<TechEra, std::vector<Technology>> infantryTechs;
  std::map<TechEra, std::vector<Technology>> armorTechs;
  std::vector<TankVariant> tankVariants;
  std::vector<DivisionTemplate> divisionTemplates;
  std::vector<Division> divisions;
  // airforce
  double airFocus;
  int totalAirStrength;
  std::map<TechEra, std::vector<Technology>> airTechs;
  std::vector<PlaneVariant> planeVariants;
  std::vector<AirWing> airWings;
  std::map<std::shared_ptr<Region>, std::shared_ptr<AirBase>> airBases;
  void addAirBase(int level);

  // industry
  std::map<TechEra, std::vector<Technology>> industryElectronicTechs;
  int civilianIndustry;
  int armsFactories;
  int dockyards;
  int researchSlots;

  bool hasTech(const std::string &techname) const;
};
} // namespace Scenario::Hoi4