#include "generic/FormatConverter.h"
namespace Scenario::Gfx {
using namespace Textures;
using namespace Fwg;
using namespace Fwg::Gfx;

const std::map<std::string, std::map<Gfx::Colour, Gfx::Colour>>
    // FormatConverter::colourMaps2{
    //     {"terrainVic3",
    //      {{Cfg::Values().colours["grassland"], Fwg::Gfx::Colour(23, 32, 15)},
    //       {Cfg::Values().colours["ice"], Fwg::Gfx::Colour(21, 12, 255)},
    //       {Cfg::Values().colours["tundra"], Fwg::Gfx::Colour(28, 24, 29)},
    //       {Cfg::Values().colours["forest"], Fwg::Gfx::Colour(13, 21, 14)},
    //       {Cfg::Values().colours["jungle"], Fwg::Gfx::Colour(32, 31, 23)},
    //       {Cfg::Values().colours["savanna"], Fwg::Gfx::Colour(26, 25, 24)},
    //       {Cfg::Values().colours["desert"], Fwg::Gfx::Colour(2, 4, 255)},
    //       {Cfg::Values().colours["snowyPeaks"], Fwg::Gfx::Colour(15, 24,
    //       255)}, {Cfg::Values().colours["rockyMountains"],
    //       Fwg::Gfx::Colour(24, 15, 22)},
    //       {Cfg::Values().colours["rockyHills"], Fwg::Gfx::Colour(24, 22,
    //       15)}, {Cfg::Values().colours["sea"], Fwg::Gfx::Colour(13, 7,
    //       255)}}}};
    FormatConverter::colourMaps2{
        {"terrainVic3",
         {{Cfg::Values().colours["rockyHills"], Fwg::Gfx::Colour(24, 22, 15)},
          {Cfg::Values().colours["snowyHills"], Fwg::Gfx::Colour(21, 12, 255)},
          {Cfg::Values().colours["rockyMountains"],
           Fwg::Gfx::Colour(24, 22, 15)},
          {Cfg::Values().colours["snowyMountains"],
           Fwg::Gfx::Colour(21, 12, 255)},
          {Cfg::Values().colours["rockyPeaks"], Fwg::Gfx::Colour(24, 22, 15)},
          {Cfg::Values().colours["snowyPeaks"], Fwg::Gfx::Colour(21, 12, 255)},
          {Cfg::Values().colours["grassland"], Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["grasslandHills"],
           Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["grasslandMountains"],
           Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["desert"], Fwg::Gfx::Colour(2, 4, 255)},
          {Cfg::Values().colours["desertHills"], Fwg::Gfx::Colour(2, 4, 255)},
          {Cfg::Values().colours["desertMountains"],
           Fwg::Gfx::Colour(2, 4, 255)},
          {Cfg::Values().colours["forest"], Fwg::Gfx::Colour(13, 21, 14)},
          {Cfg::Values().colours["forestHills"], Fwg::Gfx::Colour(13, 21, 14)},
          {Cfg::Values().colours["forestMountains"],
           Fwg::Gfx::Colour(13, 21, 14)},
          {Cfg::Values().colours["savanna"], Fwg::Gfx::Colour(26, 25, 24)},
          {Cfg::Values().colours["drysavanna"], Fwg::Gfx::Colour(26, 25, 24)},
          {Cfg::Values().colours["jungle"], Fwg::Gfx::Colour(32, 31, 23)},
          {Cfg::Values().colours["tundra"], Fwg::Gfx::Colour(28, 24, 29)},
          {Cfg::Values().colours["ice"], Fwg::Gfx::Colour(21, 12, 255)},
          {Cfg::Values().colours["marsh"], Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["urban"], Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["farm"], Fwg::Gfx::Colour(23, 32, 15)},
          {Cfg::Values().colours["sea"], Fwg::Gfx::Colour(13, 7, 255)}}}};
const std::map<std::string, std::map<Gfx::Colour, int>>
    FormatConverter::colourMaps{
        {"terrainHoi4",
         {{Cfg::Values().colours["rockyHills"], 2},
          {Cfg::Values().colours["snowyHills"], 16},
          {Cfg::Values().colours["rockyMountains"], 6},
          {Cfg::Values().colours["snowyMountains"], 16},
          {Cfg::Values().colours["rockyPeaks"], 11},
          {Cfg::Values().colours["snowyPeaks"], 16},
          {Cfg::Values().colours["grassland"], 0},
          {Cfg::Values().colours["grasslandHills"], 17},
          {Cfg::Values().colours["grasslandMountains"], 20},
          {Cfg::Values().colours["desert"], 3},
          {Cfg::Values().colours["desertHills"], 8},
          {Cfg::Values().colours["desertMountains"], 10},
          {Cfg::Values().colours["forest"], 4},
          {Cfg::Values().colours["forestHills"], 1},
          {Cfg::Values().colours["forestMountains"], 4},
          {Cfg::Values().colours["savanna"], 0},
          {Cfg::Values().colours["drysavanna"], 12},
          {Cfg::Values().colours["jungle"], 21},
          {Cfg::Values().colours["tundra"], 19},
          {Cfg::Values().colours["ice"], 2},
          {Cfg::Values().colours["marsh"], 9},
          {Cfg::Values().colours["urban"], 13},
          {Cfg::Values().colours["farm"], 5},
          {Cfg::Values().colours["sea"], 15}}},
        {"riversHoi4",
         {{Cfg::Values().colours["land"], 255},
          {Cfg::Values().colours["river"], 3},
          {Cfg::Values().colours["river"] * 0.9, 3},
          {Cfg::Values().colours["river"] * 0.8, 6},
          {Cfg::Values().colours["river"] * 0.7, 6},
          {Cfg::Values().colours["river"] * 0.6, 10},
          {Cfg::Values().colours["river"] * 0.5, 11},
          {Cfg::Values().colours["river"] * 0.3, 11},
          {Cfg::Values().colours["river"] * 0.2, 11},
          {Cfg::Values().colours["river"] * 0.1, 11},
          {Cfg::Values().colours["sea"], 254},
          {Cfg::Values().colours["riverStart"], 0},
          {Cfg::Values().colours["riverStartTributary"], 3},
          {Cfg::Values().colours["riverEnd"], 1}}},
        {"treesHoi4",
         {{Cfg::Values().colours["rockyHills"], 5},
          {Cfg::Values().colours["snowyHills"], 5},
          {Cfg::Values().colours["rockyMountains"], 5},
          {Cfg::Values().colours["snowyMountains"], 5},
          {Cfg::Values().colours["rockyPeaks"], 5},
          {Cfg::Values().colours["snowyPeaks"], 5},
          {Cfg::Values().colours["grassland"], 5},
          {Cfg::Values().colours["grasslandHills"], 6},
          {Cfg::Values().colours["grasslandMountains"], 6},
          {Cfg::Values().colours["desert"], 2},
          {Cfg::Values().colours["desertHills"], 2},
          {Cfg::Values().colours["desertMountains"], 2},
          {Cfg::Values().colours["forest"], 6},
          {Cfg::Values().colours["forestHills"], 6},
          {Cfg::Values().colours["forestMountains"], 6},
          {Cfg::Values().colours["savanna"], 3},
          {Cfg::Values().colours["drysavanna"], 2},
          {Cfg::Values().colours["jungle"], 28},
          {Cfg::Values().colours["tundra"], 5},
          {Cfg::Values().colours["ice"], 5},
          {Cfg::Values().colours["marsh"], 5},
          {Cfg::Values().colours["urban"], 5},
          {Cfg::Values().colours["farm"], 5},
          {Cfg::Values().colours["empty"], 0},
          {Cfg::Values().colours["sea"], 0}}},

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
          {Cfg::Values().colours["sea"], 0}}},
        {"riversEu4",
         {{Cfg::Values().colours["land"], 255},
          {Cfg::Values().colours["river"], 3},
          {Cfg::Values().colours["river"] * 0.9, 3},
          {Cfg::Values().colours["river"] * 0.8, 5},
          {Cfg::Values().colours["river"] * 0.7, 7},
          {Cfg::Values().colours["river"] * 0.6, 9},
          {Cfg::Values().colours["river"] * 0.5, 10},
          {Cfg::Values().colours["river"] * 0.4, 11},
          {Cfg::Values().colours["sea"], 254},
          {Cfg::Values().colours["riverStart"], 0},
          {Cfg::Values().colours["riverStartTributary"], 3},
          {Cfg::Values().colours["riverEnd"], 1}}},
        {"treesEu4",
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
          {Cfg::Values().colours["sea"], 0}}},
        {"terrainEu4",
         {{Cfg::Values().colours["rockyHills"], 1},
          {Cfg::Values().colours["snowyHills"], 16},
          {Cfg::Values().colours["rockyMountains"], 6},
          {Cfg::Values().colours["snowyMountains"], 16},
          {Cfg::Values().colours["rockyPeaks"], 6},
          {Cfg::Values().colours["snowyPeaks"], 16},
          {Cfg::Values().colours["grassland"], 0},
          {Cfg::Values().colours["grasslandHills"], 0},
          {Cfg::Values().colours["grasslandMountains"], 0},
          {Cfg::Values().colours["desert"], 4},
          {Cfg::Values().colours["desertHills"], 4},
          {Cfg::Values().colours["desertMountains"], 4},
          {Cfg::Values().colours["forest"], 1},
          {Cfg::Values().colours["forestHills"], 1},
          {Cfg::Values().colours["forestMountains"], 1},
          {Cfg::Values().colours["savanna"], 0},
          {Cfg::Values().colours["drysavanna"], 0},
          {Cfg::Values().colours["jungle"], 28},
          {Cfg::Values().colours["tundra"], 0},
          {Cfg::Values().colours["ice"], 16},
          {Cfg::Values().colours["marsh"], 0},
          {Cfg::Values().colours["urban"], 0},
          {Cfg::Values().colours["farm"], 0},
          {Cfg::Values().colours["sea"], 15}}}};

