#pragma once
#include "generic/StrategicRegion.h"
#include <array>
namespace Rpx::Hoi4 {
struct WeatherExport {
  double temperatureLow, temperatureHigh, lightRain, heavyRain, mud, blizzard,
      sandstorm, snow, clear;
  std::array<double, 9> values() const {
    return {temperatureLow, temperatureHigh, lightRain, heavyRain, mud,
            blizzard,       sandstorm,       snow,      clear};
  }
};
std::vector<WeatherExport>
convertWeather(const Arda::SuperRegion &region,
               const std::map<std::string, double> &chances,
               double deviationFactor);
} // namespace Rpx::Hoi4