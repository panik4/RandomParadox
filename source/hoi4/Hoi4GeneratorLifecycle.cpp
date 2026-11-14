#include "hoi4/Hoi4Generator.h"
#include <algorithm>
#include <stdexcept>

namespace Rpx::Hoi4 {
void Generator::onGameEnvironmentInvalidated() {
  invalidateStateEconomyData();
  invalidateStateResourceData();
}
void Generator::invalidateGameLocationData() {
  economyDataChanged();
  modData.supplyNodeConnections.clear();
  modData.supplyHubs.clear();
}

void Generator::invalidateGameAreas() {
  modData = {};
  generationState = {};
  stats = {};
}

void Generator::invalidateGameSimulationData() {
  for (const auto &province : ardaData.areaData.provinces)
    if (province) {
      province->victoryPoint.reset();
      province->positions.clear();
    }
  // Mapped states are the existing geographic regions. Retain those objects
  // while dropping everything generated from the previous civilization.
  auto states = std::move(modData.hoi4States);
  modData = {};
  modData.hoi4States = std::move(states);
  generationState = {};
  stats = {};
  for (auto &state : modData.hoi4States) {
    if (!state)
      continue;
    state->armsFactories = state->civilianFactories = state->dockyards = 0;
    state->infrastructure = state->stateCategory = 0;
    state->airport = state->rocketsite = state->supplyNode = 0;
    state->totalVictoryPoints = 0.0;
    state->buildings.clear();
    state->resolvedPorts.clear();
    state->portsResolved = false;
    state->airfieldSuitability = -1;
    state->victoryPointsMap.clear();
    state->navalBases.clear();
    state->airBase.reset();
  }
}
void Generator::economyDataChanged() { invalidateStateEconomyData(); }

void Generator::invalidateResources() { invalidateStateResourceData(); }

void Generator::invalidateCountryDetails() { invalidateCountryMechanics(); }

bool Generator::hasMappedStateData() const {
  return hasMappedAreas() &&
         modData.hoi4States.size() == ardaData.areaData.regions.size() &&
         std::equal(modData.hoi4States.begin(), modData.hoi4States.end(),
                    ardaData.areaData.regions.begin(),
                    [](const auto &state, const auto &region) {
                      return state && state == region;
                    });
}
bool Generator::hasMappedCountryData() const {
  if (!hasMappedStateData() || modData.hoi4Countries.empty() ||
      modData.hoi4Countries.size() != ardaData.countries.size())
    return false;
  for (const auto &country : modData.hoi4Countries) {
    if (!country)
      return false;
    const auto found = ardaData.countries.find(country->tag);
    if (found == ardaData.countries.end() || found->second != country)
      return false;
    for (const auto &region : country->ownedRegions)
      if (!region || region->owner != country ||
          std::find(modData.hoi4States.begin(), modData.hoi4States.end(),
                    region) == modData.hoi4States.end())
        return false;
  }
  return true;
}
bool Generator::hasStateEconomyData() const {
  return generationState.economy == Fwg::Availability::Available &&
         hasMappedStateData();
}
bool Generator::hasStateResourcesData() const {
  return generationState.resources == Fwg::Availability::Available &&
         hasMappedStateData();
}
bool Generator::hasCountryDetailsData() const {
  return generationState.countries == Fwg::Availability::Available &&
         hasStateEconomyData() && hasMappedCountryData();
}
void Generator::restoreHoi4State(const Hoi4GenerationState &state) {
  for (const auto value : {state.economy, state.resources, state.countries})
    if (value != Fwg::Availability::Available &&
        value != Fwg::Availability::Unavailable)
      throw std::invalid_argument("Invalid saved HOI4 availability");
  generationState = state;
  refreshStateIndustryTotals();
}
void Generator::invalidateCountryMechanics() {
  clearCountryForces();
  modData.supplyNodeConnections.clear();
  modData.supplyHubs.clear();
  modData.decisionData = {};
  for (const auto &province : ardaData.areaData.provinces)
    if (province) {
      province->positions.clear();
      province->victoryPoint.reset();
    }
  for (const auto &state : modData.hoi4States) {
    if (!state)
      continue;
    state->victoryPointsMap.clear();
    state->totalVictoryPoints = 0;
  }
  for (const auto &country : modData.hoi4Countries) {
    if (!country)
      continue;
    country->hullTech.clear();
    country->navyTechs.clear();
    country->shipClasses.clear();
    country->infantryTechs.clear();
    country->armorTechs.clear();
    country->airTechs.clear();
    country->industryElectronicTechs.clear();
    country->civilianIndustry = country->armsFactories = country->dockyards =
        country->researchSlots = 0;
    country->victoryPoints = 0;
    country->focusTree.clear();
  }
}
void Generator::invalidateStateEconomyData() {
  generationState.economy = Fwg::Availability::Unavailable;
  invalidateCountryMechanics();
  stats.militaryIndustry = stats.civilianIndustry = stats.navalIndustry =
      stats.totalWorldIndustry = 0;
  for (const auto &state : modData.hoi4States) {
    if (!state)
      continue;
    state->armsFactories = state->civilianFactories = state->dockyards = 0;
    state->infrastructure = state->stateCategory = 0;
    state->airport = state->rocketsite = state->supplyNode = 0;
    state->buildings.clear();
    state->navalBases.clear();
    state->resolvedPorts.clear();
    state->portsResolved = false;
    state->airfieldSuitability = -1;
  }
}
void Generator::invalidateStateResourceData() {
  generationState.resources = Fwg::Availability::Unavailable;
  for (const auto &state : ardaData.areaData.regions)
    if (state)
      state->resources.clear();
  ardaStats.totalResources.clear();
}
void Generator::onResourceConfigurationChanged() {
  invalidateStateResourceData();
}
void Generator::onIndustryConfigurationChanged() {
  invalidateStateEconomyData();
}
void Generator::onMilitaryConfigurationChanged() {
  invalidateCountryMechanics();
}
void Generator::onStateEconomyEdited() {
  invalidateCountryMechanics();
  refreshStateIndustryTotals();
}
void Generator::refreshStateIndustryTotals() {
  stats.militaryIndustry = stats.civilianIndustry = stats.navalIndustry = 0;
  for (const auto &state : modData.hoi4States) {
    if (!state)
      continue;
    stats.militaryIndustry += state->armsFactories;
    stats.civilianIndustry += state->civilianFactories;
    stats.navalIndustry += state->dockyards;
  }
  stats.totalWorldIndustry =
      stats.militaryIndustry + stats.civilianIndustry + stats.navalIndustry;
}
void Generator::onStateResourcesEdited() { gatherStatistics(); }
void Generator::clearCountryForces() {
  generationState.countries = Fwg::Availability::Unavailable;
  modData.unitStacks.clear();
  for (const auto &state : modData.hoi4States)
    if (state)
      state->airBase.reset();
  for (const auto &country : modData.hoi4Countries) {
    if (!country)
      continue;
    country->ships.clear();
    country->fleets.clear();
    country->convoyAmount = 0;
    country->units.clear();
    country->unitCount.clear();
    country->divisions.clear();
    country->divisionTemplates.clear();
    country->tankVariants.clear();
    country->planeVariants.clear();
    country->airWings.clear();
    country->airBases.clear();
    country->totalNavyStrength = country->totalAirStrength = 0;
    country->totalArmyStrength = 0;
  }
}
void Generator::onCountryTerritoryEdited() {
  refreshCountryTerritoryData();
  invalidateCountryMechanics();
}
void Generator::onCountryIdentityEdited(Hoi4Country &country) {
  country.detailsInitialized = true;
}
void Generator::onCountryPoliticsEdited(Hoi4Country &country) {
  country.politicsInitialized = true;
}
} // namespace Rpx::Hoi4
