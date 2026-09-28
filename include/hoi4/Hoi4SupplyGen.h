#pragma once
#include "ArdaGen.h"
#include "FastWorldGenerator.h"
#include "hoi4/Hoi4DataStructures.h"
namespace Rpx::Hoi4 {
// supply hubs and railroads
void generateLogistics(
    std::shared_ptr<Arda::ArdaGen> ardaGen, Hoi4Data &modData,
    const Fwg::Areas::AreaData &areaData,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &eligibleProvinces,
    Fwg::Gfx::Image &provinceMap, Fwg::Gfx::Image &countryMap);

} // namespace Rpx::Hoi4