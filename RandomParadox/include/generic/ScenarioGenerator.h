#pragma once
#include "Culture.h"
#include "FastWorldGenerator.h"
#include "Flag.h"
#include "GameProvince.h"
#include "GameRegion.h"
#include "NameGenerator.h"
#include "ScenarioContinent.h"
#include "Country.h"
#include "RandNum/RandNum.h"
#include "Religion.h"
#include "ResourceLoading.h"
#include "ParserUtils.h"
#include <map>
namespace Scenario {
struct strategicRegion {
  std::set<int> gameRegionIDs;
  // weather: month{averageTemp, standard deviation, average precipitation,
  // tempLow, tempHigh, tempNightly, snowChance, lightRainChance,
  // heavyRainChance, blizzardChance,mudChance, sandstormChance}
  std::vector<std::vector<double>> weatherMonths;
  std::string name;
};

class Generator : public Fwg::FastWorldGenerator {

protected:
  Fwg::Gfx::Bitmap typeMap;

public:
  Generator();
  Generator(const std::string &configSubFolder);
  // vars - used for every game
  NameGeneration::NameData nData;
  int numCountries;
  bool enableLoadCountries = false;
  std::string countryMappingPath = "";
  std::string regionMappingPath = "";
  bool interactive = false;
  // vars - track civil statistics
  long long worldPop = 0;
  // vars - config options
  double worldPopulationFactor = 1.0;
  double industryFactor = 1.0;
  double resourceFactor = 1.0;
  // containers - used for every game
  std::map<std::string, Fwg::Province::TerrainType> stringToTerrainType = {
      {"plains", Fwg::Province::TerrainType::grassland},
      {"forest", Fwg::Province::TerrainType::forest},
      {"ocean", Fwg::Province::TerrainType::ocean},
      {"mountain", Fwg::Province::TerrainType::mountains},
      {"marsh", Fwg::Province::TerrainType::marsh},
      {"rockyHills", Fwg::Province::TerrainType::hills},
      {"jungle", Fwg::Province::TerrainType::jungle},
      {"desert", Fwg::Province::TerrainType::desert},
      {"urban", Fwg::Province::TerrainType::urban},
      {"lakes", Fwg::Province::TerrainType::lake}};
  std::map<Fwg::Province::TerrainType, std::string> terrainTypeToString = {
      {Fwg::Province::TerrainType::grassland, "plains"},
      {Fwg::Province::TerrainType::savannah, "plains"},
      {Fwg::Province::TerrainType::peaks, "mountain"},
      {Fwg::Province::TerrainType::marsh, "marsh"},
      {Fwg::Province::TerrainType::forest, "forest"},
      {Fwg::Province::TerrainType::ocean, "ocean"},
      {Fwg::Province::TerrainType::mountains, "mountain"},
      {Fwg::Province::TerrainType::hills, "rockyHills"},
      {Fwg::Province::TerrainType::jungle, "jungle"},
      {Fwg::Province::TerrainType::desert, "desert"},
      {Fwg::Province::TerrainType::arctic, "ice"},
      {Fwg::Province::TerrainType::tundra, "plains"},
      {Fwg::Province::TerrainType::urban, "urban"},
      {Fwg::Province::TerrainType::lake, "lakes"}};

  std::vector<ScenarioContinent> scenContinents;
  Fwg::Utils::ColourTMap<std::shared_ptr<Region>> stateColours;
  std::vector<std::shared_ptr<Region>> gameRegions;
  std::vector<std::shared_ptr<GameProvince>> gameProvinces;
  std::set<std::string> tags;
  Fwg::Utils::ColourTMap<std::string> countryColourMap;
  std::map <std::string, std::shared_ptr<Country>> countries;
  Fwg::Gfx::Bitmap countryMap;
  std::vector<strategicRegion> strategicRegions;
  std::vector<std::shared_ptr<Religion>> religions;
  std::vector<std::shared_ptr<Culture>> cultures;
  // constructors/destructors
  Generator(Fwg::FastWorldGenerator &fwg);
  ~Generator();
  /* member functions*/
  // print a map showing all countries for debug purposes
  Fwg::Gfx::Bitmap dumpDebugCountrymap(const std::string &path);
  // specific preparations. Used by each game, BUT to create game scenario
  void loadRequiredResources(const std::string &gamePath);
  // generic preparations. However, if desired, there are necessary preparations
  // for every game such as reading in the existing worldmap, states, regions,
  // provinces etc
  void generateWorldCivilizations();
  // map base continents to generic paradox compatible game continents
  void mapContinents();
  // map base regions to generic paradox compatible game regions
  virtual void mapRegions();
  // apply values read from a file to override generated data
  void applyRegionInput();
  // build strategic regions from gameregions
  void generateStrategicRegions();
  // map base provinces to generic game regions
  void mapProvinces();
  // calculating amount of population in states
  void generatePopulations();

  void generateReligions();
  void generateCultures();

  // initialize states
  virtual void initializeStates();
  // initialize countries
  virtual void initializeCountries();
  // determine development from habitability, population density and randomness
  void generateDevelopment();
  // mapping terrain types of FastWorldGen to paradox compatible terrains
  Fwg::Gfx::Bitmap mapTerrain();
  // GameRegions are used for every single game,
  std::shared_ptr<Region> &findStartRegion();
  // load countries from an image and map them to regions
  void loadCountries(const std::string &countryMapPath, const std::string& mappingPath);
  // and countries are always created the same way
  template<typename T>
  void generateCountries() {
    countries.clear();
    for (auto &region : gameRegions) {
      region->assigned = false;
      region->religions.clear();
      region->cultures.clear();
    }
    auto &config = Fwg::Cfg::Values();
    Fwg::Utils::Logging::logLine("Generating Countries");
    // load tags from hoi4 that are used by the base game
    // do not use those to avoid conflicts
    if (this->enableLoadCountries) {
      // load countries
      loadCountries(config.loadMapsPath + "//countries.png",
                    this->countryMappingPath);
    } else {
      for (auto i = 0; i < numCountries; i++) {
        auto name{NameGeneration::generateName(nData)};
        T pdoxC(NameGeneration::generateTag(name, nData), i, name,
                      NameGeneration::generateAdjective(name, nData),
                      Gfx::Flag(82, 52));
        // randomly set development of countries
        pdoxC.developmentFactor = RandNum::getRandom(0.1, 1.0);
        countries.emplace(pdoxC.tag, std::make_shared<T>(pdoxC));
      }
      for (auto &pdoxCountry : countries) {
        auto startRegion(findStartRegion());
        if (startRegion->assigned || startRegion->sea)
          continue;
        pdoxCountry.second->assignRegions(6, gameRegions, startRegion,
                                          gameProvinces);
      }
      // assigns remaining regions to provinces
      if (countries.size()) {
        for (auto &gameRegion : gameRegions) {
          if (!gameRegion->sea && !gameRegion->assigned) {
            auto gR = Fwg::Utils::getNearestAssignedLand(
                gameRegions, gameRegion, config.width, config.height);
            countries.at(gR->owner)->addRegion(gameRegion);
          }
        }
      }
    }
    countryMap =
        dumpDebugCountrymap(Fwg::Cfg::Values().mapsPath + "countries.png");
  }
  // see which country neighbours which
  void evaluateNeighbours();
}; // namespace Scenario
} // namespace Scenario
