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
  superRegionMap = Image(0, 0, 24);
  this->pathcfg.gameSubPath = gameSubPath;
  this->gameType = gameType;
  ardaFactories.superRegionFactory =
      []() -> std::shared_ptr<Rpx::StrategicRegion> {
    return std::make_shared<Rpx::StrategicRegion>();
  };
}

ModGenerator::~ModGenerator() {}

void ModGenerator::mapCountries() {}

} // namespace Rpx