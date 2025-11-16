#include "eu5/Eu5ImageExporter.h"
namespace Rpx::Gfx::Eu5 {
using namespace Arda::Gfx::Textures;
using namespace Fwg;
using namespace Fwg::Gfx;

ImageExporter::ImageExporter() {}

ImageExporter::ImageExporter(const std::string &gamePath,
                             const std::string &gameTag)
    : Rpx::Gfx::ImageExporter(gamePath, gameTag) {}

ImageExporter::~ImageExporter() {}

Fwg::Gfx::Bitmap ImageExporter::dumpHeightmap(const Fwg::Gfx::Bitmap &heightMap,
                                              const std::string &path,
                                              const std::string &colourMapKey,
                                              int exportWidth,
                                              int exportHeight) const {
  auto scaledHeightmap = Fwg::Gfx::Bmp::scale(heightMap, exportWidth, exportHeight, false);
  Fwg::Gfx::Png::save(scaledHeightmap, path);

  return Fwg::Gfx::Bitmap(0, 0, 24);
}

} // namespace Rpx::Gfx::Eu5