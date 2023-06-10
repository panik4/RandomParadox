#include "generic/GenericModule.h"
namespace Scenario {
void GenericModule::createPaths(const std::string &basePath) { // mod directory
  using namespace std::filesystem;
  create_directory(basePath);
  // map
  remove_all(basePath + "\\map\\");
  remove_all(basePath + "\\gfx");
  remove_all(basePath + "\\history");
  remove_all(basePath + "\\common\\");
  remove_all(basePath + "\\localisation\\");
  create_directory(basePath + "\\map\\");
  create_directory(basePath + "\\map\\terrain\\");
  // gfx
  create_directory(basePath + "\\gfx\\");
  create_directory(basePath + "\\gfx\\flags\\");
  // history
  create_directory(basePath + "\\history\\");
  // localisation
  create_directory(basePath + "\\localisation\\");
  // common
  create_directory(basePath + "\\common\\");
}
// a method to search for the original game files on the hard drive(s)
bool GenericModule::findGame(std::string &path, const std::string &game) {
  using namespace std::filesystem;
  namespace Logging = Fwg::Utils::Logging;
  std::vector<std::string> drives{"C:\\", "D:\\", "E:\\",
                                  "F:\\", "G:\\", "H:\\"};
  // first try to find hoi4 at the configured location
  if (exists(path)) {
    Logging::logLine("Located game under ", path);
    return true;
  } else {
    Fwg::Utils::Logging::logLine(
        "Could not find game under configured path ", path,
        " it doesn't exist or is malformed. Auto search will now "
        "try to locate the game, but may not succeed. It is "
        "recommended to correctly configure the path");
    system("pause");
  }
  for (const auto &drive : drives) {
    if (exists(drive + "Program Files (x86)\\Steam\\steamapps\\common\\" +
               game)) {
      path = drive + "Program Files (x86)\\Steam\\steamapps\\common\\" + game;
      Logging::logLine("Located game under ", path);
      return true;
    } else if (exists(drive + "Program Files\\Steam\\steamapps\\common\\" +
                      game)) {
      path = drive + "Program Files\\Steam\\steamapps\\common\\" + game;
      Logging::logLine("Located game under ", path);
      return true;
    } else if (exists(drive + "Steam\\steamapps\\common\\" + game)) {
      path = drive + "Steam\\steamapps\\common\\" + game;
      Logging::logLine("Located game under ", path);
      return true;
    }
  }
  Logging::logLine("Could not find the game anywhere. Make sure the path to ",
                   game, " is configured correctly in the config files");
  return false;
}

bool GenericModule::findModFolders() {
  using namespace std::filesystem;
  namespace Logging = Fwg::Utils::Logging;
  path modsDir(gameModPath);
  if (exists(modsDir.parent_path())) {
    Logging::logLine("Located mod folder under ", modsDir.parent_path());
  } else {
    Logging::logLine(
        "Could not find parent directory of mod directory under configured "
        "path ",
        modsDir.parent_path(),
        " it doesn't exist or is malformed. Please correct the path");
    system("pause");
    return false;
  }
  if (exists(gameModsDirectory)) {
    Logging::logLine("Located mods directory folder under ", gameModsDirectory);
  } else {
    Logging::logLine(
        "Could not find game mods directory folder under configured path ",
        gameModsDirectory,
        " it doesn't exist or is malformed. Please correct the path");
    system("pause");
    return false;
  }
  return true;
}

// reads config for Hearts of Iron IV
void GenericModule::configurePaths(
    const std::string &username, const std::string &gameName,
    const boost::property_tree::ptree &gamesConf) {
  // Short alias for this namespace
  // now read the paths
  modName = gamesConf.get<std::string>(gameName + ".modName");
  gamePath = gamesConf.get<std::string>(gameName + ".gamePath");
  mappingPath = gamesConf.get<std::string>(gameName + ".mappingPath");
  gameModPath = gamesConf.get<std::string>(gameName + ".modPath") + modName;
  Fwg::Parsing::Scenario::replaceOccurences(gameModPath, "<username>",
                                            username);
  gameModsDirectory = gamesConf.get<std::string>(gameName + ".modsDirectory");
  Fwg::Parsing::Scenario::replaceOccurences(gameModsDirectory, "<username>", username);
}
} // namespace Scenario