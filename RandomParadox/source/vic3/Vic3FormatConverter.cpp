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
  for (auto tilex = 0; tilex < xTiles; tilex++) {
    for (auto tiley = 0; tiley < yTiles; tiley++) {
      Fwg::Gfx::Bitmap tileMap(
          tilesize, tilesize, 24,
          std::move(Fwg::Utils::cutBuffer(
              basePackedHeightMap.imageData, mapX, mapY, tilex * tilesize,
              (tilex + 1) * tilesize, tiley * tilesize, (tiley + 1) * tilesize,
              1)));
      auto tileMap2 = Bmp::scale(tileMap, tilesize + 1, tilesize + 1, false);

      for (auto x = 0; x < tileMap2.size(); x++) {
        auto baseIndex = tilex * (tilesize + 1) +
                         ((((tiley * 2 + tilex / 128) * (tilesize + 1)) +
                           x / (tilesize + 1)) -
                          tilex / 128) *
                             packedX;
        packedHeightMap.imageData[baseIndex + x % (tilesize + 1)] = tileMap2[x];
      }
    }
  }
}

void FormatConverter::dumpPackedHeightmap(
    const Bitmap &heightMap, const std::string &path,
    const std::string &colourMapKey) const {
  Utils::Logging::logLine("FormatConverter::Packing heightmap to ", path);
  int mapX = 16384;
  int mapY = 7232;
  int packedX = 8320;
  int packedY = 14695;
  int xTiles = 256;
  int yTiles = 113;
  if (gameTag == "Vic3") {
    auto basePackedHeightMap = heightMap;
    Fwg::Gfx::Bitmap packedHeightMap(packedX, packedY, 24);
    // TODO: Threading
    writeTile(xTiles, yTiles, basePackedHeightMap, packedHeightMap, mapX, mapY,
              packedX);
    Png::save(packedHeightMap, path + ".png", false, LCT_GREY, 16);
  } else {

    Bitmap packedHeightMap(Cfg::Values().width, Cfg::Values().height, 8);
    packedHeightMap.colourtable = colourTables.at(colourMapKey + gameTag);
    // now map from 24 bit climate map
    for (int i = 0; i < Cfg::Values().bitmapSize; i++) {
      // packedHeightMap.bit8Buffer[i] = heightMap[i].getRed();
      packedHeightMap.setColourAtIndex(
          i, packedHeightMap.lookUp(heightMap[i].getRed()));
    }
  }
}

