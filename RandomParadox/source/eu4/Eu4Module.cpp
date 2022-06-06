#include "eu4/Eu4Module.h"

Eu4Module::Eu4Module() {}

Eu4Module::~Eu4Module() {}

bool Eu4Module::createPaths() { // prepare folder structure
  try {
    // generic cleanup and path creation
    GenericModule::createPaths(gameModPath);
    // map

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

void Eu4Module::genEu4(bool useDefaultMap, bool useDefaultStates,
                       bool useDefaultProvinces, ScenarioGenerator &scenGen,
                       bool cut) {
  if (!createPaths())
    return;

  try {
    // start with the generic stuff in the Scenario Generator
    scenGen.mapRegions();
    scenGen.mapContinents();
    scenGen.generateCountries(numCountries);
    scenGen.evaluateNeighbours();
    scenGen.generateWorld();
    scenGen.dumpDebugCountrymap(Env::Instance().mapsPath + "countries.bmp");
  } catch (std::exception e) {
    std::string error = "Error while generating the Eu4 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  try {
    // generate map files. Format must be converted and colours mapped to eu4
    // compatible colours
    FormatConverter formatConverter(gamePath, "Eu4");
    formatConverter.dump8BitTerrain(gameModPath + "\\map\\terrain.bmp",
                                    "terrain", cut);
    formatConverter.dump8BitRivers(gameModPath + "\\map\\rivers.bmp", "rivers",
                                   cut);
    formatConverter.dump8BitTrees(gameModPath + "\\map\\trees.bmp", "trees",
                                  false);
    formatConverter.dump8BitHeightmap(gameModPath + "\\map\\heightmap.bmp",
                                      "heightmap");
    formatConverter.dumpTerrainColourmap(gameModPath,
                                         "\\map\\terrain\\colormap_spring.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, cut);
    formatConverter.dumpTerrainColourmap(gameModPath,
                                         "\\map\\terrain\\colormap_summer.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, cut);
    formatConverter.dumpTerrainColourmap(gameModPath,
                                         "\\map\\terrain\\colormap_autumn.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, cut);
    formatConverter.dumpTerrainColourmap(gameModPath,
                                         "\\map\\terrain\\colormap_winter.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, cut);
    formatConverter.dumpDDSFiles(
        gameModPath + "\\map\\terrain\\colormap_water_", cut, 2);
    formatConverter.dumpWorldNormal(gameModPath + "\\map\\world_normal.bmp",
                                    cut);

    // just copy over provinces.bmp, already in a compatible format
    Bitmap::SaveBMPToFile(Bitmap::findBitmapByKey("provinces"),
                          (gameModPath + ("\\map\\provinces.bmp")).c_str());

    // now do text
    Eu4Parser::writeDefaultMap(gameModPath + "\\map\\default.map",
                               scenGen.gameProvinces);
    Eu4Parser::dumpAdj(gameModPath + "\\map\\adjacency.csv", scenGen.gameProvinces);

  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
}

void Eu4Module::readEu4Config(std::string configSubFolder,
                              std::string username) {
  Logger::logLine("Reading Eu4 Config");
  const auto root =
      this->readConfig(configSubFolder, username, "Europa Universalis IV");

  // now try to locate game files
  if (!findGame(gamePath, "Europa Universalis IV")) {
    throw(std::exception("Could not locate the game. Exiting"));
  }
  // default values taken from base game
}
