#include "generic/Country.h"
namespace Scenario {
Country::Country() : ID{-1} {}

Country::Country(std::string tag, int ID, std::string name,
                 std::string adjective, Gfx::Flag flag)
    : ID{ID}, tag{tag}, name{name}, adjective{adjective}, flag{flag} {
  colour = {static_cast<unsigned char>(RandNum::getRandom(0, 255)),
            static_cast<unsigned char>(RandNum::getRandom(0, 255)),
            static_cast<unsigned char>(RandNum::getRandom(0, 255))};
  this->flag.flip();
}

void Country::assignRegions(
    int maxRegions, std::vector<std::shared_ptr<Region>> &gameRegions,
    std::shared_ptr<Region> startRegion,
    std::vector<std::shared_ptr<GameProvince>> &gameProvinces) {
  addRegion(startRegion);
  auto breakCounter = 0;
  while (ownedRegions.size() < maxRegions && breakCounter++ < 100) {
    for (const auto &gameRegion : ownedRegions) {
      if (ownedRegions.size() >= maxRegions)
        break;
      if (gameRegion == nullptr)
        continue;
      if (gameRegion->neighbours.size() == 0)
        continue;
      if (gameRegion->neighbours.size()) {
        auto &nextRegion = Fwg::Utils::selectRandom(gameRegion->neighbours);
        if (nextRegion < gameRegions.size()) {
          if (!gameRegions[nextRegion]->assigned &&
              !gameRegions[nextRegion]->sea && !gameRegions[nextRegion]->lake) {
            gameRegions[nextRegion]->assigned = true;
            addRegion(gameRegions[nextRegion]);
          }
        }
      }
    }
  }
}

void Country::addRegion(std::shared_ptr<Region> region) {
  region->assigned = true;
  for (auto &gameProvince : region->gameProvinces)
    gameProvince->owner = this->tag;
  ownedRegions.push_back(region);
}
void Country::removeRegion(std::shared_ptr<Region> region) {
  region->assigned = false;
  // region->owner = "";
  for (auto &gameProvince : region->gameProvinces)
    gameProvince->owner = "";
  if (this != nullptr && ownedRegions.size()) {
    ownedRegions.erase(
        std::remove(ownedRegions.begin(), ownedRegions.end(), region),
        ownedRegions.end());
  }
}
void Country::selectCapital() {
  // select the region with the highest population
  double max = 0;
  std::shared_ptr<Region> capitalRegion;
  if (ownedRegions.empty()) {
    Fwg::Utils::Logging::logLine("No regions found for country " + name);
    return;
  }
  for (const auto &region : ownedRegions) {
    if (region->populationFactor > max) {
      max = region->populationFactor;
      capitalRegionID = region->ID;
      capitalRegion = region;
    }
  }
  // if none found, take only region
  if (capitalRegion == nullptr) {
    capitalRegion = ownedRegions[0];
    capitalRegionID = capitalRegion->ID;
  }
  // in this region, select the single most significant location
  max = 0;
  for (const auto &location : capitalRegion->locations) {
    if (location->importance > max) {
      max = location->importance;
      capitalProvinceID = location->provinceID;
    }
  }
}
// gathers all provinces from the regions
void Country::evaluateProvinces() {
  ownedProvinces.clear();
  for (const auto &region : ownedRegions) {
    for (const auto &gameProvince : region->gameProvinces) {
      ownedProvinces.push_back(gameProvince);
    }
  }
  // sort the provinces by ID
  std::sort(
      ownedProvinces.begin(), ownedProvinces.end(),
      [](const std::shared_ptr<GameProvince> &a,
         const std::shared_ptr<GameProvince> &b) { return a->ID < b->ID; });
}
void Country::evaluatePopulations(const double worldPopulationFactor) {
  // gather all population factors of the regions
  populationFactor = 0.0;
  for (const auto &region : ownedRegions) {
    populationFactor += region->populationFactor;
  }
  worldPopulationShare = populationFactor / worldPopulationFactor;
}

void Country::evaluateDevelopment() {
  for (auto &state : this->ownedRegions) {
    // development should be weighed by the pop in the state
    averageDevelopment +=
        state->developmentFactor *
        (state->worldPopulationShare / this->worldPopulationShare);
  }
  this->averageDevelopment = averageDevelopment;
}

void Country::evaluateEconomicActivity(const double worldEconomicActivity) {
  double economicActivity = 0.0;
  for (const auto &region : ownedRegions) {
    economicActivity += region->economicActivity;
  }
  worldEconomicActivityShare = economicActivity / worldEconomicActivity;
}

void Country::evaluateProperties() {
  // first check if we are landlocked
  landlocked = true;
  for (const auto &region : ownedRegions) {
    if (region->coastal) {
      landlocked = false;
      break;
    }
  }
}

void Country::gatherCultureShares() {
  cultures.clear();
  for (const auto &region : ownedRegions) {
    for (const auto &culture : region->cultureShares) {
      if (cultures.find(culture.first) == cultures.end())
        cultures[culture.first] = 0;
      cultures[culture.first] += culture.second * region->populationFactor;
    }
  }
}

std::shared_ptr<Culture> Scenario::Country::getPrimaryCulture() const {
  // return the largest culture in the country, by evaluating all gameregions
  // according to their population multiplied with the share of the culture in
  // the region
  double max = -1.0;
  std::shared_ptr<Culture> primaryCulture;
  for (const auto &culture : cultures) {
    if (culture.second >= max) {
      max = culture.second;
      primaryCulture = culture.first;
    }
  }
  return primaryCulture;
}
} // namespace Scenario