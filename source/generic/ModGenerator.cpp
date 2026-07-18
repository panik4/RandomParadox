#include "generic/ModGenerator.h"
#include "utils/Archive.h"
namespace Logging = Fwg::Utils::Logging;
namespace Rpx {
using namespace Fwg::Gfx;

ModGenerator::ModGenerator(const std::string &configSubFolder,
                           const GameType &gameType,
                           const std::string &gameSubPath,
                           const boost::property_tree::ptree &rpdConf)
    : Arda::ArdaGen(configSubFolder) {

  Fwg::Utils::Logging::logLine("ModGenerator::ModGenerator");
  Arda::Gfx::Flag::readColourGroups();
  Arda::Gfx::Flag::readFlagTypes();
  Arda::Gfx::Flag::readFlagTemplates();
  Arda::Gfx::Flag::readSymbolTemplates();
  superRegionMap = Image(0, 0, 24);
  this->pathcfg.gameSubPath = gameSubPath;
  this->gameType = gameType;
  ardaFactories.superRegionFactory =
      []() -> std::shared_ptr<Rpx::StrategicRegion> {
    return std::make_shared<Rpx::StrategicRegion>();
  };
  auto &reg = Fwg::Utils::Serialisation::TypeRegistry::instance();
  reg.registerType<Fwg::Areas::Area, Rpx::StrategicRegion>(
      "Rpx::StrategicRegion");
  Fwg::Utils::Logging::logLine("ModGenerator::ModGenerator finished");
}

ModGenerator::~ModGenerator() {}

void ModGenerator::mapCountries() {}

void ModGenerator::save(const std::string &path) {
  std::ofstream file(path, std::ios::binary);
  Fwg::Utils::Serialisation::Archive ar(file);
  ar.writeVersion();
  areaData.serialise(ar);
  terrainData.serialise(ar);
  climateData.serialise(ar);
  climateMap.serialise(ar);
  worldMap.serialise(ar);
  segmentMap.serialise(ar);
  provinceMap.serialise(ar);
  regionMap.serialise(ar);
  locationMap.serialise(ar);
  navmeshMap.serialise(ar);
  errorMap.serialise(ar);
  ar &preModifyHeightMap &preModifyHumidityMap;
  // ArdaGen data
  ar.polymorphicPtrVector(ardaContinents);
  ar.polymorphicPtrVector(ardaRegions);
  ar.polymorphicPtrVector(ardaProvinces);
  ar.polymorphicPtrVector(superRegions);
ar &countries;
  civData.deserialise(ar);
  nData.deserialise(ar);
  typeMap.deserialise(ar);
  countryMap.deserialise(ar);
  superRegionMap.deserialise(ar);
  ar.serialiseEnum(gameType);
  ar &exportWidth &exportHeight;
  pathcfg.deserialise(ar);
}

void ModGenerator::load(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  // Hex dump first 64 bytes for diagnostics
  {
    std::vector<unsigned char> header(64, 0);
    file.read(reinterpret_cast<char*>(header.data()), 64);
    std::stringstream ss;
    ss << "File header hex: ";
    for (int i = 0; i < 64; i++) {
      char buf[4];
      snprintf(buf, sizeof(buf), "%02x", header[i]);
      ss << buf;
    }
    Fwg::Utils::Logging::logLine(ss.str());
    file.clear();
    file.seekg(0);
  }
  Fwg::Utils::Serialisation::Archive ar(file);
  resetData();
  try {
    ar.readVersion();
  } catch (...) { Fwg::Utils::Logging::logLine("  Failed at version read"); throw; }
  try { areaData.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at areaData"); throw; }
  try { terrainData.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at terrainData"); throw; }
  try { climateData.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at climateData"); throw; }
  try { climateMap.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at climateMap"); throw; }
  try { worldMap.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at worldMap"); throw; }
  try { segmentMap.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at segmentMap"); throw; }
  try { provinceMap.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at provinceMap"); throw; }
  try { regionMap.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at regionMap"); throw; }
  try { locationMap.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at locationMap"); throw; }
  try { navmeshMap.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at navmeshMap"); throw; }
  try { errorMap.deserialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at errorMap"); throw; }
  try { ar &preModifyHeightMap &preModifyHumidityMap; } catch (...) { Fwg::Utils::Logging::logLine("  Failed at preModifyMaps"); throw; }
  try { ar.polymorphicPtrVector(ardaContinents); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at ardaContinents"); throw; }
  try { ar.polymorphicPtrVector(ardaRegions); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at ardaRegions"); throw; }
  try { ar.polymorphicPtrVector(ardaProvinces); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at ardaProvinces"); throw; }
  try { ar.polymorphicPtrVector(superRegions); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at superRegions"); throw; }
  try { ar &countries; } catch (...) { Fwg::Utils::Logging::logLine("  Failed at countries"); throw; }
  try { civData.serialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at civData"); throw; }
  try { nData.serialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at nData"); throw; }
  try { typeMap.serialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at typeMap"); throw; }
  try { countryMap.serialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at countryMap"); throw; }
  try { superRegionMap.serialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at superRegionMap"); throw; }
  try { ar.serialiseEnum(gameType); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at gameType"); throw; }
  try { ar &exportWidth &exportHeight; } catch (...) { Fwg::Utils::Logging::logLine("  Failed at exportDim"); throw; }
  try { pathcfg.serialise(ar); } catch (...) { Fwg::Utils::Logging::logLine("  Failed at pathcfg"); throw; }
  mapProvinces();
  mapRegions();
  mapContinents();
}

} // namespace Rpx