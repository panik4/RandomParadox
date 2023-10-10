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
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    dump += Fwg::Parsing::readFile(entry.path().string());
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

  // Short alias for this namespace
  namespace pt = boost::property_tree;
  // Create a root
  pt::ptree metaConf;
  try {
    // Read the basic settings
    std::ifstream f("MetaConf.json");
    std::stringstream buffer;
    if (!f.good())
      Utils::Logging::logLine("Config could not be loaded");
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
    system("pause");
    return -1;
  }
  std::string username = metaConf.get<std::string>("config.username");
  std::string configSubFolder =
      metaConf.get<std::string>("config.configSubFolder");
  // Create a ptree
  pt::ptree rpdConf;
  try {
    // Read the basic settings
    std::ifstream f("configs//RandomParadox.json");
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
    system("pause");
    return -1;
  }

  std::string mapName;

  auto &config = Cfg::Values();
  // check if we can read the config
  try {
    config.readConfig(configSubFolder + "FastWorldGenerator.json");
  } catch (std::exception e) {
    Utils::Logging::logLine("Incorrect config \"FastWorldGenerator.json\"");
    Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                            e.what());
    Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or \"search for json validator\"");
    dumpInfo(e.what(), configSubFolder);
    system("pause");
    return -1;
  }
  try {
    // make sure we always have the default exports directory
    std::filesystem::create_directory("exports\\");
    GUI gui2;
    gui2.shiny(rpdConf, configSubFolder, username);
    dumpInfo("", configSubFolder);
  } catch (std::exception e) {
    Utils::Logging::logLine(e.what());
    dumpInfo(e.what(), configSubFolder);
    system("pause");
    return -1;
  }
  Utils::Logging::logLine("Done with the generation");
  return 0;
}
