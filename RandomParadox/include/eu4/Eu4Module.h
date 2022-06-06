#pragma once
#include "FastWorldGenerator.h"
#include "generic/FormatConverter.h"
#include "generic/ScenarioGenerator.h"
class Eu4Module {
  // member variables
  //Hoi4ScenarioGenerator hoi4Gen;
  int numCountries;
  std::string modName;
  std::string hoi4Path;
  std::string hoi4ModPath;
  std::string hoi4ModsDirectory;

public:
  Eu4Module();
  ~Eu4Module();
  // member functions
  // clear and create all the mod paths at each run
  bool createPaths();
  void genEu4(ScenarioGenerator &scenGen, bool cut);
  // try to locate hoi4 at configured path, if not found, try other standard
  // locations
  bool findHoi4();
  // read the config specific to hoi4
  void readConfig(std::string configSubFolder, std::string username);
};
