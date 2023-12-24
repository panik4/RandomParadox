#pragma once
#include "FastWorldGenerator.h"
#include "generic/GameRegion.h"
#include "generic/ScenarioUtils.h"

namespace Scenario::Vic3 {
class Region : public Scenario::Region {


public:
  Region();
  Region(const Scenario::Region &gameRegion);
  ~Region();
  double development;
};
} // namespace Scenario::Vic3