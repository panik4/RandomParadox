#pragma once
#include "FastWorldGenerator.h"
#include "generic/FormatConverter.h"
#include "generic/GenericModule.h"
#include "hoi4/Hoi4Generator.h"
#include "hoi4/Hoi4Parsing.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>

namespace Scenario::Hoi4 {
class Hoi4Module : GenericModule {
  Generator hoi4Gen;

  // clear and create all the mod paths at each run
  bool createPaths();
  // read the config specific to hoi4
  void readHoiConfig(const std::string &configSubFolder,
                     const std::string &username);

public:
  Hoi4Module(FastWorldGenerator &fastWorldGen,
             const std::string &configSubFolder, const std::string &username);
  ~Hoi4Module();
  // member functions
  void genHoi(bool cut);
};
} // namespace Scenario::Hoi4