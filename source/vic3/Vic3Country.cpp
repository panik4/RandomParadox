#include "vic3/Vic3Country.h"

namespace Rpx::Vic3 {
Country::Country() : Arda::Country() {}
Country::Country(std::string tag, int ID, std::string name,
                 std::string adjective, Arda::Gfx::Flag flag)
    : Arda::Country(tag, ID, name, adjective, flag) {}

Country::Country(Arda::Country &country,
                 std::vector<std::shared_ptr<Vic3::Region>> &regions) {
  for (auto &region : country.ownedRegions) {
    this->ownedVic3Regions.push_back(
        std::reinterpret_pointer_cast<Region>(region));
  }
}

Country::~Country() {}

std::vector<std::shared_ptr<Region>>
Country::getEligibleRegions(const std::string &resourceName) {
  return std::vector<std::shared_ptr<Region>>();
}

void Country::evaluateTechLevel(
    const std::map<std::string, TechnologyLevel> &techLevels) {
  auto techLevel =
      std::clamp(1.0 + (1.0 - this->averageDevelopment) * 6.0, 1.0, 7.0);
  this->techLevel = "effect_starting_technology_tier_" +
                    std::to_string((int)techLevel) + "_tech";
  if (techLevels.count(this->techLevel)) {
    for (const auto &tech : techLevels.at(this->techLevel).technologies) {
      if (this->techs.find(tech.name) == this->techs.end()) {
        Fwg::Utils::Logging::logLine("Warning: Tech ", tech.name,
                                     " doesn't exists in country ", this->name);
        continue;
      }
      this->techs[tech.name] = tech;
    }
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

} // namespace Rpx::Vic3