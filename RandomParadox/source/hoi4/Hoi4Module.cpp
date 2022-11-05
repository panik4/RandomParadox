#include "hoi4/Hoi4Module.h"
using namespace Fwg;
namespace Scenario::Hoi4 {
Hoi4Module::Hoi4Module(const boost::property_tree::ptree &gamesConf,
                       const std::string &configSubFolder,
                       const std::string &username, const bool editMode) {

  FastWorldGenerator fwg(configSubFolder);
  // read hoi configs and potentially overwrite settings for fwg
  readHoiConfig(configSubFolder, username, gamesConf);
  if (!editMode) {
    // now run the world generation
    fwg.generateWorld();
  }

  hoi4Gen = {fwg};
  if (editMode) {
    // start loading mod/game files
    readHoi();
  }
  hoi4Gen.nData = NameGeneration::prepare("resources\\names", gamePath);
}

Hoi4Module::~Hoi4Module() {}

bool Hoi4Module::createPaths() {
  // prepare folder structure
  try {
    // generic cleanup and path creation
    GenericModule::createPaths(gameModPath);
    using namespace std::filesystem;
    // map
    create_directory(gameModPath + "\\map\\supplyareas\\");
    create_directory(gameModPath + "\\map\\strategicregions\\");
    // gfx
    create_directory(gameModPath + "\\gfx\\flags\\small\\");
    create_directory(gameModPath + "\\gfx\\flags\\medium\\");
    // history
    create_directory(gameModPath + "\\history\\units\\");
    create_directory(gameModPath + "\\history\\states\\");
    create_directory(gameModPath + "\\history\\countries\\");
    // localisation
    create_directory(gameModPath + "\\localisation\\english\\");
    // common
    create_directory(gameModPath + "\\common\\national_focus\\");
    create_directory(gameModPath + "\\common\\countries\\");
    create_directory(gameModPath + "\\common\\bookmarks\\");
    create_directory(gameModPath + "\\common\\country_tags\\");
    //
    create_directory(gameModPath + "\\tutorial\\");
    return true;
  } catch (std::exception e) {
    std::string error =
        "Configured paths seem to be messed up, check Hoi4Module.json\n";
    error += "You can try fixing it yourself. Error is:\n ";
    error += e.what();
    throw(std::exception(error.c_str()));
    return false;
  }
}

// reads config for Hearts of Iron IV
void Hoi4Module::readHoiConfig(const std::string &configSubFolder,
                               const std::string &username,
                               const boost::property_tree::ptree &rpdConf) {
  Fwg::Utils::Logging::logLine("Reading Hoi4 Config");
  this->configurePaths(username, "Hearts of Iron IV", rpdConf);

  // now try to locate game files
  if (!findGame(gamePath, "Hearts of Iron IV")) {
    throw(std::exception("Could not locate the game. Exiting"));
  }
  // now try to locate game files
  if (!findModFolders()) {
    throw(std::exception("Could not locate the mod folders. Exiting"));
  }

  auto &config = Cfg::Values();
  namespace pt = boost::property_tree;
  pt::ptree hoi4Conf;
  try {
    // Read the basic settings
    std::ifstream f(configSubFolder + "//Hearts of Iron IVModule.json");
    std::stringstream buffer;
    if (!f.good())
      Fwg::Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();

    pt::read_json(buffer, hoi4Conf);
  } catch (std::exception e) {
    Fwg::Utils::Logging::logLine("Incorrect config \"RandomParadox.json\"");
    Fwg::Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                                 e.what());
    Fwg::Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
    system("pause");
  }
  // default values taken from base game
  hoi4Gen.resources = {
      {"aluminium",
       {hoi4Conf.get<double>("hoi4.aluminiumFactor"), 1169.0, 0.3}},
      {"chromium", {hoi4Conf.get<double>("hoi4.chromiumFactor"), 1250.0, 0.2}},
      {"oil", {hoi4Conf.get<double>("hoi4.oilFactor"), 1220.0, 0.1}},
      {"rubber", {hoi4Conf.get<double>("hoi4.rubberFactor"), 1029.0, 0.1}},
      {"steel", {hoi4Conf.get<double>("hoi4.steelFactor"), 2562.0, 0.5}},
      {"tungsten", {hoi4Conf.get<double>("hoi4.tungstenFactor"), 1188.0, 0.2}}};
  hoi4Gen.worldPopulationFactor =
      hoi4Conf.get<double>("scenario.worldPopulationFactor");
  hoi4Gen.industryFactor = hoi4Conf.get<double>("scenario.industryFactor");
  hoi4Gen.resourceFactor = hoi4Conf.get<double>("hoi4.resourceFactor");

