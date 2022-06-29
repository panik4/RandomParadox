#include "generic/FormatConverter.h"
namespace Scenario::Gfx {
using namespace Textures;
using namespace Fwg;
using namespace Fwg::Gfx;
const std::map<std::string, std::map<Gfx::Colour, int>>
    FormatConverter::colourMaps{
        {"terrainHoi4",
         {{Env::Instance().colours["grassland"], 0},
          {Env::Instance().colours["ice"], 19},
          {Env::Instance().colours["tundra"], 9},
          {Env::Instance().colours["forest"], 1},
          {Env::Instance().colours["jungle"], 21},
          {Env::Instance().colours["savannah"], 0},
          {Env::Instance().colours["desert"], 7},
          {Env::Instance().colours["peaks"], 16},
          {Env::Instance().colours["mountains"], 11},
          {Env::Instance().colours["hills"], 20},
          {Env::Instance().colours["sea"], 15}}},
        {"riversHoi4",
         {{Env::Instance().colours["land"], 255},
          {Env::Instance().colours["river"], 3},
          {Env::Instance().colours["river"] * 0.9, 3},
          {Env::Instance().colours["river"] * 0.8, 6},
          {Env::Instance().colours["river"] * 0.7, 6},
          {Env::Instance().colours["river"] * 0.6, 10},
          {Env::Instance().colours["river"] * 0.5, 11},
          {Env::Instance().colours["river"] * 0.4, 11},
          {Env::Instance().colours["sea"], 254},
          {Env::Instance().colours["riverStart"], 0},
          {Env::Instance().colours["riverStartTributary"], 3},
          {Env::Instance().colours["riverEnd"], 1}}},
        {"treesHoi4",
         {{Env::Instance().colours["grassland"], 0},
          {Env::Instance().colours["ice"], 0},
          {Env::Instance().colours["tundra"], 0},
          {Env::Instance().colours["forest"], 6},
          {Env::Instance().colours["jungle"], 28},
          {Env::Instance().colours["savannah"], 0},
          {Env::Instance().colours["desert"], 0},
          {Env::Instance().colours["peaks"], 0},
          {Env::Instance().colours["mountains"], 0},
          {Env::Instance().colours["hills"], 0},
          {Env::Instance().colours["empty"], 0},
          {Env::Instance().colours["sea"], 0}}},
        {"riversEu4",
         {{Env::Instance().colours["land"], 255},
          {Env::Instance().colours["river"], 3},
          {Env::Instance().colours["river"] * 0.9, 3},
          {Env::Instance().colours["river"] * 0.8, 5},
          {Env::Instance().colours["river"] * 0.7, 7},
          {Env::Instance().colours["river"] * 0.6, 9},
          {Env::Instance().colours["river"] * 0.5, 10},
          {Env::Instance().colours["river"] * 0.4, 11},
          {Env::Instance().colours["sea"], 254},
          {Env::Instance().colours["riverStart"], 0},
          {Env::Instance().colours["riverStartTributary"], 3},
          {Env::Instance().colours["riverEnd"], 1}}},
        {"treesEu4",
         {{Env::Instance().colours["grassland"], 0},
          {Env::Instance().colours["ice"], 0},
          {Env::Instance().colours["tundra"], 0},
          {Env::Instance().colours["forest"], 6},
          {Env::Instance().colours["jungle"], 28},
          {Env::Instance().colours["savannah"], 0},
          {Env::Instance().colours["desert"], 0},
          {Env::Instance().colours["peaks"], 0},
          {Env::Instance().colours["mountains"], 0},
          {Env::Instance().colours["hills"], 0},
          {Env::Instance().colours["empty"], 0},
          {Env::Instance().colours["sea"], 0}}},
        {"terrainEu4",
         {{Env::Instance().colours["grassland"], 0},
          {Env::Instance().colours["ice"], 16},
          {Env::Instance().colours["tundra"], 0},
          {Env::Instance().colours["forest"], 1},
          {Env::Instance().colours["jungle"], 0},
          {Env::Instance().colours["savannah"], 0},
          {Env::Instance().colours["desert"], 4},
          {Env::Instance().colours["peaks"], 16},
          {Env::Instance().colours["mountains"], 6},
          {Env::Instance().colours["hills"], 1},
          {Env::Instance().colours["sea"], 15}}}};

Bitmap FormatConverter::cutBaseMap(const std::string &path, const double factor,
                                   const int bit) const {
  auto &conf = Env::Instance();
  std::string sourceMap{gamePath + path};
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

void FormatConverter::dump8BitHeightmap(const Bitmap &heightMap,
                                        const std::string &path,
                                        const std::string &colourMapKey) const {
  Utils::Logging::logLine("FormatConverter::Copying heightmap to ", path);
  Bitmap hoi4Heightmap(Env::Instance().width, Env::Instance().height, 8);
  hoi4Heightmap.colourtable = colourTables.at(colourMapKey + gameTag);
  // now map from 24 bit climate map
  for (int i = 0; i < Env::Instance().bitmapSize; i++)
    hoi4Heightmap.bit8Buffer[i] = heightMap[i].getRed();
  Bmp::save(hoi4Heightmap, path);
}

void FormatConverter::dump8BitTerrain(const Bitmap &climateIn,
                                      const std::string &path,
                                      const std::string &colourMapKey,
                                      const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing terrain to ", path);
  auto &conf = Env::Instance();
  Bitmap hoi4terrain(conf.width, conf.height, 8);
  hoi4terrain.colourtable = colourTables.at(colourMapKey + gameTag);
  if (!cut) {
    // now map from 24 bit climate map
    for (int i = 0; i < conf.bitmapSize; i++) {

      // std::cout << colourMapKey + gameTag << std::endl;
      hoi4terrain.bit8Buffer[i] =
          colourMaps.at(colourMapKey + gameTag).at(climateIn[i]);
    }
  } else {
    hoi4terrain = cutBaseMap("\\map\\terrain.bmp");
  }
  Bmp::save(hoi4terrain, path);
}

void FormatConverter::dump8BitCities(const Bitmap &climateIn,
                                     const std::string &path,
                                     const std::string &colourMapKey,
                                     const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing cities to ", path);
  Bitmap cities(Env::Instance().width, Env::Instance().height, 8);
  cities.colourtable = colourTables.at(colourMapKey + gameTag);
  if (!cut) {
    for (int i = 0; i < Env::Instance().bitmapSize; i++)
      cities.bit8Buffer[i] =
          climateIn[i] == Env::Instance().colours["sea"] ? 15 : 1;
  } else {
    cities = cutBaseMap("\\map\\cities.bmp");
  }
  Bmp::save(cities, path);
}

void FormatConverter::dump8BitRivers(const Bitmap &riversIn,
                                     const std::string &path,
                                     const std::string &colourMapKey,
                                     const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing rivers to ", path);
  Bitmap rivers(Env::Instance().width, Env::Instance().height, 8);
  rivers.colourtable = colourTables.at(colourMapKey + gameTag);

  if (!cut) {
    for (int i = 0; i < Env::Instance().bitmapSize; i++)
      rivers.bit8Buffer[i] =
          colourMaps.at(colourMapKey + gameTag).at(riversIn[i]);
  } else {
    rivers = cutBaseMap("\\map\\rivers.bmp");
  }
  Bmp::save(rivers, path);
}

void FormatConverter::dump8BitTrees(const Bitmap &climate,
                                    const Bitmap &treesIn,
                                    const std::string &path,
                                    const std::string &colourMapKey,
                                    const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing trees to ", path);
  const double width = Env::Instance().width;
  constexpr auto factor = 3.4133333333333333333333333333333;
  Bitmap trees(((double)Env::Instance().width / factor),
               ((double)Env::Instance().height / factor), 8);
  trees.colourtable = colourTables.at(colourMapKey + gameTag);

  if (!cut) {
    for (auto i = 0; i < trees.bInfoHeader.biHeight; i++) {
      for (auto w = 0; w < trees.bInfoHeader.biWidth; w++) {
        double refHeight = ceil((double)i * factor);
        double refWidth =
            std::clamp((double)w * factor, 0.0, (double)Env::Instance().width);
        // map the colour from
        trees.bit8Buffer[i * trees.bInfoHeader.biWidth + w] =
            colourMaps.at(colourMapKey + gameTag)
                .at(treesIn[refHeight * width + refWidth]);
      }
    }
  } else {
    trees = cutBaseMap("\\map\\trees.bmp", (1.0 / factor));
  }
  Bmp::save(trees, path);
}

void FormatConverter::dumpDDSFiles(const Bitmap &riverMap,
                                   const Bitmap &heightMap,
                                   const std::string &path, const bool cut,
                                   const int maxFactor) const {
  Utils::Logging::logLine("FormatConverter::Writing DDS files to ", path);
  using namespace DirectX;
  const auto &width = Env::Instance().width;

  for (auto factor = 2, counter = 0; factor <= maxFactor;
       factor *= 2, counter++) {
    auto tempPath{path};
    if (gameTag == "Hoi4")
      tempPath += std::to_string(counter);
    tempPath += ".dds";
    auto imageWidth = width / factor;
    auto imageHeight = Env::Instance().height / factor;
    std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);

    for (auto h = 0; h < imageHeight; h++) {
      for (auto w = 0; w < imageWidth; w++) {
        auto referenceIndex = factor * h * width + factor * w;
        double depth = (double)heightMap[referenceIndex].getBlue() /
                       (double)Env::Instance().seaLevel;
        auto imageIndex =
            imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
        imageIndex *= 4;
        if (riverMap[referenceIndex] == Env::Instance().colours["sea"]) {
          pixels[imageIndex] = 49 * depth;
          pixels[imageIndex + 1] = 24 * depth;
          pixels[imageIndex + 2] = 16 * depth;
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

void FormatConverter::dumpTerrainColourmap(const Bitmap &climateMap,
                                           const Bitmap &cityMap,
                                           const std::string &modPath,
                                           const std::string &mapName,
                                           const DXGI_FORMAT format,
                                           const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing terrain colourmap to ",
                          modPath + mapName);
  auto &config = Env::Instance();
  const auto &height = config.height;
  const auto &width = config.width;
  int factor = 2; // map dimensions are halved
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
        if (gameTag == "Eu4") {
          pixels[imageIndex + 3] = 255;
        } else
          pixels[imageIndex + 3] =
              255.0 *
              (cityMap[colourmapIndex] /
               Env::Instance().colours["cities"]); // alpha for city lights
      }
    }
  } else {
    // load base game colourmap
    pixels = readDDS(gamePath + mapName);
    auto maxY = 1024 - config.maxY / (double)factor;
    auto minY = 1024 - config.minY / (double)factor;
    auto maxX = config.maxX / (double)factor;
    auto minX = config.minX / (double)factor;
    std::swap(minY, maxY);
    // cut it and reassign it
    pixels = Utils::cutBuffer(pixels, 2816, 1024, minX, maxX, minY, maxY, 4);
    if (config.scale) {
      pixels = Utils::scaleBuffer(pixels, abs(maxX - minX), abs(maxY - minY),
                                  config.scaleX / factor,
                                  config.scaleY / factor, 4, config.keepRatio);
    }
  }
  if (config.scale)
    writeDDS(config.scaleX / factor, config.scaleY / factor, pixels, format,
             modPath + mapName);
  else
    writeDDS(imageWidth, imageHeight, pixels, format, modPath + mapName);
}

void FormatConverter::dumpWorldNormal(const Bitmap &sobelMap,
                                      const std::string &path,
                                      const bool cut) const {
  Utils::Logging::logLine("FormatConverter::Writing normalMap to ", path);
  auto height = Env::Instance().height;
  auto width = Env::Instance().width;

  int factor = 2; // image width and height are halved
  Bitmap normalMap(width / factor, height / factor, 24);
  if (!cut) {
    for (auto i = 0; i < normalMap.bInfoHeader.biHeight; i++)
      for (auto w = 0; w < normalMap.bInfoHeader.biWidth; w++)
        normalMap.setColourAtIndex(i * normalMap.bInfoHeader.biWidth + w,
                                   sobelMap[factor * i * width + factor * w]);
  } else {
    normalMap =
        cutBaseMap("\\map\\world_normal.bmp", (1.0 / (double)factor), 24);
  }
  Bmp::save(normalMap, (path).c_str());
}

FormatConverter::FormatConverter(const std::string &gamePath,
                                 const std::string &gameTag)
    : gamePath{gamePath}, gameTag{gameTag} {
  std::string terrainsourceString = (gamePath + "\\map\\terrain.bmp");
  Bitmap terrain = Bmp::load8Bit(terrainsourceString, "terrain");
  colourTables["terrain" + gameTag] = terrain.colourtable;

  std::string citySource = (gamePath + "\\map\\terrain.bmp");
  Bitmap cities = Bmp::load8Bit(citySource, "cities");
  colourTables["cities" + gameTag] = cities.colourtable;

  std::string riverSource = (gamePath + "\\map\\rivers.bmp");
  Bitmap rivers = Bmp::load8Bit(riverSource, "rivers");
  colourTables["rivers" + gameTag] = rivers.colourtable;

  std::string treeSource = (gamePath + "\\map\\trees.bmp");
  Bitmap trees = Bmp::load8Bit(treeSource, "trees");
  colourTables["trees" + gameTag] = trees.colourtable;

  std::string heightmapSource = (gamePath + "\\map\\heightmap.bmp");
  Bitmap heightmap = Bmp::load8Bit(heightmapSource, "heightmap");
  colourTables["heightmap" + gameTag] = heightmap.colourtable;
}

FormatConverter::~FormatConverter() {}
} // namespace Scenario::Gfx