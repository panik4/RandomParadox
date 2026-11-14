#include "hoi4/Hoi4PrerequisiteChecker.h"
#include "hoi4/Hoi4Generator.h"
#include "utils/ArdaPrerequisiteChecker.h"
#include <algorithm>
#include <stdexcept>

namespace Rpx::Hoi4 {
using Base = Fwg::PrerequisiteChecker;
using ArdaCheck = Arda::PrerequisiteChecker;
Fwg::PrerequisiteResult
PrerequisiteChecker::canDeriveCountriesFromSimulation(const Generator &g) {
  auto result = ArdaCheck::canDeriveCountries(g);
  if (const auto prerequisite = stateEconomy(g); !prerequisite.available)
    result.missing.push_back({prerequisite.name, prerequisite.message});
  return result;
}
Fwg::Prerequisite PrerequisiteChecker::mappedStates(const Generator &g) {
  return {"HOI4 States", "Map HOI4 states first", g.hasMappedStateData()};
}
Fwg::Prerequisite PrerequisiteChecker::mappedCountries(const Generator &g) {
  return {"HOI4 Countries", "Map HOI4 countries first",
          g.hasMappedCountryData()};
}
Fwg::Prerequisite PrerequisiteChecker::stateEconomy(const Generator &g) {
  return {"State Economy", "Generate state economy first",
          g.hasStateEconomyData()};
}
Fwg::Prerequisite PrerequisiteChecker::stateResources(const Generator &g) {
  return {"State Resources", "Generate state resources first",
          g.hasStateResourcesData()};
}
Fwg::Prerequisite PrerequisiteChecker::countryDetails(const Generator &g) {
  return {"Country Details", "Generate country details first",
          g.hasCountryDetailsData()};
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canGenerateStateEconomy(const Generator &g) {
  const auto &cfg = Fwg::Cfg::Values();
  return Base::check({mappedStates(g), ArdaCheck::economy(g),
                      ArdaCheck::locations(g), ArdaCheck::naturalFeatures(g),
                      Base::heightmap(g, cfg), Base::provinceImage(g, cfg),
                      Base::landforms(g, cfg)});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canGenerateResources(const Generator &g) {
  const auto &cfg = Fwg::Cfg::Values();
  return Base::check(
      {mappedStates(g), Base::climate(g, cfg), Base::forests(g, cfg)});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canGenerateStateData(const Generator &g) {
  auto result = canGenerateStateEconomy(g);
  for (const auto &entry : canGenerateResources(g).missing)
    if (std::none_of(
            result.missing.begin(), result.missing.end(),
            [&](const auto &existing) { return existing.name == entry.name; }))
      result.missing.push_back(entry);
  return result;
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canGenerateCountryDetails(const Generator &g) {
  // Country generation refreshes territory and typed country views before
  // consuming them. Membership validation belongs to that refresh.
  return Base::check({ArdaCheck::countries(g), stateEconomy(g)});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canRecalculateMilitary(const Generator &g) {
  return Base::check({countryDetails(g), stateResources(g)});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canGenerateAppearance(const Generator &g) {
  return Base::check({mappedCountries(g)});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canGeneratePolitics(const Generator &g) {
  return Base::check({mappedCountries(g)});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canApplySimulationPolitics(const Generator &g) {
  return Base::check(
      {mappedCountries(g),
       ArdaCheck::simulation(g),
       {"Simulation Polities", "Generate simulation polities first",
        !g.ardaData.simulationExport.polities.empty()}});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canRebuildFactions(const Generator &g) {
  return Base::check({mappedCountries(g), stateEconomy(g)});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canGenerateWeather(const Generator &g) {
  return Base::check({ArdaCheck::strategicRegions(g),
                      {"Province Weather", "Generate province weather first",
                       g.hasProvinceWeatherData(Fwg::Cfg::Values())}});
}
Fwg::PrerequisiteResult PrerequisiteChecker::canFinalise(const Generator &g) {
  const auto &cfg = Fwg::Cfg::Values();
  auto result = canGenerateWeather(g);
  const auto mechanics = Base::check(
      {countryDetails(g),
       stateResources(g),
       Base::provinceImage(g, cfg),
       Base::heightmap(g, cfg),
       {"HOI4 Export Readiness",
        "Complete country data and valid ownership/faction references first",
        g.exportReady()}});
  result.missing.insert(result.missing.end(), mechanics.missing.begin(),
                        mechanics.missing.end());
  return result;
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canWriteTextFiles(const Generator &g) {
  const auto &cfg = Fwg::Cfg::Values();
  return Base::check(
      {countryDetails(g),
       stateResources(g),
       ArdaCheck::strategicRegions(g),
       Base::climate(g, cfg),
       Base::forests(g, cfg),
       {"Province Weather", "Generate province weather first",
        g.hasProvinceWeatherData(cfg)},
       {"HOI4 Export Readiness",
        "Complete country data and valid ownership/faction references first",
        g.exportReady()}});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canWriteImages(const Generator &g) {
  const auto &cfg = Fwg::Cfg::Values();
  return Base::check({Base::heightmap(g, cfg),
                      Base::landforms(g, cfg),
                      ArdaCheck::naturalFeatures(g),
                      ArdaCheck::locations(g),
                      Base::shading(g, cfg),
                      Base::provinceImage(g, cfg),
                      Base::climate(g, cfg),
                      Base::forests(g, cfg),
                      Base::rivers(g, cfg),
                      Base::worldImage(g, cfg),
                      {"Climate Image", "Generate climate image first",
                       g.hasClimateImageData(cfg)}});
}
Fwg::PrerequisiteResult PrerequisiteChecker::canExportMod(const Generator &g) {
  auto result = canFinalise(g);
  for (const auto &entry : canWriteTextFiles(g).missing)
    if (std::none_of(
            result.missing.begin(), result.missing.end(),
            [&](const auto &existing) { return existing.name == entry.name; }))
      result.missing.push_back(entry);
  for (const auto &entry : canWriteImages(g).missing)
    if (std::none_of(
            result.missing.begin(), result.missing.end(),
            [&](const auto &existing) { return existing.name == entry.name; }))
      result.missing.push_back(entry);
  return result;
}
void PrerequisiteChecker::require(const Fwg::PrerequisiteResult &result,
                                  std::string_view operation) {
  if (!result.ready()) {
    result.logMissing(operation);
    throw std::runtime_error(std::string(operation) +
                             ": prerequisites not fulfilled");
  }
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canExportHeightmap(const Generator &g) {
  return Base::check({Base::heightmap(g, Fwg::Cfg::Values())});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canExportNormalMap(const Generator &g) {
  return Base::check({Base::shading(g, Fwg::Cfg::Values())});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canExportTerrain(const Generator &g) {
  const auto &cfg = Fwg::Cfg::Values();
  return Base::check({Base::landforms(g, cfg), Base::climate(g, cfg),
                      Base::forests(g, cfg), ArdaCheck::naturalFeatures(g),
                      ArdaCheck::locations(g)});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canExportProvinces(const Generator &g) {
  return Base::check({Base::provinceImage(g, Fwg::Cfg::Values())});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canExportTrees(const Generator &g) {
  return Base::check({Base::landforms(g, Fwg::Cfg::Values()),
                      Base::forests(g, Fwg::Cfg::Values())});
}
Fwg::PrerequisiteResult
PrerequisiteChecker::canExportTerrainColourmap(const Generator &g) {
  return Base::check({Base::worldImage(g, Fwg::Cfg::Values()),
                      ArdaCheck::naturalFeatures(g), ArdaCheck::locations(g)});
}
} // namespace Rpx::Hoi4
