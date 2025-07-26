#include "generic/GenericModule.h"
namespace Scenario {
bool GenericModule::createPaths() { // mod directory
  return false;
}
bool GenericModule::validatePaths() { return false; }
// a method to search for the original game files on the hard drive(s)
bool GenericModule::findGame(std::string &path, const std::string &game) {
  using namespace std::filesystem;
  namespace Logging = Fwg::Utils::Logging;
  std::vector<std::string> drives{"C://", "D://", "E://",
                                  "F://", "G://", "H://"};
  // first try to find hoi4 at the configured location
  if (exists(path) && path.find(game) != std::string::npos) {
    Logging::logLine("Located game under ", path);
    return true;
  } else {
    Fwg::Utils::Logging::logLine(
        "Could not find game under configured path ", path,
        " it doesn't exist or is malformed. Auto search will now "
        "try to locate the game, but may not succeed. It is "
        "recommended to correctly configure the path");
  }
  const std::vector<std::string> commonPaths{
      "Program Files (x86)//Steam//steamapps//common//",
      "Program Files//Steam//steamapps//common//", "Steam//steamapps//common//",
      "SteamLibrary//steamapps//common//"};
  for (const auto &drive : drives) {
    for (auto &commonPath : commonPaths) {
      if (exists(drive + commonPath + game)) {
        path = drive + commonPath + game + "//";
        // now check if the executable is in the game folder
        if (!exists(path + executableSubPath)) {
          Logging::logLine("Couldn't locate executable under ",
                           path + executableSubPath,
                           " are you sure this is the game folder?");
          return false;
        }

        Logging::logLine("Located game under ", path);
        return true;
      }
    }
  }
  Logging::logLine("Could not find the game anywhere. Make sure the path to ",
                   game, " is configured correctly in the config files");
  return false;
}

bool GenericModule::validateGameModFolder(const std::string &game) {
  using namespace std::filesystem;
  namespace Logging = Fwg::Utils::Logging;
  // find the usermod directory of the relevant game
  if (exists(pathcfg.gameModsDirectory)) {
    Logging::logLine("Located mods directory folder under ",
                     Fwg::Utils::userFilter(pathcfg.gameModsDirectory,
                                            Fwg::Cfg::Values().username));
    return true;
  } else {
    Logging::logLine(
        "Could not find the games mods directory folder under configured path ",
        pathcfg.gameModsDirectory,
        " it doesn't exist or is malformed. Please correct the path");
    return false;
  }
}
bool GenericModule::autoLocateGameModFolder(const std::string &game) {
  using namespace std::filesystem;
  namespace Logging = Fwg::Utils::Logging;
  Logging::logLine("Trying to auto locate mod directory path for ", game);
  std::string autoPath = getenv("USERPROFILE");
  autoPath += "//Documents//Paradox Interactive//" + game + "//mod//";

  if (exists(autoPath)) {
    pathcfg.gameModsDirectory = autoPath;
    sanitizePath(pathcfg.gameModsDirectory);
    Logging::logLine("Auto located game mod directory is ",
                     Fwg::Utils::userFilter(pathcfg.gameModsDirectory,
                                            Fwg::Cfg::Values().username));
    pathcfg.gameModPath = autoPath + pathcfg.modName;
    sanitizePath(pathcfg.gameModPath);
    Logging::logLine("Auto located mod directory is ",
                     Fwg::Utils::userFilter(pathcfg.gameModPath,
                                            Fwg::Cfg::Values().username));
  }

  return false;
}

// try to find parent directory of where the mod should be
bool GenericModule::validateModFolder(const std::string &game) {
  using namespace std::filesystem;
  namespace Logging = Fwg::Utils::Logging;
  std::string tempPath = pathcfg.gameModPath;
  while (tempPath.back() == '/')
    tempPath.pop_back();
  path modDestinationDir(tempPath);
  if (exists(modDestinationDir.parent_path())) {
    Logging::logLine(
        "Located mod folder under ",
        Fwg::Utils::userFilter(modDestinationDir.parent_path().string(),
                               Fwg::Cfg::Values().username));
    return true;
  } else {
    Logging::logLine(
        "Could not find parent directory of mod directory under configured "
        "path ",
        Fwg::Utils::userFilter(modDestinationDir.parent_path().string(),
                               Fwg::Cfg::Values().username),
        " it doesn't exist or is malformed. Please correct the path");
    return false;
  }
}

void GenericModule::sanitizePath(std::string &path) {
  Fwg::Parsing::Scenario::replaceOccurences(path, "\\", "//");
  Fwg::Parsing::attachTrailing(path);
}

void GenericModule::generate() {}

void GenericModule::initNameData(const std::string &path,
                                 const std::string &gamePath) {
  generator->nData = NameGeneration::prepare(path, gamePath, gameType);
  //generator->civData.nData = generator->nData;
}

void GenericModule::initFormatConverter() {}

void GenericModule::writeFiles() {}

void GenericModule::writeImages() {}

// reads generic configs for every module
void GenericModule::configurePaths(
    const std::string &username, const std::string &gameName,
    const boost::property_tree::ptree &gamesConf) {
  // Short alias for this namespace
  // now read the paths
  pathcfg.modName = gamesConf.get<std::string>(gameName + ".modName");
  pathcfg.gamePath = gamesConf.get<std::string>(gameName + ".gamePath");
  sanitizePath(pathcfg.gamePath);
  pathcfg.gameModPath = gamesConf.get<std::string>(gameName + ".modPath");
  // already attach trailing before attaching the modname as the subfolder
  sanitizePath(pathcfg.gameModPath);
  pathcfg.gameModPath += pathcfg.modName;
  sanitizePath(pathcfg.gameModPath);
  Fwg::Parsing::Scenario::replaceOccurences(pathcfg.gameModPath, "<username>",
                                            username);
  Fwg::Parsing::Scenario::replaceOccurences(pathcfg.gamePath, "<username>",
                                            username);
  pathcfg.gameModsDirectory =
      gamesConf.get<std::string>(gameName + ".modsDirectory");
  sanitizePath(pathcfg.gameModsDirectory);
  Fwg::Parsing::Scenario::replaceOccurences(pathcfg.gameModsDirectory,
                                            "<username>", username);
}
} // namespace Scenario