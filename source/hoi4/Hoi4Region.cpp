#include "hoi4/Hoi4Region.h"
#include <optional>
#include <unordered_map>
#include <stdexcept>

namespace Rpx::Hoi4 {
Region::Region() {}

Region::Region(const Arda::ArdaRegion &ardaRegion)
    : Arda::ArdaRegion(ardaRegion), armsFactories{0}, civilianFactories{0},
      dockyards{0}, stateCategory{0}, stratID{0} {}

Region::~Region() {}

std::string Region::exportLine() const {
  auto line = Arda::ArdaRegion::exportLine();
  line += std::to_string(stateCategory) + ";";
  line += std::to_string(infrastructure) + ";";
  line += std::to_string(armsFactories) + ";";
  line += std::to_string(civilianFactories) + ";";
  line += std::to_string(dockyards) + ";";
  return line;
}

void Region::importLine(const std::vector<std::string> &tokens) {
  try {
    if (tokens.size() > 5)
      stateCategory = std::stoi(tokens[5]);
    if (tokens.size() > 6)
      infrastructure = std::stoi(tokens[6]);
    if (tokens.size() > 7)
      armsFactories = std::stoi(tokens[7]);
    if (tokens.size() > 8)
      civilianFactories = std::stoi(tokens[8]);
    if (tokens.size() > 9)
      dockyards = std::stoi(tokens[9]);
  } catch (const std::exception &e) {
    Fwg::Utils::Logging::logLine("Invalid HOI4 state mapping data: ",
                                 e.what());
  }
}

namespace {
struct CoastalCandidate { int pixel; int seaProvinceId; };
// Built once per region: expected O(P + C) time and O(P + C) temporary
// storage for P provinces and C coastal pixels. Each pixel checks at most
// four neighbours using the existing province image and colour hash table.
// Hash lookups are expected O(1), worst-case O(N) for N world provinces.
class CoastalCandidates {
  const Fwg::Gfx::Image &map;
  const Fwg::Areas::AreaData &areas;
  std::unordered_map<const Fwg::Areas::Province *, std::vector<CoastalCandidate>> candidates;
public:
  std::vector<const Fwg::Areas::Province *> eligible;
  CoastalCandidates(const std::vector<std::shared_ptr<Fwg::Areas::Province>> &provinces,
                    const Fwg::Gfx::Image &map, const Fwg::Areas::AreaData &areas)
      : map(map), areas(areas) {
    for (const auto &province : provinces) {
      if (!province->isLand() || !province->isCoastalToOcean()) continue;
      std::vector<CoastalCandidate> preferred, fallback;
      for (int pixel : province->coastalPixels) {
        if (pixel < 0 || pixel >= map.size() ||
            areas.provinceColourMap.at(map[pixel]).get() != province.get())
          continue;
        bool lake = false;
        auto candidate = classify(pixel, lake);
        if (!candidate) continue;
        fallback.push_back(*candidate);
        if (!lake) preferred.push_back(*candidate);
      }
      auto chosen = preferred.empty() ? std::move(fallback) : std::move(preferred);
      if (chosen.empty()) {
        Fwg::Utils::Logging::logLine("No ocean-adjacent building candidates for province ", province->ID);
        continue;
      }
      eligible.push_back(province.get());
      candidates.emplace(province.get(), std::move(chosen));
    }
  }
  std::optional<CoastalCandidate> classify(int pixel, bool &lake) const {
    if (pixel < 0 || pixel >= map.size()) return std::nullopt;
    int sea = -1;
    for (int neighbor : Fwg::Utils::Math::getNeighbours(pixel, map.width(), map.height())) {
      const auto &province = areas.provinceColourMap.at(map[neighbor]);
      lake |= province->isLake();
      if (province->isSea() && sea < 0) sea = province->ID;
    }
    if (sea < 0) return std::nullopt;
    return CoastalCandidate{pixel, sea};
  }
  std::optional<Arda::Utils::Building> building(const std::string &type,
      const Fwg::Areas::Province *province, const std::vector<float> &heightmap) const {
    if (!province) {
      if (eligible.empty()) return std::nullopt;
      province = Fwg::Utils::Random::selectRandom(eligible);
    }
    const auto found = candidates.find(province);
    if (found == candidates.end()) return std::nullopt;
    auto candidate = Fwg::Utils::Random::selectRandom(found->second);
    if (type == "naval_base_spawn") {
      for (const auto &location : province->locations) {
        if (location->type != Fwg::Civilization::LocationType::Port) continue;
        const int pixel = location->portLocationPixel;
        if (pixel < 0 || pixel >= map.size() ||
            areas.provinceColourMap.at(map[pixel]).get() != province) continue;
        bool lake = false;
        if (auto port = classify(pixel, lake)) {
          candidate = *port;
          // Prefer the port's recorded sea exit when it is directly adjacent.
          for (int n : Fwg::Utils::Math::getNeighbours(pixel, map.width(), map.height())) {
            const auto &sea = areas.provinceColourMap.at(map[n]);
            if (sea->isSea() && sea->ID == location->portExitProvinceID)
              candidate.seaProvinceId = sea->ID;
          }
          break;
        }
      }
    }
    Arda::Utils::Building result;
    result.name = type;
    result.position = Arda::Utils::Coordinate{candidate.pixel % map.width(),
        candidate.pixel / map.width(), heightmap[candidate.pixel] / 10.0, -1.57};
    result.provinceID = province->ID;
    result.relativeID = type == "naval_base_spawn" ? candidate.seaProvinceId : 0;
    return result;
  }
};
} // namespace
Arda::Utils::Building
getBuilding(const std::string &type,
            const std::vector<std::shared_ptr<Fwg::Areas::Province>> &provinces,
            const bool coastal, const std::vector<float> &heightmap,
            int relativeID = 0) {
  auto pix = 0;
  Arda::Utils::Building building;
  auto prov = Fwg::Utils::Random::selectRandom(provinces);
  if (coastal) {
    while (!prov->isCoastalToOcean())
      prov = Fwg::Utils::Random::selectRandom(provinces);
    pix = Fwg::Utils::Random::selectRandom(prov->coastalPixels);
  } else {
    while (prov->isLake())
      prov = Fwg::Utils::Random::selectRandom(provinces);
    pix = Fwg::Utils::Random::selectRandom(prov->pixels);
  }
  auto widthPos = pix % Fwg::Cfg::Values().width;
  auto heightPos = pix / Fwg::Cfg::Values().width;
  building.name = type;
  building.position = Arda::Utils::Coordinate{widthPos, heightPos,
                                              heightmap[pix] / 10.0, -1.57};
  building.relativeID = relativeID;
  building.provinceID = prov->ID;
  return building;
}

void Region::calculateBuildingPositions(const std::vector<float> &heightmap,
                                        const Fwg::Gfx::Image &provinceMap,
                                        const Fwg::Areas::AreaData &areaData) {
  if (!this->isLand())
    return;
  if (provinceMap.width() != Fwg::Cfg::Values().width ||
      provinceMap.height() != Fwg::Cfg::Values().height ||
      provinceMap.size() != heightmap.size())
    throw std::invalid_argument("Building geography dimensions do not match");
  CoastalCandidates coast(provinces, provinceMap, areaData);
  auto addCoastal = [&](const std::string &type, const Fwg::Areas::Province *province) {
    if (auto building = coast.building(type, province, heightmap)) buildings.push_back(*building);
  };
  buildings.clear();
  for (const auto &prov : provinces) {
    // add supply node buildings for each province
    if (isCoastalToOcean()) addCoastal("supply_node", nullptr);
    else buildings.push_back(getBuilding("supply_node", this->provinces, false, heightmap));
  }
  for (const auto &type : buildingTypes) {
    if (type == "arms_factory" || type == "industrial_complex") {
      for (auto i = 0; i < 6; i++) {
        buildings.push_back(
            getBuilding(type, this->provinces, false, heightmap));
      }
    } else if (type == "bunker") {
      for (const auto &prov : provinces) {
        if (!prov->isLake() && !prov->isSea()) {
          buildings.push_back(
              getBuilding(type, this->provinces, false, heightmap));
        }
      }
    } else if (type == "special_project_facility_spawn") {
      for (const auto &prov : provinces) {
        if (!prov->isLake() && !prov->isSea()) {
          buildings.push_back(
              getBuilding(type, this->provinces, false, heightmap));
        }
      }
    } else if (type == "anti_air_building") {
      for (auto i = 0; i < 3; i++)
        buildings.push_back(
            getBuilding(type, this->provinces, false, heightmap));
    } else if (type == "coastal_bunker" || type == "naval_base_spawn") {
      for (const auto &prov : provinces)
        if (prov->isCoastalToOcean()) addCoastal(type, prov.get());
    } else if (type == "dockyard" || type == "floating_harbor") {
      if (isCoastalToOcean()) addCoastal(type, nullptr);
    } else {
      buildings.push_back(getBuilding(type, this->provinces, false, heightmap));
    }
  }
}

template<class Archive>
void Region::serialize(Archive &ar, const unsigned int /*version*/) {
  ar & boost::serialization::base_object<Arda::ArdaRegion>(*this);
  ar & identifier;
  ar & armsFactories & civilianFactories & dockyards;
  ar & infrastructure & stateCategory & stratID;
  ar & airport & rocketsite & supplyNode;
  ar & totalVictoryPoints;
  ar & weatherPosition;
  ar & buildings;
  ar & victoryPointsMap;
  ar & navalBases;
  ar & airBase;
}

} // namespace Rpx::Hoi4

BOOST_CLASS_EXPORT_IMPLEMENT(Rpx::Hoi4::Region)
template void Rpx::Hoi4::Region::serialize(boost::archive::binary_oarchive&, unsigned int);
template void Rpx::Hoi4::Region::serialize(boost::archive::binary_iarchive&, unsigned int);

