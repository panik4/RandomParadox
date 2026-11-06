#include "resources/ResourceLayer.h"
#include "hoi4/Hoi4StateGen.h"

namespace Rpx::Hoi4 {

void generateStateResources(Hoi4Config modConfig, Hoi4Data &modData,
                            Hoi4Stats &stats, Arda::ArdaConfig &ardaConfig,
                            Fwg::Areas::AreaData &areaData,
                            Fwg::Climate::ClimateData &climateData,
                            std::shared_ptr<Arda::ArdaGen> ardaGen) {
  Fwg::Utils::Logging::logLine("HOI4: Digging for resources");
  struct ResourceGenResult {
    const Arda::Utils::ResConfig *config;
    std::vector<float> layer;
  };
  Fwg::Utils::Randomisation::resetRandomisation();

  std::vector<std::future<ResourceGenResult>> futures;
  futures.reserve(modConfig.resConfigs.size());
  std::vector<int> seeds; seeds.reserve(modConfig.resConfigs.size());
  for (const auto &resConfig : modConfig.resConfigs) {
    seeds.push_back(RandNum::getRandom<int>());
  }

  for (const auto &resConfig : modConfig.resConfigs) {
    futures.emplace_back(std::async(
        std::launch::async,
        [&resConfig, &seeds, &modConfig, &areaData,
         &climateData]() -> ResourceGenResult {
          std::vector<float> resPrev;

          resPrev = Arda::Resources::generateResourceLayer(resConfig, seeds[&resConfig - &modConfig.resConfigs[0]], areaData, climateData);
          return {&resConfig, std::move(resPrev)};
        }));
  }
  std::vector<ResourceGenResult> results;
  for (auto &fut : futures) results.push_back(fut.get());
  for (auto &region : ardaGen->ardaData.areaData.regions) region->resources.clear();
  for (auto &result : results) {


    if (!result.layer.empty()) {
      const auto &resConfig = *result.config;

      ardaGen->totalResourceVal(
          result.layer,
          ardaConfig.resourceFactor * resConfig.resourcePrevalence *
              modConfig.resources.at(resConfig.name).at(0),
          resConfig);
    }
  }
}

void generateStateSpecifics(
    Hoi4Config modConfig, Hoi4Data &modData, Hoi4Stats &stats,
    Arda::ArdaConfig &ardaConfig, Fwg::Areas::AreaData &areaData,
    Fwg::Terrain::TerrainData &terrainData,
    std::vector<std::shared_ptr<Arda::ArdaRegion>> &ardaRegions,
    const Fwg::Gfx::Image &provinceMap) {
  Fwg::Utils::Logging::logLine("HOI4: Planning the economy");
  Fwg::Utils::Randomisation::resetRandomisation();
  auto &config = Fwg::Cfg::Values();
  // calculate the target industry amount
  if (!std::isfinite(ardaConfig.worldIndustryFactor) || ardaConfig.worldIndustryFactor < 0)
    throw std::invalid_argument("Invalid industry factor" );
  auto targetWorldIndustry = 2000 * ardaConfig.worldIndustryFactor;
  // we need a reference to determine how industrious a state is
  double averageEconomicActivity = areaData.landRegions > 0 ? 1.0 / areaData.landRegions : 1.0;

  stats.militaryIndustry = 0;
  stats.civilianIndustry = 0;
  stats.navalIndustry = 0;
  stats.totalWorldIndustry = 0;
  // cleanup work
  for (auto &hoi4State : modData.hoi4States) {
    hoi4State->navalBases.clear();
    hoi4State->buildings.clear();
    hoi4State->dockyards = 0;
    hoi4State->civilianFactories = 0;
    hoi4State->armsFactories = 0;
  }

  double maxImportance = 0;
  for (auto &state : modData.hoi4States) {
    state->resolvedPorts.clear();
    state->portsResolved = true;
  }
  std::set<int> seenPorts;
  for (const auto &source : modData.hoi4States) {
    for (const auto &location : source->locations) {
      if (location->type != Fwg::Civilization::LocationType::Port &&
          location->secondaryType != Fwg::Civilization::LocationType::Port) continue;
      if (!seenPorts.insert(location->ID).second) continue;
      const auto port = resolvePort(*location, provinceMap, areaData, terrainData.detailedHeightMap);
      if (!port) {
        Fwg::Utils::Logging::logLine("No ocean-adjacent site for port ", location->ID);
        continue;
      }
      for (auto &target : modData.hoi4States) if (target->ID == port->stateID) {
        target->resolvedPorts.push_back(*port);
        maxImportance = std::max(maxImportance, port->importance);
        break;
      }
    }
  }
  for (auto &state : modData.hoi4States)
    for (const auto &port : state->resolvedPorts)
      state->navalBases[port.provinceID] = std::max(state->navalBases[port.provinceID],
          static_cast<int>(std::clamp((maxImportance > 0 ? port.importance / maxImportance : 0) * 10, 1.0, 10.0)));
  Fwg::Utils::Logging::logLine(config.terrainConfig.landPercentage);
  for (auto &hoi4State : modData.hoi4States) {
    // skip sea and lake states
    if (!hoi4State->isLand())
      continue;
    if (hoi4State->topographyTypes.count(
            Arda::Civilization::TopographyType::WASTELAND)) {
      hoi4State->stateCategory = 0; // wasteland
      hoi4State->infrastructure = 0;
    } else {

      double ratio =
          hoi4State->worldEconomicActivityShare / averageEconomicActivity;
      double biased = std::pow(
          ratio, 0.6); // 0.6 flattens large values more than small ones
      hoi4State->stateCategory = std::clamp((int)(1.0 + 4.0 * biased), 0, 9);

      hoi4State->infrastructure =
          std::clamp((int)(1.0 +
                           (hoi4State->worldEconomicActivityShare /
                            averageEconomicActivity) *
                               0.5 +
                           3.0 * hoi4State->averageDevelopment),
                     1, 5);

      // one province state? Must be an island state
      if (hoi4State->ardaProvinces.size() == 1) {
        // if only one province, should be an island. Make it an island state,
        // if it isn't more developed
        hoi4State->stateCategory = std::max<int>(1, hoi4State->stateCategory);
      }

      double dockChance = 0.25;
      double civChance = 0.5;
      // distribute it to military, civilian and naval factories
      if (!hoi4State->isCoastalToOcean()) {
        dockChance = 0.0;
        civChance = 0.6;
      }

      // calculate total industry in this state
      if (targetWorldIndustry != 0) {
        auto stateIndustry = std::min<double>(
            hoi4State->worldEconomicActivityShare * targetWorldIndustry, 12.0);
        // if we're below one, randomize if this state gets a actory or not
        if (stateIndustry < 1.0) {
          stateIndustry =
              RandNum::getRandom(0.0, 1.0) < stateIndustry ? 1.0 : 0.0;
        }

        while (--stateIndustry >= 0) {
          auto choice = RandNum::getRandom(0.0, 1.0);
          if (choice < dockChance) {
            hoi4State->dockyards++;
          } else if (Fwg::Utils::Math::inRange(
                         dockChance, dockChance + civChance, choice)) {
            hoi4State->civilianFactories++;

          } else {
            hoi4State->armsFactories++;
          }
        }
      }
      stats.militaryIndustry += (int)hoi4State->armsFactories;
      stats.civilianIndustry += (int)hoi4State->civilianFactories;
      stats.navalIndustry += (int)hoi4State->dockyards;
    }
    // get potential building positions
    hoi4State->calculateBuildingPositions(terrainData,
                                          provinceMap, areaData);
  }
  stats.totalWorldIndustry =
      stats.militaryIndustry + stats.civilianIndustry + stats.navalIndustry;

  Arda::Areas::saveRegions(ardaRegions, Fwg::Cfg::Values().mapsPath + "areas/",
                           Arda::Gfx::visualiseRegions(ardaRegions));
}

} // namespace Rpx::Hoi4
