#pragma once
#include "Hoi4Region.h"
#include "NationalFocus.h"
#include "generic/Country.h"
#include "hoi4/Hoi4Navies.h"
#include "hoi4/Hoi4Armor.h"
#include <array>
#include <string>
#include <vector>

namespace Scenario::Hoi4 {

class Hoi4Country : public Scenario::Country {
public:
  enum class doctrineType {
    blitz,
    infantry,
    milita,
    artillery,
    armored,
    mass,
    support,
    defensive
  };
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
  std::map<NavalHullType, std::vector<TechEra>> hullTech;
  std::map<TechEra, std::vector<Technology>> navyTechs;
  std::map<ShipClassType, std::vector<ShipClass>> shipClasses;
  std::vector<std::shared_ptr<Ship>> ships;
  std::vector<Fleet> fleets;
  int convoyAmount;

  // army
  double landFocus;
  std::vector<doctrineType> doctrines;
  std::vector<int> units;
  std::vector<int> unitCount;
  std::map<TechEra, std::vector<Technology>> infantryTechs;
  std::map<TechEra, std::vector<Technology>> armorTechs;
  std::vector<TankVariant> tankVariants;

  // airforce
  double airFocus;
  std::map<TechEra, std::vector<Technology>> airTechs;

  // industry
  std::map<TechEra, std::vector<Technology>> industryElectronicTechs;
  int civilianIndustry;
  int armsFactories;
  int dockyards;
  int researchSlots;
};
} // namespace Scenario::Hoi4