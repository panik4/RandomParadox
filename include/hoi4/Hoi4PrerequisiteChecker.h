#pragma once
#include "utils/PrerequisiteChecker.h"

namespace Rpx::Hoi4 {
class Generator;
class PrerequisiteChecker {
public:
  static Fwg::PrerequisiteResult
  canDeriveCountriesFromSimulation(const Generator &);
  static Fwg::Prerequisite mappedStates(const Generator &);
  static Fwg::Prerequisite mappedCountries(const Generator &);
  static Fwg::Prerequisite stateEconomy(const Generator &);
  static Fwg::Prerequisite stateResources(const Generator &);
  static Fwg::Prerequisite countryDetails(const Generator &);
  static Fwg::PrerequisiteResult canGenerateStateEconomy(const Generator &);
  static Fwg::PrerequisiteResult canGenerateResources(const Generator &);
  static Fwg::PrerequisiteResult canGenerateStateData(const Generator &);
  static Fwg::PrerequisiteResult canGenerateCountryDetails(const Generator &);
  static Fwg::PrerequisiteResult canRecalculateMilitary(const Generator &);
  static Fwg::PrerequisiteResult canGenerateAppearance(const Generator &);
  static Fwg::PrerequisiteResult canGeneratePolitics(const Generator &);
  static Fwg::PrerequisiteResult canApplySimulationPolitics(const Generator &);
  static Fwg::PrerequisiteResult canRebuildFactions(const Generator &);
  static Fwg::PrerequisiteResult canGenerateWeather(const Generator &);
  static Fwg::PrerequisiteResult canFinalise(const Generator &);
  static Fwg::PrerequisiteResult canWriteTextFiles(const Generator &);
  static Fwg::PrerequisiteResult canWriteImages(const Generator &);
  static Fwg::PrerequisiteResult canExportMod(const Generator &);
  static Fwg::PrerequisiteResult canExportHeightmap(const Generator &);
  static Fwg::PrerequisiteResult canExportNormalMap(const Generator &);
  static Fwg::PrerequisiteResult canExportTerrain(const Generator &);
  static Fwg::PrerequisiteResult canExportProvinces(const Generator &);
  static Fwg::PrerequisiteResult canExportTrees(const Generator &);
  static Fwg::PrerequisiteResult canExportTerrainColourmap(const Generator &);
  static void require(const Fwg::PrerequisiteResult &,
                      std::string_view operation);
};
} // namespace Rpx::Hoi4
