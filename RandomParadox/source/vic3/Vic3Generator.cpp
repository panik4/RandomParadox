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
void Generator::generateRegions(std::vector<std::shared_ptr<Region>> &regions) {
  Utils::Logging::logLine("Vic3: Dividing world into strategic regions");
  std::set<int> assignedIdeas;
  for (auto &region : regions) {
    if (assignedIdeas.find(region->ID) == assignedIdeas.end()) {
      Vic3Region vicR;
      // std::set<int>stratRegion;
      vicR.areaIDs.insert(region->ID);
      assignedIdeas.insert(region->ID);
      for (auto &neighbour : region->neighbours) {
        // should be equal in sea/land
        if (neighbour > regions.size())
          continue;
        if (regions[neighbour]->sea == region->sea &&
            assignedIdeas.find(neighbour) == assignedIdeas.end()) {
          vicR.areaIDs.insert(neighbour);
          assignedIdeas.insert(neighbour);
        }
      }
      vicR.name = NameGeneration::generateName(nData);
      vic3regions.push_back(vicR);
    }
  }
  Bitmap vic3RegionBmp(Cfg::Values().width, Cfg::Values().height, 24);
  for (auto &strat : vic3regions) {
    Colour c{
        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255),
        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255),
        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255)};
    for (auto &area : strat.areaIDs) {
      c.setBlue(regions[area]->sea ? 255 : 0);
      for (auto &prov : regions[area]->gameProvinces) {
        for (auto &pix : prov->baseProvince->pixels) {
          vic3RegionBmp.setColourAtIndex(pix, c);
        }
      }
    }
  }
  Bmp::bufferBitmap("vic3regions", vic3RegionBmp);
  Bmp::save(vic3RegionBmp, "Maps//vic3Regions.bmp");
  Fwg::Gfx::Png::save(vic3RegionBmp, "Maps//vic3Regions.png");
}
// initialize states
void Generator::initializeStates() {}
// initialize states
void Generator::initializeCountries() {}
} // namespace Scenario::Vic3