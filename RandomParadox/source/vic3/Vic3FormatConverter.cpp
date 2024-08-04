#include "vic3/Vic3FormatConverter.h"
namespace Scenario::Gfx::Vic3 {
using namespace Scenario::Gfx::Textures;
using namespace Fwg;
using namespace Fwg::Gfx;

const std::map<std::string, std::map<Fwg::Gfx::Colour, Fwg::Gfx::Colour>>
    colourMaps2{
        {"terrainVic3",
         {{Cfg::Values().colours["rockyHills"], Fwg::Gfx::Colour(14, 24, 23)},
          {Cfg::Values().colours["snowyHills"], Fwg::Gfx::Colour(21, 12, 255)},
          {Cfg::Values().colours["rockyMountains"],
           Fwg::Gfx::Colour(24, 22, 15)},
          {Cfg::Values().colours["snowyMountains"],
           Fwg::Gfx::Colour(21, 12, 255)},
          {Cfg::Values().colours["rockyPeaks"], Fwg::Gfx::Colour(24, 22, 15)},
          {Cfg::Values().colours["snowyPeaks"], Fwg::Gfx::Colour(3, 2, 4)},
          {Cfg::Values().colours["grassland"], Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["grasslandHills"],
           Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["grasslandMountains"],
           Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["desert"], Fwg::Gfx::Colour(2, 4, 255)},
          {Cfg::Values().colours["desertHills"], Fwg::Gfx::Colour(2, 4, 255)},
          {Cfg::Values().colours["desertMountains"],
           Fwg::Gfx::Colour(2, 4, 255)},
          {Cfg::Values().colours["forest"], Fwg::Gfx::Colour(31, 28, 23)},
          {Cfg::Values().colours["forestHills"], Fwg::Gfx::Colour(13, 21, 14)},
          {Cfg::Values().colours["forestMountains"],
           Fwg::Gfx::Colour(13, 21, 14)},
          {Cfg::Values().colours["savanna"], Fwg::Gfx::Colour(26, 25, 24)},
          {Cfg::Values().colours["drysavanna"], Fwg::Gfx::Colour(26, 25, 24)},
          {Cfg::Values().colours["jungle"], Fwg::Gfx::Colour(32, 31, 23)},
          {Cfg::Values().colours["tundra"], Fwg::Gfx::Colour(28, 24, 29)},
          {Cfg::Values().colours["ice"], Fwg::Gfx::Colour(21, 12, 255)},
          {Cfg::Values().colours["marsh"], Fwg::Gfx::Colour(7, 6, 23)},
          {Cfg::Values().colours["urban"], Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["farm"], Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["sea"], Fwg::Gfx::Colour(13, 7, 255)}}},
    };

const std::map<std::string, std::map<Fwg::Gfx::Colour, int>> colourMaps{
    {"riversVic3",
     {{Cfg::Values().colours["land"], 255},
      {Cfg::Values().colours["river"], 3},
      {Cfg::Values().colours["river"] * 0.9, 3},
      {Cfg::Values().colours["river"] * 0.8, 6},
      {Cfg::Values().colours["river"] * 0.7, 6},
      {Cfg::Values().colours["river"] * 0.6, 10},
      {Cfg::Values().colours["river"] * 0.5, 11},
      {Cfg::Values().colours["river"] * 0.4, 11},
      {Cfg::Values().colours["sea"], 122},
      {Cfg::Values().colours["riverStart"], 0},
      {Cfg::Values().colours["riverStartTributary"], 3},
      {Cfg::Values().colours["riverEnd"], 1}}},
    {"treesVic3",
     {{Cfg::Values().colours["rockyHills"], 0},
      {Cfg::Values().colours["snowyHills"], 0},
      {Cfg::Values().colours["rockyMountains"], 0},
      {Cfg::Values().colours["snowyMountains"], 0},
      {Cfg::Values().colours["rockyPeaks"], 0},
      {Cfg::Values().colours["snowyPeaks"], 0},
      {Cfg::Values().colours["grassland"], 0},
      {Cfg::Values().colours["grasslandHills"], 0},
      {Cfg::Values().colours["grasslandMountains"], 0},
      {Cfg::Values().colours["desert"], 0},
      {Cfg::Values().colours["desertHills"], 0},
      {Cfg::Values().colours["desertMountains"], 0},
      {Cfg::Values().colours["forest"], 6},
      {Cfg::Values().colours["forestHills"], 6},
      {Cfg::Values().colours["forestMountains"], 6},
      {Cfg::Values().colours["savanna"], 0},
      {Cfg::Values().colours["drysavanna"], 0},
      {Cfg::Values().colours["jungle"], 28},
      {Cfg::Values().colours["tundra"], 0},
      {Cfg::Values().colours["ice"], 0},
      {Cfg::Values().colours["marsh"], 0},
      {Cfg::Values().colours["urban"], 0},
      {Cfg::Values().colours["farm"], 0},
      {Cfg::Values().colours["empty"], 0},
      {Cfg::Values().colours["sea"], 0}}}

};

void FormatConverter::writeTile(int xTiles, int yTiles,
                                const Fwg::Gfx::Bitmap &basePackedHeightMap,
                                Fwg::Gfx::Bitmap &packedHeightMap, int mapX,
                                int mapY, int packedX) const {
  const int tilesize = 64;
  const int scaledTilesize = 65;
  for (auto tilex = 0; tilex < xTiles; tilex++) {
    for (auto tiley = 0; tiley < yTiles; tiley++) {
      Fwg::Gfx::Bitmap tileMap(
          tilesize, tilesize, 24,
          (Fwg::Utils::cutBuffer(basePackedHeightMap.imageData, mapX, mapY,
                                 tilex * tilesize, (tilex + 1) * tilesize,
                                 tiley * tilesize, (tiley + 1) * tilesize, 1)));
      auto tileMap2 =
          Bmp::scale(tileMap, scaledTilesize, scaledTilesize, false);

      for (auto x = 0; x < tileMap2.size(); x++) {
        auto baseX = tilex * scaledTilesize;
        auto baseIndex =
            baseX + ((((tiley * 2 + tilex / (xTiles / 2)) * (tilesize + 1)) +
                      x / (tilesize + 1)) -
                     tilex / (xTiles / 2)) *
                        packedX;
        // auto xIndex = tilex * (scaledTilesize) + x % scaledTilesize;
        // auto yIndex = tiley * (scaledTilesize) + x / (scaledTilesize);
        // auto widthMod = x % scaledTilesize;
        // auto heightMod = x / scaledTilesize;
        //// std::cout << widthMod << " " << heightMod << std::endl;
        packedHeightMap.imageData[baseIndex + x % (tilesize + 1)] = tileMap2[x];
        // auto baseIndex = xIndex + yIndex * packedX;
        // auto index = baseIndex;
        // if (index >= packedHeightMap.imageData.size()) {
        //   std::cout << baseIndex << std::endl;
        // } else {
        //   packedHeightMap.imageData[index] = tileMap2[x];
        // }
      }
    }
  }
}

Bitmap
FormatConverter::dumpPackedHeightmap(const Bitmap &heightMap,
                                     const std::string &path,
                                     const std::string &colourMapKey) const {
  Utils::Logging::logLine("FormatConverter::Packing heightmap to ", path);
  int mapX = heightMap.width();
  int ogXTiles = mapX / 64;
  int mapY = heightMap.height();
  int ogYTiles = mapY / 64;

  int xTiles = mapX / 64;
  int yTiles = mapY / 64;

  int packedX = xTiles / 2 * 65;
  int packedY = yTiles * 2 * 65 + 5;
  if (gameTag == "Vic3") {
    Fwg::Gfx::Bitmap packedHeightMap(packedX, packedY, 24);
    auto basePackedHeightMap = heightMap;
    // TODO: Threading
    // writeTile(xTiles, yTiles, basePackedHeightMap, packedHeightMap, mapX,
    // mapY,
    //          packedX);
    // basePackedHeightMap = Fwg::Gfx::Bmp::scale(basePackedHeightMap, xTiles *
    // 65,
    //                                           yTiles * 65, false);
    // packedHeightMap = Fwg::Gfx::Bitmap(xTiles * 65, yTiles * 65 + 5, 24);
    // for (int i = 0; i < basePackedHeightMap.size(); i++) {
    //  packedHeightMap.setColourAtIndex(i, basePackedHeightMap[i]);
    //}
    // Png::save(packedHeightMap, path + ".png", false, LCT_GREY, 16);
    writeTile(xTiles, yTiles, basePackedHeightMap, packedHeightMap, mapX, mapY,
              packedX);
    Png::save(packedHeightMap, path + ".png", false, LCT_GREY, 16);
    return packedHeightMap;
  } else {

    Bitmap packedHeightMap(Cfg::Values().width, Cfg::Values().height, 8);
    packedHeightMap.colourtable = colourTables.at(colourMapKey + gameTag);
    // now map from 24 bit climate map
    for (int i = 0; i < Cfg::Values().bitmapSize; i++) {
      // packedHeightMap.bit8Buffer[i] = heightMap[i].getRed();
      packedHeightMap.setColourAtIndex(
          i, packedHeightMap.lookUp(heightMap[i].getRed()));
    }
    return packedHeightMap;
  }
}
void FormatConverter::dumpIndirectionMap(const Fwg::Gfx::Bitmap &heightMap,
                                         const std::string &path) {
  int xTiles = heightMap.width() / 64;
  int yTiles = heightMap.height() / 64;
  auto indirectionMap = Fwg::Gfx::Bitmap(xTiles, yTiles, 24);
  indirectionMap.fill({0, 0, 0});
  for (int i = 0; i < indirectionMap.size(); i++) {
    indirectionMap.setColourAtIndex(i, {i % (xTiles / 2), i / (xTiles / 2), 1});
  }
  // for (int h = 0; h < indirectionMap.height(); h++) {
  //   for (int w = 0; w < indirectionMap.width(); w++) {
  //     indirectionMap.setColourAtIndex(h * indirectionMap.width() + w,
  //                                     {w%, h, 1});
  //   }
  // }
  Fwg::Gfx::Png::save(indirectionMap, path, false, LCT_RGBA, 8U, 0);
}
void FormatConverter::Vic3ColourMaps(
    const Fwg::Gfx::Bitmap &climateMap, const Fwg::Gfx::Bitmap &treesIn,
    const Fwg::Gfx::Bitmap &heightMap, const Fwg::Gfx::Bitmap &humidityMap,
    const Fwg::Civilization::CivilizationLayer &civLayer,
    const std::string &path) {
  Fwg::Utils::Logging::logLine("Vic3 Format Converter: Writing colour maps");

  auto &config = Cfg::Values();
  // need to scale to default vic3 map sizes, due to their compression
  auto scaledMap = Bmp::scale(heightMap, config.width, config.height, false);
  const auto &height = scaledMap.height();
  const auto &width = scaledMap.width();
  int factor = 1;
  auto imageWidth = width / factor;
  auto imageHeight = height / factor;

  std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);
  for (auto h = 0; h < imageHeight; h++) {
    for (auto w = 0; w < imageWidth; w++) {
      auto colourmapIndex = factor * h * width + factor * w;
      const auto &c = scaledMap[colourmapIndex];
      unsigned char val = 0;
      if (c.getBlue() > config.seaLevel)
        val = 255;
      auto imageIndex =
          imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
      imageIndex *= 4;
      writeBufferPixels(pixels, imageIndex, Fwg::Gfx::Colour(val), 255);
    }
  }
  Textures::writeMipMapDDS(imageWidth, imageHeight, pixels,
                           DXGI_FORMAT_B8G8R8A8_UNORM,
                           path + "//textures//land_mask.dds", false);

