#pragma once
#include "FastWorldGenerator.h"
#include "eu4/Eu4Generator.h"
#include "eu4/Eu4Parsing.h"
#include "generic/FormatConverter.h"
#include "generic/GenericModule.h"
#include "generic/ScenarioGenerator.h"
namespace Scenario::Eu4 {
class Module : public GenericModule {
  std::shared_ptr<Generator> eu4Gen;
  // clear and create all the mod paths at each run
  bool createPaths();
  // read the config options specific to eu4
  void readEu4Config(const std::string &configSubFolder,
                     const std::string &username,
                     const boost::property_tree::ptree &gamesConf);

public:
  Module(const boost::property_tree::ptree &rpdConf,
         const std::string &configSubFolder, const std::string &username);
  ~Module();
  // member functions
  virtual void generate();
};
} // namespace Scenario::Eu4
