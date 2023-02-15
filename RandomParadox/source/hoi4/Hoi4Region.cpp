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

Scenario::Utils::Building
getBuilding(const std::string &type, const Fwg::Province &prov,
            const bool coastal, const Fwg::Gfx::Bitmap &heightmap,
            const Fwg::Gfx::Bitmap &typeMap, int relativeID = 0) {
  auto pix = 0;
  auto &cfg = Fwg::Cfg::Values();
  Scenario::Utils::Building building;
  bool done = false;
  int coastalAttempts = 0;
  if (coastal) {
    while (!done) {
      done = true;
      coastalAttempts++;
      pix = Fwg::Utils::selectRandom(prov.coastalPixels);
      std::vector<int> neighbourPix = {pix - 1, pix + 1,
                                       pix + Fwg::Cfg::Values().width,
                                       pix - Fwg::Cfg::Values().width};
      for (auto nPix : neighbourPix) {
        if (nPix > 0 && nPix < typeMap.imageData.size())
          if (typeMap[nPix] == cfg.colours.at("lake")) {
            done = false;
          }
        // we can't find a perfect coast
        if (coastalAttempts > 50) {
          // iterate through all coastal pixels
          for (auto cPix : prov.coastalPixels) {
            // get neighbours
            neighbourPix = {cPix - 1, cPix + 1, cPix + Fwg::Cfg::Values().width,
                            cPix - Fwg::Cfg::Values().width};
            // check every neighbour for contact to ocean
            for (auto nPix : neighbourPix) {
              // if in range of image
              if (nPix >= 0 && nPix < typeMap.imageData.size())
                // if it is sea
                if (typeMap[nPix] == cfg.colours.at("sea")) {
                  pix = nPix;
                  done = true;

                }
            }
          }
        }
      }
    }
  } else {
    pix = Fwg::Utils::selectRandom(prov.pixels);
  }
  auto widthPos = pix % Fwg::Cfg::Values().width;
  auto heightPos = pix / Fwg::Cfg::Values().width;

  building.name = type;
  building.position = Scenario::Utils::Coordinate{
      widthPos, heightPos, (double)heightmap[pix].getRed() / 10.0, -1.57};
  building.relativeID = relativeID;
  return building;
}

Scenario::Utils::Building getBuilding(const std::string &type,
                                      const Fwg::Region &region,
                                      const bool coastal,
                                      const Fwg::Gfx::Bitmap &heightmap,
                                      int relativeID = 0) {
  auto pix = 0;
  Scenario::Utils::Building building;
  auto prov = Fwg::Utils::selectRandom(region.provinces);
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
  building.position = Scenario::Utils::Coordinate{
      widthPos, heightPos, (double)heightmap[pix].getRed() / 10.0, -1.57};
  building.relativeID = relativeID;
  return building;
}

void Region::calculateBuildingPositions(const Fwg::Gfx::Bitmap &heightmap,
                                        const Fwg::Gfx::Bitmap &typeMap) {
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
    } else if (type == "bunker") {
      for (const auto &prov : provinces) {
        if (!prov->isLake && !prov->sea) {
          buildings.push_back(getBuilding(type, *this, false, heightmap));
        }
      }
    } else if (type == "anti_air_building") {
      for (auto i = 0; i < 3; i++)
        buildings.push_back(getBuilding(type, *this, false, heightmap));
    } else if (type == "coastal_bunker" || type == "naval_base") {
      for (const auto &prov : provinces) {
        if (prov->coastal) {
          auto pix = Fwg::Utils::selectRandom(prov->coastalPixels);
          int ID = 0;
          if (type == "naval_base") {
            // find the ocean province this coastal building is next to
            for (const auto &neighbour : prov->neighbours)
              if (neighbour->sea && !neighbour->isLake)
                for (const auto &provPix : neighbour->pixels)
                  if (Fwg::Utils::getDistance(provPix, pix,
                                              Fwg::Cfg::Values().width) < 2.0)
                    ID = neighbour->ID;
          }
          buildings.push_back(
              getBuilding(type, *prov, true, heightmap, typeMap, ID));
        }
      }
    } else if (coastal && (type == "dockyard" || type == "floating_harbor")) {
      auto prov = Fwg::Utils::selectRandom(provinces);
      while (!prov->coastal)
        prov = Fwg::Utils::selectRandom(provinces);
      buildings.push_back(
          getBuilding(type, *prov, coastal, heightmap, typeMap));
    } else {
      buildings.push_back(getBuilding(type, *this, false, heightmap));
    }
  }
}
} // namespace Scenario::Hoi4