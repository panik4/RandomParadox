#include "hoi4/Hoi4SupplyGen.h"

namespace Rpx::Hoi4 {
std::vector<int> extractProvincesFromConnection(
    const Fwg::Civilization::Connection &connection,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces,
    const Fwg::Areas::AreaData &areas, Fwg::Gfx::Image &provinceMap) {
  if (connection.type != Fwg::Civilization::ConnectionType::Land ||
      connection.source.expired() || connection.destination.expired()) return {};
  const auto pixels = Fwg::Civilization::Locations::connectionPixels(connection);
  std::vector<int> result;
  int previousPixel = -1;
  for (int pixel : pixels) {
    if (pixel < 0 || pixel >= static_cast<int>(provinceMap.size())) return {};
    if (previousPixel >= 0) {
      const int width = provinceMap.width();
      int dx = std::abs(pixel % width - previousPixel % width);
      dx = std::min(dx, width - dx);
      if (dx + std::abs(pixel / width - previousPixel / width) != 1) return {};
    }
    previousPixel = pixel;
    const auto &province = areas.provinceColourMap.at(provinceMap[pixel]);
    if (!province || province->isSea() || province->isLake() || province->ID < 0 ||
        province->ID >= static_cast<int>(provinces.size()) || !provinces[province->ID])
      return {};
    if (!result.empty() && result.back() == province->ID) continue;
    if (!result.empty()) {
      const int previous = result.back();
      if (previous < 0 || previous >= static_cast<int>(provinces.size())) return {};
      const auto &neighbors = provinces[previous]->neighbours;
      if (std::none_of(neighbors.begin(), neighbors.end(),
                       [&](const auto &other) { return other->ID == province->ID; })) {
        Fwg::Utils::Logging::logLine("Discarding noncontiguous railway connection");
        return {};
      }
    }
    result.push_back(province->ID);
  }
  return result;
}

void generateLogistics(
    std::shared_ptr<Arda::ArdaGen> generator, Hoi4Data &data,
    const Fwg::Areas::AreaData &areas,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces,
    Fwg::Gfx::Image &provinceMap, Fwg::Gfx::Image & /*countryMap*/) {
  Fwg::Utils::Logging::logLine("HOI4: Building rail networks");
  Fwg::Utils::Randomisation::resetRandomisation();
  data.supplyNodeConnections.clear();
  std::vector<Fwg::Civilization::Locations::AreaLocationSet> groups;
  for (const auto &country : data.hoi4Countries) {
    Fwg::Civilization::Locations::AreaLocationSet group{country, {}};
    for (const auto &state : country->ownedRegions) {
      std::shared_ptr<Fwg::Civilization::Location> largestCity;
      for (const auto &location : state->locations) {
        if (location->type == Fwg::Civilization::LocationType::Port)
          group.locations.push_back(location);
        if (location->land && location->type == Fwg::Civilization::LocationType::City &&
            (!largestCity || location->size() > largestCity->size()))
          largestCity = location;
      }
      if (largestCity) group.locations.push_back(largestCity);
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
        if (!visited.insert(std::minmax(location->ID, destinationId)).second) continue;
        auto route = extractProvincesFromConnection(connection, provinces, areas, provinceMap);
        if (!route.empty()) data.supplyNodeConnections.push_back(std::move(route));
      }
    }
  }
}
} // namespace Rpx::Hoi4
