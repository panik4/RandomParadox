#pragma once
#include "FastWorldGenerator.h"
#include <numbers>
#include <optional>

namespace Rpx::Hoi4 {
inline double buildingRotation(double east, double north) {
  return std::remainder(-std::numbers::pi / 2 - std::atan2(east, north),
                        2 * std::numbers::pi);
}
inline double randomBuildingRotation() {
  return RandNum::getRandom(-std::numbers::pi, std::numbers::pi);
}
struct ResolvedPort {
  int locationID, pixel, provinceID, stateID, seaProvinceID;
  double importance;
};
// Score the stored heading as well as candidate headings without moving a site.
template<class Contains>
double airfieldFootprintScore(int centerX, int centerY, double angle,
    int length, int width, const Fwg::Gfx::Image &map,
    const std::vector<float> &heights, Contains contains) {
  double low = std::numeric_limits<double>::max(), high = -low, roughness = 0;
  auto sample = [&](int along, int across) {
    int x = static_cast<int>(std::lround(centerX + along * std::sin(angle) + across * std::cos(angle)));
    const int y = static_cast<int>(std::lround(centerY + along * std::cos(angle) - across * std::sin(angle)));
    x = (x % map.width() + map.width()) % map.width();
    return y < 0 || y >= map.height() ? -1 : y * map.width() + x;
  };
  for (int a = -length / 2; a <= length / 2; ++a)
    for (int b = -width / 2; b <= width / 2; ++b) {
      const int p = sample(a, b);
      if (p < 0 || !contains(p) || !std::isfinite(heights.at(p))) return -1;
      const double h = heights[p];
      low = std::min(low, h); high = std::max(high, h);
      if (a > -length / 2) roughness += std::abs(h - heights[sample(a - 1, b)]);
      if (b > -width / 2) roughness += std::abs(h - heights[sample(a, b - 1)]);
    }
  return (length * width / 85.0) / (1 + high - low + roughness / (length * width));
}
inline std::vector<int> buildingNeighbors(int pixel, int width, int height) {
  const int x = pixel % width, y = pixel / width;
  std::vector<int> result{y * width + (x + 1) % width,
                          y * width + (x + width - 1) % width};
  if (y) result.push_back(pixel - width);
  if (y + 1 < height) result.push_back(pixel + width);
  return result;
}
inline std::optional<ResolvedPort> resolvePort(
    const Fwg::Civilization::Location &location, const Fwg::Gfx::Image &map,
    const Fwg::Areas::AreaData &areas, const std::vector<float> &heights) {
  auto resolve = [&](int pixel) -> std::optional<ResolvedPort> {
    if (pixel < 0 || pixel >= map.size()) return std::nullopt;
    const auto province = areas.provinceColourMap.at(map[pixel]);
    if (!province || !province->isLand()) return std::nullopt;
    int sea = -1;
    for (int n : buildingNeighbors(pixel, map.width(), map.height())) {
      auto other = areas.provinceColourMap.at(map[n]);
      if (other && other->isSea() && (sea < 0 || other->ID == location.portExitProvinceID)) sea = other->ID;
    }
    if (sea < 0) return std::nullopt;
    return ResolvedPort{location.ID, pixel, province->ID, province->regionID,
                        sea, std::isfinite(location.importance) ? std::max(0.0, location.importance) : 0.0};
  };
  if (auto port = resolve(location.portLocationPixel)) return port;
  std::optional<ResolvedPort> best;
  double bestSlope = std::numeric_limits<double>::max();
  for (const auto &province : areas.provinces) {
    if (province->ID != location.provinceID || !province->isLand()) continue;
    for (int pixel : province->getNonOwningPixelView()) {
      auto port = resolve(pixel);
      if (!port) continue;
      double slope = 0;
      for (int n : buildingNeighbors(pixel, map.width(), map.height()))
        if (areas.provinceColourMap.at(map[n])->isLand())
          slope = std::max(slope, double(std::abs(heights.at(pixel) - heights.at(n))));
      if (!best || slope < bestSlope || (slope == bestSlope && pixel < best->pixel)) {
        best = port; bestSlope = slope;
      }
    }
  }
  return best;
}
} // namespace Rpx::Hoi4
