#pragma once
#include "DirectXTex.h"
#include "entities/Colour.h"
#include "io/ImageExporter.h"
#include "io/Textures.h"
#include "rendering/Images.h"
#include "utils/Cfg.h"
#include <map>
namespace Rpx::Gfx::Eu4 {
class ImageExporter : public Rpx::Gfx::ImageExporter {

public:
  // constructor/destructor
  ImageExporter();
  ImageExporter(const std::string &gamePath, const std::string &gameTag);
  ~ImageExporter();
  // member functions
};
} // namespace Rpx::Gfx::Hoi4