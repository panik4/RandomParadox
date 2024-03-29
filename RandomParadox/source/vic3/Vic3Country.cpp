#include "vic3/Vic3Country.h"

namespace Scenario::Vic3 {
Country::Country(std::string tag, int ID, std::string name,
                 std::string adjective, Gfx::Flag flag)
    : Scenario::Country(tag, ID, name, adjective, flag) {}

Country::~Country() {}

std::vector<std::shared_ptr<Region>>
Country::getEligibleRegions(const std::string &resourceName) {
  return std::vector<std::shared_ptr<Region>>();
}

void Country::evaluateTechLevel(
    const std::map<std::string, TechnologyLevel> &techLevels) {
  auto techLevel = std::clamp(1.0 + (1.0 - this->developmentFactor) * 6.0, 1.0, 7.0);
  this->techLevel = "effect_starting_technology_tier_" +
                    std::to_string((int)techLevel) + "_tech";
  for (const auto &tech : techLevels.at(this->techLevel).technologies) {
    this->techs[tech.name] = tech;
  }
}

bool Country::hasTech(const std::string &techName) const {
  return techs.find(techName) != techs.end();
}

bool Country::canUseProductionMethod(const Productionmethod &prodMethod) const {
  for (const auto &requiredTech : prodMethod.unlockingTechnologies) {
    if (!hasTech(requiredTech.first)) {
      return false;
    }
  }
  return true;
}

} // namespace Scenario::Vic3