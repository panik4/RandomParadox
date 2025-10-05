#include "eu4/Eu4Parsing.h"
using namespace Fwg;
namespace pU = Fwg::Parsing;
namespace Rpx::Eu4::Parsing {
std::string loadVanillaFile(const std::string &path,
                            const std::vector<std::string> &&filters) {
  std::string content{""};
  auto lines = pU::getLines(path);
  for (auto &line : lines) {
    if (filters.size()) {
      for (auto &filter : filters) {
        if (line.find(filter) != std::string::npos) {
          content.append(line + "\n");
          // fulfilled one of the filters, break
          break;
        }
      }
    } else {
      content.append(line + "\n");
    }
  }
  return content;
}
void writeAdj(const std::string &path,
              const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Writing Adjacencies");
  // From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment
  // empty file for now
  std::string content;
  content.append(
      "From;To;Type;Through;start_x;start_y;stop_x;stop_y;Comment\n");
  const auto &randProv = provinces[0];

  content.append(std::to_string(randProv->ID));
  content.append(";");
  content.append(std::to_string(randProv->ID));
  content.append(";");
  content.append("land");
  content.append(";");
  content.append(std::to_string(randProv->ID));
  content.append(";-1;-1;-1;-1;Filler\n");
  content.append("-1;-1;;-1;-1;-1;-1;-1;-1;");
  pU::writeFile(path, content);
}

void writeAmbientObjects(
    const std::string &path,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  Fwg::Utils::Logging::logLine("Eu4 Parser: Map: Writing Ambient Objects");
  // empty file for now
  std::string content{""};
  pU::writeFile(path, content);
}

void writeAreas(const std::string &path,
                const std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions,
                const std::string &gamePath) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Writing Areas");
  std::string content =
      loadVanillaFile(gamePath + "//map//area.txt", {"{", "}", "_area"});
  const auto templateArea =
      pU::readFile(Fwg::Cfg::Values().resourcePath + "eu4//map//area.txt");

  for (auto &region : regions) {
    std::string areaText{templateArea};
    Rpx::Parsing::replaceOccurences(areaText, "template_name",
                                    "area_" + std::to_string(region->ID + 1));
    std::string provs{""};
    for (auto &prov : region->provinces) {
      provs.append(std::to_string(prov->ID + 1));
      provs.append(" ");
    }
    Rpx::Parsing::replaceOccurences(areaText, "templateProvinces", provs);
    content.append(areaText);
  }
  pU::writeFile(path, content);
}

