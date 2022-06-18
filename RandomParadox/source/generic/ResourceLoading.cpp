#include "generic/ResourceLoading.h"
namespace ResourceLoading {
Bitmap loadProvinceMap(const std::string &gamePath) {
  return Bitmap::Load24bitBMP(gamePath + "\\map\\provinces.bmp", "provinces");
}

Bitmap loadHeightMap(const std::string &gamePath) {
  return Bitmap::Load8bitBMP(gamePath + "\\map\\heightmap.bmp", "heightmap");
}

std::vector<std::string> loadStates(const std::string &gamePath) {
  return ParserUtils::readFilesInDirectory(gamePath + "\\history\\states\\");
}

std::vector<std::string> loadDefinition(const std::string &gamePath) {
  return ParserUtils::getLines(gamePath + "\\map\\definition.csv");
}

std::vector<std::string> loadForbiddenTags(const std::string &gamePath) {
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
} // namespace ResourceLoading