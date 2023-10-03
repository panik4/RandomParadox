#pragma once
#include "generic/ParserUtils.h"
#include "utils/Logging.h"
#include "generic/ScenarioUtils.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <string>
namespace Scenario {
class GenericModule {
protected:
  bool cut;

  void configurePaths(const std::string &username, const std::string &gameName,
                      const boost::property_tree::ptree &gamesConf);
  void createPaths(const std::string &basePath);


public:
  int numCountries;
  // try to locate hoi4 at configured path, if not found, try other
  // standard locations
  bool findGame(std::string &path, const std::string &game);
  // check if configured mod directories are correct
  bool findModFolders(const std::string &game);
  Utils::Pathcfg pathcfg;
};
} // namespace Scenario