#pragma once
#include "FastWorldGenerator.h"
#include "ScenarioGenerator.h"
#include "generic/GameRegion.h"
#include "utils/Parsing.h"
#include <filesystem>
#include <string>

namespace Scenario {
template <typename T> void dumpRegions(const std::vector<T> &regions) {
  const auto &config = Fwg::Cfg::Values();
  std::string content = "";
  for (const auto &region : regions) {
    content += Fwg::Parsing::csvFormat(
        {std::to_string(region->colour.getRed()),
         std::to_string(region->colour.getGreen()),
         std::to_string(region->colour.getBlue()), region->name,
         std::to_string(region->totalPopulation)},
        ';', true);
  }
  Fwg::Parsing::writeFile(config.mapsPath + "states.txt", content);
}
}; // namespace Scenario
