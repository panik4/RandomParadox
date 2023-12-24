#include "vic3/Vic3Generator.h"
namespace Scenario::Vic3 {
using namespace Fwg;
using namespace Fwg::Gfx;

Generator::Generator() {}

Generator::Generator(const std::string &configSubFolder)
    : Scenario::Generator(configSubFolder) {
  this->terrainTypeToString.at(Fwg::Province::TerrainType::marsh) = "wetlands";
  this->terrainTypeToString.at(Fwg::Province::TerrainType::savannah) =
      "savanna";
  this->terrainTypeToString.at(Fwg::Province::TerrainType::tundra) = "snow";
  this->terrainTypeToString.at(Fwg::Province::TerrainType::arctic) = "snow";
}
//void Generator::generateRegions(std::vector<std::shared_ptr<Region>> &regions) {
//  Utils::Logging::logLine("Vic3: Dividing world into strategic regions");
//  std::set<int> assignedIdeas;
//  for (auto &region : regions) {
//    if (assignedIdeas.find(region->ID) == assignedIdeas.end()) {
//      Vic3StratRegion vicR;
//      // std::set<int>stratRegion;
//      vicR.areaIDs.insert(region->ID);
//      assignedIdeas.insert(region->ID);
//      for (auto &neighbour : region->neighbours) {
//        // should be equal in sea/land
//        if (neighbour > regions.size())
//          continue;
//        if (regions[neighbour]->sea == region->sea &&
//            assignedIdeas.find(neighbour) == assignedIdeas.end()) {
//          vicR.areaIDs.insert(neighbour);
//          assignedIdeas.insert(neighbour);
//        }
//      }
//      vicR.name = NameGeneration::generateName(nData);
//      vic3StratRegions.push_back(vicR);
//    }
//  }
//  Bitmap vic3RegionBmp(Cfg::Values().width, Cfg::Values().height, 24);
//  for (auto &strat : vic3StratRegions) {
//    Colour c{
//        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255),
//        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255),
//        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255)};
//    for (auto &area : strat.areaIDs) {
//      c.setBlue(regions[area]->sea ? 255 : 0);
//      for (auto &prov : regions[area]->gameProvinces) {
//        for (auto &pix : prov->baseProvince->pixels) {
//          vic3RegionBmp.setColourAtIndex(pix, c);
//        }
//      }
//    }
//  }
//  Bmp::bufferBitmap("vic3regions", vic3RegionBmp);
//  Bmp::save(vic3RegionBmp, "Maps//vic3Regions.bmp");
//  Fwg::Gfx::Png::save(vic3RegionBmp, "Maps//vic3Regions.png");
//}
void Generator::distributePops() {
  int landStates = 0;
  for (auto &region : vic3Regions) {
    if (region->sea)
      continue;
    // count the number of land states for resource generation
    landStates++;
    double totalStateArea = 0;
    double totalDevFactor = 0;
    double totalPopFactor = 0;
    for (const auto &gameProv : region->gameProvinces) {
      totalDevFactor +=
          gameProv->devFactor / (double)region->gameProvinces.size();
      totalPopFactor +=
          gameProv->popFactor / (double)region->gameProvinces.size();
      totalStateArea += gameProv->baseProvince->pixels.size();
    }


    region->development = totalDevFactor;
    //// only init this when it hasn't been initialized via text input before
    if (region->population < 0) {
      region->population =
          static_cast<int>(totalStateArea * 400 * totalPopFactor *
                           worldPopulationFactor * (1.0 / sizeFactor));
    }
    worldPop += (long long)region->population;
  }
}

void Generator::mapRegions() {
  Fwg::Utils::Logging::logLine("Mapping Regions");
  gameRegions.clear();
  vic3Regions.clear();

  for (auto &region : this->areas.regions) {
    std::sort(region.provinces.begin(), region.provinces.end(),
              [](const Fwg::Province *a, const Fwg::Province *b) {
                return (*a < *b);
              });
    auto gameRegion = std::make_shared<Region>(region);
    for (auto &baseRegion : gameRegion->neighbours)
      gameRegion->neighbours.push_back(baseRegion);
    // generate random name for region
    gameRegion->name = NameGeneration::generateName(nData);

    for (auto &province : gameRegion->provinces) {
      gameRegion->gameProvinces.push_back(gameProvinces[province->ID]);
    }
    // save game region to generic module container and to hoi4 specific
    // container
    gameRegions.push_back(gameRegion);
    vic3Regions.push_back(gameRegion);
  }
  // sort by gameprovince ID
  std::sort(gameRegions.begin(), gameRegions.end(),
            [](auto l, auto r) { return *l < *r; });
  // check if we have the same amount of gameProvinces as FastWorldGen provinces
  if (gameProvinces.size() != this->areas.provinces.size())
    throw(std::exception("Fatal: Lost provinces, terminating"));
  if (gameRegions.size() != this->areas.regions.size())
    throw(std::exception("Fatal: Lost regions, terminating"));
  for (const auto &gameRegion : gameRegions) {
    if (gameRegion->ID > gameRegions.size()) {
      throw(std::exception("Fatal: Invalid region IDs, terminating"));
    }
  }
  applyRegionInput();
}
// initialize states
void Generator::initializeStates() {

}
// initialize states
void Generator::initializeCountries() {}
} // namespace Scenario::Vic3