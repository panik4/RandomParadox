#include "generic/PdoxCountry.h"
namespace Scenario {
PdoxCountry::PdoxCountry() : ID{-1} {}

PdoxCountry::PdoxCountry(std::string tag, int ID, std::string name,
                         std::string adjective, Gfx::Flag flag)
    : ID{ID}, tag{tag}, name{name}, adjective{adjective}, flag{flag} {
  colour = {static_cast<unsigned char>(RandNum::getRandom(0, 255)),
            static_cast<unsigned char>(RandNum::getRandom(0, 255)),
            static_cast<unsigned char>(RandNum::getRandom(0, 255))};
}

PdoxCountry::~PdoxCountry() {}

void PdoxCountry::assignRegions(
    int maxRegions, std::vector<std::shared_ptr<Region>> &gameRegions,
    std::shared_ptr<Region> startRegion,
    std::vector <std::shared_ptr<GameProvince>> & gameProvinces) {
  addRegion(startRegion, gameRegions, gameProvinces);
  auto breakCounter = 0;
  while (ownedRegions.size() < maxRegions && breakCounter++ < 100) {
    for (const auto &gameRegion : ownedRegions) {
      if (ownedRegions.size() >= maxRegions)
        break;
      if (gameRegions[gameRegion]->neighbours.size() == 0)
        continue;
      auto &nextRegion =
          Fwg::Utils::selectRandom(gameRegions[gameRegion]->neighbours);
      if (!gameRegions[nextRegion]->assigned && !gameRegions[nextRegion]->sea) {
        gameRegions[nextRegion]->assigned = true;
        addRegion(gameRegions[nextRegion], gameRegions, gameProvinces);
      }
    }
  }
}

void PdoxCountry::addRegion(std::shared_ptr<Region> region,
                            std::vector<std::shared_ptr<Region>> &gameRegions,
                            std::vector < std::shared_ptr<GameProvince>> &
                                gameProvinces) {
  gameRegions[region->ID]->assigned = true;
  gameRegions[region->ID]->owner = tag;
  for (auto &gameProvince : gameRegions[region->ID]->gameProvinces)
    gameProvince->owner = tag;
  region->assigned = true;
  region->owner = tag;
  for (auto &gameProvince : region->gameProvinces)
    gameProvince->owner = tag;
  ownedRegions.push_back(region->ID);
}
} // namespace Scenario