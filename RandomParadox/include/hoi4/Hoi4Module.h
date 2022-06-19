#pragma once
#include "FastWorldGenerator.h"
#include "generic/FormatConverter.h"
#include "generic/GenericModule.h"
#include "hoi4/Hoi4Parsing.h"
#include "hoi4/Hoi4Generator.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>

namespace Hoi4 {

class Hoi4Module : GenericModule {

public:
  Hoi4Module();
  ~Hoi4Module();
  // member functions
  // clear and create all the mod paths at each run
  bool createPaths();
  void genHoi(bool useDefaultMap, bool useDefaultStates,
              bool useDefaultProvinces, Generator &scenGen, bool cut);
  // read the config specific to hoi4
  void readHoiConfig(std::string configSubFolder, std::string username);
};
} // namespace Hoi4