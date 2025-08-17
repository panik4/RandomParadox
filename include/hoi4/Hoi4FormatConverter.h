#pragma once
#include "DirectXTex.h"
#include "entities/Colour.h"
#include "io/FormatConverter.h"
#include "io/Textures.h"
#include "rendering/Images.h"
#include "utils/Cfg.h"
#include <map>
namespace Rpx::Gfx::Hoi4 {
class FormatConverter : public Rpx::Gfx::FormatConverter {

public:
  // constructor/destructor
  FormatConverter();
  FormatConverter(const std::string &gamePath, const std::string &gameTag);
  ~FormatConverter();
  // member functions
  void dump8BitCities(const Fwg::Gfx::Bitmap &climateIn,
                      const std::string &path, const std::string &colourMapKey,
                      const bool cut = false) const;
};
} // namespace Rpx::Gfx::Hoi4