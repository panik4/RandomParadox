#pragma once
#include "hoi4/Hoi4Country.h"

namespace Scenario::Hoi4::Effects {

std::map<std::string, std::vector<std::string>>
constructEffects(std::map<std::string, std::vector<std::string>> &effects);

} // namespace Scenario::Hoi4::Effects