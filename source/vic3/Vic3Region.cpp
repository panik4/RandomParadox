#include "vic3/Vic3Region.h"
#include "utils/Archive.h"

namespace Rpx::Vic3 {
Region::Region() {}
Region::Region(const Arda::ArdaRegion &ardaRegion)
    : Arda::ArdaRegion(ardaRegion) {}
Region::~Region() {}

int Region::supportsBuilding(const BuildingType &buildingType) {
  static std::set<std::string> genericBuildings{
      "bg_light_industry", "bg_heavy_industry",   "bg_manufacturing",
      "bg_service",        "bg_urban_facilities", "bg_power",
      "bg_government",     "bg_technology",       "bg_bureaucracy",
      "bg_trade",          "bg_infrastructure",   "bg_public_infrastructure",
      "bg_construction"};
  if (genericBuildings.find(buildingType.group) != genericBuildings.end()) {
    return 1000;
  }
  if (resources.find(buildingType.group) != resources.end()) {
    auto &res = resources.at(buildingType.group);
    if (res.amount > 0) {
      return static_cast<int>(res.capped ? res.amount : arableLand);
    }
  }

return 0;
}

void Region::serialise(Fwg::Utils::Serialisation::Archive &ar) {
  ArdaRegion::serialise(ar);
  ar &arableLand &navalExit;
}

void Region::deserialise(Fwg::Utils::Serialisation::Archive &ar) {
  serialise(ar);
}

uint32_t Region::typeTag() const {
  return Fwg::Utils::Serialisation::TypeRegistry::hashString(
      "Rpx::Vic3::Region");
}

} // namespace Rpx::Vic3