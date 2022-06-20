#pragma once
#include "FastWorldGenerator.h"
#include "eu4/Eu4Generator.h"
#include "eu4/Eu4Parsing.h"
#include "generic/FormatConverter.h"
#include "generic/GenericModule.h"
#include "generic/ScenarioGenerator.h"
namespace Scenario::Eu4 {
class Module : GenericModule {
  Generator eu4Gen;
  // clear and create all the mod paths at each run
  bool createPaths();
  // read the config options specific to eu4
  void readEu4Config(const std::string &configSubFolder,
                     const std::string &username);

public:
  Module(Fwg::FastWorldGenerator &fastWorldGen, const std::string &configSubFolder,
         const std::string &username);
  ~Module();
  // member functions
  void genEu4(bool cut);
};
} // namespace Scenario::Eu4
