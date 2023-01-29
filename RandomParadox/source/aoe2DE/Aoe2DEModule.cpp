#include "aoe2DE/Aoe2DEModule.h"
using namespace Fwg;

namespace Scenario::Aoe2 {
Module::Module(const boost::property_tree::ptree &gamesConf,
               const std::string &configSubFolder,
               const std::string &username) {
  FastWorldGenerator fwg(configSubFolder);
  // now run the world generation
  fwg.generateWorld();
  this->f = fwg;
}

Module::~Module() {}

bool Module::createPaths() { // prepare folder structure
  return true;
}

void Module::readVic3Config(const std::string &configSubFolder,
                            const std::string &username,
                            const boost::property_tree::ptree &rpdConf) {
  Utils::Logging::logLine("Reading Vic 3 Config");
}

void Module::genAoe2() {
  Fwg::Utils::Logging::logLine("AAAA");
  const auto &conf = Fwg::Cfg::Values();
  const auto &colours = conf.colours;
  const auto &templateFile =
      ParserUtils::readFile("resources/aoe2/base_skeleton.txt");
  try {
    std::string file = templateFile;
    std::string playerPos{""};
    std::string land_generation{""};
    std::string terrain_generation{""};
    std::string elevation{""};
    auto hMap = f.heightMap;
    auto tMap = f.terrainMap;
    auto cMap = f.climateMap;
    int lowEle = 0, midEle = 0, highEle = 0;
    for (int i = 0; i < hMap.size(); i++) {
      double h = i / hMap.bInfoHeader.biWidth;
      double w = i % hMap.bInfoHeader.biWidth;

      int aoe2PercentH = 100.0 * h / (double)hMap.bInfoHeader.biHeight;
      int aoe2PercentW = 100.0 * w / (double)hMap.bInfoHeader.biWidth;

      int zone = 0;
      std::string terrainType = "DESERT";
      if (cMap[i] == colours.at("jungle")) {
        terrainType = "JUNGLE";
      } else if (cMap[i] == colours.at("forest")) {
        terrainType = "WOODIES";
      } else if (cMap[i] == colours.at("hills")) {
        terrainType = "PINE_FOREST";
        zone = 3;
        lowEle++;
      } else if (cMap[i] == colours.at("mountains")) {
        midEle++;
        zone = 4;
        terrainType = "SNOW_FOREST";
      } else if (cMap[i] == colours.at("peaks")) {
        highEle++;
        zone = 5;
        terrainType = "DIRT_SNOW";
      } else if (cMap[i] == colours.at("grassland")) {
        terrainType = "GRASS";
      } else if (cMap[i] == colours.at("desert")) {
        terrainType = "DESERT";
      } else if (cMap[i] == colours.at("savannah")) {
        terrainType = "DLC_SAVANNAH";
      } else if (cMap[i] == colours.at("ice")) {
        terrainType = "ICE";
      } else if (cMap[i] == colours.at("tundra")) {
        terrainType = "SNOW";
      }

      /*      if (hMap[i].getBlue() < conf.seaLevel*0.8) {
              tokens.append("\ncreate_land DEEP_WATER {\n");
              tokens.append("terrain_type DEEP_WATER\n");
              tokens.append("land_percent 0\n");
              tokens.append("number_of_tiles 1\n");
              tokens.append(Utils::varsToString("land_position ", aoe2PercentH,
         " ", aoe2PercentW, "\n")); tokens.append("}\n"); } else */
      if (hMap[i].getBlue() < conf.seaLevel) {
        terrain_generation.append("\ncreate_land {\n");
        terrain_generation.append("terrain_type WATER\n");
        //terrain_generation.append("land_percent 0\n");
        terrain_generation.append("number_of_tiles 1\n");
        //terrain_generation.append("set_avoid_player_start_areas\n");
        terrain_generation.append(Utils::varsToString(
            "land_position ", aoe2PercentH, " ",
                                          aoe2PercentW, "\n"));
        terrain_generation.append("}\n");
      } else {
        terrain_generation.append("\ncreate_land {\n");
        terrain_generation.append("terrain_type " + terrainType + "\n");
        //terrain_generation.append("land_percent 0\n");
        terrain_generation.append("number_of_tiles 1\n");
        //terrain_generation.append("set_avoid_player_start_areas\n");
        terrain_generation.append(Utils::varsToString(
            "land_position ", aoe2PercentH, " ",
                                          aoe2PercentW, "\n"));
        if (zone) {
          terrain_generation.append("zone " + std::to_string(zone) + "\n");
        }
        terrain_generation.append("}\n");
      }
    }
     elevation.append("\ncreate_elevation 3\n{\n");
     elevation.append("base_terrain PINE_FOREST\n");
     elevation.append("number_of_tiles " + std::to_string(lowEle) + "\n");
     elevation.append("number_of_clumps " + std::to_string(lowEle) + "\n");
     elevation.append("}\n");
     elevation.append("\ncreate_elevation 5\n{\n");
     elevation.append("base_terrain SNOW_FOREST\n");
     elevation.append("number_of_tiles " + std::to_string(midEle) + "\n");
     elevation.append("number_of_clumps " + std::to_string(midEle) + "\n");
     elevation.append("}\n");

     elevation.append("\ncreate_elevation 7\n{\n");
     elevation.append("base_terrain DIRT_SNOW\n");
     elevation.append("number_of_tiles " + std::to_string(highEle) + "\n");
     elevation.append("number_of_clumps " + std::to_string(highEle) + "\n");
     elevation.append("}\n");

    ParserUtils::replaceOccurence(file, "TOKEN_TERRAIN", terrain_generation);
    ParserUtils::replaceOccurence(file, "TOKEN_ELEVATION", elevation);
    ParserUtils::writeFile("mapContent", file);

  } catch (std::exception e) {
  }
}

} // namespace Scenario::Aoe2