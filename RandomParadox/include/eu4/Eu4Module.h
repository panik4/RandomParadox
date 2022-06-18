#pragma once
#include "FastWorldGenerator.h"
#include "eu4/Eu4Parser.h"
#include "eu4/Eu4ScenarioGenerator.h"
#include "generic/FormatConverter.h"
#include "generic/GenericModule.h"
#include "generic/ScenarioGenerator.h"
class Eu4Module : GenericModule {
  // member variables

public:
  Eu4Module();
  ~Eu4Module();
  // member functions
  // clear and create all the mod paths at each run
  bool createPaths();
  void genEu4(bool useDefaultMap, bool useDefaultStates,
              bool useDefaultProvinces, ScenarioGenerator &scenGen, bool cut);
  // read the config options specific to eu4
  void readEu4Config(std::string& configSubFolder, std::string& username);
};
