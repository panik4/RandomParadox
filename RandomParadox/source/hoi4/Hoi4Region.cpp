#include "hoi4/Hoi4Region.h"

namespace Scenario::Hoi4 {
Region::Region() {}

Region::Region(const Scenario::Region &gameRegion)
    : Scenario::Region(gameRegion), armsFactories{0},
      civilianFactories{0}, development{0}, dockyards{0}, population{0}, stateCategory{0}, stratID{0} {
  resources = {{"aluminium", 0}, {"chromium", 0}, {"oil", 0},
               {"rubber", 0},    {"steel", 0},    {"tungsten", 0}};
}

Region::~Region() {}
} // namespace Scenario::Hoi4