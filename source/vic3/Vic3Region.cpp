#include "vic3/Vic3Region.h"

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

template<class Archive>
void Region::serialize(Archive &ar, const unsigned int /*version*/) {
  ar & boost::serialization::base_object<Arda::ArdaRegion>(*this);
  ar & arableLand & navalExit;
}

} // namespace Rpx::Vic3

BOOST_CLASS_EXPORT_IMPLEMENT(Rpx::Vic3::Region)
template void Rpx::Vic3::Region::serialize(boost::archive::binary_oarchive&, unsigned int);
template void Rpx::Vic3::Region::serialize(boost::archive::binary_iarchive&, unsigned int);

