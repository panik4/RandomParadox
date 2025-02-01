#pragma once
#include "hoi4/Hoi4Region.h"
#include <array>
#include <string>
#include <vector>

namespace Scenario::Hoi4 {

enum class TechEras { Interwar, Buildup };

enum class NavalHullTypes { Light, Cruiser, Heavy, Carrier, Submarine };

enum class ModuleTypes {
  
};

//enum class ShipClassEra { Interwar, Buildup };

enum class ShipClassType {
  Destroyer,
  LightCruiser,
  HeavyCruiser,
  BattleCruiser,
  BattleShip,
  Carrier,
  Submarine
};

struct ShipClass {
  ShipClassType type;
  TechEras era;
  std::string name;
  int tonnage;
  // TODO: Equipment
};

struct Ship {
  ShipClass shipClass;
  std::string name;
};

struct Fleet {
  std::string name;
  std::shared_ptr<GameProvince> startingPort;
  std::vector<std::shared_ptr<Ship>> ships;
};
} // namespace Scenario::Hoi4