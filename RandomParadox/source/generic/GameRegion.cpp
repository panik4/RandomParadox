#include "generic/GameRegion.h"
namespace Scenario {
Region::Region() {}

Region::Region(const Fwg::Region &baseRegion)
    : Fwg::Region(baseRegion), assigned(false) {}

Region::~Region() {}
} // namespace Scenario