#pragma once
#include <string>
#include "entities/Colour.h"
#include "Religion.h"
namespace Scenario {
class Culture {

public:
  std::string name;
  // ID of the province that is the center
  int centerOfCulture;
  Fwg::Gfx::Colour colour;
  std::shared_ptr<Religion> primaryReligion;
};

} // namespace Scenario