Bitmap FormatConverter::cutBaseMap(const std::string &path, const double factor,
                                   const int bit) const {
  auto &conf = Cfg::Values();
  std::string sourceMap{conf.loadMapsPath + path};
  Fwg::Utils::Logging::logLine("CUTTING mode: Cutting Map from ", sourceMap);
  Bitmap baseMap =
      bit == 24 ? Bmp::load24Bit(sourceMap, "") : Bmp::load8Bit(sourceMap, "");
  auto cutBase = Bmp::cut(baseMap, conf.minX * factor, conf.maxX * factor,
                          conf.minY * factor, conf.maxY * factor, factor);
  if (conf.scale) {
    cutBase = Bmp::scale(cutBase, conf.scaleX * factor, conf.scaleY * factor,
                         conf.keepRatio);
  }
  return cutBase;
}

void FormatConverter::dump8BitHeightmap(Bitmap &heightMap,
                                        const std::string &path,
                                        const std::string &colourMapKey) const {
  Utils::Logging::logLine("FormatConverter::Copying heightmap to ", path);
  if (gameTag == "Vic3") {
    // need to scale to default vic3 map sizes, due to their compression
    int width = heightMap.width();
    int height = heightMap.height();
    std::vector<uint8_t> pixels(width * height * 4, 0);
    for (auto h = 0; h < height; h++) {
      for (auto w = 0; w < width; w++) {
        auto colourmapIndex = h * width + w;
        const auto &c = heightMap[colourmapIndex];

        auto imageIndex = 4 * (h * width + w);
        pixels[imageIndex] = c.getBlue();
        pixels[imageIndex + 1] = c.getGreen();
        pixels[imageIndex + 2] = c.getRed();
        pixels[imageIndex + 3] = 255;
      }
    }
    DirectX::Image image(width, height, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
                         sizeof(uint8_t) * width * 4,
                         sizeof(uint8_t) * width * height, pixels.data());
    // resize the image to the full vic3 resolution
    DirectX::ScratchImage scaledImg;
    DirectX::Resize(image, 16384, 7232, DirectX::TEX_FILTER_FORCE_NON_WIC,
                    scaledImg);
    auto scaledMap = Bitmap(16384, 7232, 24);
    // write it to a BMP
    for (int i = 0; i < 16384 * 7232 * 4; i += 4) {
      scaledMap.setColourAtIndex(i / 4, (scaledImg.GetPixels()[i],
                                         scaledImg.GetPixels()[i + 1],
                                         scaledImg.GetPixels()[i + 2]));
    }
    // save the bmp as a png
    Png::save(scaledMap, path + ".png", false);
    // overwrite the current heightmap with the scaled one, for later packing
    heightMap = scaledMap;
  } else {
    Bitmap outputMap(Cfg::Values().width, Cfg::Values().height, 8);
    outputMap.colourtable = colourTables.at(colourMapKey + gameTag);
    // now map from 24 bit climate map
    for (int i = 0; i < Cfg::Values().bitmapSize; i++)
      //  outputMap.bit8Buffer[i] = heightMap[i].getRed();
      outputMap.setColourAtIndex(i, outputMap.lookUp(heightMap[i].getRed()));

    Bmp::save8bit(outputMap, path + ".bmp");
  }
}