  // flatmap
  for (auto h = 0; h < imageHeight; h++) {
    for (auto w = 0; w < imageWidth; w++) {
      auto colourmapIndex = factor * h * width + factor * w;
      const auto &c = scaledMap[colourmapIndex];
      int val = 0;
      Fwg::Gfx::Colour col;
      if (c.getBlue() > config.seaLevel) {
        col = {150, 150, 150};
      } else {
        col = {172, 179, 185};
      }
      auto imageIndex =
          imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
      imageIndex *= 4;
      writeBufferPixels(pixels, imageIndex, col, 255);
    }
  }
  Textures::writeMipMapDDS(imageWidth, imageHeight, pixels,
                           DXGI_FORMAT_B8G8R8A8_UNORM,
                           path + "//textures//flatmap.dds");

  std::fill(pixels.begin(), pixels.end(), 0);
  Textures::writeMipMapDDS(imageWidth, imageHeight, pixels,
                           DXGI_FORMAT_B8G8R8A8_UNORM,
                           path + "//textures//flatmap_overlay.dds");
  // terrain colour map
  scaledMap = Bmp::scale(climateMap, config.width, config.height, false);
  dumpTerrainColourmap(scaledMap, civLayer, path, "//textures//colormap.dds",
                       DXGI_FORMAT_B8G8R8A8_UNORM, 1, false);

