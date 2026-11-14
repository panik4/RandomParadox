#pragma once
#include "UI/Prerequisites.h"
#include "generic/ModGenerator.h"
#include "hoi4/Hoi4Generator.h"
#include "utils/ArdaPrerequisiteChecker.h"
#include "vic3/Vic3Generator.h"

namespace Rpx::UI {

// Game-specific prerequisites retained until the RandomParadox migration.
class RpxPrerequisiteChecker : public Fwg::UI::PrerequisiteChecker {
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
  // Victoria 3-Specific Prerequisites
  // =========================================================================

  static Fwg::UI::Prerequisite
  vic3ExportReady(const Rpx::Vic3::Generator &generator) {
    return {"Vic3 Export Ready", "Generate strategic regions first", [&]() {
              return Arda::PrerequisiteChecker::strategicRegions(generator)
                  .available;
            }};
  }

  // =========================================================================
  // Civilisation Tab Prerequisites (combines multiple checks)
  // =========================================================================

  static Fwg::UI::Prerequisite
  civilisationReady(bool configuredScenarioGen,
                    const Arda::ArdaGen &generator) {
    return {
        "Civilisation Ready",
        "Complete all civilisation generation steps first",
        [&generator, configuredScenarioGen]() {
          return configuredScenarioGen &&
                 Arda::PrerequisiteChecker::civilisation(generator).available;
        }};
  }

  // =========================================================================
  // Combined Game-Specific Prerequisites
  // =========================================================================

  // Generator-owned civilisation availability.
  static Fwg::UI::Prerequisite
  civilisationData(const Arda::ArdaGen &generator) {
    return {
        "Civilisation Data Complete",
        "Complete pending civilisation generation (check for orange tabs)",
        [&generator]() {
          return Arda::PrerequisiteChecker::civilisation(generator).available;
        }};
  }
};

} // namespace Rpx::UI
