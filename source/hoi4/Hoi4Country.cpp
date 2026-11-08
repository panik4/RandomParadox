#include "hoi4/Hoi4Country.h"
#include <algorithm>
#include <cmath>

namespace Rpx::Hoi4 {
Hoi4Country::Hoi4Country() : Arda::Country() {}
Hoi4Country::Hoi4Country(std::string tag, int ID, std::string name,
                         std::string adjective, Arda::Gfx::Flag flag)
    : Arda::Country(tag, ID, name, adjective, flag) {}

Hoi4Country::Hoi4Country(Country &c,
                         std::vector<std::shared_ptr<Hoi4::Region>> &hoi4Region)
    : Country(c), allowElections{true}, parties{25, 25, 25, 25} {}

Hoi4Country::~Hoi4Country() {}
void Hoi4Country::generateNamePools() {
  maleNamePool.clear();
  femaleNamePool.clear();
  surnamePool.clear();
  const double population = getTotalPopulation();
  if (ownedRegions.empty() || population <= 0)
    return;
  std::vector<std::pair<std::shared_ptr<Arda::Culture>, double>> shares(
      cultures.begin(), cultures.end());
  std::erase_if(shares, [](const auto &entry) {
    return !entry.first || !entry.first->language ||
           !std::isfinite(entry.second) || entry.second <= 0;
  });
  std::sort(shares.begin(), shares.end(), [](const auto &a, const auto &b) {
    if (a.second != b.second)
      return a.second > b.second;
    if (a.first->name != b.first->name)
      return a.first->name < b.first->name;
    return a.first->colour.toString() < b.first->colour.toString();
  });
  for (const auto &[culture, amount] : shares) {
    const int count = static_cast<int>(std::min(amount / population, 1.0) * 10);
    auto append = [&](std::string &pool, const char *category) {
      for (int i = 0; i < count; ++i)
        pool += "\"" +
                culture->language->getRandomCapitalisedWordFromVocabulary(
                    category) +
                "\" ";
    };
    append(maleNamePool, "MaleNames");
    append(femaleNamePool, "FemaleNames");
    append(surnamePool, "Surnames");
  }
}

bool Hoi4Country::addAirBase(int level) {
  if (level <= 0)
    return false;
  std::vector<std::shared_ptr<Region>> eligible;
  double total = 0;
  for (const auto &region : hoi4Regions) {
    if (region->airfieldSuitability == 0)
      continue;
    if (region->airBase && region->airBase->level >= 10)
      continue;
    if (std::none_of(region->buildings.begin(), region->buildings.end(),
                     [](const auto &b) { return b.name == "air_base"; }))
      continue;
    eligible.push_back(region);
    // Old snapshots contain a site but no runtime score; preserve that site.
    total +=
        region->airfieldSuitability > 0 ? region->airfieldSuitability : 1.0;
  }
  if (eligible.empty())
    return false;
  double choice = RandNum::getRandom(0.0, total);
  auto region = eligible.back();
  for (auto &candidate : eligible)
    if ((choice -= candidate->airfieldSuitability > 0
                       ? candidate->airfieldSuitability
                       : 1.0) <= 0) {
      region = candidate;
      break;
    }
  if (!region->airBase) {
    region->airBase = std::make_shared<AirBase>();
    region->airBase->level = std::min(level, 10);
  } else {
    region->airBase->level += std::min(level, 10 - region->airBase->level);
  }
  region->airBase->regionID = region->ID;
  for (const auto &building : region->buildings)
    if (building.name == "air_base") {
      region->airBase->provinceID = building.provinceID;
      break;
    }
  if (std::find(airBases.begin(), airBases.end(), region->airBase) ==
      airBases.end())
    airBases.push_back(region->airBase);
  return true;
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

void Hoi4Country::importLine(const std::vector<std::string> &tokens) {
  if (tokens.size() > 7)
    politicsInitialized = true;
  if (tokens.size() > 6)
    fullName = tokens[6];

  if (tokens.size() > 7) {
    const auto ideology = Arda::Utils::stringToIdeology.find(tokens[7]);
    if (ideology != Arda::Utils::stringToIdeology.end())
      this->ideology = ideology->second;
  }

  try {
    if (tokens.size() > 8)
      allowElections = std::stoi(tokens[8]) != 0;
    for (std::size_t index = 0;
         index < parties.size() && tokens.size() > 9 + index; ++index)
      parties[index] = std::stoi(tokens[9 + index]);
    if (tokens.size() > 13)
      lastElection = tokens[13];
    if (tokens.size() > 14)
      warSupport = std::stoi(tokens[14]);
    if (tokens.size() > 15)
      stability = std::stoi(tokens[15]);
    if (tokens.size() > 16)
      convoyAmount = std::stoi(tokens[16]);
    if (tokens.size() > 17)
      researchSlots = std::stoi(tokens[17]);
  } catch (const std::exception &e) {
    Fwg::Utils::Logging::logLine("Invalid HOI4 country mapping data: ",
                                 e.what());
  }
}

template <class Archive>
void Hoi4Country::serialize(Archive &ar, const unsigned int /*version*/) {
  ar &boost::serialization::base_object<Arda::Country>(*this);
  ar & maleNamePool & femaleNamePool & surnamePool;
  ar & fullName & gfxCulture & allowElections & parties & lastElection &
      warSupport & stability;
  ar & faction;
  ar & victoryPoints;
  ar & hoi4Regions;
  ar & focusTree & ideas;
  ar & totalNavyStrength;
  auto writeEnumMap = [&](auto &m) {
    uint64_t sz = m.size();
    ar & sz;
    for (auto &[k, v] : m) {
      ar & k;
      ar & v;
    }
  };
  auto readEnumMap = [&](auto &m) {
    uint64_t sz;
    ar & sz;
    m.clear();
    for (uint64_t i = 0; i < sz; ++i) {
      std::decay_t<decltype(m.begin()->first)> k{};
      decltype(m.begin()->second) v;
      ar & k;
      ar & v;
      m.emplace(std::move(k), std::move(v));
    }
  };
  if constexpr (Archive::is_saving::value) {
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
  ar & ships;
  ar & fleets & convoyAmount;
  ar & totalArmyStrength & units & unitCount;
  ar & tankVariants & divisionTemplates & divisions;
  ar & totalAirStrength;
  ar & planeVariants & airWings;
  ar & airBases;
  ar & civilianIndustry & armsFactories & dockyards & researchSlots;
}
} // namespace Rpx::Hoi4

BOOST_CLASS_EXPORT_IMPLEMENT(Rpx::Hoi4::Hoi4Country)
template void
Rpx::Hoi4::Hoi4Country::serialize(boost::archive::binary_oarchive &,
                                  unsigned int);
template void
Rpx::Hoi4::Hoi4Country::serialize(boost::archive::binary_iarchive &,
                                  unsigned int);