void FormatConverter::Vic3ColourMaps(
    const Fwg::Gfx::Bitmap &climateMap, const Fwg::Gfx::Bitmap &treesIn,
    const Fwg::Gfx::Bitmap &heightMap, const Fwg::Gfx::Bitmap &humidityMap,
    const Fwg::Civilization::CivilizationLayer &civLayer,
    const std::string &path) {
  Fwg::Utils::Logging::logLine("Vic3 Format Converter: Writing colour maps");

  auto &config = Cfg::Values();
  // need to scale to default vic3 map sizes, due to their compression
  auto scaledMap = Bmp::scale(heightMap, 8192, 3616, false);
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

  // terrain colour map
  scaledMap = Bmp::scale(climateMap, 8192, 4096, false);
  dumpTerrainColourmap(scaledMap, civLayer, path, "//textures//colormap.dds",
                       DXGI_FORMAT_B8G8R8A8_UNORM, 1, false);

  Utils::Logging::logLine(
      "FormatConverter::Writing watercolor_rgb_waterspec_a to ", path);
  using namespace DirectX;

  scaledMap = Bmp::scale(heightMap, 4096, 1808, false);
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

  // colormap_tree.dds
  scaledMap = Bmp::scale(humidityMap, 1024, 512, false);
  auto scaledHeight = Bmp::scale(heightMap, 1024, 512, false);
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

  scaledHeight = Bmp::scale(heightMap, 1024, 452, false);
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

void FormatConverter::dynamicMasks(const std::string &path) {
  // TODO: exclusion_mask.dds
  //
  auto agricultureMap = Fwg::Gfx::Bmp::load24Bit(
      Fwg::Cfg::Values().mapsPath + "//resourcelayers//agriculture.bmp", "");
  auto miningMap = Fwg::Gfx::Bmp::load24Bit(
      Fwg::Cfg::Values().mapsPath + "//resourcelayers//ores.bmp", "");
  auto jungleMap = Fwg::Gfx::Bmp::load24Bit(
      Fwg::Cfg::Values().mapsPath + "//resourcelayers//jungle.bmp", "");

  for (int i = 0; i < agricultureMap.size(); i++) {
    auto val = agricultureMap[i].getGreen();
    val += jungleMap[i].getGreen();
    agricultureMap.setColourAtIndex(i, val * 3);
  }
  Fwg::Gfx::Png::save(agricultureMap, path + "mask_dynamic_farmland.png");
}

void FormatConverter::detailIndexMap(const Fwg::Gfx::Bitmap &fwgDetailIndex,
                                     const Fwg::Gfx::Bitmap &fwgDetailIntensity,
                                     const std::string &path) {
  Utils::Logging::logLine("Vic3::Writing detailIndexMap");
  auto scaledFwgDetailIndex =
      Fwg::Gfx::Bmp::scale(fwgDetailIndex, 8192, 3616, false);
  auto scaledfwgDetailIntensity =
      Fwg::Gfx::Bmp::scale(fwgDetailIntensity, 8192, 3616, false);
  const auto &height = scaledFwgDetailIndex.height();
  const auto &width = scaledFwgDetailIndex.width();
  int factor = 1;
  auto imageWidth = width / factor;
  auto imageHeight = height / factor;
  std::vector<Fwg::Gfx::Bitmap> masks;
  auto inMap = scaledFwgDetailIndex;
  inMap.fill(0);
  for (int i = 0; i < 36; i++) {
    masks.push_back(inMap);
  }

  // we need a definition for each mask, that contains the mask name, the input
  // colour, the output colour in theory, iterate over created detail_index.tga,
  // for every pixel get values in detail_intensity, then by the index, modify
  // mask in a vector of masks. At the end, write all masks according to index
  // and therefore name

  //    {Cfg::Values().colours["rockyHills"], 13},
  //        {Cfg::Values().colours["snowyHills"], 22},
  //        {Cfg::Values().colours["rockyMountains"], 15},
  //        {Cfg::Values().colours["snowyMountains"], 22},
  //        {Cfg::Values().colours["rockyPeaks"], 16},
  //        {Cfg::Values().colours["snowyPeaks"], 22},
  //        {Cfg::Values().colours["grassland"], 23},
  //        {Cfg::Values().colours["grasslandHills"], 13},
  //        {Cfg::Values().colours["grasslandMountains"], 14},
  //        {Cfg::Values().colours["desert"], 1},
  //        {Cfg::Values().colours["desertHills"], 19}, //10
  //        {Cfg::Values().colours["desertMountains"], 20},
  //        {Cfg::Values().colours["forest"], 26},
  //        {Cfg::Values().colours["forestHills"], 13},
  //        {Cfg::Values().colours["forestMountains"], 14},
  //        {Cfg::Values().colours["savanna"], 5}, //15
  //        {Cfg::Values().colours["drysavanna"], 6},
  //        {Cfg::Values().colours["jungle"], 28},
  //        {Cfg::Values().colours["tundra"], 29},
  //        {Cfg::Values().colours["ice"], 22},
  //         {Cfg::Values().colours["marsh"], 9},
  //        {Cfg::Values().colours["urban"], 24},
  //        {Cfg::Values().colours["farm"], 25}, {
  //      Cfg::Values().colours["sea"], 1
  std::map<int, int> testmap{{0, 12},  {1, 21},  {2, 14},  {3, 21},  {4, 15},
                             {5, 21},  {6, 22},  {7, 12},  {8, 13},  {9, 0},
                             {10, 18}, {11, 19}, {12, 30}, {13, 12}, {14, 13},
                             {15, 34}, {16, 35}, {17, 32}, {18, 28}, {19, 21},
                             {20, 8},  {21, 23}, {22, 24}, {255, 1}};
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
  std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);
  for (auto h = 0; h < imageHeight; h++) {
    for (auto w = 0; w < imageWidth; w++) {
      auto colourmapIndex = factor * h * width + factor * w;
      auto &c = scaledFwgDetailIndex[colourmapIndex];
      auto imageIndex =
          imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
      imageIndex *= 4;
      try {
        pixels[imageIndex] = testmap.at(c.getBlue());
        pixels[imageIndex + 1] = testmap.at(c.getGreen());
        pixels[imageIndex + 2] = testmap.at(c.getRed());
        pixels[imageIndex + 3] = 255;
        // get the intensity of the colours
        const auto &intensity = scaledfwgDetailIntensity[imageIndex / 4];
        // now for every current channel, write the mask using both index and
        // colour
        Fwg::Gfx::Colour intensityColour;
        for (int i = 0; i < 3; i++) {
          auto maskIndex = pixels[imageIndex + i];
          // now locate which mask index we need to write to
          masks[maskIndex].setColourAtIndex(colourmapIndex,
                                            intensity.getBGR()[2 - i]);
        }
      } catch (std::exception e) {
        std::cout << c << std::endl;
      }
    }
  }

  Textures::writeTGA(8192, 3616, pixels, path + "//terrain//detail_index.tga");
  for (int i = 0; i < masks.size(); i++) {
    Fwg::Gfx::Png::save(masks[i],
                        path + "//terrain//mask_" + nameMapping.at(i) + ".png",
                        true, LCT_GREY, 8);
  }
}

void FormatConverter::detailIntensityMap(
    const Fwg::Gfx::Bitmap &fwgDetailIntensity, const std::string &path) {

  Utils::Logging::logLine("Vic3::Writing detailIntensity Map");
  auto copy = Fwg::Gfx::Bmp::scale(fwgDetailIntensity, 8192, 3616, false);
  const auto &height = copy.height();
  const auto &width = copy.width();
  int factor = 1;
  auto imageWidth = width / factor;
  auto imageHeight = height / factor;
  std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);
  for (auto i = 0; i < copy.size(); i++) {
    auto imageIndex = i * 4;
    auto &c = copy[i];
    pixels[imageIndex] = c.getBlue();
    pixels[imageIndex + 1] = c.getGreen();
    pixels[imageIndex + 2] = c.getRed();
    pixels[imageIndex + 3] = 0;
  }
  Textures::writeTGA(8192, 3616, pixels,
                     path + "//terrain//detail_intensity.tga");
}

FormatConverter::FormatConverter(const std::string &gamePath,
                                 const std::string &gameTag)
    : Scenario::Gfx::FormatConverter(gamePath, gameTag) {}

FormatConverter::~FormatConverter() {}
} // namespace Scenario::Gfx::Vic3