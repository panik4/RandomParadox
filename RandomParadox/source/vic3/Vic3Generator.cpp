#include "vic3/Vic3Generator.h"
namespace Scenario::Vic3 {
using namespace Fwg;
using namespace Fwg::Gfx;

Generator::Generator() : Scenario::Generator() {}

Generator::Generator(const std::string &configSubFolder)
    : Scenario::Generator(configSubFolder) {
  this->terrainTypeToString.at(Fwg::Province::TerrainType::marsh) = "wetlands";
  this->terrainTypeToString.at(Fwg::Province::TerrainType::savannah) =
      "savanna";
  this->terrainTypeToString.at(Fwg::Province::TerrainType::tundra) = "snow";
  this->terrainTypeToString.at(Fwg::Province::TerrainType::arctic) = "snow";
}

void Generator::distributePops() {
  auto overallpopFactor =
      1000000.0 * worldPopulationFactor * (1.0 / Fwg::Cfg::Values().sizeFactor);
  for (auto &region : vic3Regions) {
    if (region->sea)
      continue;

    //// only init this when it hasn't been initialized via text input before
    if (region->totalPopulation < 0) {
      region->totalPopulation = static_cast<int>(region->populationFactor * overallpopFactor);
    }
    worldPop += (long long)region->totalPopulation;
  }
}
void Generator::totalArableLand(const std::vector<float> &arableLand) {
  const auto baseWorldArableSlots = 50000.0;
  auto totalArable = 0.0f;
  for (auto &val : arableLand) {
    totalArable += val;
  }
  for (auto &reg : vic3Regions) {
    auto resShare = 0.0;
    for (const auto &prov : reg->provinces) {
      for (const auto &pix : prov->pixels) {
        resShare += arableLand[pix];
      }
    }
    // basically fictive value from given input of how often this resource
    // appears
    auto stateArable = baseWorldArableSlots * (resShare / totalArable);
    reg->arableLand = stateArable;
  }
}
void Generator::totalResourceVal(const std::vector<double> &resPrev,
                                 double resourceModifier, ResourceType type) {
  const auto baseResourceAmount = 2500.0 * resourceModifier;
  auto totalRes = 0.0;
  for (auto &val : resPrev) {
    totalRes += val;
  }
  for (auto &reg : vic3Regions) {
    auto resShare = 0.0;
    for (const auto &prov : reg->provinces) {
      for (const auto &pix : prov->pixels) {
        resShare += resPrev[pix];
      }
    }
    // basically fictive value from given input of how often this resource
    // appears
    auto stateRes = baseResourceAmount * (resShare / totalRes);
    reg->resources.insert({type, stateRes});
  }
}

void Generator::distributeResources() {
  const auto &cfg = Fwg::Cfg::Values();

  // distribute arable land to states
  totalArableLand(climateData.arableLand);

  struct NoiseConfig {
    double fractalFrequency;
    double tanFactor;
    double cutOff;
    double mountainBonus;
  };
  NoiseConfig defaultNoise{0.02, 0.0, 0.8, 2.0};
  NoiseConfig semiRareNoise{0.015, 0.0, 0.85, 2.0};
  NoiseConfig rareLargePatch{0.005, 0.0, 0.7, 0.0};
  NoiseConfig rareNoise{0.01, 0.0, 0.9, 2.0};
  NoiseConfig agriNoise{0.24, 0.0, 0.0, 0.0};

  struct ResConfig {
    ResourceType resType;
    std::string name;
    double resourcePrevalence;
    bool random = false;
    NoiseConfig noiseConfig;
    bool considerClimate = false;
    std::map<Fwg::ClimateGeneration::Detail::ClimateTypeIndex, double>
        climateEffects;
    bool considerTrees = false;
    std::map<Fwg::ClimateGeneration::Detail::TreeType, double> treeEffects;
    bool considerSea = false;
    double oceanFactor = 0.0;
    double lakeFactor = 0.0;
  };
  using CTI = Fwg::ClimateGeneration::Detail::ClimateTypeIndex;
  // config for all resource types
  std::vector<ResConfig> resConfigs{
      {ResourceType::COAL, "Coal", 5.0, true, defaultNoise},
      {ResourceType::GOLDMINES, "Goldmines", 0.2, true, rareNoise},
      {ResourceType::GOLDFIELDS, "Goldfields", 0.2, true, rareNoise},
      {ResourceType::IRON, "Iron", 5.0, true, defaultNoise},
      {ResourceType::LEAD, "Lead", 2.0, true, semiRareNoise},
      {ResourceType::OIL, "Oil", 1.0, true, rareLargePatch},
      {ResourceType::SULFUR, "Sulfur", 2.0, true, semiRareNoise},
      {ResourceType::LOGGING,
       "Logging",
       10.0,
       false,
       agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 0.8},
        {CTI::COLDSEMIARID, 0.1},
        {CTI::HOTSEMIARID, 0.1},
        {CTI::CONTINENTALCOLD, 1.0},
        {CTI::CONTINENTALHOT, 1.0},
        {CTI::CONTINENTALWARM, 0.8},
        {CTI::TEMPERATECOLD, 1.0},
        {CTI::TEMPERATEWARM, 0.8},
        {CTI::TEMPERATEHOT, 0.6}}},
      {ResourceType::LIVESTOCK,
       "Livestock",
       10.0,
       false,
       agriNoise,
       true,
       {{CTI::POLARTUNDRA, 0.2},
        {CTI::TROPICSMONSOON, 0.7},
        {CTI::TROPICSRAINFOREST, 0.6},
        {CTI::TROPICSSAVANNA, 0.8},
        {CTI::COLDSEMIARID, 0.5},
        {CTI::HOTSEMIARID, 0.4},
        {CTI::CONTINENTALCOLD, 0.9},
        {CTI::CONTINENTALHOT, 0.9},
        {CTI::CONTINENTALWARM, 0.9},
        {CTI::TEMPERATECOLD, 1.0},
        {CTI::TEMPERATEWARM, 1.0},
        {CTI::TEMPERATEHOT, 1.0}}},
      {ResourceType::RUBBER,
       "Rubber",
       5.0,
       false,
       agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 0.5}}},
      {ResourceType::BANANA,
       "Banana",
       10.0,
       false,
       agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 1.0},
        {CTI::TROPICSSAVANNA, 0.9}}},
      {ResourceType::COFFEE,
       "Coffee",
       5.0,
       false,
       agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 1.0},
        {CTI::TROPICSSAVANNA, 1.0},
        {CTI::HOTSEMIARID, 1.0}}},
      {ResourceType::COTTON,
       "Cotton",
       10.0,
       false,
       agriNoise,
       true,
       {{CTI::TEMPERATEHOT, 0.8},
        {CTI::HOTSEMIARID, 1.0},
        {CTI::TROPICSMONSOON, 0.8},
        {CTI::TROPICSRAINFOREST, 0.4},
        {CTI::TROPICSSAVANNA, 0.4}}},
      {ResourceType::DYE,
       "Dye",
       5.0,
       false,
       agriNoise,
       true,
       {{CTI::HOTSEMIARID, 1.0},
        {CTI::TROPICSMONSOON, 0.8},
        {CTI::TROPICSRAINFOREST, 0.4},
        {CTI::TROPICSSAVANNA, 0.4}}},
      {ResourceType::OPIUM,
       "Opium",
       4.0,
       false,
       agriNoise,
       true,
       {{CTI::HOTSEMIARID, 0.8}}},
      {ResourceType::SILK,
       "Silk",
       5.0,
       false,
       agriNoise,
       true,
       {{CTI::TEMPERATEHOT, 1.0}}},
      {ResourceType::SUGAR,
       "Sugar",
       1.0,
       false,
       agriNoise,
       true,
       {{CTI::TEMPERATEHOT, 0.8},
        {CTI::HOTSEMIARID, 1.0},
        {CTI::TROPICSMONSOON, 0.8},
        {CTI::TROPICSRAINFOREST, 0.4},
        {CTI::TROPICSSAVANNA, 0.4}}},
      {ResourceType::TEA,
       "Tea",
       1.0,
       false,
       agriNoise,
       true,
       {{CTI::HOTSEMIARID, 1.0},
        {CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.4},
        {CTI::TROPICSSAVANNA, 0.6}}},
      {ResourceType::TOBACCO,
       "Tobacco",
       1.0,
       false,
       agriNoise,
       true,
       {{CTI::HOTSEMIARID, 1.0},
        {CTI::TROPICSMONSOON, 0.8},
        {CTI::TROPICSRAINFOREST, 0.4},
        {CTI::TROPICSSAVANNA, 0.4}}},
      {ResourceType::VINYARDS,
       "Vinyards",
       1.0,
       false,
       agriNoise,
       true,
       {{CTI::TEMPERATEHOT, 1.0}, {CTI::TEMPERATEWARM, 0.8}}},
      {ResourceType::WHEAT,
       "Wheat",
       1.0,
       false,
       agriNoise,
       true,
       {{CTI::CONTINENTALHOT, 1.0},
        {CTI::CONTINENTALWARM, 0.8},
        {CTI::TEMPERATEHOT, 1.0},
        {CTI::TEMPERATEWARM, 0.8},
        {CTI::HOTSEMIARID, 0.8}}},
      {ResourceType::MAIZE,
       "Maize",
       1.0,
       false,
       agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 1.0},
        {CTI::CONTINENTALHOT, 1.0},
        {CTI::CONTINENTALWARM, 0.8},
        {CTI::TEMPERATEHOT, 1.0},
        {CTI::TEMPERATEWARM, 0.8},
        {CTI::HOTSEMIARID, 0.8}}},
      {ResourceType::MILLET,
       "Millet",
       1.0,
       false,
       agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TROPICSSAVANNA, 1.0},
        {CTI::HOTSEMIARID, 0.8}}},
      {ResourceType::RICE,
       "Rice",
       1.0,
       false,
       agriNoise,
       true,
       {{CTI::TROPICSMONSOON, 1.0},
        {CTI::TROPICSRAINFOREST, 0.8},
        {CTI::TEMPERATEWARM, 0.8},
        {CTI::HOTSEMIARID, 0.8}}},
      {ResourceType::RYE,
       "Rye",
       1.0,
       false,
       agriNoise,
       true,
       {{CTI::CONTINENTALCOLD, 1.0},
        {CTI::CONTINENTALWARM, 0.8},
        {CTI::TEMPERATECOLD, 1.0},
        {CTI::COLDSEMIARID, 0.8}}},
      {ResourceType::FISH,
       "Fish",
       1.0,
       false,
       defaultNoise,
       false,
       {},
       false,
       {},
       true,
       1.0,
       1.0},
      {ResourceType::WHALING,
       "Whale",
       1.0,
       false,
       defaultNoise,
       false,
       {},
       false,
       {},
       true,
       1.0,
       0.0}};

  for (auto &resConfig : resConfigs) {
    std::vector<double> resPrev;
    if (resConfig.random) {
      resPrev = Fwg::Civilization::Resources::randomResourceLayer(
          resConfig.name, resConfig.noiseConfig.fractalFrequency,
          resConfig.noiseConfig.tanFactor, resConfig.noiseConfig.cutOff,
          resConfig.noiseConfig.mountainBonus);
    } else if (resConfig.considerSea) {
      resPrev = Fwg::Civilization::Resources::coastDependentLayer(
          resConfig.name, resConfig.oceanFactor, resConfig.lakeFactor,
          areas.provinces);
    } else {
      resPrev = Fwg::Civilization::Resources::climateDependentLayer(
          resConfig.name, resConfig.noiseConfig.fractalFrequency,
          resConfig.noiseConfig.tanFactor, resConfig.noiseConfig.cutOff,
          resConfig.noiseConfig.mountainBonus, resConfig.considerClimate,
          resConfig.climateEffects, resConfig.considerTrees,
          resConfig.treeEffects, climateData);
    }
    if (resPrev.size())
      totalResourceVal(resPrev, resConfig.resourcePrevalence,
                       resConfig.resType);
  }
}

