#include "hoi4/Hoi4Weather.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace Rpx::Hoi4 {
std::vector<WeatherExport> convertWeather(const Arda::SuperRegion &region,
    const std::map<std::string, double> &chances, double deviationFactor) {
  if (region.weatherMonths.size() != 12)
    throw std::runtime_error("Incomplete superregion weather");
  if (!std::isfinite(deviationFactor)) throw std::invalid_argument("Invalid weather deviation factor");
  std::vector<WeatherExport> result;
  result.reserve(12);
  for (const auto &month : region.weatherMonths) {
    if (!std::isfinite(month.temperatureCelsius) || !std::isfinite(month.deviation) || !std::isfinite(month.moisture))
      throw std::runtime_error("Invalid superregion weather");
    const double heat = std::clamp((month.temperatureCelsius + 40.0) / 80.0, 0.0, 1.0);
    WeatherExport weather{};
    weather.temperatureLow = month.temperatureCelsius;
    weather.temperatureHigh = month.temperatureCelsius + month.deviation * deviationFactor;
    weather.lightRain = chances.at("baseLightRainChance") * (1.0 - heat) * month.moisture;
    weather.heavyRain = chances.at("baseHeavyRainChance") * heat * month.moisture;
    weather.mud = chances.at("baseMudChance") * (2.0 * weather.heavyRain + weather.lightRain);
    weather.blizzard = std::clamp(chances.at("baseBlizzardChance") - heat, 0.0, 0.2) * month.moisture;
    weather.sandstorm = std::clamp((heat - 0.8) * chances.at("baseSandstormChance"), 0.0, 0.1) * std::clamp(0.2 - month.moisture, 0.0, 0.2);
    weather.snow = std::clamp(chances.at("baseSnowChance") - heat, 0.0, 0.2) * month.moisture;
    weather.clear = 1.0 - weather.lightRain - weather.heavyRain - weather.blizzard - weather.sandstorm - weather.snow;
    result.push_back(weather);
  }
  return result;
}
}