void writeClimate(const std::string &path,
                  const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Writing climate");
  /* climate types:
   * tropical, arid, arctic, mild_winter, normal_winter, severe_winter,
   * impassable, mild_monsoon, normal_monsoon, severe_monsoon
   */
  auto content =
      pU::readFile(Fwg::Cfg::Values().resourcePath + "eu4//map//climate.txt");
  std::string tropical{""};
  std::string arid{""};
  std::string arctic{""};
  std::string mild_winter{""};
  std::string normal_winter{""};
  std::string severe_winter{""};
  std::string impassable{""};
  std::string mild_monsoon{""};
  std::string normal_monsoon{""};
  std::string severe_monsoon{""};

  for (const auto &province : provinces) {
    const auto provID = std::to_string(province->ID + 1);
    auto minTemp = 400.0;
    auto maxTemp = -200.0;
    auto maxPrecipitation = 0.0;
    for (const auto &temp : province->weatherMonths) {
      // find min and max temperatures
      Fwg::Utils::switchIfComparator(temp[1], minTemp, std::less());
      Fwg::Utils::switchIfComparator(temp[1], maxTemp, std::greater());
      Fwg::Utils::switchIfComparator(temp[2], maxPrecipitation, std::greater());
    }
    if (minTemp < 0.1)
      severe_winter.append(provID + " ");
    else if (minTemp < 0.25)
      normal_winter.append(provID + " ");
    else if (minTemp < 0.4)
      mild_winter.append(provID + " ");

    if (minTemp < 0.1 && maxTemp < 0.4)
      arctic.append(provID + " ");
    if (province->terrainType == "rockyMountains")
      impassable.append(provID + " ");

    if (province->terrainType == "jungle")
      tropical.append(provID + " ");
    if (province->terrainType == "desert")
      arid.append(provID + " ");
    else if (maxTemp > 0.80 && maxPrecipitation < 0.5)
      arid.append(provID + " ");
    if (maxTemp > 0.85 && maxPrecipitation > 0.6)
      mild_monsoon.append(provID + " ");
    else if (maxTemp > 0.85 && maxPrecipitation > 0.7)
      normal_monsoon.append(provID + " ");
    else if (maxTemp > 0.85 && maxPrecipitation > 0.8)
      severe_monsoon.append(provID + " ");
  }
  Rpx::Parsing::replaceOccurences(content, "templateTropical", tropical);
  Rpx::Parsing::replaceOccurences(content, "templateArid", arid);
  Rpx::Parsing::replaceOccurences(content, "templateArctic", arctic);
  Rpx::Parsing::replaceOccurences(content, "templateMildWinter", mild_winter);
  Rpx::Parsing::replaceOccurences(content, "templateNormalWinter",
                                  normal_winter);
  Rpx::Parsing::replaceOccurences(content, "templateSevereWinter",
                                  severe_winter);
  Rpx::Parsing::replaceOccurences(content, "templateImpassable", impassable);
  Rpx::Parsing::replaceOccurences(content, "templateMildMonsoon", mild_monsoon);
  Rpx::Parsing::replaceOccurences(content, "templateNormalMonsoon",
                                  normal_monsoon);
  Rpx::Parsing::replaceOccurences(content, "templateSevereMonsoon",
                                  severe_monsoon);
  pU::writeFile(path, content);
}

void writeColonialRegions(
    const std::string &path, const std::string &gamePath,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  std::string content = loadVanillaFile(
      gamePath + "//common//colonial_regions//00_colonial_regions.txt",
      {"{", "}", "=", "_"});
  int baseCompatProv = 1;
  while (Rpx::Parsing::replaceOccurence(content, "provinces = {",
                                        "provinces = \n\t{\n\t\t " +
                                            std::to_string(baseCompatProv++)))
    ;

  // Rpx::Parsing::replaceLines(content, "owns =", "");
  pU::writeFile(path, content);
}

