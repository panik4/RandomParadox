#include "eu4/Eu4Generator.h"
namespace Scenario::Eu4 {
using namespace Fwg;
using namespace Fwg::Gfx;

Generator::Generator() {}

Generator::Generator(const std::string &configSubFolder)
    : Scenario::ModGenerator(configSubFolder) {}

void Generator::generateRegions(
    std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions) {
  Fwg::Utils::Logging::logLine("Eu4: Dividing world into strategic regions");
  std::set<int> assignedIdeas;
  for (auto &region : regions) {
    if (assignedIdeas.find(region->ID) == assignedIdeas.end()) {
      eu4Region euR;
      // std::set<int>stratRegion;
      euR.areaIDs.insert(region->ID);
      assignedIdeas.insert(region->ID);
      for (auto &neighbour : region->neighbours) {
        // should be equal in sea/land
        if (neighbour > regions.size())
          continue;
        if (regions[neighbour]->isSea() == region->isSea() &&
            assignedIdeas.find(neighbour) == assignedIdeas.end()) {
          euR.areaIDs.insert(neighbour);
          assignedIdeas.insert(neighbour);
        }
      }
      eu4regions.push_back(euR);
    }
  }
  Bitmap eu4RegionBmp(Cfg::Values().width, Cfg::Values().height, 24);
  for (auto &strat : eu4regions) {
    Colour c{
        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255),
        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255),
        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255)};
    for (auto &area : strat.areaIDs) {
      c.setBlue(regions[area]->isSea() ? 255 : 0);
      for (auto &prov : regions[area]->ardaProvinces) {
        for (auto &pix : prov->baseProvince->pixels) {
          eu4RegionBmp.setColourAtIndex(pix, c);
        }
      }
    }
  }
  Bmp::bufferBitmap("eu4regions", eu4RegionBmp);
  Bmp::save(eu4RegionBmp, Fwg::Cfg::Values().mapsPath + "//eu4Regions.bmp");
}
// initialize states
void Generator::initializeStates() {}
// initialize states
void Generator::mapCountries() {}
} // namespace Scenario::Eu4