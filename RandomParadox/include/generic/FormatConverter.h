#pragma once
#include "DirectXTex.h"
#include "Textures.h"
#include "entities/Colour.h"
#include "terrain/ClimateGeneration.h"
#include "utils/Bitmap.h"
#include "utils/Cfg.h"
#include <map>
namespace Scenario::Gfx {
class FormatConverter {
protected:
  // map of maps of colours, defines which FastWorldGen colour
  // should be mapped to which game compatible colour
  const static std::map<std::string, std::map<Fwg::Gfx::Colour, int>>
      colourMaps;
  const static std::map<std::string, std::map<int, int>> indexMaps;
  const static std::map<std::string,
                        std::map<Fwg::Gfx::Colour, Fwg::Gfx::Colour>>
      colourMaps2;
  std::map<std::string, std::vector<unsigned char>> colourTables;
  std::string gamePath;
  std::string gameTag;

public:
  // constructor/destructor
  FormatConverter(const std::string &gamePath, const std::string &gameTag);
  ~FormatConverter();
  // member functions
  void writeBufferPixels(std::vector<unsigned char> &pixels, int index,
                         const Fwg::Gfx::Colour &colour,
                         unsigned char alphaValue);
  Fwg::Gfx::Bitmap cutBaseMap(const std::string &path,
                              const double factor = 1.0,
                              const int bit = 8) const;
  void dump8BitHeightmap(Fwg::Gfx::Bitmap &heightMap, const std::string &path,
                         const std::string &colourMapKey) const;
  void dump8BitTerrain(const Fwg::ClimateGeneration::ClimateData &climateIn,
                       const Fwg::Civilization::CivilizationLayer &civLayer,
                       const std::string &path, const std::string &colourMapKey,
                       const bool cut = false) const;
  void dump8BitCities(const Fwg::Gfx::Bitmap &climateIn,
                      const std::string &path, const std::string &colourMapKey,
                      const bool cut = false) const;
  void dump8BitRivers(const Fwg::ClimateGeneration::ClimateData &climateIn,
                      const std::string &path, const std::string &colourMapKey,
                      const bool cut = false) const;
  void dump8BitTrees(const Fwg::ClimateGeneration::ClimateData &climateIn,
                     const std::string &path, const std::string &colourMapKey,
                     const bool cut = false) const;
  void dumpDDSFiles(const Fwg::Gfx::Bitmap &riverMap,
                    const Fwg::Gfx::Bitmap &heightMap, const std::string &path,
                    const bool cut = false, const int maxFactor = 2) const;
  void
  dumpTerrainColourmap(const Fwg::Gfx::Bitmap &climateMap,
                       const Fwg::Civilization::CivilizationLayer &civLayer,
                       const std::string &modPath, const std::string &mapName,
                       const DXGI_FORMAT format, int scaleFactor,
                       const bool cut = false) const;
  void dumpWorldNormal(const Fwg::Gfx::Bitmap &sobelMap,
                       const std::string &path, const bool cut) const;
};
} // namespace Scenario::Gfx