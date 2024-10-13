#pragma once
#include "Country.h"
#include "Culture.h"
#include "FastWorldGenerator.h"
#include "Flag.h"
#include "GameProvince.h"
#include "GameRegion.h"
#include "NameGenerator.h"
#include "ParserUtils.h"
#include "RandNum/RandNum.h"
#include "Religion.h"
#include "ResourceLoading.h"
#include "ScenarioContinent.h"
#include "SuperRegion.h"
#include <map>
namespace Scenario {
class StrategicRegion : public SuperRegion {
public:
  // weather: month{averageTemp, standard deviation, average precipitation,
  // tempLow, tempHigh, tempNightly, snowChance, lightRainChance,
  // heavyRainChance, blizzardChance,mudChance, sandstormChance}
  std::vector<std::vector<double>> weatherMonths;
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
  std::string countryMappingPath = "";
  std::string regionMappingPath = "";
  // vars - track civil statistics
  long long worldPop = 0;
  double worldEconomicActivity = 0;
  // vars - config options
  double worldPopulationFactor = 1.0;
  double worldIndustryFactor = 1.0;
  double resourceFactor = 1.0;
  // containers - used for every game
  std::vector<ScenarioContinent> scenContinents;
  std::vector<std::shared_ptr<Region>> gameRegions;
  std::vector<std::shared_ptr<GameProvince>> gameProvinces;
  std::set<std::string> tags;
  Fwg::Utils::ColourTMap<std::string> countryColourMap;
  std::map<std::string, std::shared_ptr<Country>> countries;
  Fwg::Gfx::Bitmap countryMap;
  Fwg::Gfx::Bitmap stratRegionMap;
  std::vector<StrategicRegion> strategicRegions;
  std::vector<std::shared_ptr<Religion>> religions;
  std::vector<std::shared_ptr<Culture>> cultures;
  std::map<int, std::vector<std::string>> countryImportanceScores;
  // constructors/destructors
  Generator(Fwg::FastWorldGenerator &fwg);
  ~Generator();
  /* member functions*/
  // print a map showing all countries for debug purposes
  Fwg::Gfx::Bitmap visualiseCountries(Fwg::Gfx::Bitmap &countryBmp,
                                       const int ID = -1);
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
  Fwg::Gfx::Bitmap visualiseStrategicRegions(const int ID = -1);
  // map base provinces to generic game regions
  void mapProvinces();
  // calculating amount of population in states
  void generatePopulationFactors();
  // determine development from habitability, population density and randomness
  void generateDevelopment();
  // determine development from habitability, population density and randomness
  virtual void generateEconomicActivity();
  // determine importance from population, development and economicActivity
  void generateImportance();


  void generateReligions();
  void generateCultures();

  virtual void cutFromFiles(const std::string &gamePath);
  // initialize states
  virtual void initializeStates();
  // initialize countries
  virtual void mapCountries();
  // mapping terrain types of FastWorldGen to module compatible terrains, only
  // implemented for some modules
  virtual Fwg::Gfx::Bitmap mapTerrain();
  // GameRegions are used for every single game,
  std::shared_ptr<Region> &findStartRegion();
  // load countries from an image and map them to regions
  template <typename T>
  void loadCountries(const std::string &countryMapPath,
                     const std::string &mappingPath) {
    int counter = 0;
    std::vector<std::string> mappingFileLines;
    Fwg::Utils::ColourTMap<std::vector<std::string>> mapOfCountries;
    try {
      mappingFileLines = Fwg::Parsing::getLines(mappingPath);
      for (auto &line : mappingFileLines) {
        auto tokens = Fwg::Parsing::getTokens(line, ';');
        auto colour = Fwg::Gfx::Colour(
            std::stoi(tokens[0]), std::stoi(tokens[1]), std::stoi(tokens[2]));
        mapOfCountries.setValue(colour, tokens);
      }
    } catch (std::exception e) {
      Fwg::Utils::Logging::logLine(
          "Exception while parsing country input, ", e.what(),
          " continuing with randomly generated countries");
    }
    countryMap =
        Fwg::IO::Reader::loadAnySupported(countryMapPath, Fwg::Cfg::Values());
    Fwg::Utils::ColourTMap<std::vector<std::shared_ptr<Scenario::Region>>>
        mapOfRegions;
    for (auto &region : gameRegions) {
      if (region->sea)
        continue;

      Fwg::Utils::ColourTMap<int> likeliestOwner;
      Fwg::Gfx::Colour selectedCol;

      for (auto province : region->gameProvinces) {
        if (!province->baseProvince->sea) {
          //  we have the colour already
          auto colour = countryMap[province->baseProvince->pixels[0]];

          if (likeliestOwner.find(colour)) {
            likeliestOwner[colour] += province->baseProvince->pixels.size();

          } else {
            likeliestOwner.setValue(colour,
                                    province->baseProvince->pixels.size());
          }
          int max = 0;

          for (auto &potOwner : likeliestOwner.getMap()) {
            if (potOwner.second > max) {
              max = potOwner.second;
              selectedCol = likeliestOwner.getKeyColour(potOwner.first);
            }
          }
        }
      }
      if (mapOfRegions.find(selectedCol)) {
        mapOfRegions[selectedCol].push_back(region);
      } else {
        mapOfRegions.setValue(selectedCol, {region});
      }
    }
    for (auto &entry : mapOfRegions.getMap()) {
      auto entryCol = mapOfRegions.getKeyColour(entry.first);
      if (mapOfCountries.find(entryCol)) {
        auto tokens = mapOfCountries[entryCol];
        auto colour = Fwg::Gfx::Colour(
            std::stoi(tokens[0]), std::stoi(tokens[1]), std::stoi(tokens[2]));
        T pdoxC(tokens[3], counter++, tokens[4], tokens[5], Gfx::Flag(82, 52));
        pdoxC.colour = colour;
        for (auto &region : entry.second) {
          pdoxC.addRegion(region);
        }
        countries.emplace(pdoxC.tag, std::make_shared<T>(pdoxC));
        nData.tags.insert(pdoxC.tag);
      } else {

        auto name{NameGeneration::generateName(nData)};
        T pdoxC(NameGeneration::generateTag(name, nData), counter++, name,
                NameGeneration::generateAdjective(name, nData),
                Gfx::Flag(82, 52));
        pdoxC.colour = mapOfRegions.getKeyColour(entry.first);
        for (auto &region : entry.second) {
          pdoxC.addRegion(region);
        }
        countries.emplace(pdoxC.tag, std::make_shared<T>(pdoxC));
        nData.tags.insert(pdoxC.tag);
      }
    }
  }

  // and countries are always created the same way
  template <typename T> void generateCountries() {
    countries.clear();
    countryMap.clear();
    for (auto &region : gameRegions) {
      region->assigned = false;
      region->religions.clear();
      region->cultures.clear();
      region->owner = "";
    }
    auto &config = Fwg::Cfg::Values();
    Fwg::Utils::Logging::logLine("Generating Countries");
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

    visualiseCountries(countryMap);
    Fwg::Gfx::Png::save(countryMap,
                        Fwg::Cfg::Values().mapsPath + "countries.png");
  }
  // see which country neighbours which
  void evaluateCountryNeighbours();
  // calculate how strong each country is
  virtual void evaluateCountries();
  virtual void printStatistics();

}; // namespace Scenario
} // namespace Scenario
