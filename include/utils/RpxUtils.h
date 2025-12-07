#pragma once
#include "FastWorldGenerator.h"
#include "parsing/ParserUtils.h"
#include <string>
#include <vector>
enum class GameType { Generic, Hoi4, Vic3, Eu4, Eu5 };
namespace Rpx::Utils {
struct Pathcfg {
  std::string modName;
  std::string gamePath;
  std::string gameModPath;
  std::string gameModsDirectory;
  std::string mappingPath;
  // optional for every game
  std::string gameSubPath = "";
};

// replace slash type and append slashes
inline void sanitizePath(std::string &path) {
  Rpx::Parsing::replaceOccurences(path, "\\", "//");
  Fwg::Parsing::attachTrailing(path);
}
// try to find parent directory of where the mod should be
inline bool validateModFolder(const Pathcfg &pathcfg) {
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

// a method to search for the original game files on the hard drive(s)
inline bool findGame(std::string &path, const std::string &game,
                     const std::string &gameSubPath) {
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
        if (!exists(path + gameSubPath)) {
          Logging::logLine("Couldn't locate executable under ",
                           path + gameSubPath,
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

inline bool validateGameModFolder(const Pathcfg &pathcfg) {
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

inline std::string getEnvVar(const char *name) {
  size_t len = 0;
  errno_t err = getenv_s(&len, nullptr, 0, name); // query required size
  if (err || len == 0) {
    return {}; // empty string if not found
  }

  std::vector<char> buffer(len); // allocate buffer
  err = getenv_s(&len, buffer.data(), buffer.size(), name);
  if (err) {
    throw std::runtime_error("Failed to get environment variable");
  }

  return std::string(buffer.data());
}

inline bool autoLocateGameModFolder(const std::string &game, Pathcfg &pathcfg) {
  using namespace std::filesystem;
  namespace Logging = Fwg::Utils::Logging;
  Logging::logLine("Trying to auto locate mod directory path for ", game);
  std::string autoPath = getEnvVar("USERPROFILE");
  autoPath += "//Documents//Paradox Interactive//" + game + "//mod//";

  if (exists(autoPath)) {
    pathcfg.gameModsDirectory = autoPath;
    Rpx::Utils::sanitizePath(pathcfg.gameModsDirectory);
    Logging::logLine("Auto located game mod directory is ",
                     Fwg::Utils::userFilter(pathcfg.gameModsDirectory,
                                            Fwg::Cfg::Values().username));
    pathcfg.gameModPath = autoPath + pathcfg.modName;
    Rpx::Utils::sanitizePath(pathcfg.gameModPath);
    Logging::logLine("Auto located mod directory is ",
                     Fwg::Utils::userFilter(pathcfg.gameModPath,
                                            Fwg::Cfg::Values().username));
  }

  return false;
}

// reads generic configs for every module
inline void configurePaths(const std::string &username,
                           const std::string &gameName,
                           const boost::property_tree::ptree &gamesConf,
                           Pathcfg &pathcfg) {
  // Short alias for this namespace
  // now read the paths
  pathcfg.modName = gamesConf.get<std::string>(gameName + ".modName");
  pathcfg.gamePath = gamesConf.get<std::string>(gameName + ".gamePath");
  Rpx::Utils::sanitizePath(pathcfg.gamePath);
  pathcfg.gameModPath = gamesConf.get<std::string>(gameName + ".modPath");
  // already attach trailing before attaching the modname as the subfolder
  Rpx::Utils::sanitizePath(pathcfg.gameModPath);
  pathcfg.gameModPath += pathcfg.modName;
  Rpx::Utils::sanitizePath(pathcfg.gameModPath);
  Rpx::Parsing::replaceOccurences(pathcfg.gameModPath, "<username>", username);
  Rpx::Parsing::replaceOccurences(pathcfg.gamePath, "<username>", username);
  pathcfg.gameModsDirectory =
      gamesConf.get<std::string>(gameName + ".modsDirectory");
  Rpx::Utils::sanitizePath(pathcfg.gameModsDirectory);
  Rpx::Parsing::replaceOccurences(pathcfg.gameModsDirectory, "<username>",
                                  username);
}

}; // namespace Rpx::Utils
