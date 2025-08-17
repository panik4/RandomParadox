#include "generic/ModGenerator.h"
namespace Logging = Fwg::Utils::Logging;
namespace Rpx {
using namespace Fwg::Gfx;

ModGenerator::ModGenerator(const std::string &configSubFolder,
                           const GameType &gameType,
                           const std::string &gameSubPath,
                           const boost::property_tree::ptree &rpdConf)
    : Arda::ArdaGen(configSubFolder) {
  Arda::Gfx::Flag::readColourGroups();
  Arda::Gfx::Flag::readFlagTypes();
  Arda::Gfx::Flag::readFlagTemplates();
  Arda::Gfx::Flag::readSymbolTemplates();
  superRegionMap = Bitmap(0, 0, 24);
  this->gameSubPath = gameSubPath;
  this->gameType = gameType;
}

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