#include "eu4/Eu4Module.h"
namespace Eu4 {
Module::Module() {}

Module::~Module() {}

bool Module::createPaths() { // prepare folder structure
  try {
    // generic cleanup and path creation
    GenericModule::createPaths(gameModPath);
    // map
    // history
    std::filesystem::create_directory(gameModPath + "\\history\\provinces\\");
    std::filesystem::create_directory(gameModPath +
                                      "\\common\\colonial_regions\\");
    std::filesystem::create_directory(gameModPath +
                                      "\\common\\trade_companies\\");

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

void Module::genEu4(Generator &eu4Gen, bool cut) {
  if (!createPaths())
    return;

  try {
    // start with the generic stuff in the Scenario Generator
    eu4Gen.mapRegions();
    eu4Gen.mapContinents();
    eu4Gen.generateCountries(numCountries);
    eu4Gen.evaluateNeighbours();
    eu4Gen.generateWorld();
    eu4Gen.dumpDebugCountrymap(Env::Instance().mapsPath + "countries.bmp");
    eu4Gen.generateRegions(eu4Gen.gameRegions);
  } catch (std::exception e) {
    std::string error = "Error while generating the Eu4 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  try {
    // generate map files. Format must be converted and colours mapped to eu4
    // compatible colours
    Graphics::FormatConverter formatConverter(gamePath, "Eu4");
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
    formatConverter.dumpDDSFiles(gameModPath + "\\map\\terrain\\colormap_water",
                                 cut, 2);
    formatConverter.dumpWorldNormal(gameModPath + "\\map\\world_normal.bmp",
                                    cut);

    // just copy over provinces.bmp, already in a compatible format
    Bitmap::SaveBMPToFile(Bitmap::findBitmapByKey("provinces"),
                          (gameModPath + ("\\map\\provinces.bmp")).c_str());
    {
      using namespace Eu4::Parsing;
      // now do text
      writeAdj(gameModPath + "\\map\\adjacencies.csv", eu4Gen.gameProvinces);
      writeAmbientObjects(gameModPath + "\\map\\ambient_object.txt",
                          eu4Gen.gameProvinces);
      Eu4::Parsing::writeAreas(gameModPath + "\\map\\area.txt",
                               eu4Gen.gameRegions, gamePath);
      writeColonialRegions(
          gameModPath + "\\common\\colonial_regions\\00_colonial_regions.txt",
          gamePath, eu4Gen.gameProvinces);
      writeClimate(gameModPath + "\\map\\climate.txt", eu4Gen.gameProvinces);
      writeContinent(gameModPath + "\\map\\continent.txt",
                     eu4Gen.gameProvinces);
      writeDefaultMap(gameModPath + "\\map\\default.map", eu4Gen.gameProvinces);
      writeDefinition(gameModPath + "\\map\\definition.csv",
                      eu4Gen.gameProvinces);
      writePositions(gameModPath + "\\map\\positions.txt",
                     eu4Gen.gameProvinces);
      writeRegions(gameModPath + "\\map\\region.txt", gamePath,
                   eu4Gen.getEu4Regions());
      writeSuperregion(gameModPath + "\\map\\superregion.txt", gamePath,
                       eu4Gen.gameRegions);
      writeTerrain(gameModPath + "\\map\\terrain.txt", eu4Gen.gameProvinces);
      writeTradeCompanies(
          gameModPath + "\\common\\trade_companies\\00_trade_companies.txt",
          gamePath, eu4Gen.gameProvinces);
      writeTradewinds(gameModPath + "\\map\\trade_winds.txt",
                      eu4Gen.gameProvinces);

      copyDescriptorFile("resources\\eu4\\descriptor.mod", gameModPath,
                         gameModsDirectory, modName);

      writeProvinces(gameModPath + "\\history\\provinces\\",
                     eu4Gen.gameProvinces, eu4Gen.gameRegions);
      writeLoc(gameModPath + "\\localisation\\", gamePath, eu4Gen.gameRegions,
               eu4Gen.gameProvinces, eu4Gen.getEu4Regions());
    }

  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
}

void Module::readEu4Config(std::string &configSubFolder,
                           std::string &username) {
  Logger::logLine("Reading Eu4 Config");
  const auto root =
      this->readConfig(configSubFolder, username, "Europa Universalis IV");

  // now try to locate game files
  if (!findGame(gamePath, "Europa Universalis IV")) {
    throw(std::exception("Could not locate the game. Exiting"));
  }
  // default values taken from base game
}
} // namespace Eu4