#include "hoi4/Hoi4Module.h"
using namespace Fwg;
namespace Rpx::Hoi4 {
Hoi4Module::Hoi4Module(const boost::property_tree::ptree &gamesConf,
                       const std::string &configSubFolder,
                       const std::string &username, const bool editMode) {
  generator =
      std::make_shared<Rpx::Hoi4::Generator>(configSubFolder, gamesConf);
  hoi4Gen = std::reinterpret_pointer_cast<Rpx::Hoi4::Generator, Arda::ArdaGen>(
      generator);
}

Hoi4Module::~Hoi4Module() {}


void Hoi4Module::prepareData() {}

// void Hoi4Module::modEdit(std::string &path) {
//   auto &config = Fwg::Cfg::Values();
//   path.append("//");
//   hoi4Gen->heightMap = Fwg::IO::Reader::readGenericImage(
//       path + "map//heightmap.bmp", Fwg::Cfg::Values());
//   if (hoi4Gen->heightMap.initialised()) {
//     hoi4Gen->genSobelMap(config);
//     hoi4Gen->genLand();
//     hoi4Gen->genLand();
//     hoi4Gen->genHumidity(config);
//     hoi4Gen->loadClimate(path + "map//terrain.bmp");
//   }
// }


void Hoi4Module::readHoi(std::string &path) {
  path.append("//");
  auto &config = Fwg::Cfg::Values();
  bool bufferedCut = config.cut;
  config.cut = false;
  auto heightmap = Fwg::IO::Reader::readGenericImage(
      path + "map//heightmap.bmp", config, false);
  hoi4Gen->loadHeight(config, heightmap);
  hoi4Gen->genSobelMap(config);
  hoi4Gen->genLand();
  hoi4Gen->loadClimate(config, path + "map//terrain.bmp");
  hoi4Gen->provinceMap =
      Fwg::IO::Reader::readGenericImage(path + "map//provinces.bmp", config);
  //// read in game or mod files
  hoi4Gen->climateData.habitabilities.resize(hoi4Gen->provinceMap.size());
  Hoi4::Parsing::Reading::readProvinces(hoi4Gen->terrainData,
                                        hoi4Gen->climateData, path,
                                        "provinces.bmp", hoi4Gen->areaData);
  hoi4Gen->wrapupProvinces(config);
  // get the provinces into ardaProvinces
  hoi4Gen->mapProvinces();
  // load existing states: we first get all the state files and parse their
  // provinces for land regions (including lakes) then we need to get the
  // strategic region files, and for every strategic region that is a sea state,
  // we create a sea region?
  //Hoi4::Parsing::Reading::readStates(path, hoi4Gen);

  // ensure continents are created via the details in definition.csv.
  // Which means we also need to load the existing continents file to match
  // those with each other, so another export does not overwrite the continents
  std::map<int, Areas::Continent> continents;
  for (auto &prov : hoi4Gen->areaData.provinces) {
    if (prov->continentID != -1) {
      if (continents.find(prov->continentID) == continents.end()) {
        Areas::Continent continent("", prov->continentID);
        continents.insert({prov->continentID, continent});
      } else {
        continents.at(prov->continentID).provinces.push_back(prov);
      }
    }
  }
  hoi4Gen->areaData.continents.clear();
  for (auto &c : continents) {
    hoi4Gen->areaData.continents.push_back(c.second);
  }

  // get the provinces into ardaProvinces
  // hoi4Gen->mapProvinces();
  // get the states from files to initialize ardaRegions
  // Hoi4::Parsing::Reading::readStates(gamePath, *hoi4Gen);
  // try {
  //  hoi4Gen->mapRegions();
  //} catch (std::exception e) {
  //  Fwg::Utils::Logging::logLine("Error while mapping regions, ", e.what());
  //};
  //// read the colour codes from the game/mod files
  // hoi4Gen->countryColourMap =
  //     Hoi4::Parsing::Reading::readColourMapping(pathcfg.gamePath);
  //// now initialize hoi4 states from the ardaRegions
  // hoi4Gen->mapTerrain();
  // for (auto &c : hoi4Gen->countries) {
  //   auto fCol = hoi4Gen->countryColourMap.valueSearch(c.first);
  //   if (fCol != Fwg::Gfx::Colour{0, 0, 0}) {
  //     c.second->colour = fCol;
  //   } else {
  //     do {
  //       // generate random colour as long as we have a duplicate
  //       c.second->colour = Fwg::Gfx::Colour(RandNum::getRandom(1, 254),
  //                                           RandNum::getRandom(1, 254),
  //                                           RandNum::getRandom(1, 254));
  //     } while (hoi4Gen->countryColourMap.find(c.second->colour));
  //     hoi4Gen->countryColourMap.setValue(c.second->colour, c.first);
  //   }
  // }
  // hoi4Gen->mapCountries();
  //// read in further state details from map files
  // Hoi4::Parsing::Reading::readAirports(pathcfg.gamePath,
  // hoi4Gen->hoi4States);
  // Hoi4::Parsing::Reading::readRocketSites(pathcfg.gamePath,
  //                                         hoi4Gen->hoi4States);
  // Hoi4::Parsing::Reading::readBuildings(pathcfg.gamePath,
  // hoi4Gen->hoi4States);
  // Hoi4::Parsing::Reading::readSupplyNodes(pathcfg.gamePath,
  //                                         hoi4Gen->hoi4States);
  // Hoi4::Parsing::Reading::readWeatherPositions(pathcfg.gamePath,
  //                                              hoi4Gen->hoi4States);
  config.cut = bufferedCut;
}


} // namespace Rpx::Hoi4