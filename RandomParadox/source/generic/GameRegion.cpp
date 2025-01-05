#include "generic/GameRegion.h"
namespace Scenario {
Region::Region() {}

Region::Region(const Fwg::Region &baseRegion)
    : Fwg::Region(baseRegion), assigned(false), totalPopulation{-1} {}

Region::~Region() {}
void Region::sumPopulations() {
  double totalShare = static_cast<double>(gameProvinces.size());
  for (auto &religion : religions) {
    // if we don't find the religion, add it to the map
    if (religions.find(religion.first) == religions.end()) {
      religions[religion.first] = 0.0;
    }
    // now add the share found in this province, but divide it by the amount
    // of provinces
    religions[religion.first] += religion.second / totalShare;
  }

  for (auto &culture : cultures) {
    // if we don't find the religion, add it to the map
    if (cultures.find(culture.first) == cultures.end()) {
      cultures[culture.first] = 0.0;
    }
    // now add the share found in this province, but divide it by the amount
    // of provinces
    cultures[culture.first] += culture.second / totalShare;
  }
}
void Region::findLocator(Fwg::Civilization::LocationType locationType,
                         int maxAmount) {
  std::shared_ptr<Fwg::Civilization::Location> addLocation;
  int maxPixels = 0;
  for (auto &location : locations) {
    if (location->type == locationType) {
      if (location->pixels.size() > maxPixels) {
        // locator.xPos = location->position.widthCenter;
        // locator.yPos = location->position.heightCenter;
        maxPixels = location->pixels.size();
        addLocation = location;
      }
    }
  }
  // add to significant locations
  if (maxPixels > 0) {
    significantLocations.push_back(addLocation);
  }
}

void Region::findPortLocator(int maxAmount) {
  findLocator(Fwg::Civilization::LocationType::Port, maxAmount);
}

void Region::findCityLocator(int maxAmount) {
  findLocator(Fwg::Civilization::LocationType::City, maxAmount);
}
void Region::findMineLocator(int maxAmount) {
  findLocator(Fwg::Civilization::LocationType::Mine, maxAmount);
}

void Region::findFarmLocator(int maxAmount) {
  findLocator(Fwg::Civilization::LocationType::Farm, maxAmount);
}

void Region::findWoodLocator(int maxAmount) {
  findLocator(Fwg::Civilization::LocationType::Forest, maxAmount);
}
void Region::findWaterLocator(int maxAmount) {
  findLocator(Fwg::Civilization::LocationType::WaterNode, maxAmount);
}
void Region::findWaterPortLocator(int maxAmount) {
  findLocator(Fwg::Civilization::LocationType::WaterPort, maxAmount);
}
std::shared_ptr<Fwg::Civilization::Location>
Region::getLocation(Fwg::Civilization::LocationType type) {
  for (auto &location : locations) {
    if (location->type == type) {
      return location;
    }
  }
  return nullptr;
}
} // namespace Scenario