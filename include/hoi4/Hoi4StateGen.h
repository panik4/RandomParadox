#pragma once
#include "ArdaGen.h"
#include "FastWorldGenerator.h"
#include "hoi4/Hoi4DataStructures.h"
namespace Rpx::Hoi4 {
// give resources to states
void generateStateResources(Hoi4Config modConfig, Arda::ArdaConfig &ardaConfig,
                            Fwg::Areas::AreaData &areaData,
                            Fwg::Climate::ClimateData &climateData,
                            std::shared_ptr<Arda::ArdaGen> ardaGen);
// industry, development, population, state category
void generateStateSpecifics(
    Hoi4Data &modData, Hoi4Stats &stats, Arda::ArdaConfig &ardaConfig,
    Fwg::Areas::AreaData &areaData, Fwg::Terrain::TerrainData &terrainData,
    std::vector<std::shared_ptr<Arda::ArdaRegion>> &ardaRegions,
    const Fwg::Gfx::Image &provinceMap);

} // namespace Rpx::Hoi4