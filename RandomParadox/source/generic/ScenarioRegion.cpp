#include "generic/ScenarioRegion.h"
namespace Scenario {
Scenario::Region::Region() {}

Scenario::Region::Region(const Fwg::Region &baseRegion)
    : Fwg::Region(baseRegion), assigned(false) {}

} // namespace Scenario