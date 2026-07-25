#pragma once
#include "areas/SuperRegion.h"
#include "utils/SerialisationFwd.h"

namespace Rpx {
class StrategicRegion : public Arda::SuperRegion {
public:
  // weather: month{averageTemp, standard deviation, average precipitation,
  // tempLow, tempHigh, tempNightly, snowChance, lightRainChance,
  // heavyRainChance, blizzardChance,mudChance, sandstormChance}
  std::vector<std::vector<double>> weatherMonths;

  template<class Archive>
  void serialize(Archive &ar, const unsigned int /*version*/);
  void forceStrategicRegionLink();
};
} // namespace Rpx

BOOST_CLASS_EXPORT_KEY(Rpx::StrategicRegion)
