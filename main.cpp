#include "FastWorldGenerator.h"
#include "UI/GUI.h"
#include "aoe2DE/Aoe2DEModule.h"
#include "eu4/Eu4Module.h"
#include "generic/ScenarioGenerator.h"
#include "generic/Textures.h"
#include "hoi4/Hoi4Module.h"
#include "vic3/Vic3Module.h"
#include <filesystem>

using namespace Fwg;
void dumpInfo(const std::string &error, const std::string &configSubFolder) {
  std::string dump = "";
  std::string path = configSubFolder;
  if (path.length() > 0) {
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
      if (entry.is_directory()) {
        continue; // Skip directories
      }
      dump += Fwg::Parsing::readFile(entry.path().string());
    }
  }
  dump += std::to_string(Cfg::Values().seed);
  dump += "\n";
  for (auto layerSeed : Cfg::Values().seeds) {
    dump += std::to_string(layerSeed);
    dump += "\n";
  }
  dump += error;
  dump += Utils::Logging::Logger::logInstance.getFullLog();
  Fwg::Parsing::writeFile("log.txt", dump);
}

int main() {
  Fwg::Utils::Logging::logLine("Starting the config loading");
  // Short alias for this namespace
  namespace pt = boost::property_tree;
  // Create a root

  pt::ptree metaConf;
  try {
    Fwg::Utils::Logging::logLine("Starting the loading of MetaConf.json");
    std::ifstream f("MetaConf.json");
    std::stringstream buffer;
    if (!f.good()) {
      Utils::Logging::logLine("Config could not be loaded");
    }
    buffer << f.rdbuf();
    Parsing::replaceInStringStream(buffer, "\\", "//");
    pt::read_json(buffer, metaConf);
  } catch (std::exception e) {
    Utils::Logging::logLine("Incorrect config \"MetaConf.json\"");
    Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                            e.what());
    Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
    dumpInfo(e.what(), "");
    return -1;
  }
  std::string username = metaConf.get<std::string>("config.username");
  std::string workingDirectory =
      metaConf.get<std::string>("config.workingDirectory");
  std::string configSubFolder =
      workingDirectory + metaConf.get<std::string>("config.configSubFolder");
  // Create a ptree
  pt::ptree rpdConf;
  try {
    Fwg::Utils::Logging::logLine(
        "Starting the loading of configs//RandomParadox.json");
    // Read the basic settings
    std::ifstream f(workingDirectory + "configs//RandomParadox.json");
    std::stringstream buffer;
    if (!f.good())
      Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();
    Parsing::replaceInStringStream(buffer, "\\", "//");

    pt::read_json(buffer, rpdConf);
  } catch (std::exception e) {
    Utils::Logging::logLine("Incorrect config \"RandomParadox.json\"");
    Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                            e.what());
    Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
    dumpInfo(e.what(), configSubFolder);
    return -1;
  }

  std::string mapName;

  auto &config = Cfg::Values();
  config.workingDirectory = workingDirectory;
  // check if we can read the config
  try {
    Fwg::Utils::Logging::logLine("Starting the loading of ",
                                 configSubFolder + "FastWorldGenerator.json");
    config.readConfig(configSubFolder);
  } catch (std::exception e) {
    Utils::Logging::logLine("Incorrect config \"FastWorldGenerator.json\"");
    Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                            e.what());
    Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or \"search for json validator\"");
    dumpInfo(e.what(), configSubFolder);
    return -1;
  }
  try {
    Fwg::Utils::Logging::logLine("Creating the exports folder");
    // make sure we always have the default exports directory
    std::filesystem::create_directory(workingDirectory + "exports//");
    GUI gui2;
    Fwg::Utils::Logging::logLine("Starting the GUI");
    gui2.shiny(rpdConf, configSubFolder, username);
    Fwg::Utils::Logging::logLine("Exited the GUI");
    dumpInfo("", configSubFolder);
  } catch (std::exception e) {
    Utils::Logging::logLine(e.what());
    dumpInfo(e.what(), configSubFolder);
    return -1;
  }
  Utils::Logging::logLine("Done with the generation");
  return 0;
}
