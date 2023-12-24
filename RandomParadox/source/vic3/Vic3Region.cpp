#include "vic3/Vic3Region.h"

namespace Scenario::Vic3 {
Region::Region() {}
Region::Region(const Scenario::Region &gameRegion)
    : Scenario::Region(gameRegion) {}
Region::~Region() {}

} // namespace Scenario::Vic3