void FormatConverter::dump8BitTerrain(const Bitmap &climateIn,
                                      const std::string &path,
                                      const std::string &colourMapKey,
                                      const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing terrain to ", path);
  auto &conf = Cfg::Values();
  Bitmap hoi4terrain(conf.width, conf.height, 8);
  hoi4terrain.colourtable = colourTables.at(colourMapKey + gameTag);
  if (!cut) {
    // now map from 24 bit climate map
    for (int i = 0; i < conf.bitmapSize; i++) {
      hoi4terrain.setColourAtIndex(
          i, hoi4terrain.lookUp(
                 colourMaps.at(colourMapKey + gameTag).at(climateIn[i])));
    }
  } else {
    hoi4terrain = cutBaseMap("//terrain.bmp");
  }
  Bmp::save8bit(hoi4terrain, path);
}

void FormatConverter::dump8BitCities(const Bitmap &climateIn,
                                     const std::string &path,
                                     const std::string &colourMapKey,
                                     const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing cities to ", path);
  Bitmap cities(Cfg::Values().width, Cfg::Values().height, 8);
  cities.colourtable = colourTables.at(colourMapKey + gameTag);
  if (!cut) {
    for (int i = 0; i < Cfg::Values().bitmapSize; i++)
      cities.setColourAtIndex(
          i,
          cities.lookUp(climateIn[i] == Cfg::Values().colours["sea"] ? 15 : 1));
  } else {
    cities = cutBaseMap("//cities.bmp");
  }
  Bmp::save8bit(cities, path);
}

