
#include "hoi4/goals/Effects.h"
namespace Scenario::Hoi4::Effects {
std::map<std::string, std::vector<std::string>>
constructEffects(std::map<std::string, std::vector<std::string>> &effects) {
  std::map<std::string, std::vector<std::string>> retEffects;
  for (auto &[key, value] : effects) {
    std::vector<std::string> retValue;
    if (key == "add_political_power") {
      retValue.push_back("add_political_power");
      retValue.push_back(value[0]);
    }
  }
  return retEffects;
}

} // namespace Scenario::Hoi4::Effects
