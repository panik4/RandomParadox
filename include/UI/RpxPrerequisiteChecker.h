#pragma once
#include "ArdaUI/ArdaPrerequisiteChecker.h"
#include "generic/ModGenerator.h"
#include "hoi4/Hoi4Generator.h"
#include "vic3/Vic3Generator.h"

namespace Rpx::UI {

// Inherit Arda prerequisites and add RandomParadox-specific ones
class RpxPrerequisiteChecker : public Arda::UI::ArdaPrerequisiteChecker {
public:
  // =========================================================================
  // Path & Configuration Prerequisites
  // =========================================================================

  static Fwg::UI::Prerequisite pathsValidated(bool validatedPaths) {
    return {"Validated Paths", "Validate game and mod paths first",
            [=]() { return validatedPaths; }};
  }

  static Fwg::UI::Prerequisite scenarioConfigured(bool configuredScenarioGen) {
    return {"Scenario Configured", "Configure scenario generation first",
            [=]() { return configuredScenarioGen; }};
  }

  // =========================================================================
  // Module/Generator Prerequisites
  // =========================================================================

  static Fwg::UI::Prerequisite countryDetailsReady(bool requireCountryDetails) {
    return {"Country Details",
            "Generate country details (red button in Countries tab)",
            [=]() { return !requireCountryDetails; }};
  }

  // =========================================================================
  // HOI4-Specific Prerequisites
  // =========================================================================

  static Fwg::UI::Prerequisite
  hoi4StatesInitialised(const Rpx::Hoi4::Generator &generator) {
    return {"HOI4 States", "Generate state data first",
            [&]() { return generator.modData.statesInitialised; }};
  }

  static Fwg::UI::Prerequisite
  hoi4ExportReady(const Rpx::Hoi4::Generator &generator,
                  bool requireCountryDetails) {
    return {"HOI4 Export Ready",
            "Need strategic regions, initialized states, and country data",
            [&, requireCountryDetails]() {
              return !generator.superRegions.empty() &&
                     generator.provinceMap.size() > 0 &&
                     generator.modData.statesInitialised &&
                     !requireCountryDetails;
            }};
  }

  // =========================================================================
  // Victoria 3-Specific Prerequisites
  // =========================================================================

  static Fwg::UI::Prerequisite
  vic3ExportReady(const Rpx::Vic3::Generator &generator) {
    return {"Vic3 Export Ready", "Generate strategic regions first",
            [&]() { return !generator.superRegions.empty(); }};
  }

  // =========================================================================
  // Civilisation Tab Prerequisites (combines multiple checks)
  // =========================================================================

  static Fwg::UI::Prerequisite
  civilisationReady(bool configuredScenarioGen, bool redoDevelopment,
                    bool redoPopulation, bool redoTopography, bool redoCulture,
                    bool redoLocations) {
    return {"Civilisation Ready",
            "Complete all civilisation generation steps first", [=]() {
              return configuredScenarioGen && !redoDevelopment &&
                     !redoPopulation && !redoTopography && !redoCulture &&
                     !redoLocations;
            }};
  }

  // =========================================================================
  // Combined Game-Specific Prerequisites
  // =========================================================================

  // Helper to check if all civ data is complete (no redo flags set)
  static Fwg::UI::Prerequisite
  noCivRedosPending(bool redoDevelopment, bool redoPopulation,
                    bool redoTopography, bool redoCulture, bool redoLocations) {
    return {"Civilisation Data Complete",
            "Complete pending civilisation generation (check for orange tabs)",
            [=]() {
              return !redoDevelopment && !redoPopulation && !redoTopography &&
                     !redoCulture && !redoLocations;
            }};
  }
};

} // namespace Rpx::UI