void Generator::mapRegions() {
  Fwg::Utils::Logging::logLine("Mapping Regions");
  gameRegions.clear();
  vic3Regions.clear();

  for (auto &region : this->areas.regions) {
    std::sort(region.provinces.begin(), region.provinces.end(),
              [](const Fwg::Province *a, const Fwg::Province *b) {
                return (*a < *b);
              });
    auto gameRegion = std::make_shared<Region>(region);
    // generate random name for region
    gameRegion->name = NameGeneration::generateName(nData);

    for (auto &province : gameRegion->provinces) {
      gameRegion->gameProvinces.push_back(gameProvinces[province->ID]);
    }
    // save game region to generic module container and to hoi4 specific
    // container
    gameRegions.push_back(gameRegion);
    vic3Regions.push_back(gameRegion);
  }
  // sort by gameprovince ID
  std::sort(gameRegions.begin(), gameRegions.end(),
            [](auto l, auto r) { return *l < *r; });
  // check if we have the same amount of gameProvinces as FastWorldGen
  // provinces
  if (gameProvinces.size() != this->areas.provinces.size())
    throw(std::exception("Fatal: Lost provinces, terminating"));
  if (gameRegions.size() != this->areas.regions.size())
    throw(std::exception("Fatal: Lost regions, terminating"));
  for (const auto &gameRegion : gameRegions) {
    if (gameRegion->ID > gameRegions.size()) {
      throw(std::exception("Fatal: Invalid region IDs, terminating"));
    }
  }
  applyRegionInput();
}
// initialize states
void Generator::initializeStates() {}
void Generator::mapCountries() {
  for (auto &country : countries) {

    std::shared_ptr<Country> vic3 =
        std::reinterpret_pointer_cast<Vic3::Country, Scenario::Country>(country.second);
    for (auto &region : vic3->ownedRegions) {
      vic3->ownedVic3Regions.push_back(
          std::reinterpret_pointer_cast<Vic3::Region, Scenario::Region>(region));
    }
    vic3Countries.emplace(country.first, vic3);
  }
}
// set tech levels, give techs, count pops, cultures and religions, set
// diplomatic relations (e.g. puppets, markets, protectorates)
void Generator::initializeCountries() {
  auto &cfg = Fwg::Cfg::Values();
  // count pops
  for (auto &cEntry : vic3Countries) {
    auto &c = cEntry.second;
    auto totalPop = 0;
    auto averageDevelopment = 0.0;
    for (auto &state : c->ownedVic3Regions) {
      // to count total pop
      totalPop += state->totalPopulation;
    }
    c->pop = totalPop;
    for (auto &state : c->ownedVic3Regions) {
      // development should be weighed by the pop in the state
      averageDevelopment += state->development *
                            ((double)state->totalPopulation / (double)totalPop);
    }
    if (cfg.debugLevel > 5) {
      Fwg::Utils::Logging::logLine(c->tag, " has a population of ", c->pop);
    }
  }
}
} // namespace Scenario::Vic3