#include "hoi4/Hoi4Country.h"

namespace Rpx::Hoi4 {
Hoi4Country::Hoi4Country() : Arda::Country() {}
Hoi4Country::Hoi4Country(std::string tag, int ID, std::string name,
                         std::string adjective, Arda::Gfx::Flag flag)
    : Arda::Country(tag, ID, name, adjective, flag) {}

Hoi4Country::Hoi4Country(Country &c,
                         std::vector<std::shared_ptr<Hoi4::Region>> &hoi4Region)
    : Country(c), allowElections{true}, bully{0.0}, parties{25, 25, 25, 25} {}

Hoi4Country::~Hoi4Country() {}

void Hoi4Country::addAirBase(int level) {
  // selects a random region to add the airbase to
  auto region = Fwg::Utils::selectRandom(hoi4Regions);
  if (region->airBase == nullptr) {
    region->airBase = std::make_shared<AirBase>();
    region->airBase->level = level;
    // find the building for the airbase
    for (const auto &building : region->buildings) {
      if (building.name == "air_base") {
        region->airBase->provinceID = building.provinceID;
        break;
      }
    }
    airBases.insert({region, region->airBase});
  } else {
    region->airBase->level += level;
    region->airBase->level = std::min<int>(region->airBase->level, 10);
  }
}

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

} // namespace Rpx::Hoi4