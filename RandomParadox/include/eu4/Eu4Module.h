#pragma once
#include "FastWorldGenerator.h"
#include "eu4/Eu4Generator.h"
#include "eu4/Eu4Parsing.h"
#include "generic/FormatConverter.h"
#include "generic/GenericModule.h"
#include "generic/ScenarioGenerator.h"
namespace Eu4 {
class Module : GenericModule {
  // member variables

public:
  Module();
  ~Module();
  // member functions
  // clear and create all the mod paths at each run
  bool createPaths();
  void genEu4(bool useDefaultMap, bool useDefaultStates,
              bool useDefaultProvinces, Generator &scenGen, bool cut);
  // read the config options specific to eu4
  void readEu4Config(std::string &configSubFolder, std::string &username);
};
} // namespace Eu4
