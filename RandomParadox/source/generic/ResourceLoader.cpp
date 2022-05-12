#include "generic/ResourceLoader.h"

ResourceLoader::ResourceLoader() {}

ResourceLoader::~ResourceLoader() {}

Bitmap ResourceLoader::loadProvinceMap(std::string gamePath) {
  return Bitmap::Load24bitBMP((gamePath + ("\\map\\provinces.bmp")).c_str(),
                              "provinces");
}

Bitmap ResourceLoader::loadHeightMap(std::string gamePath) {
  return Bitmap::Load8bitBMP((gamePath + ("\\map\\heightmap.bmp")).c_str(),
                             "heightmap");
}

std::vector<std::string> ResourceLoader::loadStates(std::string gamePath) {
  return ParserUtils::readFilesInDirectory(gamePath + "\\history\\states\\");
}

std::vector<std::string> ResourceLoader::loadDefinition(std::string gamePath) {
  return ParserUtils::getLines(gamePath + "\\map\\definition.csv");
}

std::vector<std::string>
ResourceLoader::loadForbiddenTags(std::string gamePath) {
  std::vector<std::string> tags;
  auto lines = ParserUtils::getLines(
      gamePath + "\\common\\country_tags\\00_countries.txt");
  for (const auto &line : lines) {
    auto tag = line.substr(0, 3);
    tags.push_back(tag);
  }
  lines = ParserUtils::getLines(gamePath +
                                "\\common\\country_tags\\01_countries.txt");
  for (const auto &line : lines) {
    auto tag = line.substr(0, 3);
    tags.push_back(tag);
  }
  return tags;
}
