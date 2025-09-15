#pragma once
#include "DirectXTex.h"
#include "io/Textures.h"
#include "climate/ClimateData.h"
#include "entities/Colour.h"
#include "rendering/Images.h"
#include "utils/Cfg.h"
#include <map>
namespace Rpx::Gfx {
class ImageExporter {
protected:
  // map of maps of colours, defines which FastWorldGen colour
  // should be mapped to which game compatible colour
  const static std::map<std::string, std::map<Fwg::Gfx::Colour, int>>
      colourMaps;
  const static std::map<std::string, std::map<int, int>> indexMaps;

  std::map<std::string, std::vector<unsigned char>> colourTables;
  std::string gamePath;
  std::string gameTag;

public:
  // constructor/destructor
  ImageExporter();
  ImageExporter(const std::string &gamePath, const std::string &gameTag);
  ~ImageExporter();
  // member functions
  void writeBufferPixels(std::vector<unsigned char> &pixels, int index,
                         const Fwg::Gfx::Colour &colour,
                         unsigned char alphaValue);
  Fwg::Gfx::Bitmap cutBaseMap(const std::string &path,
                              const double factor = 1.0,
                              const int bit = 8) const;
  void dump8BitHeightmap(const std::vector<float> &altitudeData,
                         const std::string &path,
                         const std::string &colourMapKey) const;
  void dump8BitTerrain(const Fwg::Terrain::TerrainData &terrainData,
                       const Fwg::Climate::ClimateData &climateIn,
                       const Fwg::Civilization::CivilizationLayer &civLayer,
                       const std::string &path, const std::string &colourMapKey,
                       const bool cut = false) const;

  void dump8BitRivers(const Fwg::Terrain::TerrainData &terrainData,
                      const Fwg::Climate::ClimateData &climateIn,
                      const std::string &path, const std::string &colourMapKey,
                      const bool cut = false) const;
  void dump8BitTrees(const Fwg::Terrain::TerrainData &terrainData,
                     const Fwg::Climate::ClimateData &climateIn,
                     const std::string &path, const std::string &colourMapKey,
                     const bool cut = false) const;
  void dumpDDSFiles(const std::vector<float> &heightMap,
                    const std::string &path, const bool cut = false,
                    const int maxFactor = 2) const;
  void
  dumpTerrainColourmap(const Fwg::Gfx::Bitmap &climateMap,
                       const Fwg::Civilization::CivilizationLayer &civLayer,
                       const std::string &modPath, const std::string &mapName,
                       const DXGI_FORMAT format, int scaleFactor,
                       const bool cut = false) const;
  void dumpWorldNormal(const Fwg::Gfx::Bitmap &sobelMap,
                       const std::string &path, const bool cut) const;
};
} // namespace Arda::Gfx