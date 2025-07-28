#include "generic/ModGenerator.h"
namespace Logging = Fwg::Utils::Logging;
namespace Rpx {
using namespace Fwg::Gfx;
ModGenerator::ModGenerator() : Arda::ArdaGen() {}

ModGenerator::ModGenerator(const std::string &configSubFolder)
    : Arda::ArdaGen(configSubFolder) {
  Arda::Gfx::Flag::readColourGroups();
  Arda::Gfx::Flag::readFlagTypes();
  Arda::Gfx::Flag::readFlagTemplates();
  Arda::Gfx::Flag::readSymbolTemplates();
  superRegionMap = Bitmap(0, 0, 24);
}

ModGenerator::ModGenerator(Arda::ArdaGen &scenGen) : Arda::ArdaGen(scenGen) {}

ModGenerator::~ModGenerator() {}



void ModGenerator::mapCountries() {}

Fwg::Gfx::Bitmap ModGenerator::mapTerrain() {
  Bitmap typeMap(climateMap.width(), climateMap.height(), 24);
  auto &colours = Fwg::Cfg::Values().colours;
  typeMap.fill(colours.at("sea"));
  Logging::logLine("Mapping Terrain");
  for (auto &ardaRegion : ardaRegions) {
    for (auto &gameProv : ardaRegion->ardaProvinces) {
    }
  }
  Png::save(typeMap, Fwg::Cfg::Values().mapsPath + "/typeMap.png");
  return typeMap;
}

void ModGenerator::cutFromFiles(const std::string &gamePath) {
  Fwg::Utils::Logging::logLine("Unimplemented cutting");
}

} // namespace Rpx