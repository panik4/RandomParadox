#include "FastWorldGenerator.h"
#include "eu4/Eu4Module.h"
#include "generic/ScenarioGenerator.h"
#include "hoi4/Hoi4Module.h"
#include <filesystem>
using namespace Fwg;
void dumpInfo(const std::string &error, const std::string &configSubFolder) {
  std::string dump = "";
  std::string path = configSubFolder;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    dump += Scenario::ParserUtils::readFile(entry.path().string());
  }
  dump += std::to_string(Env::Instance().seed);
  dump += "\n";
  for (auto layerSeed : Env::Instance().seeds) {
    dump += std::to_string(layerSeed);
    dump += "\n";
  }
  dump += error;
  dump += Utils::Logging::logInstance.getFullLog();
  Scenario::ParserUtils::writeFile("runDump.txt", dump);
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

  bool writeMaps, genHoi4Scenario, genEu4Scenario;
  try {
    // if debug is enabled in the config, a directory subtree containing
    // visualisation of many maps will be created
    writeMaps = rpdConf.get<bool>("randomScenario.writeMaps");
    // generate hoi4 scenario or not
    genHoi4Scenario = rpdConf.get<bool>("randomScenario.genhoi4");
    genEu4Scenario = rpdConf.get<bool>("randomScenario.geneu4");
  } catch (std::exception e) {
    Utils::Logging::logLine("Error reading boost::property_tree");
    Utils::Logging::logLine(
        "Did you rename a field in the json file?. Error is: ", e.what());
    dumpInfo(e.what(), configSubFolder);
    system("pause");
    return -1;
  }

  auto &config = Env::Instance();
  // check if we can read the config
  try {
    config.getConfig(configSubFolder + "FastWorldGenerator.json");
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

  // if we don't want the FastWorldGenerator output in MapsPath, debug = 0 turns
  // this off
  if (!writeMaps) {
    config.writeMaps = false;
  }
  try {
    if (genHoi4Scenario) {
      // generate hoi4 scenario
      Scenario::Hoi4::Hoi4Module hoi4Mod(rpdConf, configSubFolder, username);
      hoi4Mod.genHoi();
      dumpInfo("", configSubFolder);
      system("pause");
    }
    if (genEu4Scenario) {
      // need to run fwg with different settings from hoi4, even if it ran there
      // already
      FastWorldGenerator fwg(configSubFolder);
      config.seaLevel = 95;
      config.seaProvFactor *= 0.5;
      config.landProvFactor *= 0.4;
      // now run the world generation
      fwg.generateWorld();
      // create eu4module and have it run the scenario generation
      Scenario::Eu4::Module eu4(rpdConf, configSubFolder, username);
      eu4.genEu4();
      dumpInfo("", configSubFolder);
      system("pause");
    }
  } catch (std::exception e) {
    Utils::Logging::logLine(e.what());
    dumpInfo(e.what(), configSubFolder);
    system("pause");
    return -1;
  }
  Utils::Logging::logLine("Done with the generation");
  return 0;
}
