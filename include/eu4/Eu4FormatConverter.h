#pragma once
#include "DirectXTex.h"
#include "entities/Colour.h"
#include "io/FormatConverter.h"
#include "io/Textures.h"
#include "rendering/Images.h"
#include "utils/Cfg.h"
#include <map>
namespace Rpx::Gfx::Eu4 {
class FormatConverter : public Rpx::Gfx::FormatConverter {

public:
  // constructor/destructor
  FormatConverter();
  FormatConverter(const std::string &gamePath, const std::string &gameTag);
  ~FormatConverter();
  // member functions
};
} // namespace Rpx::Gfx::Hoi4