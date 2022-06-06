#include "generic/FormatConverter.h"
const std::map<std::string, std::map<Colour, int>> FormatConverter::colourMaps{
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
      {Env::Instance().namedColours["sea"], 0}}},
    {"riversEu4",
     {{Env::Instance().namedColours["land"], 255},
      {Env::Instance().namedColours["river"], 3},
      {Env::Instance().namedColours["river"] * 0.9, 3},
      {Env::Instance().namedColours["river"] * 0.8, 5},
      {Env::Instance().namedColours["river"] * 0.7, 7},
      {Env::Instance().namedColours["river"] * 0.6, 9},
      {Env::Instance().namedColours["river"] * 0.5, 10},
      {Env::Instance().namedColours["river"] * 0.4, 11},
      {Env::Instance().namedColours["sea"], 254},
      {Env::Instance().namedColours["riverStart"], 0},
      {Env::Instance().namedColours["riverStartTributary"], 3},
      {Env::Instance().namedColours["riverEnd"], 1}}},
    {"treesEu4",
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
      {Env::Instance().namedColours["sea"], 0}}},
    {"terrainEu4",
     {{Env::Instance().namedColours["grassland"], 0},
      {Env::Instance().namedColours["ice"], 16},
      {Env::Instance().namedColours["tundra"], 0},
      {Env::Instance().namedColours["forest"], 1},
      {Env::Instance().namedColours["jungle"], 0},
      {Env::Instance().namedColours["savannah"], 0},
      {Env::Instance().namedColours["desert"], 4},
      {Env::Instance().namedColours["peaks"], 16},
      {Env::Instance().namedColours["mountains"], 6},
      {Env::Instance().namedColours["hills"], 1},
      {Env::Instance().namedColours["sea"], 15}}}};

Bitmap FormatConverter::cutBaseMap(const std::string &path, const double factor,
                                   const int bit) const {
  auto &conf = Env::Instance();
  std::string sourceMap{gamePath + path};
  Bitmap baseMap = bit == 24 ? Bitmap::Load24bitBMP(sourceMap.c_str(), "")
                             : Bitmap::Load8bitBMP(sourceMap.c_str(), "");
  auto &cutBase = baseMap.cut(conf.minX * factor, conf.maxX * factor,
                              conf.minY * factor, conf.maxY * factor, factor);
  if (conf.scale) {
    cutBase = cutBase.scale(conf.scaleX * factor, conf.scaleY * factor,
                            conf.keepRatio);
  }
  return cutBase;
}

void FormatConverter::dump8BitHeightmap(const std::string path,
                                        const std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Copying heightmap to ", path);
  Bitmap hoi4Heightmap(Env::Instance().width, Env::Instance().height, 8);
  hoi4Heightmap.getColourtable() = colourTables.at(colourMapKey + gameTag);
  // now map from 24 bit climate map
  const auto &heightmap = Bitmap::findBitmapByKey("heightmap");
  for (int i = 0; i < Env::Instance().bitmapSize; i++)
    hoi4Heightmap.bit8Buffer[i] = heightmap.getColourAtIndex(i).getRed();
  Bitmap::SaveBMPToFile(hoi4Heightmap, (path).c_str());
}

void FormatConverter::dump8BitTerrain(const std::string path,
                                      const std::string colourMapKey,
                                      const bool cut) const {
  Logger::logLine("FormatConverter::Writing terrain to ", path);
  auto &conf = Env::Instance();
  Bitmap hoi4terrain(conf.width, conf.height, 8);
  hoi4terrain.getColourtable() = colourTables.at(colourMapKey + gameTag);
  if (!cut) {
    // now map from 24 bit climate map
    const auto &climate = Bitmap::findBitmapByKey("climate");
    for (int i = 0; i < conf.bitmapSize; i++) {

      // std::cout << colourMapKey + gameTag << std::endl;
      hoi4terrain.bit8Buffer[i] =
          colourMaps.at(colourMapKey + gameTag).at(climate.getColourAtIndex(i));
    }
  } else {
    hoi4terrain = cutBaseMap("\\map\\terrain.bmp");
  }
  Bitmap::SaveBMPToFile(hoi4terrain, (path).c_str());
}

