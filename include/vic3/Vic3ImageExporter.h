#pragma once
#include "DirectXTex.h"
#include "entities/Colour.h"
#include "io/ImageExporter.h"
#include "io/Textures.h"
#include "rendering/Images.h"
#include "utils/Cfg.h"
#include "civilisation/CivilisationLayer.h"
#include <map>
namespace Rpx::Gfx::Vic3 {
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
  Fwg::Gfx::Bitmap dumpPackedHeightmap(const Fwg::Gfx::Bitmap &heightMap,
                                       const std::string &path,
                                       const std::string &colourMapKey) const;

  void Vic3ColourMaps(const Fwg::Gfx::Bitmap &climateMap,
                      const Fwg::Gfx::Bitmap &heightMap,
                      Fwg::Climate::ClimateData &climateData,
                      const Arda::Civilization::CivilizationLayer &civLayer,
                      const std::string &path);

  void dumpIndirectionMap(const Fwg::Gfx::Bitmap &heightMap,
                          const std::string &path);
  void dynamicMasks(const std::string &path,
                    const Fwg::Climate::ClimateData &climateData,
                    const Arda::Civilization::CivilizationLayer &civLayer);

  void contentSource(const std::string &path,
                     const Fwg::Climate::ClimateData &climateData,
                     const Arda::Civilization::CivilizationLayer &civLayer);
  void detailMaps(const Fwg::Terrain::TerrainData &terrainData,
                  const Fwg::Climate::ClimateData &climateData,
                  const Arda::Civilization::CivilizationLayer &civLayer,
                  const std::string &path);
};
} // namespace Rpx::Gfx::Vic3