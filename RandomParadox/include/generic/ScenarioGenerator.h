#pragma once
#include "CivilizationGeneration.h"
#include "Country.h"
#include "FastWorldGenerator.h"
#include "Flag.h"
#include "GameProvince.h"
#include "GameRegion.h"
#include "NameGenerator.h"
#include "ParserUtils.h"
#include "RandNum.h"
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
  std::map<int, std::vector<std::shared_ptr<Country>>> countryImportanceScores;
  Civilization::CivilizationData civData;
  // constructors/destructors
  Generator(Fwg::FastWorldGenerator &fwg);
  ~Generator();
  /* member functions*/
  // print a map showing all countries for debug purposes
  Fwg::Gfx::Bitmap visualiseCountries(Fwg::Gfx::Bitmap &countryBmp,
                                      const int ID = -1);
  // specific preparations. Used by each game, BUT to create game scenario
  void loadRequiredResources(const std::string &gamePath);

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
        Fwg::IO::Reader::readGenericImage(countryMapPath, Fwg::Cfg::Values());
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

        // auto name{NameGeneration::generateName(nData)};
        T pdoxC("", counter++, "", "", Gfx::Flag(82, 52));
        pdoxC.colour = mapOfRegions.getKeyColour(entry.first);
        for (auto &region : entry.second) {
          pdoxC.addRegion(region);
        }

        auto region = pdoxC.ownedRegions[0];
        pdoxC.tag = NameGeneration::generateTag(region->name, nData);
        pdoxC.evaluatePopulations();
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
      region->owner = nullptr;
    }
    auto &config = Fwg::Cfg::Values();
    Fwg::Utils::Logging::logLine("Generating Countries");

    for (auto i = 0; i < numCountries; i++) {

      T country(std::to_string(i), i, "DUMMY", "", Gfx::Flag(82, 52));


      countries.emplace(country.tag, std::make_shared<T>(country));
    }
    // assigns remaining regions to provinces
    //if (countries.size()) {
    //  for (auto &gameRegion : gameRegions) {
    //    if (!gameRegion->sea && !gameRegion->assigned) {
    //      auto gR = Fwg::Utils::getNearestAssignedLand(
    //          gameRegions, gameRegion, config.width, config.height);
    //     //gR->owner->addRegion(gameRegion);
    //    }
    //  }
    //}
    //for (auto &country : countries) {
    //  country.second->evaluatePopulations();
    //  country.second->gatherCultureShares();
    //}
    //visualiseCountries(countryMap);
    //Fwg::Gfx::Png::save(countryMap,
    //                    Fwg::Cfg::Values().mapsPath + "countries.png");
    distributeCountries();
  }

  void distributeCountries();
  // see which country neighbours which
  void evaluateCountryNeighbours();
  // calculate how strong each country is
  virtual void evaluateCountries();
  virtual void printStatistics();

  virtual void writeTextFiles();
  virtual void writeLocalisation();
  virtual void writeImages();

}; // namespace Scenario
} // namespace Scenario