void FormatConverter::dump8BitCities(const std::string path,
                                     const std::string colourMapKey,
                                     const bool cut) const {
  Logger::logLine("FormatConverter::Writing cities to ", path);
  Bitmap cities(Env::Instance().width, Env::Instance().height, 8);
  cities.getColourtable() = colourTables.at(colourMapKey + gameTag);

  const auto &climate = Bitmap::findBitmapByKey("climate");
  if (!cut) {
    for (int i = 0; i < Env::Instance().bitmapSize; i++)
      cities.bit8Buffer[i] =
          climate.getColourAtIndex(i) == Env::Instance().namedColours["sea"]
              ? 15
              : 1;
  } else {
    cities = cutBaseMap("\\map\\cities.bmp");
  }

  Bitmap::SaveBMPToFile(cities, (path).c_str());
}

void FormatConverter::dump8BitRivers(const std::string path,
                                     const std::string colourMapKey,
                                     const bool cut) const {
  Logger::logLine("FormatConverter::Writing rivers to ", path);
  Bitmap rivers(Env::Instance().width, Env::Instance().height, 8);
  rivers.getColourtable() = colourTables.at(colourMapKey + gameTag);

  if (!cut) {
    const auto &rivers2 = Bitmap::findBitmapByKey("rivers");
    for (int i = 0; i < Env::Instance().bitmapSize; i++)
      rivers.bit8Buffer[i] =
          colourMaps.at(colourMapKey + gameTag).at(rivers2.getColourAtIndex(i));
  } else {
    rivers = cutBaseMap("\\map\\rivers.bmp");
  }
  Bitmap::SaveBMPToFile(rivers, (path).c_str());
}

void FormatConverter::dump8BitTrees(const std::string path,
                                    const std::string colourMapKey,
                                    const bool cut) const {
  Logger::logLine("FormatConverter::Writing trees to ", path);
  const double width = Env::Instance().width;
  constexpr auto factor = 3.4133333333333333333333333333333;
  Bitmap trees(((double)Env::Instance().width / factor),
               ((double)Env::Instance().height / factor), 8);
  trees.getColourtable() = colourTables.at(colourMapKey + gameTag);

  const auto &climate = Bitmap::findBitmapByKey("climate");
  const auto &fGenTrees = Bitmap::findBitmapByKey("trees");
  if (!cut) {
    for (auto i = 0; i < trees.bInfoHeader.biHeight; i++) {
      for (auto w = 0; w < trees.bInfoHeader.biWidth; w++) {
        double refHeight = ceil((double)i * factor);
        double refWidth =
            std::clamp((double)w * factor, 0.0, (double)Env::Instance().width);
        // map the colour from
        trees.bit8Buffer[i * trees.bInfoHeader.biWidth + w] =
            colourMaps.at(colourMapKey + gameTag)
                .at(fGenTrees.getColourAtIndex(refHeight * width + refWidth));
      }
    }
  } else {
    trees = cutBaseMap("\\map\\trees.bmp", (1.0 / factor));
  }
  Bitmap::SaveBMPToFile(trees, (path).c_str());
}

