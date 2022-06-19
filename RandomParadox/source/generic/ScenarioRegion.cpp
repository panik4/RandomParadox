#include "generic/ScenarioRegion.h"
namespace Scenario {
Region::Region() {}

Scenario::Region::Region(const FastWorldGen::Region &baseRegion)
    : FastWorldGen::Region(baseRegion), assigned(false) {}

} // namespace Scenario