#pragma once
#include "ArdaGen.h"
#include "FastWorldGenerator.h"
#include "hoi4/Hoi4Airforce.h"
#include "hoi4/Hoi4DataStructures.h"
#include "hoi4/Hoi4Tech.h"
#include "utils/SerialisationFwd.h"
#include <array>
#include <map>
#include <string>
#include <vector>
namespace Rpx::Hoi4 {

// generate tech levels
void generateTechLevels(Hoi4Data &modData);

} // namespace Rpx::Hoi4