void FormatConverter::dump8BitRivers(const Bitmap &riversIn,
                                     const std::string &path,
                                     const std::string &colourMapKey,
                                     const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing rivers to ", path);

  Bitmap rivers(Cfg::Values().width, Cfg::Values().height, 8);
  rivers.colourtable = colourTables.at(colourMapKey + gameTag);
  if (!cut) {
    for (int i = 0; i < Cfg::Values().bitmapSize; i++)
      try {
        rivers.setColourAtIndex(
            i, rivers.lookUp(
                   colourMaps.at(colourMapKey + gameTag).at(riversIn[i])));
      } catch (std::exception e) {
        Utils::Logging::logLine("Error at index ", i, " colour", riversIn[i],
                                e.what());
      }
  } else {
    rivers = cutBaseMap("//rivers.bmp");
  }
  if (gameTag == "Vic3") {
    auto scaledMap = Bmp::scale(rivers, 8192, 3616, false);
    Png::save(scaledMap, path + ".png");
  } else {
    Bmp::save8bit(rivers, path + ".bmp");
  }
}

void FormatConverter::dump8BitTrees(const Bitmap &climate,
                                    const Bitmap &treesIn,
                                    const std::string &path,
                                    const std::string &colourMapKey,
                                    const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing trees to ", path);
  const double width = Cfg::Values().width;
  constexpr auto factor = 3.4133333333333333333333333333333;
  Bitmap trees(((double)Cfg::Values().width / factor),
               ((double)Cfg::Values().height / factor), 8);
  trees.colourtable = colourTables.at(colourMapKey + gameTag);

  if (!cut) {
    for (auto i = 0; i < trees.height(); i++) {
      for (auto w = 0; w < trees.width(); w++) {
        double refHeight = ceil((double)i * factor);
        double refWidth =
            std::clamp((double)w * factor, 0.0, (double)Cfg::Values().width);
        // map the colour from
        trees.setColourAtIndex(
            i * trees.width() + w,
            trees.lookUp(colourMaps.at(colourMapKey + gameTag)
                             .at(treesIn[refHeight * width + refWidth])));
      }
    }
  } else {
    trees = cutBaseMap("//trees.bmp", (1.0 / factor));
  }
  Bmp::save8bit(trees, path);
}