void FormatConverter::dumpDDSFiles(const std::string path, const bool cut,
                                   const int maxFactor) const {
  Logger::logLine("FormatConverter::Writing DDS files to ", path);
  using namespace DirectX;
  const auto &riverBMP = Bitmap::findBitmapByKey("rivers");
  const auto &heightBMP = Bitmap::findBitmapByKey("heightmap");
  const auto &width = Env::Instance().width;

  for (int factor = 2, counter = 0; factor <= maxFactor;
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
        double depth =
            (double)heightBMP.getColourAtIndex(referenceIndex).getBlue() /
            (double)Env::Instance().seaLevel;
        auto imageIndex =
            imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
        imageIndex *= 4;
        if (riverBMP.getColourAtIndex(referenceIndex) ==
            Env::Instance().namedColours["sea"]) {
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
    TextureWriter::writeDDS(imageWidth, imageHeight, pixels,
                            DXGI_FORMAT_B8G8R8A8_UNORM, tempPath);
  }
}

void FormatConverter::dumpTerrainColourmap(const std::string ModPath,
                                           const std::string mapName, const DXGI_FORMAT format,
                                           const bool cut) const {
  Logger::logLine("FormatConverter::Writing terrain colourmap to ", ModPath + mapName);
  auto &config = Env::Instance();
  const auto &climateMap = Bitmap::findBitmapByKey("climate2");
  const auto &cityMap = Bitmap::findBitmapByKey("cities");
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
        const auto &c = climateMap.getColourAtIndex(colourmapIndex);
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
            (cityMap.getColourAtIndex(colourmapIndex) /
             Env::Instance().namedColours["cities"]); // alpha for city lights
      }
    }
  } else {
    // load base game colourmap
    pixels = TextureWriter::readDDS(gamePath + mapName);
    auto maxY = 1024 - config.maxY / (double)factor;
    auto minY = 1024 - config.minY / (double)factor;
    auto maxX = config.maxX / (double)factor;
    auto minX = config.minX / (double)factor;
    std::swap(minY, maxY);
    // cut it and reassign it
    pixels = UtilLib::cutBuffer(pixels, 2816, 1024, minX, maxX, minY, maxY, 4);
    if (config.scale) {
      pixels = UtilLib::scaleBuffer(
          pixels, abs(maxX - minX), abs(maxY - minY), config.scaleX / factor,
          config.scaleY / factor, 4, config.keepRatio);
    }
  }
  if (config.scale)
    TextureWriter::writeDDS(config.scaleX / factor, config.scaleY / factor,
                            pixels, format,
                            ModPath + mapName);
  else
    TextureWriter::writeDDS(imageWidth, imageHeight, pixels,
                            format, ModPath + mapName);
}

void FormatConverter::dumpWorldNormal(const std::string path,
                                      const bool cut) const {
  Logger::logLine("FormatConverter::Writing normalMap to ", path);
  auto height = Env::Instance().height;
  auto width = Env::Instance().width;
  const auto &sobelMap = Bitmap::findBitmapByKey("sobel");

  int factor = 2; // image width and height are halved
  Bitmap normalMap(width / factor, height / factor, 24);
  if (!cut) {
    for (auto i = 0; i < normalMap.bInfoHeader.biHeight; i++)
      for (auto w = 0; w < normalMap.bInfoHeader.biWidth; w++)
        normalMap.setColourAtIndex(
            i * normalMap.bInfoHeader.biWidth + w,
            sobelMap.getColourAtIndex(factor * i * width + factor * w));
  } else {
    normalMap =
        cutBaseMap("\\map\\world_normal.bmp", (1.0 / (double)factor), 24);
  }
  Bitmap::SaveBMPToFile(normalMap, (path).c_str());
}

FormatConverter::FormatConverter(const std::string gamePath,
                                 const std::string gameTag)
    : gamePath{gamePath}, gameTag{gameTag} {
  std::string terrainsourceString = (gamePath + "\\map\\terrain.bmp");
  Bitmap terrain = Bitmap::Load8bitBMP(terrainsourceString.c_str(), "terrain");
  colourTables["terrain" + gameTag] = terrain.getColourtable();

  std::string citySource = (gamePath + "\\map\\terrain.bmp");
  Bitmap cities = Bitmap::Load8bitBMP(citySource.c_str(), "cities");
  colourTables["cities" + gameTag] = cities.getColourtable();

  std::string riverSource = (gamePath + "\\map\\rivers.bmp");
  Bitmap rivers = Bitmap::Load8bitBMP(riverSource.c_str(), "rivers");
  colourTables["rivers" + gameTag] = rivers.getColourtable();

  std::string treeSource = (gamePath + "\\map\\trees.bmp");
  Bitmap trees = Bitmap::Load8bitBMP(treeSource.c_str(), "trees");
  colourTables["trees" + gameTag] = trees.getColourtable();

  std::string heightmapSource = (gamePath + "\\map\\heightmap.bmp");
  Bitmap heightmap = Bitmap::Load8bitBMP(heightmapSource.c_str(), "heightmap");
  colourTables["heightmap" + gameTag] = heightmap.getColourtable();
}

FormatConverter::~FormatConverter() {}
