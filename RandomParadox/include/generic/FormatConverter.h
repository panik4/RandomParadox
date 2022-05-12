#pragma once
#include "entities/Colour.h"
#include "utils/Bitmap.h"
#include "utils/Env.h"
#include "DirectXTex.h"
#include "TextureWriter.h"
#include <map>

class FormatConverter {
  // map of maps of colours, defines which FastWorldGen colour
  // should be mapped to which game compatible colour
  const static std::map<std::string, std::map<Colour, int>> colourMaps;
  std::map<std::string, std::vector<unsigned char>> colourTables;

public:
  // constructor/destructor
  FormatConverter(std::string hoiPath);
  ~FormatConverter();
  // member functions
  void dump8BitHeightmap(std::string path, std::string colourMapKey) const;
  void dump8BitTerrain(std::string path, std::string colourMapKey) const;
  void dump8BitCities(std::string path, std::string colourMapKey) const;
  void dump8BitRivers(std::string path, std::string colourMapKey) const;
  void dump8BitTrees(std::string path, std::string colourMapKey) const;
  void dumpDDSFiles(std::string path) const;
  void dumpTerrainColourmap(std::string path) const;
  void dumpWorldNormal(std::string path) const;
};