void writeContinent(
    const std::string &path,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {

  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Writing continents");
  auto content =
      pU::readFile(Fwg::Cfg::Values().resourcePath + "eu4//map//continent.txt");
  // must not be more than 6 continents!
  std::array<std::vector<int>, 6> continentMap;
  for (const auto &province : provinces) {
    if (province->continentID >= 0 &&
        province->continentID != -1 &&
        province->continentID < continentMap.size()) {

        continentMap.at(province->continentID)
            .push_back(province->ID + 1);
    }
  }
  auto count = 0;
  for (auto &continent : continentMap) {
    std::string continentProvs{""};
    for (auto elem : continent) {
      continentProvs.append(std::to_string(elem) + " ");
    }
    Rpx::Parsing::replaceOccurences(
        content, "template" + std::to_string(count++), continentProvs);
  }
  pU::writeFile(path, content);
}
void writeDefaultMap(
    const std::string &path,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Writing default map");
  auto content =
      pU::readFile(Fwg::Cfg::Values().resourcePath + "eu4//map//default.map");
  Rpx::Parsing::replaceOccurences(content, "templateWidth",
                                  std::to_string(Cfg::Values().width));
  Rpx::Parsing::replaceOccurences(content, "templateHeight",
                                  std::to_string(Cfg::Values().height));
  Rpx::Parsing::replaceOccurences(content, "templateProvinces",
                                  std::to_string(provinces.size() + 1));
  std::string seaStarts{""};
  std::string lakes{""};
  for (const auto &province : provinces) {
    if (province->isSea()) {
      seaStarts.append(std::to_string(province->ID + 1) + " ");
      if (seaStarts.size() % 76 < 10 && seaStarts.size() >= 10)
        seaStarts.append("\n\t\t\t\t");
    } else if (province->isLake()) {
      lakes.append(std::to_string(province->ID + 1) + " ");
      if (lakes.size() % 76 < 10 && lakes.size() >= 10)
        lakes.append("\n\t\t\t");
    }
  }
  Rpx::Parsing::replaceOccurences(content, "templateSeaStarts", seaStarts);
  Rpx::Parsing::replaceOccurences(content, "templateLakes", lakes);
  pU::writeFile(path, content);
}

void writeDefinition(
    const std::string &path,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Defining Provinces");
  std::string content{"province;red;green;blue;x;x\n"};
  for (const auto &prov : provinces) {
    std::vector<std::string> arguments{
        std::to_string(prov->ID + 1),
        std::to_string(prov->colour.getRed()),
        std::to_string(prov->colour.getGreen()),
        std::to_string(prov->colour.getBlue()),
        "x",
        "x"};
    content.append(pU::csvFormat(arguments, ';', false));
  }
  pU::writeFile(path, content);
}

void writePositions(
    const std::string &path,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Writing Positions");

  /* for positions, rotation and height, order is:
   * city model (xy position, rotation, height),
   * standing unit model (xy position, rotation, height)
   * province name text (xy position, rotation, height)
   * province port (xy position, rotation, height)
   * province trade route model (xy position, rotation, height)
   * fighting unit models (xy position, rotation, height)
   * trade wind icon (xy position, rotation, height)
   */
  std::string content{""};
  const auto templateProvince =
      pU::readFile(Fwg::Cfg::Values().resourcePath + "eu4//map//positions.txt");
  for (const auto &prov : provinces) {
    std::string provincePositions{templateProvince};
    Rpx::Parsing::replaceOccurences(provincePositions, "templateID",
                                    std::to_string(prov->ID + 1));
    const auto &baseProv = prov;
    ;
    const std::string centerString = {
        std::to_string((double)baseProv->position.widthCenter) + " " +
        std::to_string((double)baseProv->position.heightCenter)};
    // slowly replace by more sensible arguments
    std::vector<std::string> arguments{centerString, centerString, centerString,
                                       centerString, centerString, centerString,
                                       centerString};
    Rpx::Parsing::replaceOccurences(provincePositions, "templatePositions",
                                    pU::csvFormat(arguments, ' ', false));
    content.append(provincePositions);
  }
  pU::writeFile(path, content);
}

void writeRegions(const std::string &path, const std::string &gamePath,
                  const std::vector<eu4Region> &eu4regions) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Writing Regions");
  std::string content =
      loadVanillaFile(gamePath + "//map//region.txt", {"{", "}", "areas"});
  while (Rpx::Parsing::removeBracketBlockFromKey(content, "monsoon")) {
  }
  const auto templateRegion =
      pU::readFile(Fwg::Cfg::Values().resourcePath + "eu4//map//region.txt");
  for (const auto &eu4Region : eu4regions) {
    auto regionStr{templateRegion};
    std::string areaString{""};
    Rpx::Parsing::replaceOccurences(regionStr, "templateRegion",
                                    eu4Region.name);
    for (const auto areaID : eu4Region.areaIDs) {
      areaString.append("area_" + std::to_string(areaID + 1) + " ");
    }
    Rpx::Parsing::replaceOccurences(regionStr, "templateAreaList", areaString);
    content.append(regionStr);
  }

  pU::writeFile(path, content);
}

void writeSuperregion(const std::string &path, const std::string &gamePath,
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Writing Superregions");
  // not really necessary

  std::string content = loadVanillaFile(gamePath + "//map//superregion.txt",
                                        {"{", "}", "superregion"});
  pU::writeFile(path, content);
}

void writeTerrain(const std::string &path,
                  const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Writing Terrain");
  // copying for now, as overwrites of terrain type are not a necessity
  pU::writeFile(path, pU::readFile(Fwg::Cfg::Values().resourcePath +
                                   "eu4//map//terrain.txt"));
}

