#pragma once
#include "FastWorldGenerator.h"
#include "areas/ArdaRegion.h"
#include "generic/VictoryPoint.h"
#include "hoi4/Hoi4Airforce.h"
#include "hoi4/Hoi4BuildingPlacement.h"
#include "utils/RpxUtils.h"
#include "utils/SerialisationFwd.h"

namespace Rpx::Hoi4 {

class Region : public Arda::ArdaRegion {
  const std::vector<std::string> buildingTypes{
      "arms_factory",       "industrial_complex",
      "air_base",           "bunker",
      "coastal_bunker",     "dockyard",
      "naval_base_spawn",   "anti_air_building",
      "synthetic_refinery", "nuclear_reactor_spawn",
      "rocket_site_spawn",  "radar_station",
      "fuel_silo",          "floating_harbor",
      "stronghold_network", "special_project_facility_spawn"};

public:
  // member variables
  // the identifier the game uses for this state, referred to by localisation,
  // history, foci, etc
  std::string identifier = "";
  int armsFactories = 0;
  int civilianFactories = 0;
  int dockyards = 0;

  int infrastructure = 0;
  int stateCategory = 0;
  int stratID = 0;
  int airport = 0, rocketsite = 0, supplyNode = 0;
  double totalVictoryPoints = 0.0;
  Arda::Utils::WeatherPosition weatherPosition;
  // Containers
  std::vector<Arda::Utils::Building> buildings;
  // Derived runtime data; snapshots retain their stored building positions.
  std::vector<ResolvedPort> resolvedPorts;
  bool portsResolved = false;
  double airfieldSuitability = -1; // Unknown until generated or reconstructed.
  std::map<int, std::shared_ptr<Arda::VictoryPoint>> victoryPointsMap;
  std::map<int, int> navalBases;
  std::shared_ptr<AirBase> airBase;
  // Constructors/Destructors
  Region();
  Region(const Arda::ArdaRegion &ardaRegion);
  ~Region();

  // member functions
  void calculateBuildingPositions(const Fwg::Terrain::TerrainData &terrain,
                                  const Fwg::Gfx::Image &provinceMap,
                                  const Fwg::Areas::AreaData &areaData);
  void calculateBuildingPositions(const std::vector<float> &heightmap,
                                  const Fwg::Gfx::Image &provinceMap,
                                  const Fwg::Areas::AreaData &areaData);
  std::string exportLine() const;
  void importLine(const std::vector<std::string> &tokens) override;

  // serialisation
  template<class Archive>
  void serialize(Archive &ar, const unsigned int /*version*/);
};
} // namespace Rpx::Hoi4

BOOST_CLASS_EXPORT_KEY(Rpx::Hoi4::Region)