  //  passed to generic ScenarioGenerator
  numCountries = hoi4Conf.get<int>("scenario.numCountries");
  config.loadMapsPath = hoi4Conf.get<std::string>("fastworldgen.loadMapsPath");
  config.heightmapIn = config.loadMapsPath +
                       hoi4Conf.get<std::string>("fastworldgen.heightMapName");
  cut = config.cut;
  // check if config settings are fine
  config.sanityCheck();
}

void Hoi4Module::genHoi() {
  if (!createPaths())
    return;
  try {
    // start with the generic stuff in the Scenario Generator
    hoi4Gen.mapProvinces();
    hoi4Gen.mapRegions();
    hoi4Gen.mapContinents();
    hoi4Gen.generateCountries(numCountries, gamePath);
    hoi4Gen.initializeCountries();
    hoi4Gen.evaluateNeighbours();
    hoi4Gen.generateWorld();
    Fwg::Gfx::Bitmap countryMap =
        hoi4Gen.dumpDebugCountrymap(Cfg::Values().mapsPath + "countries.bmp");

    // now generate hoi4 specific stuff
    hoi4Gen.generateCountrySpecifics();
    hoi4Gen.generateStateSpecifics(hoi4Gen.gameRegions.size());
    hoi4Gen.generateStateResources();
    hoi4Gen.generateStrategicRegions();
    hoi4Gen.generateWeather();
    hoi4Gen.evaluateCountries();
    hoi4Gen.generateLogistics(countryMap);
    NationalFocus::buildMaps();
    hoi4Gen.evaluateCountryGoals();
    hoi4Gen.generateCountryUnits();
  } catch (std::exception e) {
    std::string error = "Error while generating the Hoi4 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  // now start writing game files
  try {
    // generate map files. Format must be converted and colours mapped to hoi4
    // compatible colours
    Gfx::FormatConverter formatConverter(gamePath, "Hoi4");
    formatConverter.dump8BitTerrain(hoi4Gen.fwg.climateMap,
                                    gameModPath + "\\map\\terrain.bmp",
                                    "terrain", cut);
    formatConverter.dump8BitCities(hoi4Gen.fwg.climateMap,
                                   gameModPath + "\\map\\cities.bmp", "cities",
                                   cut);
    formatConverter.dump8BitRivers(
        hoi4Gen.fwg.riverMap, gameModPath + "\\map\\rivers.bmp", "rivers", cut);
    formatConverter.dump8BitTrees(hoi4Gen.fwg.climateMap, hoi4Gen.fwg.treeMap,
                                  gameModPath + "\\map\\trees.bmp", "trees",
                                  false);
    formatConverter.dump8BitHeightmap(hoi4Gen.fwg.heightMap,
                                      gameModPath + "\\map\\heightmap.bmp",
                                      "heightmap");
    formatConverter.dumpTerrainColourmap(
        hoi4Gen.fwg.summerMap, hoi4Gen.fwg.cityMap, gameModPath,
        "\\map\\terrain\\colormap_rgb_cityemissivemask_a.dds",
        DXGI_FORMAT_B8G8R8A8_UNORM, cut);
    formatConverter.dumpDDSFiles(
        hoi4Gen.fwg.riverMap, hoi4Gen.fwg.heightMap,
        gameModPath + "\\map\\terrain\\colormap_water_", cut, 8);
    formatConverter.dumpWorldNormal(
        hoi4Gen.fwg.sobelMap, gameModPath + "\\map\\world_normal.bmp", cut);

    using namespace Parsing::Writing;
    compatibilityHistory(gameModPath + "\\history\\countries\\", gamePath,
                         hoi4Gen.fwg.areas.regions);
    historyCountries(gameModPath + "\\history\\countries\\",
                     hoi4Gen.hoi4Countries);
    historyUnits(gameModPath + "\\history\\units\\", hoi4Gen.hoi4Countries);
    commonCountryTags(gameModPath + "\\common\\country_tags\\02_countries.txt",
                      hoi4Gen.hoi4Countries);
    commonCountries(gameModPath + "\\common\\countries\\",
                    gamePath + "\\common\\countries\\colors.txt",
                    hoi4Gen.hoi4Countries);
    adj(gameModPath + "\\map\\adjacencies.csv");
    airports(gameModPath + "\\map\\airports.txt", hoi4Gen.fwg.areas.regions);
    buildings(gameModPath + "\\map\\buildings.txt", hoi4Gen.hoi4States,
              hoi4Gen.fwg.heightMap);
    continents(gameModPath + "\\map\\continents.txt",
               hoi4Gen.fwg.areas.continents);
    definition(gameModPath + "\\map\\definition.csv", hoi4Gen.gameProvinces);
    unitStacks(gameModPath + "\\map\\unitstacks.txt",
               hoi4Gen.fwg.areas.provinces, hoi4Gen.fwg.heightMap);
    rocketSites(gameModPath + "\\map\\rocketsites.txt",
                hoi4Gen.fwg.areas.regions);
    strategicRegions(gameModPath + "\\map\\strategicregions",
                     hoi4Gen.fwg.areas.regions, hoi4Gen.strategicRegions);
    states(gameModPath + "\\history\\states", hoi4Gen.hoi4Countries);
    flags(gameModPath + "\\gfx\\flags\\", hoi4Gen.hoi4Countries);
    weatherPositions(gameModPath + "\\map\\weatherpositions.txt",
                     hoi4Gen.fwg.areas.regions, hoi4Gen.strategicRegions);
    adjacencyRules(gameModPath + "\\map\\adjacency_rules.txt");
    supply(gameModPath + "\\map\\", hoi4Gen.supplyNodeConnections);
    stateNames(gameModPath + "\\localisation\\english\\",
               hoi4Gen.hoi4Countries);
    countryNames(gameModPath + "\\localisation\\english\\",
                 hoi4Gen.hoi4Countries, hoi4Gen.nData);
    strategicRegionNames(gameModPath + "\\localisation\\english\\",
                         hoi4Gen.strategicRegions);
    foci(gameModPath + "\\common\\national_focus\\", hoi4Gen.hoi4Countries,
         hoi4Gen.nData);
    commonBookmarks(gameModPath + "\\common\\bookmarks\\",
                    hoi4Gen.hoi4Countries, hoi4Gen.strengthScores);
    tutorials(gameModPath + "\\tutorial\\tutorial.txt");
    Parsing::copyDescriptorFile("resources\\hoi4\\descriptor.mod", gameModPath,
                                gameModsDirectory, modName);

    // just copy over provinces.bmp, already in a compatible format
    Fwg::Gfx::Bmp::save(hoi4Gen.fwg.provinceMap,
                        (gameModPath + ("\\map\\provinces.bmp")).c_str());
  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  // now if everything worked, print info about world and pause for user to
  // see
  hoi4Gen.printStatistics();
}
void Hoi4Module::mapCountries(bool multiCore, bool stateExport,
                              const std::string &inputMap) {
  // prepare folder structure
  // using namespace std::filesystem;
  // try {
  //  remove_all(gameModPath);
  //  create_directory(gameModPath);
  //  // history
  //  create_directory(gameModPath + "\\history\\");
  //  // gfx
  //  create_directory(gameModPath + "\\history\\states\\");
  //} catch (std::exception e) {
  //  std::string error =
  //      "Configured paths seem to be messed up, check Hoi4Module.json\n";
  //  error += "You can try fixing it yourself. Error is:\n ";
  //  error += e.what();
  //  throw(std::exception(error.c_str()));
  //}

  // Parsing::copyDescriptorFile("resources\\hoi4\\descriptor-mapping.mod",
  //                             gameModPath, gameModsDirectory, modName);
}
void Hoi4Module::readHoi() {
  // read in game or mod files
  Hoi4::Parsing::Reading::readProvinces(gamePath, "provinces.bmp",
                                        hoi4Gen.fwg.areas,
                                        hoi4Gen.stringToTerrainType);
  // get the provinces into GameProvinces
  hoi4Gen.mapProvinces();
  // get the states from files to initialize gameRegions
  Hoi4::Parsing::Reading::readStates(gamePath, hoi4Gen);
  // read the colour codes from the game/mod files
  hoi4Gen.colourMap = Hoi4::Parsing::Reading::readColourMapping(gamePath);
  // now initialize hoi4 states from the gameRegions
  hoi4Gen.mapTerrain();
  for (auto &c : hoi4Gen.countries) {
    auto fCol = hoi4Gen.colourMap.valueSearch(c.first);
    if (fCol != Fwg::Gfx::Colour{0, 0, 0}) {
      c.second.colour = fCol;
    } else {
      do {
        // generate random colour as long as we have a duplicate
        c.second.colour = Fwg::Gfx::Colour(RandNum::getRandom(1, 254),
                                           RandNum::getRandom(1, 254),
                                           RandNum::getRandom(1, 254));
      } while (hoi4Gen.colourMap.find(c.second.colour));
      hoi4Gen.colourMap.setValue(c.second.colour, c.first);
    }
  }
  hoi4Gen.initializeCountries();
  // read in further state details from map files
  Hoi4::Parsing::Reading::readAirports(gamePath, hoi4Gen.hoi4States);
  Hoi4::Parsing::Reading::readRocketSites(gamePath, hoi4Gen.hoi4States);
  Hoi4::Parsing::Reading::readBuildings(gamePath, hoi4Gen.hoi4States);
  Hoi4::Parsing::Reading::readSupplyNodes(gamePath, hoi4Gen.hoi4States);
  Hoi4::Parsing::Reading::readWeatherPositions(gamePath, hoi4Gen.hoi4States);
}
void Hoi4Module::mapEdit() {
  // prepare folder structure
  using namespace std::filesystem;
  try {
    remove_all(gameModPath);
    create_directory(gameModPath);
    // history
    create_directory(gameModPath + "\\history\\");
    create_directory(gameModPath + "\\history\\states\\");
    // map
    create_directory(gameModPath + "\\map\\");
    // common
    create_directory(gameModPath + "\\common\\");
  } catch (std::exception e) {
    std::string error =
        "Configured paths seem to be messed up, check Hoi4Module.json\n";
    error += "You can try fixing it yourself. Error is:\n ";
    error += e.what();
    throw(std::exception(error.c_str()));
  }

  /* generate world from input heightmap
   * compare differences between heightmaps for edit mask
   *  merge all maps with mask, so rest stays the same
   * for province map, take notice which provinces are getting removed
   * replace province IDs by replaced provinces. Recalculate positions for these
   * provinces add these provinces to empty states
   *
   */
  Scenario::Hoi4::MapPainting::ChangeHolder changes;

  // in case we want to edit provinces
  if (true) {
    // first edit province.bmp, and update some relevant files

    auto &config = Fwg::Cfg::Values();
    config.mapsToEdit.insert("provinceMap");
    config.mapsToEdit.insert("stateMap");
    config.mapsToEdit.insert("countryMap");

    Scenario::Hoi4::MapPainting::Provinces::edit(
        mappingPath, gameModPath, "provinces.bmp", hoi4Gen, changes);
    // map them again
    hoi4Gen.mapProvinces();

    Scenario::Hoi4::MapPainting::States::updateStates(hoi4Gen, changes);

    // get the states from files to initialize gameRegions
    // Hoi4::Parsing::Reading::readStates(gamePath, hoi4Gen);
    hoi4Gen.initializeCountries();
    Scenario::Hoi4::MapPainting::States::edit(mappingPath, gameModPath,
                                              "states.bmp", hoi4Gen, changes);


    hoi4Gen.initializeCountries();


    Scenario::Hoi4::MapPainting::Countries::edit(
        mappingPath, gameModPath, "countries.bmp", hoi4Gen, changes);

    // finalize edits
    // get the new internal representation of the game state into mod files

    // update province related files
    Hoi4::Parsing::Writing::definition(gameModPath + "\\map\\definition.csv",
                                       hoi4Gen.gameProvinces);

    // update states according to previously generated (and potentially edited)
    // state map will automatically correct province assignment
    /* Must edit
     *   - airfields
     *   - rocketsites
     *   - buildings.txt
     *   - supply_nodes.txt
     *   - weatherpositions
     *
     *
     *
     */

    /* UPDATE references to province IDs
     *  Map files:
     *   - unitstacks.txt
     *   - definition.csv
     *   - strategic regions
     *
     * History files:
     *   -
     *
     *
     * Common files:
     *   - events
     *   - decisions
     *   - ...?
     *
     *
     *
     */
  }
}
} // namespace Scenario::Hoi4