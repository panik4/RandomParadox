#include "generic/GenericModule.h"

void GenericModule::createPaths(const std::string basePath) { // mod directory
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
  create_directory(basePath + "\\history\\countries\\");
  // localisation
  create_directory(basePath + "\\localisation\\");
  // common
  create_directory(basePath + "\\common\\");
  create_directory(basePath + "\\common\\countries\\");
  create_directory(basePath + "\\common\\bookmarks\\");
  create_directory(basePath + "\\common\\country_tags\\");
}
// a method to search for the original game files on the hard drive(s)
bool GenericModule::findGame(std::string &path, const std::string game) {
  using namespace std::filesystem;
  std::vector<std::string> drives{"C:\\", "D:\\", "E:\\",
                                  "F:\\", "G:\\", "H:\\"};
  // first try to find hoi4 at the configured location
  if (exists(path)) {
    return true;
  } else {
    Logger::logLine("Could not find game under configured path ", path,
                    " it doesn't exist or is malformed. Auto search will now "
                    "try to locate the game, but may not succeed. It is "
                    "recommended to correctly configure the path");
    system("pause");
  }
  for (const auto &drive : drives) {
    if (exists(drive + "Program Files (x86)\\Steam\\steamapps\\common\\" +
               game)) {
      path = drive + "Program Files (x86)\\Steam\\steamapps\\common\\" + game;
      Logger::logLine("Located game under ", path);
      return true;
    } else if (exists(drive + "Program Files\\Steam\\steamapps\\common\\" +
                      game)) {
      path = drive + "Program Files\\Steam\\steamapps\\common\\" + game;
      Logger::logLine("Located game under ", path);
      return true;
    } else if (exists(drive + "Steam\\steamapps\\common\\" + game)) {
      path = drive + "Steam\\steamapps\\common\\" + game;
      Logger::logLine("Located game under ", path);
      return true;
    }
  }
  Logger::logLine("Could not find the game anywhere. Make sure the path to ",
                  game, " is configured correctly in the config files");
  return false;
}

// reads config for Hearts of Iron IV
const boost::property_tree::ptree
GenericModule::readConfig(const std::string configSubFolder,
                          const std::string username,
                          const std::string gameName) {
  // Short alias for this namespace
  namespace pt = boost::property_tree;
  // Create a root
  pt::ptree root;
  std::ifstream f(configSubFolder + gameName + "Module.json");
  std::stringstream buffer;
  if (!f.good()) {
    Logger::logLine("Config could not be loaded");
  }
  buffer << f.rdbuf();
  try {
    pt::read_json(buffer, root);
  } catch (std::exception e) {
    std::string error =
        "Incorrect config " + configSubFolder + gameName + " Module.json\n";
    error += "You can try fixing it yourself. Error is: \n";
    error += e.what();
    error += "\n";
    error += "Otherwise try running it through a json validator, e.g. "
             "\"https://jsonlint.com/\" or search for \"json validator\"\n";
    throw(std::exception(error.c_str()));
  }
  // now read the paths
  modName = root.get<std::string>("module.modName");
  gamePath = root.get<std::string>("module.gamePath");
  gameModPath = root.get<std::string>("module.modPath") + modName;
  ParserUtils::replaceOccurences(gameModPath, "<username>", username);
  gameModsDirectory = root.get<std::string>("module.modsDirectory");
  ParserUtils::replaceOccurences(gameModsDirectory, "<username>", username);
  // passed to generic ScenarioGenerator
  numCountries = root.get<int>("scenario.numCountries");

  // return tree to specific module
  return root;
}
