#pragma once
#include <string>
#include "entities/Colour.h"
namespace Scenario {
class Religion {

public:
  std::string name;
  // ID of the province that is the center
  int centerOfReligion;
  Fwg::Gfx::Colour colour;
 //std::vector<Scenario::GameProvince> centersOfReligion;
};

} // namespace Scenario