  Utils::Logging::logLine(
      "FormatConverter::Writing watercolor_rgb_waterspec_a to ", path);
  using namespace DirectX;

  scaledMap = Bmp::scale(heightMap, config.width / 2, config.height / 2, false);
  imageWidth = scaledMap.width();
  imageHeight = scaledMap.height();
  for (auto h = 0; h < imageHeight; h++) {
    for (auto w = 0; w < imageWidth; w++) {
      auto referenceIndex = h * imageWidth + w;
      double depth = (double)scaledMap[referenceIndex].getBlue() /
                     (double)Cfg::Values().seaLevel;
      auto imageIndex =
          imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
      imageIndex *= 4;
      if (depth < 1.0) {
        writeBufferPixels(
            pixels, imageIndex,
            Fwg::Gfx::Colour(16.0 * depth, 24.0 * depth, 49.0 * depth), 255);
      } else {
        writeBufferPixels(pixels, imageIndex, Fwg::Gfx::Colour(50, 100, 100),
                          255);
      }
    }
  }
  Textures::writeMipMapDDS(
      imageWidth, imageHeight, pixels, DXGI_FORMAT_B8G8R8A8_UNORM,
      path + "//water//watercolor_rgb_waterspec_a.dds", true);
  std::fill(pixels.begin(), pixels.end(), 0);
  Textures::writeMipMapDDS(imageWidth / 4, imageHeight / 4, pixels,
                           DXGI_FORMAT_B8G8R8A8_UNORM,
                           path + "//water//foam_map.dds");
  Textures::writeMipMapDDS(imageWidth / 8, imageHeight / 8, pixels,
                           DXGI_FORMAT_B8G8R8A8_UNORM,
                           path + "//water//flowmap.dds");
  // colormap_tree.dds
  scaledMap =
      Bmp::scale(humidityMap, config.width / 8, config.height / 8, false);
  auto scaledHeight =
      Bmp::scale(heightMap, config.width / 8, config.height / 8, false);
  imageWidth = scaledMap.width();
  imageHeight = scaledMap.height();
  Fwg::Gfx::Colour baseColour = {40, 140, 120};
  Fwg::Gfx::Colour baseColour2 = {40, 100, 110};
  for (auto h = 0; h < imageHeight; h++) {
    for (auto w = 0; w < imageWidth; w++) {
      // use imagewidth here, as we simply compare two equally sized images
      auto referenceIndex = h * imageWidth + w;
      double humidity = (double)scaledMap[referenceIndex].getBlue() / 255.0;
      auto imageIndex =
          imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
      imageIndex *= 4;
      auto c = scaledMap[referenceIndex];
      Fwg::Gfx::Colour col =
          baseColour * humidity + baseColour2 * (1.0 - humidity);
      if (scaledHeight[referenceIndex].getBlue() <
          (double)Cfg::Values().seaLevel) {
        col = {74, 131, 129};
      }
      writeBufferPixels(pixels, imageIndex, col, 255);
    }
  }
  writeDDS(imageWidth, imageHeight, pixels, DXGI_FORMAT_B8G8R8A8_UNORM,
           path + "//textures//colormap_tree.dds");

