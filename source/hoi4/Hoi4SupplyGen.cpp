#include "hoi4/Hoi4SupplyGen.h"
#include "areas/ProvinceRoute.h"

namespace Rpx::Hoi4 {
std::vector<std::shared_ptr<Fwg::Civilization::Location>>
selectSupplyLocations(const Arda::ArdaRegion &state) {
  using Fwg::Civilization::LocationType;
  std::map<int, std::shared_ptr<Fwg::Civilization::Location>> selected;
  std::shared_ptr<Fwg::Civilization::Location> largestCity;
  for (const auto &location : state.locations) {
    if (!location || !location->land)
      continue;
    if (location->type == LocationType::Port ||
        location->secondaryType == LocationType::Port)
      selected.emplace(location->ID, location);
    if (location->type == LocationType::City ||
        location->secondaryType == LocationType::City)
      if (!largestCity || location->size() > largestCity->size() ||
          (location->size() == largestCity->size() &&
           location->ID < largestCity->ID))
        largestCity = location;
  }
  if (largestCity)
    selected.emplace(largestCity->ID, largestCity);
  std::vector<std::shared_ptr<Fwg::Civilization::Location>> result;
  for (const auto &[id, location] : selected)
    result.push_back(location);
  return result;
}
std::vector<int> extractProvincesFromConnection(
    const Fwg::Civilization::Connection &connection,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces,
    const Fwg::Areas::AreaData &areas, Fwg::Gfx::Image &provinceMap) {
  if (connection.type != Fwg::Civilization::ConnectionType::Land ||
      connection.source.expired() || connection.destination.expired())
    return {};
  const auto pixels =
      Fwg::Civilization::Locations::connectionPixels(connection);
  auto result = Arda::Areas::traceProvinceRoute(pixels, areas, provinceMap);
  for (int id : result)
    if (id < 0 || id >= provinces.size() || !provinces[id] ||
        provinces[id]->isSea() || provinces[id]->isLake())
      return {};
  return result;
}

void updateSupplyHubPositions(Hoi4Data &data,
                              const Fwg::Terrain::TerrainData &terrain,
                              const Fwg::Areas::AreaData &areas,
                              const Fwg::Gfx::Image &provinceMap) {
  if (terrain.detailedHeightMap.size() != provinceMap.size())
    throw std::invalid_argument("Supply hub terrain dimensions do not match");
  std::vector<std::pair<std::shared_ptr<Region>, Arda::Utils::Building>>
      pending;
  std::set<int> seen;
  for (const auto &hub : data.supplyHubs) {
    if (hub.pixel < 0 || hub.pixel >= provinceMap.size() ||
        !seen.insert(hub.provinceID).second)
      throw std::invalid_argument("Invalid or duplicated supply hub");
    const auto province = areas.provinceColourMap.at(provinceMap[hub.pixel]);
    if (!province || !province->isLand() || province->ID != hub.provinceID)
      throw std::invalid_argument("Supply hub is outside its province");
    const auto state =
        std::find_if(data.hoi4States.begin(), data.hoi4States.end(),
                     [&](const auto &candidate) {
                       return candidate->ID == province->regionID;
                     });
    if (state == data.hoi4States.end())
      throw std::invalid_argument("Supply hub state is missing");
    pending.push_back(
        {*state,
         {"supply_node",
          {hub.pixel % provinceMap.width(), hub.pixel / provinceMap.width(),
           terrain.detailedHeightMap.at(hub.pixel) / 10.0,
           randomBuildingRotation()},
          0,
          hub.provinceID}});
  }
  for (auto &state : data.hoi4States)
    std::erase_if(state->buildings, [](const auto &building) {
      return building.name == "supply_node";
    });
  for (auto &[state, building] : pending)
    state->buildings.push_back(std::move(building));
}

void generateLogistics(
    std::shared_ptr<Arda::ArdaGen> generator, Hoi4Data &data,
    const Fwg::Areas::AreaData &areas,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces,
    Fwg::Gfx::Image &provinceMap, Fwg::Gfx::Image & /*countryMap*/) {
  Fwg::Utils::Logging::logLine("HOI4: Building rail networks");
  Fwg::Utils::Randomisation::resetRandomisation();
  data.supplyNodeConnections.clear();
  data.supplyHubs.clear();
  struct HubCandidate {
    int pixel;
    double importance;
    int locationID;
  };
  std::map<int, HubCandidate> hubs;
  auto addHub =
      [&](int pixel,
          const std::shared_ptr<Fwg::Civilization::Location> &location) {
        const auto province = areas.provinceColourMap.at(provinceMap[pixel]);
        if (!province || !location)
          return;
        const auto existing = hubs.find(province->ID);
        HubCandidate candidate{pixel, location->importance, location->ID};
        if (existing == hubs.end() ||
            candidate.importance > existing->second.importance ||
            (candidate.importance == existing->second.importance &&
             std::pair(candidate.locationID, pixel) <
                 std::pair(existing->second.locationID,
                           existing->second.pixel)))
          hubs.insert_or_assign(province->ID, candidate);
      };
  std::vector<Fwg::Civilization::Locations::AreaLocationSet> groups;
  for (const auto &country : data.hoi4Countries) {
    Fwg::Civilization::Locations::AreaLocationSet group{country, {}};
    for (const auto &state : country->ownedRegions) {
      const auto locations = selectSupplyLocations(*state);
      group.locations.insert(group.locations.end(), locations.begin(),
                             locations.end());
    }
    groups.push_back(std::move(group));
  }
  std::vector<std::shared_ptr<Fwg::Areas::Area>> navigationAreas(
      areas.provinces.begin(), areas.provinces.end());
  generator->genNavmesh(groups, navigationAreas);
  std::set<std::pair<int, int>> visited;
  for (const auto &group : groups) {
    for (const auto &location : group.locations) {
      for (const auto &[destinationId, connection] : location->connections) {
        if (!visited.insert(std::minmax(location->ID, destinationId)).second)
          continue;
        auto route = extractProvincesFromConnection(connection, provinces,
                                                    areas, provinceMap);
        if (route.size() > 1) {
          const auto pixels =
              Fwg::Civilization::Locations::connectionPixels(connection);
          addHub(pixels.front(), connection.source.lock());
          addHub(pixels.back(), connection.destination.lock());
          data.supplyNodeConnections.push_back(std::move(route));
        }
      }
    }
  }
  for (const auto &[provinceID, hub] : hubs) {
    data.supplyHubs.push_back({provinceID, hub.pixel});
  }
  updateSupplyHubPositions(data, generator->terrainData, areas, provinceMap);
}
} // namespace Rpx::Hoi4
