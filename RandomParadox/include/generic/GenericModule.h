#pragma once
#include "generic/ParserUtils.h"
#include "utils/Logging.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <string>
namespace Scenario {
class GenericModule {

protected:
  int numCountries;
  std::string modName;
  std::string gamePath;
  std::string gameModPath;
  std::string gameModsDirectory;
  const boost::property_tree::ptree
  readConfig(const std::string &configSubFolder, const std::string &username,
             const std::string &gameName);
  void createPaths(const std::string &basePath);
  // try to locate hoi4 at configured path, if not found, try other
  // standard locations
  bool findGame(std::string &path, const std::string &game);
  // check if configured mod directories are correct
  bool findModFolders();
};
} // namespace Scenario