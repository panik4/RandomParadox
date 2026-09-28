#include "hoi4/Hoi4SupplyGen.h"

namespace Rpx::Hoi4 {
std::vector<int> findProvinceBridge(
    int startID, int endID,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &ardaProvinces) {

  std::queue<int> q;
  std::unordered_map<int, int> parent;
  parent[startID] = -1;
  q.push(startID);

  while (!q.empty()) {
    int cur = q.front();
    q.pop();

    if (cur == endID)
      break;

    for (const auto &neighArea : ardaProvinces[cur]->neighbours) {
      int neighID = neighArea->ID;

      if (parent.count(neighID))
        continue;

      const auto &prov = ardaProvinces[neighID];
      if (prov->isSea() || prov->isLake())
        continue;

      parent[neighID] = cur;
      q.push(neighID);
    }
  }

  if (!parent.count(endID))
    return {};

  std::vector<int> path;
  for (int p = endID; p != -1; p = parent[p])
    path.push_back(p);

  std::reverse(path.begin(), path.end());
  return path;
}
std::vector<int> extractProvincesFromConnection(
    const Fwg::Civilization::Connection &conn,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &ardaProvinces,
    const Fwg::Areas::AreaData &areaData, Fwg::Gfx::Image &provinceMap) {
  std::vector<int> connectionPixels;
  connectionPixels.reserve(conn.connectingPixels.size() + 2);

  // Step 0: full pixel path including endpoints
  connectionPixels.push_back(conn.source->position.weightedCenter);
  connectionPixels.insert(connectionPixels.end(), conn.connectingPixels.begin(),
                          conn.connectingPixels.end());
  connectionPixels.push_back(conn.destination->position.weightedCenter);

  // Step 1: map pixels to provinces, removing duplicates and ignoring sea/lake
  std::vector<int> rawProvinces;
  rawProvinces.reserve(connectionPixels.size());

  int lastProvinceID = -1;
  for (int pix : connectionPixels) {
    auto provinceColour = provinceMap[pix];
    auto prov = areaData.provinceColourMap.at(provinceColour);

    if (!prov || prov->isSea() || prov->isLake())
      continue;

    if (prov->ID != lastProvinceID) {
      rawProvinces.push_back(prov->ID);
      lastProvinceID = prov->ID;
    }
  }

  if (rawProvinces.size() < 2)
    return rawProvinces; // trivial path

  // Step 2: enforce contiguity, inserting bridges where needed
  std::vector<int> fixedPath;
  fixedPath.reserve(rawProvinces.size());

  fixedPath.push_back(rawProvinces[0]); // always start with first

  for (size_t i = 0; i + 1 < rawProvinces.size(); ++i) {
    int a = rawProvinces[i];
    int b = rawProvinces[i + 1];

    // check adjacency
    bool adjacent = false;
    for (const auto &neigh : ardaProvinces[a]->neighbours) {
      if (neigh->ID == b) {
        adjacent = true;
        break;
      }
    }

    if (!adjacent) {
      // find bridge from a -> b, returns [a, x1, x2, ..., b]
      auto bridge = findProvinceBridge(a, b, ardaProvinces);
      if (bridge.size() > 2) {
        // insert interior provinces only
        fixedPath.insert(fixedPath.end(), bridge.begin() + 1, bridge.end() - 1);
      }
    }

    // always append the next province
    fixedPath.push_back(b);
  }

  return fixedPath;
}

void generateLogistics(
    std::shared_ptr<Arda::ArdaGen> ardaGen, Hoi4Data &modData,
    const Fwg::Areas::AreaData &areaData,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &eligibleProvinces,
    Fwg::Gfx::Image &provinceMap, Fwg::Gfx::Image &countryMap) {
  Fwg::Utils::Logging::logLine("HOI4: Building rail networks");
  Fwg::Utils::Randomisation::resetRandomisation();
  auto &supplyNodeConnections = modData.supplyNodeConnections;
  supplyNodeConnections.clear();
  auto width = Fwg::Cfg::Values().width;
  // create a copy of the country map for
  // visualisation of the logistics
  auto logistics = countryMap;

  std::vector<Fwg::Civilization::Locations::AreaLocationSet> navmeshLocations;

  for (auto countryID = 0; auto &country : modData.hoi4Countries) {
    Fwg::Civilization::Locations::AreaLocationSet areaLocationSet;
    areaLocationSet.area = country; // or &state, depending on your model
    country->ID = countryID++;
    for (const auto &state : country->ownedRegions) {

      std::shared_ptr<Fwg::Civilization::Location> largestCity = nullptr;
      float largestArea = -1.0f;

      for (const auto &loc : state->locations) {

        // collect ports
        if (loc->type == Fwg::Civilization::LocationType::Port) {
          areaLocationSet.locations.push_back(loc);
        }

        // track largest city (land only, non-port)
        if (loc->land && loc->type == Fwg::Civilization::LocationType::City) {
          if (loc->size() > largestArea) {
            largestArea = loc->size();
            largestCity = loc;
          }
        }
      }

      // ensure we have at least one inland anchor
      if (largestCity) {
        areaLocationSet.locations.push_back(largestCity);
      }
    }
    navmeshLocations.push_back(areaLocationSet);
  }
  std::vector<std::shared_ptr<Fwg::Areas::Area>> customNavigationPoints;
  customNavigationPoints.reserve(areaData.provinces.size());
  for (auto &province : areaData.provinces) {
    customNavigationPoints.push_back(province);
  }
  ardaGen->genNavmesh(navmeshLocations, customNavigationPoints);

  std::set<std::pair<const std::shared_ptr<Fwg::Civilization::Location>,
                     const std::shared_ptr<Fwg::Civilization::Location>>>
      visited;

  for (const auto &areaLocationSet : navmeshLocations) {
    for (const auto &loc : areaLocationSet.locations) {

      for (const auto &[destLoc, conn] : loc->connections) {

        const auto a = loc;
        const auto b = destLoc;

        // normalize edge key (undirected)
        auto key = std::minmax(a, b);

        // if (visited.count(key))
        //   continue;

        // visited.insert(key);

        auto provinces =
            extractProvincesFromConnection(conn, eligibleProvinces, areaData, provinceMap);

        if (!provinces.empty()) {
          supplyNodeConnections.push_back(std::move(provinces));
        }
      }
    }
  }
}

} // namespace Rpx::Hoi4