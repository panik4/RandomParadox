#pragma once
#include "areas/SuperRegion.h"
#include "utils/Archive.h"

namespace Rpx {
class StrategicRegion : public Arda::SuperRegion {
public:
  // weather: month{averageTemp, standard deviation, average precipitation,
  // tempLow, tempHigh, tempNightly, snowChance, lightRainChance,
  // heavyRainChance, blizzardChance,mudChance, sandstormChance}
  std::vector<std::vector<double>> weatherMonths;

  void serialise(Fwg::Utils::Serialisation::Archive &ar) override;
  void deserialise(Fwg::Utils::Serialisation::Archive &ar) override;
  uint32_t typeTag() const override;
};
} // namespace Rpx