#include "hoi4/Hoi4Region.h"

namespace Scenario::Hoi4 {
Region::Region() {}

Region::Region(const Scenario::Region &gameRegion)
    : Scenario::Region(gameRegion), armsFactories{0}, civilianFactories{0},
      development{0}, dockyards{0}, population{0}, stateCategory{0}, stratID{
                                                                         0} {
  resources = {{"aluminium", 0}, {"chromium", 0}, {"oil", 0},
               {"rubber", 0},    {"steel", 0},    {"tungsten", 0}};
}

Region::~Region() {}

Scenario::Utils::Building getBuilding(const std::string &type,
                                      const Fwg::Region &region,
                                      const bool coastal,
                                      const Fwg::Gfx::Bitmap &heightmap,
                                      int relativeID = 0) {

  auto prov = Fwg::Utils::selectRandom(region.provinces);
  auto pix = 0;
  Scenario::Utils::Building building;
  if (coastal) {
    while (!prov->coastal)
      prov = Fwg::Utils::selectRandom(region.provinces);
    pix = Fwg::Utils::selectRandom(prov->coastalPixels);
  } else {
    while (prov->isLake)
      prov = Fwg::Utils::selectRandom(region.provinces);
    pix = Fwg::Utils::selectRandom(prov->pixels);
  }
  auto widthPos = pix % Fwg::Cfg::Values().width;
  auto heightPos = pix / Fwg::Cfg::Values().width;
  building.name = type;
  building.position = Scenario::Utils::Position{
      widthPos, heightPos, (double)heightmap[pix].getRed() / 10.0, -1.57};
  building.relativeID = relativeID;
  return building;
}

void Region::calculateBuildingPositions(const Fwg::Gfx::Bitmap &heightmap) {
  if (this->sea)
    return;
  bool coastal = false;
  for (const auto &prov : provinces) {
    if (prov->coastal)
      coastal = true;
    // add supply node buildings for each province
    buildings.push_back(getBuilding("supply_node", *this, coastal, heightmap));
  }
  for (const auto &type : buildingTypes) {
    if (type == "arms_factory" || type == "industrial_complex") {
      for (auto i = 0; i < 6; i++) {
        buildings.push_back(getBuilding(type, *this, false, heightmap));
      }
    }
    else if (type == "bunker") {
      for (const auto &prov : provinces) {
        if (!prov->isLake && !prov->sea) {
          buildings.push_back(getBuilding(type, *this, false, heightmap));
        }
      }
    }
    else if (type == "anti_air_building") {
      for (auto i = 0; i < 3; i++)
        buildings.push_back(getBuilding(type, *this, false, heightmap));
    }
    else if (type == "coastal_bunker" || type == "naval_base") {
      for (const auto &prov : provinces) {
        if (prov->coastal) {
          auto pix = Fwg::Utils::selectRandom(prov->coastalPixels);
          int ID = 0;
          if (type == "naval_base") {
            // find the ocean province this coastal building is next to
            for (const auto &neighbour : prov->neighbours)
              if (neighbour->sea)
                for (const auto &provPix : neighbour->pixels)
                  if (Fwg::Utils::getDistance(provPix, pix,
                                              Fwg::Cfg::Values().width) < 2.0)
                    ID = neighbour->ID;
          }
          buildings.push_back(getBuilding(type, *this, false, heightmap, ID));
        }
      }
    }
    else if (type == "dockyard" || type == "floating_harbor") {
      buildings.push_back(getBuilding(type, *this, coastal, heightmap));
    } else {
      buildings.push_back(getBuilding(type, *this, false, heightmap));
    }
  }
}
} // namespace Scenario::Hoi4