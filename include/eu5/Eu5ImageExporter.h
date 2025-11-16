#pragma once
#include "DirectXTex.h"
#include "entities/Colour.h"
#include "io/ImageExporter.h"
#include "io/Textures.h"
#include "rendering/Images.h"
#include "utils/Cfg.h"
#include "civilisation/CivilisationLayer.h"
#include <map>
namespace Rpx::Gfx::Eu5 {
class ImageExporter : public Rpx::Gfx::ImageExporter {

public:
  // constructor/destructor
  ImageExporter();
  ImageExporter(const std::string &gamePath, const std::string &gameTag);
  ~ImageExporter();
  // member functions
  void writeTile(int xTiles, int yTiles,
                 const Fwg::Gfx::Bitmap &basePackedHeightMap,
                 Fwg::Gfx::Bitmap &packedHeightMap, int mapX, int mapY,
                 int packedX) const;
  Fwg::Gfx::Bitmap dumpHeightmap(const Fwg::Gfx::Bitmap &heightMap,
                                       const std::string &path,
                                       const std::string &colourMapKey, int exportWidth, int exportHeight) const;

};
} // namespace Rpx::Gfx::Vic3