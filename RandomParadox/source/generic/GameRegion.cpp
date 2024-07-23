#include "generic/GameRegion.h"
namespace Scenario {
Region::Region() {}

Region::Region(const Fwg::Region &baseRegion)
    : Fwg::Region(baseRegion), assigned(false), totalPopulation{-1} {}

Region::~Region() {}
void Region::sumPopulations() {
  double totalShare = static_cast<double>(gameProvinces.size());
  for (const auto &prov : gameProvinces) {
    for (auto &religion : prov->religions) {
      // if we don't find the religion, add it to the map
      if (religions.find(religion.first) == religions.end()) {
        religions[religion.first] = 0.0;
      }
      // now add the share found in this province, but divide it by the amount
      // of provinces
      religions[religion.first] += religion.second / totalShare;
    }
  }

  for (const auto &prov : gameProvinces) {
    for (auto &culture : prov->cultures) {
      // if we don't find the religion, add it to the map
      if (cultures.find(culture.first) == cultures.end()) {
        cultures[culture.first] = 0.0;
      }
      // now add the share found in this province, but divide it by the amount
      // of provinces
      cultures[culture.first] += culture.second / totalShare;
    }
  }
}
void Region::findPortLocator(int maxAmount) {
  for (auto &province : provinces) {
    if (province->coastal) {
      Locator locator;
      locator.type = LocatorType::PORT;
      locator.xPos = province->position.widthCenter;
      locator.yPos = province->position.heightCenter;
      locators.insert({locator.type, locator});
      break;
    }
  }
}
void Region::findCityLocator(int maxAmount) {
  auto &cfg = Fwg::Cfg::Values();
  Locator cityLocator;
  cityLocator.type = LocatorType::CITY;
  int usedProv;
  int cityPixels = 0;

  // find province with most amount of citypixels
  for (auto &province : provinces) {
    auto &provCityPixels = province->cityPixels;
    if (provCityPixels.size() > cityPixels) {
      cityLocator.xPos = provCityPixels[0] % cfg.width;
      cityLocator.yPos = provCityPixels[0] / cfg.width;
      usedProv = province->ID;
    }
  }
  // if we didn't find any suitable city province, take a random locator
  if (!locators.size()) {
    cityLocator.xPos = provinces[0]->position.widthCenter;
    cityLocator.yPos = provinces[0]->position.heightCenter;
    usedProv = provinces[0]->ID;
  }
  locators.insert({cityLocator.type, cityLocator});
}
void Region::findMineLocator(int maxAmount) {
  Locator locator;
  locator.type = LocatorType::MINE;
  auto &randProv = Fwg::Utils::selectRandom(provinces);
  locator.xPos = randProv->position.widthCenter;
  locator.yPos = randProv->position.heightCenter;
  locators.insert({locator.type, locator});
}
void Region::findFarmLocator(int maxAmount) {
  Locator locator;
  locator.type = LocatorType::FARM;
  auto &randProv = Fwg::Utils::selectRandom(provinces);
  locator.xPos = randProv->position.widthCenter;
  locator.yPos = randProv->position.heightCenter;
  locators.insert({locator.type, locator});
}
void Region::findWoodLocator(int maxAmount) {
  Locator locator;
  locator.type = LocatorType::WOOD;
  auto &randProv = Fwg::Utils::selectRandom(provinces);
  locator.xPos = randProv->position.widthCenter;
  locator.yPos = randProv->position.heightCenter;
  locators.insert({locator.type, locator});
}
} // namespace Scenario