void FormatConverter::dumpDDSFiles(const Bitmap &riverMap,
                                   const Bitmap &heightMap,
                                   const std::string &path, const bool cut,
                                   const int maxFactor) const {
  Utils::Logging::logLine("FormatConverter::Writing DDS files to ", path);
  using namespace DirectX;
  const auto &width = Cfg::Values().width;

  for (auto factor = 2, counter = 0; factor <= maxFactor;
       factor *= 2, counter++) {
    auto tempPath{path};
    if (gameTag == "Hoi4")
      tempPath += std::to_string(counter);
    tempPath += ".dds";
    auto imageWidth = width / factor;
    auto imageHeight = Cfg::Values().height / factor;
    std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);

    for (auto h = 0; h < imageHeight; h++) {
      for (auto w = 0; w < imageWidth; w++) {
        auto referenceIndex = factor * h * width + factor * w;
        double depth = (double)heightMap[referenceIndex].getBlue() /
                       (double)Cfg::Values().seaLevel;
        auto imageIndex =
            imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
        imageIndex *= 4;
        if (riverMap[referenceIndex] == Cfg::Values().colours["sea"]) {
          pixels[imageIndex] = static_cast<unsigned char>(49.0 * depth);
          pixels[imageIndex + 1] = static_cast<unsigned char>(24.0 * depth);
          pixels[imageIndex + 2] = static_cast<unsigned char>(16.0 * depth);
          pixels[imageIndex + 3] = 255;
        } else {
          pixels[imageIndex] = 100;
          pixels[imageIndex + 1] = 100;
          pixels[imageIndex + 2] = 50;
          pixels[imageIndex + 3] = 255;
        }
      }
    }
    writeDDS(imageWidth, imageHeight, pixels, DXGI_FORMAT_B8G8R8A8_UNORM,
             tempPath);
  }
}

void FormatConverter::dumpTerrainColourmap(
    const Bitmap &climateMap, const Bitmap &cityMap, const std::string &modPath,
    const std::string &mapName, const DXGI_FORMAT format, int scaleFactor,
    const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing terrain colourmap to ",
                          modPath + mapName);
  auto &cfg = Cfg::Values();
  const auto &height = climateMap.height();
  const auto &width = climateMap.width();
  int factor = scaleFactor;
  auto imageWidth = width / factor;
  auto imageHeight = height / factor;

  std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);
  if (!cut) {
    for (auto h = 0; h < imageHeight; h++) {
      for (auto w = 0; w < imageWidth; w++) {
        auto colourmapIndex = factor * h * width + factor * w;
        const auto &c = climateMap[colourmapIndex];
        auto imageIndex =
            imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
        imageIndex *= 4;
        pixels[imageIndex] = c.getBlue();
        pixels[imageIndex + 1] = c.getGreen();
        pixels[imageIndex + 2] = c.getRed();
        if (gameTag == "Eu4" || gameTag == "Vic3") {
          pixels[imageIndex + 3] = 255;
        } else
          // alpha for city lights
          pixels[imageIndex + 3] = static_cast<unsigned char>(
              255.0 * (cityMap[colourmapIndex] / cfg.colours["cities"]));
      }
    }
  } else {
    // load base game colourmap
    pixels = readDDS(gamePath + mapName);
    auto maxY = 1024 - cfg.maxY / (double)factor;
    auto minY = 1024 - cfg.minY / (double)factor;
    auto maxX = cfg.maxX / (double)factor;
    auto minX = cfg.minX / (double)factor;
    std::swap(minY, maxY);
    // cut it and reassign it
    pixels = Utils::cutBuffer(pixels, 2816, 1024, minX, maxX, minY, maxY, 4);
    if (cfg.scale) {
      pixels = Utils::scaleBuffer(pixels, abs(maxX - minX), abs(maxY - minY),
                                  cfg.scaleX / factor, cfg.scaleY / factor, 4,
                                  cfg.keepRatio);
    }
  }
  if (gameTag == "Vic3") {
    Textures::writeMipMapDDS(imageWidth, imageHeight, pixels, format,
                             modPath + mapName, true);
  } else {

    if (cfg.scale)
      writeDDS(cfg.scaleX / factor, cfg.scaleY / factor, pixels, format,
               modPath + mapName);
    else
      writeDDS(imageWidth, imageHeight, pixels, format, modPath + mapName);
  }
}

