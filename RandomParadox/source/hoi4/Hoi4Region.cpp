#include "hoi4/Hoi4Region.h"

namespace Scenario::Hoi4 {
Hoi4Region::Hoi4Region() {}

Hoi4Region::Hoi4Region(const Scenario::Region &gameRegion)
    : Scenario::Region(gameRegion), stratID{0}, armsFactories{0},
      civilianFactories{0}, development{0}, dockyards{0}, population{0},
      stateCategory{0} {
  resources = {{"aluminium", 0}, {"chromium", 0}, {"oil", 0},
               {"rubber", 0},    {"steel", 0},    {"tungsten", 0}};
}

Hoi4Region::~Hoi4Region() {}
} // namespace Scenario::Hoi4