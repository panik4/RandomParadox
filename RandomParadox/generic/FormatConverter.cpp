#include "FormatConverter.h"

void FormatConverter::dump8BitHeightmap(const std::string path,
                                        const std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Copying heightmap to ", path);
  Bitmap hoi4Heightmap(Data::getInstance().width, Data::getInstance().height,
                       8);
  hoi4Heightmap.getColourtable() = colourTables.at(colourMapKey);
  // now map from 24 bit climate map
  const auto &heightmap = Bitmap::findBitmapByKey("heightmap");
  for (int i = 0; i < Data::getInstance().bitmapSize; i++)
    hoi4Heightmap.bit8Buffer[i] = heightmap.getColourAtIndex(i).getRed();
  Bitmap::SaveBMPToFile(hoi4Heightmap, (path).c_str());
}

void FormatConverter::dump8BitTerrain(std::string path,
                                      std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Writing terrain to ", path);
  Bitmap hoi4terrain(Data::getInstance().width, Data::getInstance().height, 8);
  hoi4terrain.getColourtable() = colourTables.at(colourMapKey);
  // now map from 24 bit climate map
  const auto &climate = Bitmap::findBitmapByKey("climate");
  for (int i = 0; i < Data::getInstance().bitmapSize; i++)
    hoi4terrain.bit8Buffer[i] =
        colourMaps.at(colourMapKey).at(climate.getColourAtIndex(i));
  Bitmap::SaveBMPToFile(hoi4terrain, (path).c_str());
}

void FormatConverter::dump8BitCities(std::string path,
                                     std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Writing cities to ", path);
  Bitmap cities(Data::getInstance().width, Data::getInstance().height, 8);
  cities.getColourtable() = colourTables.at(colourMapKey);

  const auto &climate = Bitmap::findBitmapByKey("climate");
  for (int i = 0; i < Data::getInstance().bitmapSize; i++)
    cities.bit8Buffer[i] =
        climate.getColourAtIndex(i) == Data::getInstance().namedColours["sea"]
            ? 15
            : 1;
  Bitmap::SaveBMPToFile(cities, (path).c_str());
}

void FormatConverter::dump8BitRivers(std::string path,
                                     std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Writing rivers to ", path);
  Bitmap rivers(Data::getInstance().width, Data::getInstance().height, 8);
  rivers.getColourtable() = colourTables.at(colourMapKey);

  const auto &rivers2 = Bitmap::findBitmapByKey("rivers");
  for (int i = 0; i < Data::getInstance().bitmapSize; i++)
    rivers.bit8Buffer[i] =
        colourMaps.at(colourMapKey).at(rivers2.getColourAtIndex(i));
  Bitmap::SaveBMPToFile(rivers, (path).c_str());
}

void FormatConverter::dump8BitTrees(std::string path,
                                    std::string colourMapKey) const {
  Logger::logLine("FormatConverter::Writing trees to ", path);
  const double width = Data::getInstance().width;
  constexpr auto factor = 3.4133333333333333333333333333333;
  Bitmap trees(((double)Data::getInstance().width / factor),
               ((double)Data::getInstance().height / factor), 8);
  trees.getColourtable() = colourTables.at(colourMapKey);

  const auto &climate = Bitmap::findBitmapByKey("climate");
  const auto &fGenTrees = Bitmap::findBitmapByKey("trees");
  for (auto i = 0; i < trees.bInfoHeader.biHeight; i++) {
    for (auto w = 0; w < trees.bInfoHeader.biWidth; w++) {
      double refHeight = ceil((double)i * factor);
      double refWidth = UtilLib::clamp((double)w * factor, 0.0,
                                       (double)Data::getInstance().width);
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
  const auto &width = Data::getInstance().width;

  for (int factor = 2, counter = 0; factor <= 8; factor *= 2, counter++) {
    auto tempPath{path};
    tempPath += std::to_string(counter);
    tempPath += ".dds";
    auto imageWidth = width / factor;
    auto imageHeight = Data::getInstance().height / factor;
    std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);

    for (auto h = 0; h < imageHeight; h++) {
      for (auto w = 0; w < imageWidth; w++) {
        auto referenceIndex = factor * h * width + factor * w;
        double depth =
            (double)heightBMP.getColourAtIndex(referenceIndex).getBlue() /
            (double)Data::getInstance().seaLevel;
        auto imageIndex =
            imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
        imageIndex *= 4;
        if (riverBMP.getColourAtIndex(referenceIndex) ==
            Data::getInstance().namedColours["sea"]) {
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
  const auto &width = Data::getInstance().width;
  int factor = 2; // map dimensions are halved
  auto imageWidth = width / factor;
  auto imageHeight = Data::getInstance().height / factor;

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
           Data::getInstance().namedColours["cities"]); // alpha for city lights
    }
  }
  TextureWriter::writeDDS(imageWidth, imageHeight, pixels,
                          DXGI_FORMAT_B8G8R8A8_UNORM, path);
}

void FormatConverter::dumpWorldNormal(std::string path) const {
  Logger::logLine("FormatConverter::Writing normalMap to ", path);
  auto height = Data::getInstance().height;
  auto width = Data::getInstance().width;
  const auto &heightBMP = Bitmap::findBitmapByKey("heightmap");

  int factor = 2; // image width and height are halved
  Bitmap normalMap(width / factor, height / factor, 24);
  const auto sobelMap = heightBMP.sobelFilter();
  for (auto i = 0; i < normalMap.bInfoHeader.biHeight; i++)
    for (auto w = 0; w < normalMap.bInfoHeader.biWidth; w++)
      normalMap.setColourAtIndex(i * normalMap.bInfoHeader.biWidth + w,
                                 sobelMap[factor * i * width + factor * w]);
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
