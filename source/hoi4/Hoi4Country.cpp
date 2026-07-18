#include "hoi4/Hoi4Country.h"

namespace Rpx::Hoi4 {
Hoi4Country::Hoi4Country() : Arda::Country() {}
Hoi4Country::Hoi4Country(std::string tag, int ID, std::string name,
                         std::string adjective, Arda::Gfx::Flag flag)
    : Arda::Country(tag, ID, name, adjective, flag) {}

Hoi4Country::Hoi4Country(Country &c,
                         std::vector<std::shared_ptr<Hoi4::Region>> &hoi4Region)
    : Country(c), allowElections{true}, parties{25, 25, 25, 25} {}

Hoi4Country::~Hoi4Country() {}

void Hoi4Country::addAirBase(int level) {
  // selects a random region to add the airbase to
  auto region = Fwg::Utils::Random::selectRandom(hoi4Regions);
  if (region->airBase == nullptr) {
    region->airBase = std::make_shared<AirBase>();
    region->airBase->level = level;
    region->airBase->regionID = region->ID;
    // find the building for the airbase
    for (const auto &building : region->buildings) {
      if (building.name == "air_base") {
        region->airBase->provinceID = building.provinceID;
        break;
      }
    }
    airBases.push_back(region->airBase);
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

std::string Hoi4Country::exportLine() const {
  auto base = Arda::Country::exportLine();
  base += fullName + ";";
  base += Arda::Utils::ideologyToString.at(ideology) + ";";
  base += std::to_string(allowElections) + ";";
  base += std::to_string(parties[0]) + ";";
  base += std::to_string(parties[1]) + ";";
  base += std::to_string(parties[2]) + ";";
  base += std::to_string(parties[3]) + ";";
  base += lastElection + ";";
  base += std::to_string(warSupport) + ";";
  base += std::to_string(stability) + ";";
  base += std::to_string(convoyAmount) + ";";
  base += std::to_string(researchSlots) + ";";
  return base;
}

void Hoi4Country::serialise(Fwg::Utils::Serialisation::Archive &ar) {
  Country::serialise(ar);
  ar &fullName &gfxCulture &allowElections &parties &lastElection &warSupport &stability;
  ar &faction;
  ar &victoryPoints;
  ar.polymorphicPtrVector(hoi4Regions);
  ar &focusTree &ideas;
  ar &totalNavyStrength;
  // Enum-keyed maps need manual serialisation
  auto writeEnumMap = [&](auto &m) {
    uint64_t sz = m.size();
    ar &sz;
    for (auto &[k, v] : m) {
      ar.serialiseEnum(k);
      ar &v;
    }
  };
  auto readEnumMap = [&](auto &m) {
    uint64_t sz;
    ar &sz;
    m.clear();
    for (uint64_t i = 0; i < sz; ++i) {
      std::decay_t<decltype(m.begin()->first)> k{};
      decltype(m.begin()->second) v;
      ar.serialiseEnum(k);
      ar &v;
      m.emplace(std::move(k), std::move(v));
    }
  };
  if (ar.isWriting()) {
    writeEnumMap(hullTech);
    writeEnumMap(navyTechs);
    writeEnumMap(shipClasses);
    writeEnumMap(infantryTechs);
    writeEnumMap(armorTechs);
    writeEnumMap(airTechs);
    writeEnumMap(industryElectronicTechs);
  } else {
    readEnumMap(hullTech);
    readEnumMap(navyTechs);
    readEnumMap(shipClasses);
    readEnumMap(infantryTechs);
    readEnumMap(armorTechs);
    readEnumMap(airTechs);
    readEnumMap(industryElectronicTechs);
  }
  ar.ptrVector(ships);
  ar &fleets &convoyAmount;
  ar &totalArmyStrength &units &unitCount;
  ar &tankVariants &divisionTemplates &divisions;
  ar &totalAirStrength;
  ar &planeVariants &airWings;
  ar.ptrVector(airBases);
  ar &civilianIndustry &armsFactories &dockyards &researchSlots;
}

void Hoi4Country::deserialise(Fwg::Utils::Serialisation::Archive &ar) { serialise(ar); }

uint32_t Hoi4Country::typeTag() const {
  return Fwg::Utils::Serialisation::TypeRegistry::hashString("Rpx::Hoi4::Hoi4Country");
}
} // namespace Rpx::Hoi4