#pragma once
#include "generic/ArdaGen.h"
#include "io/ResourceLoading.h"
#include "namegeneration/NameGenerator.h"
#include "rendering/Images.h"
#include <map>
namespace Scenario {
class StrategicRegion : public Arda::SuperRegion {
public:
  // weather: month{averageTemp, standard deviation, average precipitation,
  // tempLow, tempHigh, tempNightly, snowChance, lightRainChance,
  // heavyRainChance, blizzardChance,mudChance, sandstormChance}
  std::vector<std::vector<double>> weatherMonths;
};

class ModGenerator : public Arda::ArdaGen {

public:
  // vars - used for every game
  NameGeneration::NameData nData;
  std::vector<StrategicRegion> strategicRegions;
  ModGenerator();
  ModGenerator(const std::string &configSubFolder);
  ModGenerator(Arda::ArdaGen &scenGen);
  ~ModGenerator();

  // build strategic regions from ardaRegions
  void generateStrategicRegions();
  Fwg::Gfx::Bitmap visualiseStrategicRegions(const int ID = -1);

  // load countries from an image and map them to regions
  template <typename T>
  void loadCountries(const std::string &countryMapPath,
                     const std::string &mappingPath) {
    int counter = 0;
    countries.clear();
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
    Fwg::Utils::ColourTMap<std::vector<std::shared_ptr<Arda::ArdaRegion>>>
        mapOfRegions;
    for (auto &region : ardaRegions) {
      if (region->isSea() || region->isLake())
        continue;

      Fwg::Utils::ColourTMap<int> likeliestOwner;
      Fwg::Gfx::Colour selectedCol;

      for (auto province : region->ardaProvinces) {
        if (!province->baseProvince->isSea()) {
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
              selectedCol = potOwner.first;
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
      auto entryCol = entry.first;
      if (mapOfCountries.find(entryCol)) {
        auto tokens = mapOfCountries[entryCol];
        auto colour = Fwg::Gfx::Colour(
            std::stoi(tokens[0]), std::stoi(tokens[1]), std::stoi(tokens[2]));
        T country(tokens[3], counter++, tokens[4], tokens[5],
                  Arda::Gfx::Flag(82, 52));
        country.colour = colour;
        for (auto &region : entry.second) {
          country.addRegion(region);
        }
        countries.insert({country.tag, std::make_shared<T>(country)});
      } else {
        T country(std::to_string(counter), counter++, "", "",
                  Arda::Gfx::Flag(82, 52));
        country.colour = entry.first;
        for (auto &region : entry.second) {
          country.addRegion(region);
        }

        auto region = country.ownedRegions[0];
        countries.insert({country.tag, std::make_shared<T>(country)});
      }
    }
    for (auto &country : countries) {
      country.second->gatherCultureShares();
      auto culture = country.second->getPrimaryCulture();
      auto language = culture->language;
      // only generate name and tag if this country was not in the input
      // mappings
      if (!country.second->name.size()) {
        country.second->name = language->generateGenericCapitalizedWord();
        country.second->tag =
            NameGeneration::generateTag(country.second->name, nData);
      }
      country.second->adjective =
          language->getAdjectiveForm(country.second->name);
      for (auto &region : country.second->ownedRegions) {
        region->owner = country.second;
      }
      country.second->evaluatePopulations(civData.worldPopulationFactorSum);
      country.second->gatherCultureShares();
    }
  }
}; // namespace Scenario
} // namespace Scenario
