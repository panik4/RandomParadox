#pragma once
#include "NameGenerator.h"
#include "entities/Colour.h"
#include <string>
namespace Scenario {
class CultureGroup;

class Culture {

public:
  std::string name;
  // ID of the province that is the center
  int centerOfCulture;
  Fwg::Gfx::Colour colour;
  std::shared_ptr<Scenario::Language> language;
  std::shared_ptr<CultureGroup> cultureGroup;
};

} // namespace Scenario