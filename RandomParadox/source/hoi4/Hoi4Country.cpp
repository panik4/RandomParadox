#include "hoi4/Hoi4Country.h"

namespace Scenario::Hoi4 {
Hoi4Country::Hoi4Country(std::string tag, int ID, std::string name,
                         std::string adjective, Gfx::Flag flag)
    : Scenario::Country(tag, ID, name, adjective, flag) {}

Hoi4Country::Hoi4Country(Country &c,
                         std::vector<std::shared_ptr<Hoi4::Region>> &hoi4Region)
    : Country(c), allowElections{true}, bully{0.0}, parties{25, 25, 25, 25} {
  for (auto &region : c.ownedRegions) {
    this->hoi4Regions.push_back(std::reinterpret_pointer_cast<Region>(region));
  }
}

Hoi4Country::~Hoi4Country() {}

} // namespace Scenario::Hoi4