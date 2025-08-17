#pragma once

namespace Rpx {
class StrategicRegion : public Arda::SuperRegion {
public:
  // weather: month{averageTemp, standard deviation, average precipitation,
  // tempLow, tempHigh, tempNightly, snowChance, lightRainChance,
  // heavyRainChance, blizzardChance,mudChance, sandstormChance}
  std::vector<std::vector<double>> weatherMonths;
};
} // namespace Rpx