void FormatConverter::dumpWorldNormal(const Bitmap &sobelMap,
                                      const std::string &path,
                                      const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing normalMap to ", path);
  auto height = Cfg::Values().height;
  auto width = Cfg::Values().width;

  int factor = 2; // image width and height are halved
  Bitmap normalMap(width / factor, height / factor, 24);
  if (!cut) {
    for (auto i = 0; i < normalMap.height(); i++)
      for (auto w = 0; w < normalMap.width(); w++)
        normalMap.setColourAtIndex(i * normalMap.width() + w,
                                   sobelMap[factor * i * width + factor * w]);
  } else {
    normalMap = cutBaseMap("//world_normal.bmp", (1.0 / (double)factor), 24);
    for (auto i = 0; i < 5; i++)
      normalMap.imageData = Bmp::filter(normalMap);
  }
  Bmp::save(normalMap, (path).c_str());
}

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
    Png::save(packedHeightMap, path + ".png");
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

void FormatConverter::Vic3ColourMaps(const Fwg::Gfx::Bitmap &climateMap,
                                     const Fwg::Gfx::Bitmap &treesIn,
                                     const Fwg::Gfx::Bitmap &heightMap,
                                     const Fwg::Gfx::Bitmap &humidityMap,
                                     const std::string &path) {

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
      int val = 0;
      if (c.getBlue() > config.seaLevel)
        val = 255;
      auto imageIndex =
          imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
      imageIndex *= 4;
      pixels[imageIndex] = val;
      pixels[imageIndex + 1] = val;
      pixels[imageIndex + 2] = val;
      pixels[imageIndex + 3] = 255;
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
      pixels[imageIndex] = col.getRed();
      pixels[imageIndex + 1] = col.getGreen();
      pixels[imageIndex + 2] = col.getBlue();
      pixels[imageIndex + 3] = 255;
    }
  }
  Textures::writeMipMapDDS(imageWidth, imageHeight, pixels,
                           DXGI_FORMAT_B8G8R8A8_UNORM,
                           path + "//textures//flatmap.dds");

  // terrain colour map
  scaledMap = Bmp::scale(climateMap, 8192, 4096, false);
  dumpTerrainColourmap(scaledMap, scaledMap, path, "//textures//colormap.dds",
                       DXGI_FORMAT_B8G8R8A8_UNORM, 1, false);

  Utils::Logging::logLine(
      "FormatConverter::Writing watercolor_rgb_waterspec_a to ", path);
  using namespace DirectX;

  scaledMap = Bmp::scale(heightMap, 4096, 1808, false);
  imageWidth = scaledMap.width();
  imageHeight = scaledMap.height();
  for (auto h = 0; h < imageHeight; h++) {
    for (auto w = 0; w < imageWidth; w++) {
      auto referenceIndex = h * width + w;
      double depth = (double)scaledMap[referenceIndex].getBlue() /
                     (double)Cfg::Values().seaLevel;
      auto imageIndex =
          imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
      imageIndex *= 4;
      if (depth < 1.0) {
        pixels[imageIndex] = static_cast<unsigned char>(49.0 * depth);
        pixels[imageIndex + 1] = static_cast<unsigned char>(24.0 * depth);
        pixels[imageIndex + 2] = static_cast<unsigned char>(16.0 * depth);
        pixels[imageIndex + 3] = 255;
      } else {
        pixels[imageIndex] = 100;
        pixels[imageIndex + 1] = 100;
        pixels[imageIndex + 2] = 50;
        pixels[imageIndex + 3] = 255;
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
      auto referenceIndex = h * width + w;
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

      pixels[imageIndex] = col.getRed();
      pixels[imageIndex + 1] = col.getGreen();
      pixels[imageIndex + 2] = col.getBlue();
      pixels[imageIndex + 3] = 255;
    }
  }
  writeDDS(imageWidth, imageHeight, pixels, DXGI_FORMAT_B8G8R8A8_UNORM,
           path + "//textures//colormap_tree.dds");

  scaledHeight = Bmp::scale(heightMap, 1024, 452, false);
  imageWidth = scaledHeight.width();
  imageHeight = scaledHeight.height();
  for (auto h = 0; h < imageHeight; h++) {
    for (auto w = 0; w < imageWidth; w++) {
      auto referenceIndex = h * width + w;
      double humidity = (double)scaledMap[referenceIndex].getBlue() / 255.0;
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

      pixels[imageIndex] = col;
      pixels[imageIndex + 1] = col;
      pixels[imageIndex + 2] = col;
      pixels[imageIndex + 3] = 255;
    }
  }

  Textures::writeMipMapDDS(imageWidth, imageHeight, pixels,
                           DXGI_FORMAT_B8G8R8A8_UNORM,
                           path + "//textures//windmap_tree.dds", true);
}