  scaledHeight =
      Bmp::scale(heightMap, config.width / 8, config.height / 8, false);
  imageWidth = scaledHeight.width();
  imageHeight = scaledHeight.height();
  for (auto h = 0; h < imageHeight; h++) {
    for (auto w = 0; w < imageWidth; w++) {
      // use imagewidth here, as we simply compare two equally sized images
      auto referenceIndex = h * imageWidth + w;
      auto imageIndex =
          imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
      imageIndex *= 4;
      auto c = scaledMap[referenceIndex];
      unsigned char col = 0;
      if (scaledHeight[referenceIndex].getBlue() <
          (double)Cfg::Values().seaLevel) {
        col = 0;
      } else {
        col = 128;
      }
      writeBufferPixels(pixels, imageIndex, Fwg::Gfx::Colour(col), 255);
    }
  }
  Textures::writeMipMapDDS(imageWidth, imageHeight, pixels,
                           DXGI_FORMAT_B8G8R8A8_UNORM,
                           path + "//textures//windmap_tree.dds", true);
}

void FormatConverter::dynamicMasks(
    const std::string &path,
    const Fwg::ClimateGeneration::ClimateData &climateData,
    const Fwg::Civilization::CivilizationLayer &civLayer) {
  // TODO: exclusion_mask.dds
  //

  Utils::Logging::logLine("Vic3::Writing dynamic masks");
  const auto &config = Fwg::Cfg::Values();

  std::vector<double> dynamicMask(config.bitmapSize);
  Fwg::Gfx::Png::save(
      Fwg::Gfx::Bmp::scale(
          Fwg::Gfx::Bitmap(config.width, config.height, 24, dynamicMask),
          config.width, config.height, false),
      path + "mask_dynamic_mining.png");
  for (int i = 0; i < civLayer.agriculture.size(); i++) {
    auto val = civLayer.agriculture[i];
    dynamicMask[i] = val * 255.0;
  }
  Fwg::Gfx::Png::save(
      Fwg::Gfx::Bmp::scale(
          Fwg::Gfx::Bitmap(config.width, config.height, 24, dynamicMask),
          config.width, config.height, false),
      path + "mask_dynamic_farmland.png");
  for (int i = 0; i < climateData.treeCoverage.size(); i++) {
    dynamicMask[i] = 0;
    auto val = climateData.treeCoverage[i];
    if (val != Fwg::ClimateGeneration::Detail::TreeTypeIndex::NONE)
      dynamicMask[i] = 255.0;
  }

  Fwg::Gfx::Png::save(
      Fwg::Gfx::Bmp::scale(
          Fwg::Gfx::Bitmap(config.width, config.height, 24, dynamicMask),
          config.width, config.height, false),
      path + "mask_dynamic_forestry.png");
}
void FormatConverter::contentSource(
    const std::string &path,
    const Fwg::ClimateGeneration::ClimateData &climateData,
    const Fwg::Civilization::CivilizationLayer &civLayer) {

  Utils::Logging::logLine("Vic3::Writing content source masks");
  const auto &config = Fwg::Cfg::Values();

  std::vector<std::string> maskNames = {"african_dynamic_farmland_objects",
                                        "african_dynamic_forestry_objects",
                                        "african_dynamic_mining_objects",
                                        "arabic_dynamic_farmland_objects",
                                        "arabic_dynamic_forestry_objects",
                                        "arabic_dynamic_mining_objects",
                                        "asian_dynamic_farmland_objects",
                                        "asian_dynamic_forestry_objects",
                                        "asian_dynamic_mining_objects",
                                        "baobab_01",
                                        "bush_01",
                                        "bush_02",
                                        "bush_dry_02",
                                        "cypress_mediterranean_dense_01",
                                        "cypress_mediterranean_sparse_01",
                                        "dense_cypress_01",
                                        "european_dynamic_farmland_objects",
                                        "european_dynamic_forestry_objects",
                                        "european_dynamic_mining_objects",
                                        "iceberg_01",
                                        "latin_dynamic_farmland_objects",
                                        "latin_dynamic_forestry_objects",
                                        "latin_dynamic_mining_objects",
                                        "oak_01",
                                        "oak_dense_01",
                                        "palm_dense_01",
                                        "pine_dense_01",
                                        "pine_sparse_01",
                                        "rainforest_01",
                                        "savanna_tree_01",
                                        "sparse_rainforest_01"};

  Fwg::Gfx::Bitmap emptyMap(config.width / 2, config.height / 2, 24);
  for (const auto &maskName : maskNames) {
    Fwg::Gfx::Png::save(emptyMap, path + "mask_" + maskName + ".png");
  }
}
void FormatConverter::detailMaps(
    const Fwg::ClimateGeneration::ClimateData &climateData,
    const Fwg::Civilization::CivilizationLayer &civLayer,
    const std::string &path) {
  using Et = Fwg::ElevationTypeIndex;
  using Clt = Fwg::ClimateGeneration::Detail::ClimateTypeIndex;
  using ft = Fwg::ClimateGeneration::Detail::TreeTypeIndex;
  std::map<Et, int> elevationMap{
      {Et::CLIFF, 16},      {Et::DEEPOCEAN, 6}, {Et::LAKE, 6},
      {Et::HIGHLANDS, 18},  {Et::HILLS, 16},    {Et::LOWHILLS, 13},
      {Et::MOUNTAINS, 14},  {Et::OCEAN, 5},     {Et::PEAKS, 15},
      {Et::STEEPPEAKS, 15}, {Et::VALLEY, 12}};
  std::map<Clt, int> climateMap{{Clt::COLDDESERT, 1},
                                {Clt::COLDSEMIARID, 25},
                                {Clt::CONTINENTALCOLD, 22},
                                {Clt::CONTINENTALHOT, 24},
                                {Clt::CONTINENTALWARM, 23},
                                {Clt::DESERT, 0},
                                {Clt::HOTSEMIARID, 26},
                                {Clt::POLARARCTIC, 21},
                                {Clt::POLARTUNDRA, 28},
                                {Clt::ROCK, 15},
                                {Clt::SNOW, 21},
                                {Clt::TEMPERATECOLD, 22},
                                {Clt::TEMPERATEHOT, 24},
                                {Clt::TEMPERATEWARM, 23},
                                {Clt::TROPICSMONSOON, 34},
                                {Clt::TROPICSRAINFOREST, 32},
                                {Clt::TROPICSSAVANNA, 35},
                                {Clt::WATER, 5}};
  std::map<ft, int> treeMap{{ft::SPARSE, 34},          {ft::TEMPERATEMIXED, 30},
                            {ft::TEMPERATENEEDLE, 31}, {ft::TROPICALDRY, 32},
                            {ft::TROPICALMOIST, 32},   {ft::BOREAL, 31}};

  std::map<int, std::string> nameMapping{{0, "desert_01"},
                                         {1, "desert_03"},
                                         {2, "desert_04"},
                                         {3, "desert_05"},
                                         {4, "desert_06"},
                                         {5, "beach_01"},
                                         {6, "beach_02"},
                                         {7, "beach_03"},
                                         {8, "marchlands_01"},
                                         {9, "marchlands_02"},
                                         {10, "marchlands_03"},
                                         {11, "marchlands_04"},
                                         {12, "cliff_granite_01"},
                                         {13, "cliff_granite_02"},
                                         {14, "cliff_granite_06"},
                                         {15, "cliff_granite_07"},
                                         {16, "cliff_limestone_02"},
                                         {17, "cliff_limestone_03"},
                                         {18, "cliff_sandstone_03"},
                                         {19, "cliff_sandstone_04"},
                                         {20, "cliff_sandstone_05"},
                                         {21, "snow_02"},
                                         {22, "grasslands_01"},
                                         {23, "grasslands_02"},
                                         {24, "grasslands_05"},
                                         {25, "grasslands_06"},
                                         {26, "grasslands_07"},
                                         {27, "grasslands_08"},
                                         {28, "permafrost_01"},
                                         {29, "permafrost_03"},
                                         {30, "woodlands_01"},
                                         {31, "woodlands_02"},
                                         {32, "woodlands_03"},
                                         {33, "rocks_01"},
                                         {34, "savanna_01"},
                                         {35, "savanna_03"}};
  Utils::Logging::logLine("Vic3::Writing detailMaps");
  const auto &config = Fwg::Cfg::Values();
  Fwg::Gfx::Bitmap detailIndexBmp(config.width, config.height, 24);
  Fwg::Gfx::Bitmap detailIntensity(config.width, config.height, 24);
  for (auto i = 0; i < climateData.climates.size(); i++) {
    std::array<unsigned char, 3> colour;
    std::array<float, 3> intensities;

    for (auto chanceIndex = 0; chanceIndex < 3; chanceIndex++) {
      auto type = climateData.climates[i].getChances(chanceIndex).second;
      auto intensity = climateData.climates[i].getChances(chanceIndex).first *
                       (1.0 / pow(2.0, (double)chanceIndex));
      auto mappedType = climateMap.at(type);
      colour[chanceIndex] = mappedType;
      intensities[chanceIndex] = intensity;
    }

    auto elevType = climateData.landForms[i].landForm;
    if (elevType != ElevationTypeIndex::PLAINS &&
        elevType != ElevationTypeIndex::HIGHLANDS) {
      colour[2] = elevationMap.at(elevType);
      intensities[2] =
          std::clamp(climateData.landForms[i].inclination * 0.5f, 0.0f, 1.0f);
    }

    auto treeType = climateData.treeCoverage[i];
    if (treeType != ft::NONE) {
      colour[1] = treeMap.at(treeType);
      intensities[1] = 1.0;
    }
    double intensitySum = intensities[0] + intensities[1] + intensities[2];
    detailIndexBmp.setColourAtIndex(i, {colour[2], colour[1], colour[0]});
    detailIntensity.setColourAtIndex(i,
                                     {intensities[0] / intensitySum * 255.0f,
                                      intensities[1] / intensitySum * 255.0f,
                                      intensities[2] / intensitySum * 255.0f});
  }

  if (Cfg::Values().debugLevel > 0) {
    Fwg::Gfx::Png::save(detailIndexBmp,
                        config.mapsPath + "Vic3//" + "detailIndex.png");
    Fwg::Gfx::Png::save(detailIntensity,
                        config.mapsPath + "Vic3//" + "detailIntensity.png");
  }

  auto scaledDetailIndex =
      Fwg::Gfx::Bmp::scale(detailIndexBmp, config.width, config.height, false);

  if (Cfg::Values().debugLevel > 0) {
    Fwg::Gfx::Png::save(scaledDetailIndex,
                        config.mapsPath + "Vic3//" + "sdetailIndex.png");
  }
  auto scaledDetailIntensity =
      Fwg::Gfx::Bmp::scale(detailIntensity, config.width, config.height, false);
  if (Cfg::Values().debugLevel > 0) {
    Fwg::Gfx::Png::save(scaledDetailIntensity,
                        config.mapsPath + "Vic3//" + "sdetailIntensity.png");
  }

  const auto &height = scaledDetailIndex.height();
  const auto &width = scaledDetailIndex.width();
  std::vector<Fwg::Gfx::Bitmap> masks;
  auto inMap = scaledDetailIndex;
  inMap.fill(0);
  for (int i = 0; i < 36; i++) {
    masks.push_back(inMap);
  }
  std::vector<uint8_t> pixels(width * height * 4, 0);
  std::vector<uint8_t> intensityPixels(width * height * 4, 0);
  for (auto h = 0; h < height; h++) {
    for (auto w = 0; w < width; w++) {
      auto colourmapIndex = h * width + w;
      auto &c = scaledDetailIndex[colourmapIndex];
      auto imageIndex = height * width - (h * width + (width - w));
      imageIndex *= 4;
      try {
        pixels[imageIndex] = (c.getRed());
        pixels[imageIndex + 1] = (c.getGreen());
        pixels[imageIndex + 2] = (c.getBlue());
        pixels[imageIndex + 3] = 255;
        // get the intensity of the colours
        const auto &intensity = scaledDetailIntensity[colourmapIndex];
        // now for every current channel, write the mask using both index and
        // colour
        Fwg::Gfx::Colour intensityColour;
        for (int i = 0; i < 3; i++) {
          auto maskIndex = pixels[imageIndex + i];
          // now locate which mask index we need to write to
          masks[maskIndex].setColourAtIndex(colourmapIndex,
                                            intensity.getBGR()[i]);
          intensityPixels[imageIndex + i] = intensity.getBGR()[i];
        }
        intensityPixels[imageIndex + 4] = 0;
      } catch (std::exception e) {
        std::cout << c << std::endl;
      }
    }
  }
  Textures::writeTGA(config.width, config.height, pixels,
                     path + "//terrain//detail_index.tga");
  Textures::writeTGA(config.width, config.height, intensityPixels,
                     path + "//terrain//detail_intensity.tga");

  for (int i = 0; i < masks.size(); i++) {
    Fwg::Gfx::Png::save(masks[i],
                        path + "//terrain//mask_" + nameMapping.at(i) + ".png",
                        true, LCT_GREY, 8);
  }
}

FormatConverter::FormatConverter(const std::string &gamePath,
                                 const std::string &gameTag)
    : Scenario::Gfx::FormatConverter(gamePath, gameTag) {}

FormatConverter::~FormatConverter() {}
} // namespace Scenario::Gfx::Vic3