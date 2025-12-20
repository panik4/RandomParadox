#pragma once
#include "DirectXTex.h"
#include "civilisation/CivilisationLayer.h"
#include "entities/Colour.h"
#include "io/ImageExporter.h"
#include "areas/ArdaProvince.h"
#include "areas/ArdaContinent.h"

#include "io/Textures.h"
#include "rendering/Images.h"
#include "utils/Cfg.h"
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
                 const Fwg::Gfx::Image &basePackedHeightMap,
                 Fwg::Gfx::Image &packedHeightMap, int mapX, int mapY,
                 int packedX) const;
  Fwg::Gfx::Image dumpHeightmap(const std::vector<float> &heightMap,
                                 const std::string &path,
                                 const std::string &colourMapKey,
                                 int exportWidth, int exportHeight) const;
  Fwg::Gfx::Image dumpDecalMasks(const Fwg::Terrain::TerrainData &terrainData,
                             const Fwg::Climate::ClimateData &climateData,
                             const std::string &path,
                             const std::string &colourMapKey, int exportWidth,
                             int exportHeight) const;
  Fwg::Gfx::Image
  dumpTerrainMasks(const Fwg::Terrain::TerrainData &terrainData,
                   const Fwg::Climate::ClimateData &climateData,
                   const std::string &path, const std::string &colourMapKey,
                   int exportWidth, int exportHeight) const;

  void Eu5ColourMaps(const Fwg::Terrain::TerrainData &terrainData,
                     const Fwg::Climate::ClimateData &climateData,
                     const Arda::Civilization::CivilizationLayer &civLayer,
                     const std::string &path, int exportWidth,
                     int exportHeight);

  void mapObjectMasks(
      const Fwg::Terrain::TerrainData &terrainData,
      const Fwg::Climate::ClimateData &climateData,
      const Arda::Civilization::CivilizationLayer &civLayer,
                      const std::string &path, int exportWidth,
                      int exportHeight);

  void writeLocations(
      const Fwg::Gfx::Image &provinceMap,
      const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces,
      const std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions,
      const std::vector<std::shared_ptr<Arda::ArdaContinent>> &continents,
      const Arda::Civilization::CivilizationLayer &civLayer,
      const std::string &path, int exportWidth, int exportHeight) const;
};
} // namespace Rpx::Gfx::Eu5