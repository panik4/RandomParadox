#pragma once
#include "hoi4/Hoi4Country.h"

namespace Scenario::Hoi4::Selectors {
std::shared_ptr<Scenario::Hoi4::Region>
getRandomRegion(const Hoi4Country &country);

} // namespace Scenario::Hoi4::Selectors