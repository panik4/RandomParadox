#pragma once
#include "FastWorldGenerator.h"
#include "generic/FormatConverter.h"
#include "generic/GenericModule.h"
#include "hoi4/Hoi4Generator.h"
#include "hoi4/Hoi4Parsing.h"
#include "hoi4/Hoi4MapPainting.h"
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
                     const std::string &username,
                     const boost::property_tree::ptree &rpdConf);

public:
  Hoi4Module(const boost::property_tree::ptree &rpdConf,
             const std::string &configSubFolder, const std::string &username, const bool mapCountries);
  ~Hoi4Module();
  // member functions
  void genHoi();
  void mapCountries();
};
} // namespace Scenario::Hoi4