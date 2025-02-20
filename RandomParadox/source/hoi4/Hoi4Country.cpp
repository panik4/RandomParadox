#include "hoi4/Hoi4Country.h"

namespace Scenario::Hoi4 {
Hoi4Country::Hoi4Country(std::string tag, int ID, std::string name,
                         std::string adjective, Gfx::Flag flag)
    : Scenario::Country(tag, ID, name, adjective, flag) {}

Hoi4Country::Hoi4Country(Country &c,
                         std::vector<std::shared_ptr<Hoi4::Region>> &hoi4Region)
    : Country(c), allowElections{true}, bully{0.0}, parties{25, 25, 25, 25} {}

Hoi4Country::~Hoi4Country() {}

bool Hoi4Country::hasTech(const std::string &techname) const {
  // check all the different tech maps for the tech.
  // this is horribly inefficient, use sparingly
  auto checkTechMap =
      [&](const std::map<TechEra, std::vector<Technology>> &techMap) {
        for (const auto &techera : techMap) {
          for (const auto &tech : techera.second) {
            if (tech.name == techname) {
              return true;
            }
          }
        }
        return false;
      };

  // Check all the different tech maps for the tech
  return checkTechMap(infantryTechs) || checkTechMap(armorTechs) ||
         checkTechMap(navyTechs) || checkTechMap(industryElectronicTechs) ||
         checkTechMap(airTechs);
}

} // namespace Scenario::Hoi4