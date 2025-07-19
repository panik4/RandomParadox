#pragma once
#include "FastWorldGenerator.h"
#include "generic/VictoryPoint.h"
namespace Scenario {
enum class PositionType {
  Standstill,
  StandstillRG,
  Attacking,
  Defending,
  UnitMoving,
  UnitMovingRG,
  UnitDisembarking,
  UnitDisembarkingRG,
  ShipInPort,
  ShipInPortMoving,
  VictoryPoint
};
struct ScenarioPosition {
  Fwg::Position position;
  PositionType type;
  int typeIndex;
};
class GameProvince {
public:
  int ID;
  std::string name;
  std::string owner;
  std::string terrainType;
  double popFactor;
  double devFactor;
  double cityShare;
  std::shared_ptr<Fwg::Areas::Province> baseProvince;
  std::shared_ptr<VictoryPoint> victoryPoint;
  // containers
  std::vector<GameProvince> neighbours;
  // these positions are used for victory points, units, etc
  std::vector<ScenarioPosition> positions;

  // constructors/destructor
  GameProvince(std::shared_ptr<Fwg::Areas::Province> province);
  GameProvince();
  ~GameProvince();
  // operators
  bool operator==(const GameProvince &right) const { return ID == right.ID; };
  bool operator<(const GameProvince &right) const { return ID < right.ID; };
  std::string toHexString();
};
} // namespace Scenario
