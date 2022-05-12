#include "generic/FormatConverter.h"
const std::map<std::string, std::map<Colour, int>>
     FormatConverter::colourMaps{
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

void FormatConverter::dump8BitHeightmap(const std::string path,
                                        const std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Copying heightmap to ", path);
  Bitmap hoi4Heightmap(Env::Instance().width, Env::Instance().height, 8);
  hoi4Heightmap.getColourtable() = colourTables.at(colourMapKey);
  // now map from 24 bit climate map
  const auto &heightmap = Bitmap::findBitmapByKey("heightmap");
  for (int i = 0; i < Env::Instance().bitmapSize; i++)
    hoi4Heightmap.bit8Buffer[i] = heightmap.getColourAtIndex(i).getRed();
  Bitmap::SaveBMPToFile(hoi4Heightmap, (path).c_str());
}

void FormatConverter::dump8BitTerrain(std::string path,
                                      std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Writing terrain to ", path);
  Bitmap hoi4terrain(Env::Instance().width, Env::Instance().height, 8);
  hoi4terrain.getColourtable() = colourTables.at(colourMapKey);
  // now map from 24 bit climate map
  const auto &climate = Bitmap::findBitmapByKey("climate");
  for (int i = 0; i < Env::Instance().bitmapSize; i++)
    hoi4terrain.bit8Buffer[i] =
        colourMaps.at(colourMapKey).at(climate.getColourAtIndex(i));
  Bitmap::SaveBMPToFile(hoi4terrain, (path).c_str());
}

void FormatConverter::dump8BitCities(std::string path,
                                     std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Writing cities to ", path);
  Bitmap cities(Env::Instance().width, Env::Instance().height, 8);
  cities.getColourtable() = colourTables.at(colourMapKey);

  const auto &climate = Bitmap::findBitmapByKey("climate");
  for (int i = 0; i < Env::Instance().bitmapSize; i++)
    cities.bit8Buffer[i] =
        climate.getColourAtIndex(i) == Env::Instance().namedColours["sea"] ? 15
                                                                           : 1;
  Bitmap::SaveBMPToFile(cities, (path).c_str());
}

void FormatConverter::dump8BitRivers(std::string path,
                                     std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Writing rivers to ", path);
  Bitmap rivers(Env::Instance().width, Env::Instance().height, 8);
  rivers.getColourtable() = colourTables.at(colourMapKey);

  const auto &rivers2 = Bitmap::findBitmapByKey("rivers");
  for (int i = 0; i < Env::Instance().bitmapSize; i++)
    rivers.bit8Buffer[i] =
        colourMaps.at(colourMapKey).at(rivers2.getColourAtIndex(i));
  Bitmap::SaveBMPToFile(rivers, (path).c_str());
}

void FormatConverter::dump8BitTrees(std::string path,
                                    std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Writing trees to ", path);
  const double width = Env::Instance().width;
  constexpr auto factor = 3.4133333333333333333333333333333;
  Bitmap trees(((double)Env::Instance().width / factor),
               ((double)Env::Instance().height / factor), 8);
  trees.getColourtable() = colourTables.at(colourMapKey);

  const auto &climate = Bitmap::findBitmapByKey("climate");
  const auto &fGenTrees = Bitmap::findBitmapByKey("trees");
  for (auto i = 0; i < trees.bInfoHeader.biHeight; i++) {
    for (auto w = 0; w < trees.bInfoHeader.biWidth; w++) {
      double refHeight = ceil((double)i * factor);
      double refWidth =
          std::clamp((double)w * factor, 0.0, (double)Env::Instance().width);
      // map the colour from
      trees.bit8Buffer[i * trees.bInfoHeader.biWidth + w] =
          colourMaps.at(colourMapKey)
              .at(fGenTrees.getColourAtIndex(refHeight * width + refWidth));
    }
  }
  Bitmap::SaveBMPToFile(trees, (path).c_str());
}

void FormatConverter::dumpDDSFiles(std::string path) const {
  Logger::logLine("FormatConverter::Writing DDS files to ", path);
  using namespace DirectX;
  const auto &riverBMP = Bitmap::findBitmapByKey("rivers");
  const auto &heightBMP = Bitmap::findBitmapByKey("heightmap");
  const auto &width = Env::Instance().width;

  for (int factor = 2, counter = 0; factor <= 8; factor *= 2, counter++) {
    auto tempPath{path};
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

void FormatConverter::dumpTerrainColourmap(std::string path) const {
  Logger::logLine("FormatConverter::Writing terrain colourmap to ", path);
  const auto &climateMap = Bitmap::findBitmapByKey("climate2");
  const auto &cityMap = Bitmap::findBitmapByKey("cities");
  const auto &width = Env::Instance().width;
  int factor = 2; // map dimensions are halved
  auto imageWidth = width / factor;
  auto imageHeight = Env::Instance().height / factor;

  std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);
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
      pixels[imageIndex + 3] =
          255.0 *
          (cityMap.getColourAtIndex(colourmapIndex) /
           Env::Instance().namedColours["cities"]); // alpha for city lights
    }
  }
  TextureWriter::writeDDS(imageWidth, imageHeight, pixels,
                          DXGI_FORMAT_B8G8R8A8_UNORM, path);
}

void FormatConverter::dumpWorldNormal(std::string path) const {
  Logger::logLine("FormatConverter::Writing normalMap to ", path);
  auto height = Env::Instance().height;
  auto width = Env::Instance().width;
  const auto &heightBMP = Bitmap::findBitmapByKey("heightmap");

  int factor = 2; // image width and height are halved
  Bitmap normalMap(width / factor, height / factor, 24);
  const auto sobelMap = heightBMP.sobelFilter();
  for (auto i = 0; i < normalMap.bInfoHeader.biHeight; i++)
    for (auto w = 0; w < normalMap.bInfoHeader.biWidth; w++)
      normalMap.setColourAtIndex(
          i * normalMap.bInfoHeader.biWidth + w,
          sobelMap.getColourAtIndex(factor * i * width + factor * w));
  Bitmap::SaveBMPToFile(normalMap, (path).c_str());
}

FormatConverter::FormatConverter(std::string hoiPath) {
  std::string terrainsourceString = (hoiPath + "\\map\\terrain.bmp");
  Bitmap terrain = Bitmap::Load8bitBMP(terrainsourceString.c_str(), "terrain");
  colourTables["terrainHoi4"] = terrain.getColourtable();

  std::string citySource = (hoiPath + "\\map\\terrain.bmp");
  Bitmap cities = Bitmap::Load8bitBMP(citySource.c_str(), "cities");
  colourTables["citiesHoi4"] = cities.getColourtable();

  std::string riverSource = (hoiPath + "\\map\\rivers.bmp");
  Bitmap rivers = Bitmap::Load8bitBMP(riverSource.c_str(), "rivers");
  colourTables["riversHoi4"] = rivers.getColourtable();

  std::string treeSource = (hoiPath + "\\map\\trees.bmp");
  Bitmap trees = Bitmap::Load8bitBMP(treeSource.c_str(), "trees");
  colourTables["treesHoi4"] = trees.getColourtable();

  std::string heightmapSource = (hoiPath + "\\map\\heightmap.bmp");
  Bitmap heightmap = Bitmap::Load8bitBMP(heightmapSource.c_str(), "heightmap");
  colourTables["heightmapHoi4"] = heightmap.getColourtable();
}

FormatConverter::~FormatConverter() {}
