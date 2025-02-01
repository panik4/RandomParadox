#pragma once
#include "Hoi4Region.h"
#include "NationalFocus.h"
#include "generic/Country.h"
#include "hoi4/Hoi4Navies.h"
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
  std::string ideology;
  bool allowElections;
  double bully;
  int victoryPoints;
  // typedef doctrineType mytype;
  std::vector<doctrineType> doctrines;
  std::vector<int> units;
  std::vector<int> unitCount;

  std::map<ShipClassType, std::vector<ShipClass>> shipClasses;
  std::vector<std::shared_ptr<Ship>> ships;
  std::vector<Fleet> fleets;
  std::array<int, 4> parties;
  std::vector<std::shared_ptr<Region>> hoi4Regions;
  std::vector<FocusBranch> focusBranches;
  std::string focusTree;
  std::string ideas;
  double navalFocus;
  std::map<NavalHullType, std::vector<TechEra>> hullTech;
  std::map<TechEra, std::vector<Module>> moduleTech;
  std::map<TechEra, std::vector<ArmyTech>> armyTech;
  double airFocus;
  double landFocus;
  int civilianIndustry;
  int armsFactories;
  int dockyards;
};
} // namespace Scenario::Hoi4