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
              !gameRegions[nextRegion]->sea) {
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
  region->owner = tag;
  for (auto &gameProvince : region->gameProvinces)
    gameProvince->owner = tag;
  ownedRegions.push_back(region);
}
} // namespace Scenario