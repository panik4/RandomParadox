#pragma once
#include "CultureGroup.h"
#include "FastWorldGenerator.h"
#include "NameGenerator.h"
#include "Religion.h"
#include "generic/GameRegion.h"

namespace Scenario::Civilization {

struct CivilizationData {
  NameGeneration::NameData nData;
  std::vector<std::shared_ptr<Religion>> religions;
  std::vector<std::shared_ptr<Culture>> cultures;
  std::vector<std::shared_ptr<CultureGroup>> cultureGroups;
  std::vector<std::shared_ptr<Scenario::LanguageGroup>> languageGroups;
  std::map<std::string, std::shared_ptr<Scenario::Language>> languages;
  double worldPopulationFactorSum = 0.0;
  double worldEconomicActivitySum = 0.0;
};
// generic preparations. However, if desired, there are necessary preparations
// for every game such as reading in the existing worldmap, states, regions,
// provinces etc
void generateWorldCivilizations(
    std::vector<std::shared_ptr<Region>> &regions,
    std::vector<std::shared_ptr<GameProvince>> &gameProvinces,
    CivilizationData &civData);
void generateReligions(
    CivilizationData &civData,
    std::vector<std::shared_ptr<GameProvince>> &gameProvinces);
void generateCultures(CivilizationData &civData,
                      std::vector<std::shared_ptr<Region>> &gameRegions);
void distributeLanguages(CivilizationData &civData);
// calculating amount of population in states
void generatePopulationFactors(CivilizationData &civData,
                               std::vector<std::shared_ptr<Region>> &regions);
// determine development from habitability, population density and randomness
void generateDevelopment(std::vector<std::shared_ptr<Region>> &regions);
// determine development from habitability, population density and randomness
void generateEconomicActivity(CivilizationData &civData,
                              std::vector<std::shared_ptr<Region>> &regions);
// determine importance from population, development and economicActivity
void generateImportance(std::vector<std::shared_ptr<Region>> &regions);
// after having generated cultures, generate names for the regions
void nameRegions(std::vector<std::shared_ptr<Region>> &regions);
} // namespace Scenario::Civilization
