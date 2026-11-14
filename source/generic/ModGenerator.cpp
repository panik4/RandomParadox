#include "generic/ModGenerator.h"
namespace Logging = Fwg::Utils::Logging;
namespace Rpx {
using namespace Fwg::Gfx;

ModGenerator::ModGenerator(const std::string &configSubFolder,
                           const GameType &gameType,
                           const std::string &gameSubPath)
    : Arda::ArdaGen(configSubFolder) {

  Fwg::Utils::Logging::logLine("ModGenerator::ModGenerator");
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

void ModGenerator::save(const std::string &path, const Fwg::Cfg &config) {
  std::ofstream file(path, std::ios::binary);
  boost::archive::binary_oarchive ar(file);
  saveFwgData(ar, config);
  ar << ardaData.areaData.continents << ardaData.areaData.regions
     << ardaData.areaData.provinces << ardaData.areaData.superRegions;
  ar << ardaData.countries << ardaData.civData << ardaData.nData;
  ar << countryMap << superRegionMap;
  ar << gameType << exportWidth << exportHeight;
  ar << pathcfg;
  ar << ardaData.civLayer;
  saveArdaState(ar);
}

void ModGenerator::load(const std::string &path, Fwg::Cfg &config) {
  std::ifstream file(path, std::ios::binary);
  boost::archive::binary_iarchive ar(file);
  loadFwgData(ar, config);
  ar >> ardaData.areaData.continents >> ardaData.areaData.regions >>
      ardaData.areaData.provinces >> ardaData.areaData.superRegions;
  ar >> ardaData.countries >> ardaData.civData >> ardaData.nData;
  ar >> countryMap >> superRegionMap;
  ar >> gameType >> exportWidth >> exportHeight;
  ar >> pathcfg;
  ar >> ardaData.civLayer;
  mapProvinces();
  mapRegions();
  mapContinents();
  loadArdaState(ar);
}

} // namespace Rpx
