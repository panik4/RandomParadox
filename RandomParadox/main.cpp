#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "generic/ScenarioGenerator.h"
#include "hoi4/Hoi4Module.h"
#include <filesystem>
void dumpInfo(std::string error) {
  auto dump = ParserUtils::readFile("RandomParadox.json");
  dump += ParserUtils::readFile("Hoi4Module.json");
  dump += ParserUtils::readFile("FastWorldGenerator.json");
  dump += std::to_string(Data::getInstance().seed);
  dump += "\n";
  for (auto layerSeed : Data::getInstance().seeds) {
    dump += std::to_string(layerSeed);
    dump += "\n";
  }
  dump += error;
  dump += Logger::getFullLog();
  ParserUtils::writeFile("runDump.txt", dump);
}

int main() {
  // Short alias for this namespace
  namespace pt = boost::property_tree;
  // Create a root
  pt::ptree root;
  try {
    // Read the basic settings
    std::ifstream f("RandomParadox.json");
    std::stringstream buffer;
    if (!f.good())
      Logger::logLine("Config could not be loaded");
    buffer << f.rdbuf();

    pt::read_json(buffer, root);
  } catch (std::exception e) {
    Logger::logLine("Incorrect config \"RandomParadox.json\"");
    Logger::logLine("You can try fixing it yourself. Error is: ", e.what());
    Logger::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
    dumpInfo(e.what());
    system("pause");
    return -1;
  }

  bool useDefaultMap = false;
  bool useDefaultStates = false;
  bool useDefaultProvinces = false;
  bool writeMaps, genHoi4Scenario, useGlobalExistingHeightmap;
  std::string globalHeightMapPath;
  double latLow, latHigh;
  try {
    // if debug is enabled in the config, a directory subtree containing
    // visualisation of many maps will be created
    writeMaps = root.get<bool>("randomScenario.writeMaps");
    // generate hoi4 scenario or not
    genHoi4Scenario = root.get<bool>("randomScenario.genhoi4");
    // use the same input heightmap for every scenario/map generation
    useGlobalExistingHeightmap =
        root.get<bool>("randomScenario.inputheightmap");
    // get the path
    globalHeightMapPath = root.get<std::string>("randomScenario.heightmapPath");
    // read the configured latitude range. 0.0 = 90 degrees south, 2.0 = 90
    // degrees north
    latLow = root.get<double>("randomScenario.latitudeLow");
    latHigh = root.get<double>("randomScenario.latitudeHigh");
  } catch (std::exception e) {
    Logger::logLine("Error reading boost::property_tree");
    Logger::logLine("Did you rename a field in the json file?. Error is: ",
                     e.what());
    dumpInfo(e.what());
    system("pause");
    return -1;
  }

  // check if we can read the config
  try {
    Data::getInstance().getConfig("FastWorldGenerator.json");
  } catch (std::exception e) {
    Logger::logLine("Incorrect config \"FastWorldGenerator.json\"");
    Logger::logLine("You can try fixing it yourself. Error is: ", e.what());
    Logger::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or \"search for json validator\"");
    dumpInfo(e.what());
    system("pause");
    return -1;
  }
  // if we don't want the FastWorldGenerator output in MapsPath, debug = 0 turns
  // this off
  if (!writeMaps) {
    Data::getInstance().writeMaps = false;
  }
  /*try*/ {
    NameGenerator::prepare();
    FastWorldGenerator fastWorldGen;
    Hoi4Module hoi4Mod;

    hoi4Mod.readConfig();
    if (!useDefaultMap) {
      // if we configured to use an existing heightmap
      if (useGlobalExistingHeightmap) {
        // overwrite settings of fastworldgen
        Data::getInstance().heightmapIn = globalHeightMapPath;
        Data::getInstance().genHeight = false;
        Data::getInstance().latLow = latLow;
        Data::getInstance().latHigh = latHigh;
      }
      Data::getInstance().sanityCheck();
      // now run the world generation
      fastWorldGen.generateWorld();
    }
    // now start the generation of the scenario with the generated map files
    ScenarioGenerator sG(fastWorldGen);
    // and now check if we need to generate game specific files
    if (genHoi4Scenario)
      // generate hoi4 scenario
      hoi4Mod.genHoi(useDefaultMap, useDefaultStates, useDefaultProvinces, sG);
  }
  // catch (std::exception e) {
  //	Logger::logLine(e.what());
  //	dumpInfo(e.what());
  //	system("pause");
  //	return -1;
  // }
  Logger::logLine("Done with the generation");
  dumpInfo("");
  system("pause");
  return 0;
}
