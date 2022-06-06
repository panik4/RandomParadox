#pragma once
#include "FastWorldGenerator.h"
#include "generic/FormatConverter.h"
#include "generic/ScenarioGenerator.h"
#include "generic/GenericModule.h"
class Eu4Module : GenericModule {
  // member variables
  //Hoi4ScenarioGenerator hoi4Gen;

public:
  Eu4Module();
  ~Eu4Module();
  // member functions
  // clear and create all the mod paths at each run
  bool createPaths();
  void genEu4(bool useDefaultMap, bool useDefaultStates,
              bool useDefaultProvinces, ScenarioGenerator &scenGen, bool cut);
  // read the config specific to hoi4
  void readEu4Config(std::string configSubFolder, std::string username);
};
