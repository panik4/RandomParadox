#pragma once
#include "DirectXTex.h"
#include "TextureWriter.h"
#include "entities/Colour.h"
#include "utils/Bitmap.h"
#include "utils/Env.h"
#include <map>

class FormatConverter {
  // map of maps of colours, defines which FastWorldGen colour
  // should be mapped to which game compatible colour
  const static std::map<std::string, std::map<Colour, int>> colourMaps;
  std::map<std::string, std::vector<unsigned char>> colourTables;

public:
  // constructor/destructor
  FormatConverter(const std::string hoiPath);
  ~FormatConverter();
  // member functions
  void dump8BitHeightmap(const std::string path,
                         const std::string colourMapKey) const;
  void dump8BitTerrain(const std::string path,
                       const std::string colourMapKey) const;
  void dump8BitCities(const std::string path,
                      const std::string colourMapKey) const;
  void dump8BitRivers(const std::string path,
                      const std::string colourMapKey) const;
  void dump8BitTrees(const std::string path,
                     const std::string colourMapKey) const;
  void dumpDDSFiles(const std::string path) const;
  void dumpTerrainColourmap(const std::string path) const;
  void dumpWorldNormal(const std::string path) const;
};
