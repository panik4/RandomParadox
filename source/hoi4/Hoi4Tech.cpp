#include "hoi4/Hoi4Tech.h"

namespace Scenario::Hoi4 {
bool hasTechnology(
    const std::map<TechEra, std::vector<Technology>> &techs,
    const std::string &techName) {
  for (auto era : techs) {
    for (auto tech : era.second) {
      if (tech.name == techName) {
        return true;
      }
    }
  }
  return false;
}
} // namespace Scenario::Hoi4