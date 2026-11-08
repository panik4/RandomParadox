#pragma once
#include "ArdaGen.h"
#include "FastWorldGenerator.h"
#include "hoi4/Hoi4DataStructures.h"
namespace Rpx::Hoi4 {
std::vector<std::shared_ptr<Fwg::Civilization::Location>>
selectSupplyLocations(const Arda::ArdaRegion &state);
std::vector<int> extractProvincesFromConnection(
    const Fwg::Civilization::Connection &connection,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces,
    const Fwg::Areas::AreaData &areas, Fwg::Gfx::Image &provinceMap);
void updateSupplyHubPositions(Hoi4Data &data,
                              const Fwg::Terrain::TerrainData &terrain,
                              const Fwg::Areas::AreaData &areas,
                              const Fwg::Gfx::Image &provinceMap);
// supply hubs and railroads
void generateLogistics(
    std::shared_ptr<Arda::ArdaGen> ardaGen, Hoi4Data &modData,
    const Fwg::Areas::AreaData &areaData,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &eligibleProvinces,
    Fwg::Gfx::Image &provinceMap, Fwg::Gfx::Image &countryMap);

} // namespace Rpx::Hoi4
