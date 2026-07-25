#include "generic/ModGenerator.h"
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
  Fwg::Utils::Logging::logLine("ModGenerator::ModGenerator finished");
}

ModGenerator::~ModGenerator() {}

void ModGenerator::mapCountries() {}

void ModGenerator::save(const std::string &path) {
  std::ofstream file(path, std::ios::binary);
  boost::archive::binary_oarchive ar(file);
  ar << areaData << terrainData << climateData;
  ar << climateMap << worldMap << segmentMap << provinceMap << regionMap;
  ar << locationMap << navmeshMap << errorMap;
  ar << preModifyHeightMap << preModifyHumidityMap;
  ar << ardaContinents << ardaRegions << ardaProvinces << superRegions;
  ar << countries << civData << nData;
  ar << typeMap << countryMap << superRegionMap;
  ar << gameType << exportWidth << exportHeight;
  ar << pathcfg;
}

void ModGenerator::load(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  boost::archive::binary_iarchive ar(file);
  resetData();
  ar >> areaData >> terrainData >> climateData;
  ar >> climateMap >> worldMap >> segmentMap >> provinceMap >> regionMap;
  ar >> locationMap >> navmeshMap >> errorMap;
  ar >> preModifyHeightMap >> preModifyHumidityMap;
  ar >> ardaContinents >> ardaRegions >> ardaProvinces >> superRegions;
  ar >> countries >> civData >> nData;
  ar >> typeMap >> countryMap >> superRegionMap;
  ar >> gameType >> exportWidth >> exportHeight;
  ar >> pathcfg;
  mapProvinces();
  mapRegions();
  mapContinents();
}

} // namespace Rpx