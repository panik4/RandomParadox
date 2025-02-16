#pragma once
#include "hoi4/Hoi4Country.h"
#include "hoi4/goals/Goal.h"

namespace Scenario::Hoi4::Effects {

void constructEffects(std::vector<EffectGrouping> &effects,
                      std::shared_ptr<Hoi4Country> &hoi4Country);

} // namespace Scenario::Hoi4::Effects