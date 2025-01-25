#include "hoi4/goals/Selectors.h"
namespace Scenario::Hoi4::Selectors {
std::shared_ptr<Scenario::Hoi4::Region>
getRandomRegion(const Hoi4Country &country) {
  if (country.hoi4Regions.empty()) {
    return nullptr;
  }
  return Fwg::Utils::selectRandom(country.hoi4Regions);
}
std::shared_ptr<Scenario::Hoi4::Hoi4Country> getOpposingIdeologyNeighbour(
    const Hoi4Country &country,
    const std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
        &hoi4Countries) {
  const std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
      potentialTargets;
  for (auto &neighbour : country.neighbours) {
  }

  return nullptr;
}
std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
Scenario::Hoi4::Selectors::getOpposingIdeologyNeighbours(
    const std::shared_ptr<Scenario::Hoi4::Hoi4Country> country,
    const std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
        &hoi4Countries) {
  std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>> potentialTargets;
  for (auto &neighbour : country->neighbours) {
    // cast the neighbour to a std::shared_ptr<Scenario::Hoi4::Hoi4Country

    auto neighbourHoi4 =
        std::dynamic_pointer_cast<Scenario::Hoi4::Hoi4Country>(neighbour);

    if (neighbourHoi4->ideology != country->ideology) {
      potentialTargets.push_back(neighbourHoi4);
    }
  }
  return potentialTargets;
}
} // namespace Scenario::Hoi4::Selectors
