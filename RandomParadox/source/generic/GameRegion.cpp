#include "generic/GameRegion.h"
namespace Scenario {
Region::Region() {}

Region::Region(const FastWorldGen::Region &baseRegion)
    : FastWorldGen::Region(baseRegion), assigned(false) {}

Region::~Region() {}
} // namespace Scenario