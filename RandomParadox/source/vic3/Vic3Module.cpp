#include "vic3/Vic3Module.h"
using namespace Fwg;

namespace Scenario::Vic3 {
Module::Module(const boost::property_tree::ptree &gamesConf,
               const std::string &configSubFolder,
               const std::string &username) {
  FastWorldGenerator fwg(configSubFolder);
  // read eu4 configs and potentially overwrite settings for fwg
  readVic3Config(configSubFolder, username, gamesConf);
  // now run the world generation
  fwg.generateWorld();
  vic3Gen = {fwg};
  vic3Gen.nData = NameGeneration::prepare("resources\\names", gamePath + "");
}

Module::~Module() {}

bool Module::createPaths() { // prepare folder structure
  try {
    using namespace std::filesystem;
    create_directory(gameModPath);
    // generic cleanup and path creation
    gameModPath += "\\game\\";
    std::cout << gameModPath << std::endl;
    remove_all(gameModPath + "\\map_data\\");
    remove_all(gameModPath + "\\common\\");
    remove_all(gameModPath + "\\localization\\");
    create_directory(gameModPath);
    create_directory(gameModPath + "\\map_data\\");
    create_directory(gameModPath + "\\map_data\\state_regions\\");
    create_directory(gameModPath + "\\common\\");
    create_directory(gameModPath + "\\localization\\");
    return true;
  } catch (std::exception e) {
    std::string error = "Configured paths seem to be messed up, check Europa "
                        "Universalis IVModule.json\n";
    error += "You can try fixing it yourself. Error is:\n ";
    error += e.what();
    throw(std::exception(error.c_str()));
    return false;
  }
}

void Module::readVic3Config(const std::string &configSubFolder,
                           const std::string &username,
                           const boost::property_tree::ptree &rpdConf) {
  Utils::Logging::logLine("Reading Vic 3 Config");
  this->configurePaths(username, "Victoria 3", rpdConf);

  // now try to locate game files
  if (!findGame(gamePath, "Victoria 3")) {
    throw(std::exception("Could not locate the game. Exiting"));
  }
  // now try to locate game files
  if (!findModFolders()) {
    throw(std::exception("Could not locate the mod folders. Exiting"));
  }
  auto &config = Cfg::Values();
  namespace pt = boost::property_tree;
  pt::ptree eu4Conf;
  try {
    // Read the basic settings
    std::ifstream f(configSubFolder + "//Victoria3Module.json");
    std::stringstream buffer;
    if (!f.good())
      Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();

    pt::read_json(buffer, eu4Conf);
  } catch (std::exception e) {
    Utils::Logging::logLine(
        "Incorrect config \"Europa Universalis IVModule.json\"");
    Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                            e.what());
    Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
    system("pause");
  }
  //  passed to generic ScenarioGenerator
  numCountries = eu4Conf.get<int>("scenario.numCountries");
  config.seaLevel = 95;
  config.seaProvFactor *= 0.7;
  config.landProvFactor *= 0.7;
  config.loadMapsPath = eu4Conf.get<std::string>("fastworldgen.loadMapsPath");
  config.heightmapIn = config.loadMapsPath +
                       eu4Conf.get<std::string>("fastworldgen.heightMapName");
  cut = config.cut;
  // check if config settings are fine
  config.sanityCheck();
}

void Module::genVic3() {
  if (!createPaths())
    return;

  try {
    // start with the generic stuff in the Scenario Generator
    vic3Gen.mapProvinces();
    vic3Gen.mapRegions();
    vic3Gen.mapContinents();
    vic3Gen.generateCountries(numCountries, gamePath);
    vic3Gen.evaluateNeighbours();
    vic3Gen.generateWorld();
    vic3Gen.dumpDebugCountrymap(Cfg::Values().mapsPath + "countries.bmp");
    vic3Gen.generateRegions(vic3Gen.gameRegions);
  } catch (std::exception e) {
    std::string error = "Error while generating the Vic3 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  try {
    // generate map files. Format must be converted and colours mapped to eu4
    // compatible colours
    Gfx::FormatConverter formatConverter(gamePath, "Vic3");


    using namespace Fwg::Gfx;
    // just copy over provinces.bmp, already in a compatible format
    Bmp::save(vic3Gen.fwg.provinceMap, gameModPath + "\\map\\provinces.bmp");
    {
      using namespace Parsing;
      // now do text

    }

  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
}

} // namespace Scenario::Eu4