#pragma once
#include "generic/Country.h"
#include "vic3/Vic3Region.h"
#include "vic3/Vic3Utils.h"
#include <array>
#include <string>
#include <vector>

namespace Scenario::Vic3 {
class Country : public Scenario::Country {
public:
  Country(std::string tag, int ID, std::string name, std::string adjective,
          Gfx::Flag flag);
  ~Country();
  std::vector<std::shared_ptr<Region>> ownedVic3Regions;
};
} // namespace Scenario::Vic3