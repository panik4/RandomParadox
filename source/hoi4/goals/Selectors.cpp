#include "hoi4/goals/Selectors.h"
namespace Rpx::Hoi4::Selectors {
std::shared_ptr<Rpx::Hoi4::Region>
getRandomRegion(const std::shared_ptr<Rpx::Hoi4::Hoi4Country> &country) {
  if (country->hoi4Regions.empty()) {
    return nullptr;
  }
  return Fwg::Utils::selectRandom(country->hoi4Regions);
}
std::shared_ptr<Rpx::Hoi4::Region> getRandomCoastalRegion(
    const std::shared_ptr<Rpx::Hoi4::Hoi4Country> &country) {
  std::vector<std::shared_ptr<Rpx::Hoi4::Region>> coastalRegions;
  for (auto &region : country->hoi4Regions) {
    if (region->coastal) {
      coastalRegions.push_back(region);
    }
  }
  if (coastalRegions.empty()) {
    return nullptr;
  }
  return Fwg::Utils::selectRandom(coastalRegions);
}
std::shared_ptr<Rpx::Hoi4::Hoi4Country> getOpposingIdeologyNeighbour(
    const Hoi4Country &country,
    const std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>>
        &hoi4Countries) {
  const std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>>
      potentialTargets;

  return nullptr;
}
std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>>
Rpx::Hoi4::Selectors::getOpposingIdeologyNeighbours(
    const std::shared_ptr<Rpx::Hoi4::Hoi4Country> country,
    const std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>>
        &hoi4Countries) {
  std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>> potentialTargets;
  for (auto &neighbour : country->neighbours) {
    // cast the neighbour to a std::shared_ptr<Rpx::Hoi4::Hoi4Country

    auto neighbourHoi4 =
        std::dynamic_pointer_cast<Rpx::Hoi4::Hoi4Country>(neighbour);

    if (neighbourHoi4->ideology != country->ideology) {
      potentialTargets.push_back(neighbourHoi4);
    }
  }
  return potentialTargets;
}
std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>>
Rpx::Hoi4::Selectors::getOpposingIdeologyNeighboursNeighbours(
    const std::shared_ptr<Rpx::Hoi4::Hoi4Country> country,
    const std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>>
        &hoi4Countries) {
  auto closestOpposingNeighbors =
      getOpposingIdeologyNeighbours(country, hoi4Countries);
  std::set<std::shared_ptr<Rpx::Hoi4::Hoi4Country>>
      directOpposingNeighbours;
  for (auto &neighbour : closestOpposingNeighbors) {
    directOpposingNeighbours.insert(neighbour);
  }
  std::set<std::shared_ptr<Rpx::Hoi4::Hoi4Country>> potentialTargets;
  for (auto &directNeighbour : directOpposingNeighbours) {
    // cast the neighbour to a std::shared_ptr<Rpx::Hoi4::Hoi4Country
    auto directNeighbourHoi4 =
        std::dynamic_pointer_cast<Rpx::Hoi4::Hoi4Country>(directNeighbour);

    for (auto &indirectNeighbour : directNeighbourHoi4->neighbours) {
      // cast the neighbour to a std::shared_ptr<Rpx::Hoi4::Hoi4Country
      auto indirectNeighbourHoi4 =
          std::dynamic_pointer_cast<Rpx::Hoi4::Hoi4Country>(
              indirectNeighbour);
      // ensure we don't add the same country twice, as the closest neighbour
      // is already targeted by a wargoal
      if (!directOpposingNeighbours.contains(indirectNeighbourHoi4) &&
          indirectNeighbourHoi4->ideology != country->ideology) {
        potentialTargets.insert(indirectNeighbourHoi4);
      }
    }
  }
  // convert the set to a vector
  return std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>>(
      potentialTargets.begin(), potentialTargets.end());
}
} // namespace Rpx::Hoi4::Selectors
