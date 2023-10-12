#include "generic/ResourceLoading.h"
using namespace Fwg;
using namespace Fwg::Gfx;
namespace Scenario::ResourceLoading {
Bitmap loadProvinceMap(const std::string &gamePath) {
  return Bmp::load24Bit(gamePath + "//map//provinces.bmp", "provinces");
}

Bitmap loadHeightMap(const std::string &gamePath) {
  return Bmp::load8Bit(gamePath + "//map//heightmap.bmp", "heightmap");
}

std::vector<std::string> loadStates(const std::string &gamePath) {
  return Parsing::readFilesInDirectory(gamePath + "//history//states//");
}

std::vector<std::string> loadDefinition(const std::string &gamePath) {
  return Parsing::getLines(gamePath + "//map//definition.csv");
}

std::vector<std::string> loadForbiddenTags(const std::string &gamePath) {
  std::vector<std::string> tags;
  if (gamePath.find("Victoria") != std::string::npos) {
    //auto lines = Parsing::Scenario::getLines(
    //    gamePath + "//common//country_definitions//00_countries.txt");
    //for (const auto &line : lines) {
    //  auto tag = line.substr(0, 3);
    //  tags.push_back(tag);
    //}
  } else {

    if (std::filesystem::exists(gamePath +
                                "//common//country_tags//00_countries.txt")) {
      auto lines = Parsing::getLines(
          gamePath + "//common//country_tags//00_countries.txt");
      for (const auto &line : lines) {
        auto tag = line.substr(0, 3);
        tags.push_back(tag);
      }
      if (gamePath.find("Europa Universalis") == std::string::npos) {
        lines = Parsing::getLines(
            gamePath + "//common//country_tags//01_countries.txt");
      }
      for (const auto &line : lines) {
        auto tag = line.substr(0, 3);
        tags.push_back(tag);
      }
    }
  }
  return tags;
}
} // namespace Scenario::ResourceLoading