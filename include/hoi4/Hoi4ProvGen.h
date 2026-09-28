#pragma once
#include "ArdaGen.h"
#include "FastWorldGenerator.h"
#include "hoi4/Hoi4DataStructures.h"
namespace Rpx::Hoi4 {

void generatePositions(
    Fwg::Terrain::TerrainData &terrainData,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &ardaProvinces);

} // namespace Rpx::Hoi4