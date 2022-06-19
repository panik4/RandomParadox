#include "eu4/Eu4Generator.h"
namespace Scenario::Eu4 {
using namespace FastWorldGen;
Generator::Generator(FastWorldGenerator &fwg) : Scenario::Generator(fwg) {}
void Generator::generateRegions(std::vector<GameRegion> &regions) {
  Logger::logLine("HOI4: Dividing world into strategic regions");
  std::set<int> assignedIdeas;
  for (auto &region : regions) {
    if (assignedIdeas.find(region.ID) == assignedIdeas.end()) {
      eu4Region euR;
      // std::set<int>stratRegion;
      euR.areaIDs.insert(region.ID);
      assignedIdeas.insert(region.ID);
      for (auto &neighbour : region.neighbours) {
        // should be equal in sea/land
        if (neighbour > regions.size())
          continue;
        if (regions[neighbour].sea == region.sea &&
            assignedIdeas.find(neighbour) == assignedIdeas.end()) {
          euR.areaIDs.insert(neighbour);
          assignedIdeas.insert(neighbour);
        }
      }
      euR.name = NameGenerator::generateName();
      eu4regions.push_back(euR);
    }
  }
  Bitmap eu4RegionBmp(Env::Instance().width, Env::Instance().height, 24);
  for (auto &strat : eu4regions) {
    Colour c{static_cast<unsigned char>(RandNum::randNum() % 255),
             static_cast<unsigned char>(RandNum::randNum() % 255),
             static_cast<unsigned char>(RandNum::randNum() % 255)};
    for (auto &area : strat.areaIDs) {
      c.setBlue(regions[area].sea ? 255 : 0);
      for (auto &prov : regions[area].gameProvinces) {
        for (auto &pix : prov.baseProvince->pixels) {
          eu4RegionBmp.setColourAtIndex(pix, c);
        }
      }
    }
  }
  Bitmap::bufferBitmap("eu4regions", eu4RegionBmp);
  Bitmap::SaveBMPToFile(eu4RegionBmp, "Maps\\eu4Regions.bmp");
}

} // namespace Scenario::Eu4