#pragma once
#include "DirectXTex.h"
#include "generic/FormatConverter.h"
#include "generic/Textures.h"
#include "entities/Colour.h"
#include "utils/Bitmap.h"
#include "utils/Cfg.h"
#include <map>
namespace Scenario::Gfx::Vic3 {
class FormatConverter : public Scenario::Gfx::FormatConverter {

public:
  // constructor/destructor
  FormatConverter(const std::string &gamePath, const std::string &gameTag);
  ~FormatConverter();
  // member functions
  void writeTile(int xTiles, int yTiles,
                 const Fwg::Gfx::Bitmap &basePackedHeightMap,
                 Fwg::Gfx::Bitmap &packedHeightMap, int mapX, int mapY,
                 int packedX) const;
  void dumpPackedHeightmap(const Fwg::Gfx::Bitmap &heightMap,
                           const std::string &path,
                           const std::string &colourMapKey) const;
  void Vic3ColourMaps(const Fwg::Gfx::Bitmap &climateMap,
                      const Fwg::Gfx::Bitmap &treesIn,
                      const Fwg::Gfx::Bitmap &heightMap,
                      const Fwg::Gfx::Bitmap &humidityMap,
                      const std::string &path);
  void dynamicMasks(const std::string &path);
  void detailIndexMap(const Fwg::Gfx::Bitmap &fwgDetailIndex,
                      const Fwg::Gfx::Bitmap &fwgDetailIntensity,
                      const std::string &path);
  void detailIntensityMap(const Fwg::Gfx::Bitmap &fwgDetailIntensity,
                          const std::string &path);
};
} // namespace Scenario::Gfx