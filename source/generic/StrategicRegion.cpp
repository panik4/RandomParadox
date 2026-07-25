#include "generic/StrategicRegion.h"
static int debugStrategicRegionExport = [] {
  std::cout << "StrategicRegion export registered\n";
  return 0;
}();
namespace Rpx {
template <class Archive>
void StrategicRegion::serialize(Archive &ar, const unsigned int /*version*/) {
  ar &boost::serialization::base_object<Arda::SuperRegion>(*this);
  ar & weatherMonths;
}
void StrategicRegion::forceStrategicRegionLink() {
  std::cout << "" << std::endl;
}
} // namespace Rpx

BOOST_CLASS_EXPORT_IMPLEMENT(Rpx::StrategicRegion)
template void Rpx::StrategicRegion::serialize(boost::archive::binary_oarchive &,
                                              unsigned int);
template void Rpx::StrategicRegion::serialize(boost::archive::binary_iarchive &,
                                              unsigned int);
