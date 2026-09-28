#pragma once
#include "countries/Country.h"
#include "hoi4/Hoi4Armor.h"
#include "hoi4/Hoi4Army.h"
#include "hoi4/Hoi4DataStructures.h"
#include "hoi4/Hoi4FocusGen.h"
#include "hoi4/Hoi4Navies.h"
#include "hoi4/Hoi4Utils.h"
#include <array>
#include <string>
#include <vector>

namespace Rpx::Hoi4 {
Arda::Utils::Ideology
ideologyFromGovernment(Arda::Simulation::GovernmentForm government);
void setSimulationParties(
    Rpx::Hoi4::Hoi4Country &country,
    const Arda::Simulation::SimulationPolityExport &polityExport);
void assignStartingLaws(Hoi4Country &country);
// determine the total amount of VPs per country, and distribute them in a
// country
void distributeVictoryPoints(
    Hoi4Data &modData, Hoi4Stats &stats,
    std::vector<std::shared_ptr<Arda::ArdaProvince>> &ardaProvinces);
// generate characters
void generateCharacters(Hoi4Data &modData);
void generateFocusTrees(Hoi4Data &modData);
void generateRandomDecisions(
    Hoi4Data &modData,
    std::vector<std::shared_ptr<Arda::ArdaRegion>> &ardaRegions);
void evaluateCountryStrength(Hoi4Data &modData, Hoi4Stats &stats,
                             Arda::ArdaData &ardaData,
                             Arda::ArdaConfig &ardaConfig);

} // namespace Rpx::Hoi4