void writeTradeCompanies(
    const std::string &path, const std::string &gamePath,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  std::string content = loadVanillaFile(
      gamePath + "//common//trade_companies//00_trade_companies.txt",
      {"{", "}", "="});
  int baseCompatProv = 1;
  while (Rpx::Parsing::replaceOccurence(content, "provinces = {",
                                        "provinces = \n\t{\n\t\t " +
                                            std::to_string(baseCompatProv++))) {
  };
  pU::writeFile(path, content);
}

void writeTradewinds(
    const std::string &path,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Map: Writing Tradewinds");
  // empty for now, as tradewinds are not a necessity
  pU::writeFile(path, pU::readFile(Fwg::Cfg::Values().resourcePath +
                                   "eu4//map//trade_winds.txt"));
}

void copyDescriptorFile(const std::string &sourcePath,
                        const std::string &destPath,
                        const std::string &modsDirectory,
                        const std::string &modName) {
  Fwg::Utils::Logging::logLine("EU4 Parser: Copying Descriptor file");
  auto descriptorText = pU::readFile(sourcePath);
  Rpx::Parsing::replaceOccurences(descriptorText, "templateName", modName);
  auto modText{descriptorText};
  Rpx::Parsing::replaceOccurences(descriptorText, "templatePath", "");
  pU::writeFile(destPath + "//descriptor.mod", descriptorText);
  Rpx::Parsing::replaceOccurences(
      modText, "templatePath",
      Fwg::Utils::varsToString("path=\"", destPath, "\""));
  pU::writeFile(modsDirectory + "//" + modName + ".mod", modText);
}

void writeProvinces(const std::string &path,
                    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces,
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions) {
  Fwg::Utils::Logging::logLine(
      "Eu4 Parser: History: Drawing Areas::Province Borders");
  auto templateContent = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                      "eu4//history//provinceTemplate.txt");
  for (const auto &region : regions) {
    for (const auto &prov : region->ardaProvinces) {
      // make sure lakes and wastelands are empty
      if (prov->isLake() ||
          prov->terrainType == "rockyMountains") {
        pU::writeFile(path + "//" + std::to_string(prov->ID + 1) + "-a.txt",
                      "");
      } else {
        std::string content{templateContent};
        pU::writeFile(path + "//" + std::to_string(prov->ID + 1) + "-a.txt",
                      content);
      }
    }
  }
}

void writeLoc(const std::string &path, const std::string &gamePath,
              const std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions,
              const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces,
              const std::vector<eu4Region> &eu4regions) {

  Fwg::Utils::Logging::logLine("Eu4 Parser: Localisation: Writing Area Names");
  std::vector<std::string> locKeys{"l_english", "l_german", "l_french",
                                   "l_spanish"};
  for (const auto &locKey : locKeys) {
    std::string content = pU::readFile(
        gamePath + "//localisation//areas_regions_" + locKey + ".yml");
    for (const auto &region : regions)
      content += " area_" + std::to_string(region->ID + 1) + ":0 \"" +
                 "Areaname" + std::to_string(region->ID + 1) + "\"\n";

    pU::writeFile(path + "areas_regions_" + locKey + ".yml", content, false);
  }
  for (const auto &locKey : locKeys) {
    std::string content = locKey + ":\n"; /*
    std::string content = pU::readFile(
        gamePath + "//localisation//prov_names_adj_" + locKey + ".yml");*/
    for (const auto &province : provinces)
      content += " PROV_ADJ" + std::to_string(province->ID + 1) + ":0 \"" +
                 "PROV_ADJ" + std::to_string(province->ID + 1) + "\"\n";

    pU::writeFile(path + "prov_names_adj_" + locKey + ".yml", content, true);
  }
  for (const auto &locKey : locKeys) {
    std::string content = locKey + ":\n";
    // pU::readFile(
    //    gamePath + "//localisation//prov_names_" + locKey + ".yml");
    for (const auto &province : provinces)
      content += " PROV" + std::to_string(province->ID + 1) + ":0 \"" + "PROV" +
                 std::to_string(province->ID + 1) + "\"\n";

    pU::writeFile(path + "prov_names_" + locKey + ".yml", content, true);
  }
}
} // namespace Rpx::Eu4::Parsing