#include "vic3/Vic3Generator.h"
namespace Scenario::Vic3 {
using namespace Fwg;
using namespace Fwg::Gfx;

Generator::Generator() {}

Generator::Generator(FastWorldGenerator &fwg) : Scenario::Generator(fwg) {}
void Generator::generateRegions(std::vector<std::shared_ptr<Region>> &regions) {
  Utils::Logging::logLine("Vic3: Dividing world into strategic regions");
  std::set<int> assignedIdeas;
  for (auto &region : regions) {
    if (assignedIdeas.find(region->ID) == assignedIdeas.end()) {
      Vic3Region euR;
      // std::set<int>stratRegion;
      euR.areaIDs.insert(region->ID);
      assignedIdeas.insert(region->ID);
      for (auto &neighbour : region->neighbours) {
        // should be equal in sea/land
        if (neighbour > regions.size())
          continue;
        if (regions[neighbour]->sea == region->sea &&
            assignedIdeas.find(neighbour) == assignedIdeas.end()) {
          euR.areaIDs.insert(neighbour);
          assignedIdeas.insert(neighbour);
        }
      }
      euR.name = NameGeneration::generateName(nData);
      vic3regions.push_back(euR);
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
  Bmp::save(vic3RegionBmp, "Maps\\vic3Regions.bmp");
  Fwg::Gfx::Png::save(vic3RegionBmp, "Maps\\vic3Regions.png");
}

} // namespace Scenario::Vic3