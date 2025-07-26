#pragma once
#include "FastWorldGenerator.h"
#include "io/FormatConverter.h"
#include "generic/GenericModule.h"
#include "generic/ArdaGen.h"
namespace Rpx::Aoe2 {
class Module : GenericModule {
  Fwg::FastWorldGenerator f;
  // clear and create all the mod paths at each run
  bool createPaths();
  // read the config options specific to eu4
  void readVic3Config(const std::string &configSubFolder,
                     const std::string &username,
                     const boost::property_tree::ptree &gamesConf);

public:
  Module(const boost::property_tree::ptree &rpdConf,
         const std::string &configSubFolder, const std::string &username);
  ~Module();
  // member functions
  void genAoe2();
};
} // namespace Rpx::Eu4
