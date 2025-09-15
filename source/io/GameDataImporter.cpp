#include "io/GameDataImporter.h"
using namespace Fwg;
using namespace Fwg::Gfx;
namespace Rpx::IO {
Bitmap loadProvinceMap(const std::string &gamePath) {
  return Fwg::IO::Reader::readGenericImage(gamePath + "//map//provinces.bmp",
                                           Fwg::Cfg::Values());
}

Bitmap loadHeightMap(const std::string &gamePath) {
  return Fwg::IO::Reader::readGenericImage(gamePath + "//map//heightmap.bmp",
                                           Fwg::Cfg::Values());
}

std::vector<std::string> loadStates(const std::string &gamePath) {
  return Fwg::Parsing::readFilesInDirectory(gamePath + "//history//states//");
}

std::vector<std::string> loadDefinition(const std::string &gamePath) {
  return Fwg::Parsing::getLines(gamePath + "//map//definition.csv");
}

std::vector<std::string> loadForbiddenTags(const std::string &gamePath) {
  std::vector<std::string> tags;
  if (gamePath.find("Victoria") != std::string::npos) {
    // auto lines = Rpx::Parsing::getLines(
    //     gamePath + "//common//country_definitions//00_countries.txt");
    // for (const auto &line : lines) {
    //   auto tag = line.substr(0, 3);
    //   tags.push_back(tag);
    // }

  } else {

    if (std::filesystem::exists(gamePath +
                                "//common//country_tags//00_countries.txt")) {
      auto lines = Fwg::Parsing::getLines(
          gamePath + "//common//country_tags//00_countries.txt");
      for (const auto &line : lines) {
        if (line.size() < 3)
          continue;
        auto tag = line.substr(0, 3);
        tags.push_back(tag);
      }
      if (gamePath.find("Europa Universalis") != std::string::npos) {
        lines = Fwg::Parsing::getLines(
            gamePath +
                                  "//common//country_tags//01_countries.txt");
      }
      for (const auto &line : lines) {
        if (line.size() < 3)
          continue;
        auto tag = line.substr(0, 3);
        tags.push_back(tag);
      }
    }
  }
  return tags;
}
} // namespace Rpx::IO