void FormatConverter::dynamicMasks(const std::string &path) {
  // TODO: exclusion_mask.dds
  //
}

void FormatConverter::detailIndexMap(const Fwg::Gfx::Bitmap &climateMap,
                                     const std::string &path) {

  auto copy = Fwg::Gfx::Bmp::scale(climateMap, 8192, 3616, false);
  const auto &height = copy.height();
  const auto &width = copy.width();
  int factor = 1;
  auto imageWidth = width / factor;
  auto imageHeight = height / factor;

  std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);
  for (auto h = 0; h < imageHeight; h++) {
    for (auto w = 0; w < imageWidth; w++) {
      auto colourmapIndex = factor * h * width + factor * w;
      const auto &c = colourMaps2.at("terrainVic3").at(copy[colourmapIndex]);
      auto imageIndex =
          imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
      imageIndex *= 4;
      pixels[imageIndex] = c.getBlue();
      pixels[imageIndex + 1] = c.getGreen();
      pixels[imageIndex + 2] = c.getRed();
      pixels[imageIndex + 3] = 255;
    }
  }

  Textures::writeTGA(8192, 3616, pixels, path + "//terrain//detail_index.tga");
}

FormatConverter::FormatConverter(const std::string &gamePath,
                                 const std::string &gameTag)
    : gamePath{gamePath}, gameTag{gameTag} {
  std::string mapFolderName = "//map";
  if (gameTag == "Vic3")
    mapFolderName.append("_data");
  std::string terrainsourceString =
      (gamePath + mapFolderName + "//terrain.bmp");
  Bitmap terrain = Bmp::load24Bit(terrainsourceString, "terrain");
  colourTables["terrain" + gameTag] = terrain.colourtable;

  std::string citySource = (gamePath + mapFolderName + "//terrain.bmp");
  Bitmap cities = Bmp::load24Bit(citySource, "cities");
  colourTables["cities" + gameTag] = cities.colourtable;

  std::string riverSource = (gamePath + mapFolderName + "//rivers.bmp");
  Bitmap rivers = Bmp::load24Bit(riverSource, "rivers");
  colourTables["rivers" + gameTag] = rivers.colourtable;

  std::string treeSource = (gamePath + mapFolderName + "//trees.bmp");
  Bitmap trees = Bmp::load24Bit(treeSource, "trees");
  colourTables["trees" + gameTag] = trees.colourtable;

  std::string heightmapSource = (gamePath + mapFolderName + "//heightmap.bmp");
  Bitmap heightmap = Bmp::load24Bit(heightmapSource, "heightmap");
  colourTables["heightmap" + gameTag] = heightmap.colourtable;
}

FormatConverter::~FormatConverter() {}
} // namespace Scenario::Gfx