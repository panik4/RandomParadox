#include "hoi4/Hoi4Region.h"
#include "hoi4/Hoi4SupplyGen.h"
#include <optional>
#include <stdexcept>
#include <unordered_map>

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
    Fwg::Utils::Logging::logLine("Invalid HOI4 state mapping data: ", e.what());
  }
}

namespace {
struct CoastalCandidate {
  int pixel;
  int seaProvinceId;
};
// Built once per region: expected O(P + C) time and O(P + C) temporary
// storage for P provinces and C coastal pixels. Each pixel checks at most
// four neighbours using the existing province image and colour hash table.
// Hash lookups are expected O(1), worst-case O(N) for N world provinces.
class CoastalCandidates {
  const Fwg::Gfx::Image &map;
  const Fwg::Areas::AreaData &areas;
  const std::vector<ResolvedPort> &ports;
  std::unordered_map<const Fwg::Areas::Province *,
                     std::vector<CoastalCandidate>>
      candidates;

public:
  std::vector<const Fwg::Areas::Province *> eligible;
  CoastalCandidates(
      const std::vector<std::shared_ptr<Fwg::Areas::Province>> &provinces,
      const Fwg::Gfx::Image &map, const Fwg::Areas::AreaData &areas,
      const std::vector<ResolvedPort> &ports)
      : map(map), areas(areas), ports(ports) {
    for (const auto &province : provinces) {
      if (!province->isLand())
        continue;
      if (!province->isCoastalToOcean() && province->coastalPixels.empty() &&
          std::none_of(ports.begin(), ports.end(), [&](const auto &port) {
            return port.provinceID == province->ID;
          }))
        continue;
      std::vector<CoastalCandidate> preferred, fallback;
      std::vector<int> pixels(province->coastalPixels.begin(),
                              province->coastalPixels.end());
      for (const auto &port : ports)
        if (port.provinceID == province->ID)
          pixels.push_back(port.pixel);
      if (pixels.empty()) {
        const auto view = province->getNonOwningPixelView();
        pixels.assign(view.begin(), view.end());
      }
      std::sort(pixels.begin(), pixels.end());
      pixels.erase(std::unique(pixels.begin(), pixels.end()), pixels.end());
      for (int pixel : pixels) {
        if (pixel < 0 || pixel >= map.size() ||
            areas.provinceColourMap.at(map[pixel]).get() != province.get())
          continue;
        bool lake = false;
        auto candidate = classify(pixel, lake);
        if (!candidate)
          continue;
        fallback.push_back(*candidate);
        if (!lake)
          preferred.push_back(*candidate);
      }
      auto chosen =
          preferred.empty() ? std::move(fallback) : std::move(preferred);
      if (chosen.empty()) {
        Fwg::Utils::Logging::logLine(
            "No ocean-adjacent building candidates for province ",
            province->ID);
        continue;
      }
      eligible.push_back(province.get());
      candidates.emplace(province.get(), std::move(chosen));
    }
  }
  std::vector<int> neighbors(int pixel) const {
    const int x = pixel % map.width(), y = pixel / map.width();
    std::vector<int> result{y * map.width() + (x + 1) % map.width(),
                            y * map.width() +
                                (x + map.width() - 1) % map.width()};
    if (y > 0)
      result.push_back(pixel - map.width());
    if (y + 1 < map.height())
      result.push_back(pixel + map.width());
    return result;
  }
  std::optional<CoastalCandidate> classify(int pixel, bool &lake) const {
    if (pixel < 0 || pixel >= map.size())
      return std::nullopt;
    int sea = -1;
    for (int neighbor : neighbors(pixel)) {
      const auto &province = areas.provinceColourMap.at(map[neighbor]);
      lake |= province->isLake();
      if (province->isSea() && sea < 0)
        sea = province->ID;
    }
    if (sea < 0)
      return std::nullopt;
    return CoastalCandidate{pixel, sea};
  }
  std::vector<Arda::Utils::Building>
  options(const std::string &type, const Fwg::Areas::Province *restriction,
          const std::vector<float> &heightmap) const {
    std::vector<Arda::Utils::Building> result;
    for (const auto *province : eligible) {
      if (restriction && restriction != province)
        continue;
      auto available = candidates.at(province);
      if (type == "naval_base_spawn") {
        std::optional<CoastalCandidate> anchor;
        double importance = -std::numeric_limits<double>::max();
        int locationId = std::numeric_limits<int>::max();
        for (const auto &port : ports) {
          if (port.provinceID != province->ID)
            continue;
          if (port.importance > importance ||
              (port.importance == importance && port.locationID < locationId)) {
            anchor = CoastalCandidate{port.pixel, port.seaProvinceID};
            importance = port.importance;
            locationId = port.locationID;
          }
        }
        if (anchor)
          available = {*anchor};
      }
      for (const auto &candidate : available) {
        double east = 0, north = 0;
        int fallback = -1;
        for (int n : neighbors(candidate.pixel)) {
          const auto other = areas.provinceColourMap.at(map[n]);
          if (!other->isSea())
            continue;
          if (fallback < 0)
            fallback = n;
          double dx = n % map.width() - candidate.pixel % map.width();
          if (dx > map.width() / 2)
            dx -= map.width();
          if (dx < -map.width() / 2)
            dx += map.width();
          east += dx;
          north += n / map.width() - candidate.pixel / map.width();
          if (type == "naval_base_spawn" &&
              other->ID == candidate.seaProvinceId) {
            east = dx;
            north = n / map.width() - candidate.pixel / map.width();
            break;
          }
        }
        if (east == 0 && north == 0 && fallback >= 0) {
          east = std::remainder(
              double(fallback % map.width() - candidate.pixel % map.width()),
              double(map.width()));
          north = fallback / map.width() - candidate.pixel / map.width();
        }
        result.push_back(
            {type,
             {candidate.pixel % map.width(), candidate.pixel / map.width(),
              heightmap[candidate.pixel] / 10.0, buildingRotation(east, north)},
             type == "naval_base_spawn" ? candidate.seaProvinceId : 0,
             province->ID});
      }
    }
    return result;
  }
};
} // namespace
void Region::calculateBuildingPositions(
    const Fwg::Terrain::TerrainData &terrain,
    const Fwg::Gfx::Image &provinceMap, const Fwg::Areas::AreaData &areaData) {
  const auto &heightmap = terrain.detailedHeightMap;
  if (!terrain.inclination.empty() &&
      terrain.inclination.size() != heightmap.size())
    throw std::invalid_argument("Building inclination dimensions do not match");
  if (!this->isLand())
    return;
  if (provinceMap.width() != Fwg::Cfg::Values().width ||
      provinceMap.height() != Fwg::Cfg::Values().height ||
      provinceMap.size() != heightmap.size())
    throw std::invalid_argument("Building geography dimensions do not match");
  // Standalone placement callers also resolve ports; state generation supplies
  // the complete cross-state registry before reaching this method.
  if (!portsResolved) {
    resolvedPorts.clear();
    auto sourceLocations = locations;
    for (const auto &province : provinces)
      sourceLocations.insert(sourceLocations.end(), province->locations.begin(),
                             province->locations.end());
    std::set<int> seen;
    for (const auto &location : sourceLocations) {
      if (location->type != Fwg::Civilization::LocationType::Port &&
          location->secondaryType != Fwg::Civilization::LocationType::Port)
        continue;
      if (!seen.insert(location->ID).second)
        continue;
      if (auto port = resolvePort(*location, provinceMap, areaData, heightmap);
          port && port->stateID == ID)
        resolvedPorts.push_back(*port);
    }
  }
  CoastalCandidates coast(provinces, provinceMap, areaData, resolvedPorts);
  std::vector<Arda::Utils::Building> placements;

  struct Candidate {
    int pixel;
    int province;
    double weight;
    double cityDistance;
  };
  std::unordered_map<int, Candidate> cachedCandidates;
  std::vector<int> cityPixels;
  std::unordered_map<int, double> provinceWeights;
  size_t area = 0;
  for (const auto &province : provinces)
    if (province->isLand())
      area += province->size();
  for (const auto &location : locations) {
    if (location->type != Fwg::Civilization::LocationType::City &&
        location->secondaryType != Fwg::Civilization::LocationType::City)
      continue;
    const auto pixels = location->getNonOwningPixelView();
    if (pixels.empty())
      cityPixels.push_back(location->position.weightedCenter);
    else
      cityPixels.insert(cityPixels.end(), pixels.begin(), pixels.end());
  }
  auto distance = [&](int a, int b) {
    double dx = std::abs(a % provinceMap.width() - b % provinceMap.width());
    dx = std::min(dx, provinceMap.width() - dx);
    const double dy = a / provinceMap.width() - b / provinceMap.width();
    return std::sqrt(dx * dx + dy * dy);
  };

  for (const auto &province : provinces) {
    if (!province->isLand())
      continue;
    if (!std::isfinite(province->averageDevelopment) ||
        province->averageDevelopment < 0)
      throw std::invalid_argument("Invalid province development");
    provinceWeights[province->ID] =
        province->size() * (0.25 + std::max(0.0, province->averageDevelopment));
    std::vector<double> slopes;
    for (int pixel : province->getNonOwningPixelView()) {
      if (pixel < 0 || pixel >= heightmap.size())
        throw std::invalid_argument("Invalid building pixel");
      if (!std::isfinite(heightmap[pixel]) ||
          areaData.provinceColourMap.at(provinceMap[pixel]).get() !=
              province.get())
        throw std::invalid_argument("Invalid building geography");
      double slope = 0;
      for (int n : Fwg::Utils::Math::getNeighbours(pixel, provinceMap.width(),
                                                   provinceMap.height())) {
        const auto other = areaData.provinceColourMap.at(provinceMap[n]);
        if (other && other->isLand())
          slope = std::max(slope,
                           double(std::abs(heightmap[pixel] - heightmap[n])));
      }
      if (!terrain.inclination.empty())
        slope = terrain.inclination[pixel];
      if (!std::isfinite(slope) || slope < 0)
        throw std::invalid_argument("Invalid building inclination");
      slopes.push_back(slope);
      double nearest = std::numeric_limits<double>::max();
      cachedCandidates.emplace(pixel,
                               Candidate{pixel, province->ID, slope, nearest});
    }
    if (!slopes.empty()) {
      std::sort(slopes.begin(), slopes.end());
      const double percentile = slopes[(slopes.size() - 1) * 95 / 100];
      for (int pixel : province->getNonOwningPixelView())
        cachedCandidates.at(pixel).weight =
            1.0 / (1 + 4 * cachedCandidates.at(pixel).weight /
                           (percentile > 0 ? percentile : 1));
    }
  }
  // Multi-source city distance over state land, computed once per placement
  // pass.
  std::vector<int> frontier;
  for (int pixel : cityPixels) {
    const auto found = cachedCandidates.find(pixel);
    if (found != cachedCandidates.end() && found->second.cityDistance != 0) {
      found->second.cityDistance = 0;
      frontier.push_back(pixel);
    }
  }
  for (size_t i = 0; i < frontier.size(); ++i) {
    const int pixel = frontier[i];
    const int x = pixel % provinceMap.width(), y = pixel / provinceMap.width();
    for (int dy = -1; dy <= 1; ++dy)
      for (int dx = -1; dx <= 1; ++dx) {
        if ((!dx && !dy) || y + dy < 0 || y + dy >= provinceMap.height())
          continue;
        const int neighbor =
            (y + dy) * provinceMap.width() +
            (x + dx + provinceMap.width()) % provinceMap.width();
        const auto found = cachedCandidates.find(neighbor);
        if (found != cachedCandidates.end() &&
            found->second.cityDistance == std::numeric_limits<double>::max()) {
          found->second.cityDistance =
              cachedCandidates.at(pixel).cityDistance + 1;
          frontier.push_back(neighbor);
        }
      }
  }
  std::set<int> reserved;
  for (const auto &location : selectSupplyLocations(*this)) {
    const int pixel = location->type == Fwg::Civilization::LocationType::Port
                          ? location->portLocationPixel
                          : location->position.weightedCenter;
    if (cachedCandidates.contains(pixel))
      reserved.insert(pixel);
  }
  std::set<int> hubProvinces;
  for (int pixel : reserved)
    hubProvinces.insert(cachedCandidates.at(pixel).province);
  for (const auto *province : coast.eligible) {
    const auto choices = coast.options("naval_base_spawn", province, heightmap);
    if (choices.size() == 1)
      reserved.insert(choices.front().position.z * provinceMap.width() +
                      choices.front().position.x);
  }
  const size_t planned = 22 + 2 * provinceWeights.size() +
                         2 * coast.eligible.size() +
                         (coast.eligible.empty() ? 0 : 2) + hubProvinces.size();
  const double spacing = std::max(1.0, 0.5 * std::sqrt(double(area) / planned));
  const int cellSize = std::max(1, static_cast<int>(std::ceil(spacing)));
  const int columns = (provinceMap.width() + cellSize - 1) / cellSize;
  std::unordered_map<long long, std::vector<int>> occupiedCells;
  std::set<int> occupiedPixels;
  auto occupy = [&](int pixel) {
    if (occupiedPixels.insert(pixel).second)
      occupiedCells[static_cast<long long>(pixel / provinceMap.width() /
                                           cellSize) *
                        columns +
                    pixel % provinceMap.width() / cellSize]
          .push_back(pixel);
  };
  for (int pixel : reserved)
    occupy(pixel);
  auto record = [&](Arda::Utils::Building building) {
    occupy(building.position.z * provinceMap.width() + building.position.x);
    placements.push_back(std::move(building));
  };
  auto clearAt = [&](int pixel, double separation, bool allowReuse) {
    if (!allowReuse && occupiedPixels.contains(pixel))
      return false;
    const int column = pixel % provinceMap.width() / cellSize;
    const int row = pixel / provinceMap.width() / cellSize;
    for (int dy = -1; dy <= 1; ++dy)
      for (int dx = -2; dx <= 2; ++dx) {
        if (row + dy < 0)
          continue;
        const int wrapped = (column + dx % columns + columns) % columns;
        const auto found = occupiedCells.find(
            static_cast<long long>(row + dy) * columns + wrapped);
        if (found != occupiedCells.end())
          for (int other : found->second)
            if (distance(pixel, other) < separation)
              return false;
      }
    return true;
  };
  auto place =
      [&](const std::string &type,
          const std::vector<std::shared_ptr<Fwg::Areas::Province>> &allowed,
          bool, const std::vector<float> &,
          const std::vector<Arda::Utils::Building> *coastalOptions = nullptr) {
        std::vector<const Candidate *> candidates;
        const bool factory =
            type == "arms_factory" || type == "industrial_complex";
        if (coastalOptions) {
          for (const auto &option : *coastalOptions)
            candidates.push_back(&cachedCandidates.at(
                option.position.z * provinceMap.width() + option.position.x));
        } else {
          for (const auto &province : allowed) {
            if (!province->isLand())
              continue;
            for (int pixel : province->getNonOwningPixelView()) {
              if (pixel < 0 || pixel >= heightmap.size())
                throw std::invalid_argument("Invalid building pixel");
              candidates.push_back(&cachedCandidates.at(pixel));
            }
          }
        }
        if (candidates.empty())
          throw std::invalid_argument(
              "No land available for building placement");
        bool allowReuse = false;
        for (double separation = spacing;;
             separation = separation > 1 ? std::max(1.0, separation / 2) : 0) {
          double radius = factory && !cityPixels.empty()
                              ? 2 * spacing
                              : std::numeric_limits<double>::max();
          for (;;) {
            double total = 0;
            std::vector<const Candidate *> eligible;
            for (const auto *candidatePointer : candidates) {
              const auto &candidate = *candidatePointer;
              if (candidate.cityDistance > radius)
                continue;
              const bool clear =
                  clearAt(candidate.pixel, separation, allowReuse);
              if (clear) {
                eligible.push_back(&candidate);
                total += candidate.weight;
              }
            }
            if (!eligible.empty()) {
              std::unordered_map<int, double> flatTotals;
              for (auto candidate : eligible)
                flatTotals[candidate->province] += candidate->weight;
              total = 0;
              for (const auto &[id, sum] : flatTotals)
                total += provinceWeights.at(id);
              double choice = RandNum::getRandom(0.0, total);
              const Candidate *chosen = eligible.back();
              for (auto candidate : eligible)
                if ((choice -= candidate->weight /
                               flatTotals.at(candidate->province) *
                               provinceWeights.at(candidate->province)) <= 0) {
                  chosen = candidate;
                  break;
                }
              if (coastalOptions)
                for (const auto &option : *coastalOptions)
                  if (option.position.z * provinceMap.width() +
                          option.position.x ==
                      chosen->pixel)
                    return option;
              return Arda::Utils::Building{type,
                                           {chosen->pixel % provinceMap.width(),
                                            chosen->pixel / provinceMap.width(),
                                            heightmap[chosen->pixel] / 10.0,
                                            randomBuildingRotation()},
                                           0,
                                           chosen->province};
            }
            if (radius == std::numeric_limits<double>::max())
              break;
            if (radius >= provinceMap.width() + provinceMap.height())
              radius = std::numeric_limits<double>::max();
            else
              radius *= 2;
          }
          if (separation == 0)
            allowReuse = true;
        }
      };
  auto airfield = [&]() {
    Arda::Utils::Building best;
    double bestScore = -1;
    auto evaluate = [&](const Arda::Utils::Building &center) {
      for (const auto [length, width] :
           {std::pair{17, 5}, {9, 3}, {5, 1}, {1, 1}}) {
        for (int axis = 0; axis < 8; ++axis) {
          const double angle = axis * std::numbers::pi / 8;
          const double score = airfieldFootprintScore(
              center.position.x, center.position.z, angle, length, width,
              provinceMap, heightmap,
              [&](int pixel) { return cachedCandidates.contains(pixel); });
          if (score < 0)
            continue;
          if (score > bestScore ||
              (score == bestScore && RandNum::getRandom(0, 1))) {
            best = center;
            bestScore = score;
            const double heading =
                angle + (RandNum::getRandom(0, 1) ? std::numbers::pi : 0);
            best.position.rotation =
                buildingRotation(std::sin(heading), std::cos(heading));
          }
        }
      }
    };
    std::vector<const Candidate *> centers;
    for (double separation = spacing;;
         separation = separation > 1 ? std::max(1.0, separation / 2) : 0) {
      for (const auto &province : provinces)
        if (province->isLand())
          for (int pixel : province->getNonOwningPixelView())
            if (clearAt(pixel, separation, false))
              centers.push_back(&cachedCandidates.at(pixel));
      if (!centers.empty() || separation == 0)
        break;
    }
    if (centers.empty())
      for (const auto &province : provinces)
        if (province->isLand())
          for (int pixel : province->getNonOwningPixelView())
            centers.push_back(&cachedCandidates.at(pixel));
    std::unordered_map<int, double> flatTotals;
    for (const auto *candidate : centers)
      flatTotals[candidate->province] += candidate->weight;
    double total = 0;
    std::vector<double> cumulative;
    for (const auto *candidate : centers) {
      total += candidate->weight / flatTotals.at(candidate->province) *
               provinceWeights.at(candidate->province);
      cumulative.push_back(total);
    }
    for (int i = 0; i < 64 && !centers.empty(); ++i) {
      const auto index =
          std::min(centers.size() - 1,
                   static_cast<size_t>(
                       std::lower_bound(cumulative.begin(), cumulative.end(),
                                        RandNum::getRandom(0.0, total)) -
                       cumulative.begin()));
      const auto &candidate = *centers[index];
      evaluate({"air_base",
                {candidate.pixel % provinceMap.width(),
                 candidate.pixel / provinceMap.width(),
                 heightmap[candidate.pixel] / 10.0, 0},
                0,
                candidate.province});
    }
    if (bestScore < 0)
      for (const auto &[pixel, candidate] : cachedCandidates)
        evaluate({"air_base",
                  {pixel % provinceMap.width(), pixel / provinceMap.width(),
                   heightmap[pixel] / 10.0, 0},
                  0,
                  candidate.province});
    if (bestScore < 0)
      throw std::invalid_argument("No valid airfield site in state " + name);
    airfieldSuitability = bestScore;
    return best;
  };
  auto addCoastal = [&](const std::string &type,
                        const Fwg::Areas::Province *province) {
    auto choices = coast.options(type, province, heightmap);
    if (choices.empty())
      return;
    if (type == "naval_base_spawn" && choices.size() == 1)
      record(choices.front());
    else
      record(place(type, provinces, true, heightmap, &choices));
  };
  for (const auto &type : buildingTypes) {
    if (type == "arms_factory" || type == "industrial_complex") {
      for (auto i = 0; i < 6; i++) {
        record(place(type, this->provinces, false, heightmap));
      }
    } else if (type == "air_base") {
      record(airfield());
    } else if (type == "bunker") {
      for (const auto &prov : provinces) {
        if (!prov->isLake() && !prov->isSea()) {
          record(place(type, {prov}, false, heightmap));
        }
      }
    } else if (type == "special_project_facility_spawn") {
      for (const auto &prov : provinces) {
        if (!prov->isLake() && !prov->isSea()) {
          record(place(type, {prov}, false, heightmap));
        }
      }
    } else if (type == "anti_air_building") {
      for (auto i = 0; i < 3; i++)
        record(place(type, this->provinces, false, heightmap));
    } else if (type == "coastal_bunker" || type == "naval_base_spawn") {
      for (const auto &prov : provinces)
        addCoastal(type, prov.get());
    } else if (type == "dockyard" || type == "floating_harbor") {
      addCoastal(type, nullptr);
    } else {
      record(place(type, this->provinces, false, heightmap));
    }
  }
  buildings = std::move(placements);
}

void Region::calculateBuildingPositions(const std::vector<float> &heightmap,
                                        const Fwg::Gfx::Image &provinceMap,
                                        const Fwg::Areas::AreaData &areaData) {
  Fwg::Terrain::TerrainData terrain;
  terrain.detailedHeightMap = heightmap;
  calculateBuildingPositions(terrain, provinceMap, areaData);
}
template <class Archive>
void Region::serialize(Archive &ar, const unsigned int /*version*/) {
  ar &boost::serialization::base_object<Arda::ArdaRegion>(*this);
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
template void Rpx::Hoi4::Region::serialize(boost::archive::binary_oarchive &,
                                           unsigned int);
template void Rpx::Hoi4::Region::serialize(boost::archive::binary_iarchive &,
                                           unsigned int);
