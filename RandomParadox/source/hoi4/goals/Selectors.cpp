
#include "hoi4/goals/Selectors.h"
namespace Scenario::Hoi4::Selectors {
std::shared_ptr<Scenario::Hoi4::Region>
getRandomRegion(const Hoi4Country &country) {
  if (country.ownedRegions.empty()) {
    return nullptr;
  }
  return Fwg::Utils::selectRandom(country.hoi4Regions);
}
} // namespace Scenario::Hoi4::Selectors
