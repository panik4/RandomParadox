#pragma once
#include "FastWorldGenerator.h"
#include "areas/ArdaRegion.h"
#include "utils/RpxUtils.h"
#include "vic3/Vic3Utils.h"
namespace Rpx::Vic3 {

class Region : public Arda::ArdaRegion {

public:
  Region();
  Region(const Arda::ArdaRegion &ardaRegion);
  ~Region();
  double arableLand = 0.0;
  std::map<std::string, Building> buildings;
  int supportsBuilding(const BuildingType &buildingType);
  int navalExit = -1;
};
} // namespace Rpx::Vic3