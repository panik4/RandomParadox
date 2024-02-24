#include "vic3/Vic3Country.h"

namespace Scenario::Vic3 {
Country::Country(std::string tag, int ID, std::string name,
                 std::string adjective, Gfx::Flag flag)
    : Scenario::Country(tag, ID, name, adjective, flag) {}


Country::~Country() {}

std::vector<std::shared_ptr<Region>>
Country::getEligibleRegions(const std::string &resourceName) {
  return std::vector<std::shared_ptr<Region>>();
}

} // namespace Scenario::Vic3