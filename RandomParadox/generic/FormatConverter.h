#pragma once
#include "../FastWorldGen/FastWorldGen/entities/Colour.h"
#include "../FastWorldGen/FastWorldGen/utils/Bitmap.h"
#include "../FastWorldGen/FastWorldGen/utils/Env.h"
#include "DirectXTex.h"
#include "TextureWriter.h"
#include <map>

class FormatConverter {
  // map of maps of colours, defines which FastWorldGen colour
  // should be mapped to which game compatible colour
  std::map<std::string, std::map<Colour, int>> colourMaps{
      {"terrainHoi4",
       {{Env::Instance().namedColours["grassland"], 0},
        {Env::Instance().namedColours["ice"], 19},
        {Env::Instance().namedColours["tundra"], 9},
        {Env::Instance().namedColours["forest"], 1},
        {Env::Instance().namedColours["jungle"], 21},
        {Env::Instance().namedColours["savannah"], 0},
        {Env::Instance().namedColours["desert"], 7},
        {Env::Instance().namedColours["peaks"], 16},
        {Env::Instance().namedColours["mountains"], 11},
        {Env::Instance().namedColours["hills"], 20},
        {Env::Instance().namedColours["sea"], 15}}},
      {"riversHoi4",
       {{Env::Instance().namedColours["land"], 255},
        {Env::Instance().namedColours["river"], 3},
        {Env::Instance().namedColours["river"] * 0.9, 3},
        {Env::Instance().namedColours["river"] * 0.8, 6},
        {Env::Instance().namedColours["river"] * 0.7, 6},
        {Env::Instance().namedColours["river"] * 0.6, 10},
        {Env::Instance().namedColours["river"] * 0.5, 11},
        {Env::Instance().namedColours["river"] * 0.4, 11},
        {Env::Instance().namedColours["sea"], 254},
        {Env::Instance().namedColours["riverStart"], 0},
        {Env::Instance().namedColours["riverStartTributary"], 3},
        {Env::Instance().namedColours["riverEnd"], 1}}},
      {"treesHoi4",
       {{Env::Instance().namedColours["grassland"], 0},
        {Env::Instance().namedColours["ice"], 0},
        {Env::Instance().namedColours["tundra"], 0},
        {Env::Instance().namedColours["forest"], 6},
        {Env::Instance().namedColours["jungle"], 28},
        {Env::Instance().namedColours["savannah"], 0},
        {Env::Instance().namedColours["desert"], 0},
        {Env::Instance().namedColours["peaks"], 0},
        {Env::Instance().namedColours["mountains"], 0},
        {Env::Instance().namedColours["hills"], 0},
        {Env::Instance().namedColours["empty"], 0},
        {Env::Instance().namedColours["sea"], 0}}}};
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
