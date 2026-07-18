#include "generic/StrategicRegion.h"
#include "utils/Archive.h"

namespace Rpx {
void StrategicRegion::serialise(Fwg::Utils::Serialisation::Archive &ar) {
  SuperRegion::serialise(ar);
  ar &weatherMonths;
}
void StrategicRegion::deserialise(Fwg::Utils::Serialisation::Archive &ar) { serialise(ar); }
uint32_t StrategicRegion::typeTag() const {
  return Fwg::Utils::Serialisation::TypeRegistry::hashString("Rpx::StrategicRegion");
}
} // namespace Rpx