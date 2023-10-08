#pragma once
#include "FastWorldGenerator.h"
#include "generic/FormatConverter.h"
#include "generic/GenericModule.h"
#include "generic/ScenarioGenerator.h"
#include "vic3/Vic3Generator.h"
#include "vic3/Vic3Parsing.h"
namespace Scenario::Vic3 {
class Module : public GenericModule {
  std::shared_ptr<Generator> vic3Gen;
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
  virtual void generate();
};
} // namespace Scenario::Vic3
