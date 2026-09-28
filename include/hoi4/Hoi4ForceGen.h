#pragma once
#include "countries/Country.h"
#include "hoi4/Hoi4DataStructures.h"
#include "hoi4/Hoi4Navies.h"

namespace Rpx::Hoi4 {

void generateArmorVariants(Hoi4Config modConfig, Hoi4Data &modData,
                           Hoi4Stats &stats);
void generateAirVariants(Hoi4Config modConfig, Hoi4Data &modData,
                         Hoi4Stats &stats);
// determine unit composition, templates
void generateCountryUnits(Hoi4Config modConfig, Hoi4Data &modData,
                          Hoi4Stats &stats);
// determine unit composition, templates
void generateCountryNavies(
    Hoi4Config modConfig, Hoi4Data &modData, Hoi4Stats &stats,
    std::vector<std::shared_ptr<Arda::ArdaProvince>> &eligibleProvinces);
} // namespace Rpx::Hoi4