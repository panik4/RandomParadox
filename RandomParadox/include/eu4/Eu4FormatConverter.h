#pragma once
#include "DirectXTex.h"
#include "entities/Colour.h"
#include "generic/FormatConverter.h"
#include "generic/Textures.h"
#include "utils/Bitmap.h"
#include "utils/Cfg.h"
#include <map>
namespace Scenario::Gfx::Eu4 {
class FormatConverter : public Scenario::Gfx::FormatConverter {

public:
  // constructor/destructor
  FormatConverter();
  FormatConverter(const std::string &gamePath, const std::string &gameTag);
  ~FormatConverter();
  // member functions
};
} // namespace Scenario::Gfx::Hoi4