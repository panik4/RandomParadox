#include "eu4/Eu4Parser.h"

void Eu4Parser::writeAdj(const std::string path,
                         const std::vector<GameProvince> &provinces) {
  Logger::logLine("EU4 Parser: Map: Writing Adjacencies");
  // From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment
  // empty file for now
  std::string content;
  content.append(
      "From;To;Type;Through;start_x;start_y;stop_x;stop_y;Comment\n");
  const auto &randProv = provinces[0];
  content.append(std::to_string(randProv.ID));
  content.append(";");
  content.append(std::to_string(randProv.ID));
  content.append(";");
  content.append("land");
  content.append(";");
  content.append(std::to_string(randProv.ID));
  content.append(";-1;-1;-1;-1;Filler\n");
  content.append("-1;-1;;-1;-1;-1;-1;-1;-1;");
  pU::writeFile(path, content);
}

void Eu4Parser::writeAmbientObjects(
    const std::string path, const std::vector<GameProvince> &provinces) {
  Logger::logLine("Eu4 Parser: Map: Writing Ambient Objects");
  // empty file for now
  std::string content{""};
  pU::writeFile(path, content);
}

void Eu4Parser::writeAreas(const std::string path,
                           const std::vector<GameRegion> &regions) {
  Logger::logLine("EU4 Parser: Map: Writing Areas");
  std::string content{""};
  const auto templateArea = pU::readFile("resources\\eu4\\map\\area.txt");

  for (auto &region : regions) {
    std::string areaText{templateArea};
    pU::replaceOccurences(areaText, "template_name",
                          region.name + std::to_string(region.ID));
    std::string provs{""};
    for (auto &prov : region.provinces) {
      provs.append(std::to_string(prov->ID));
      provs.append(" ");
    }
    pU::replaceOccurences(areaText, "templateProvinces", provs);
    content.append(areaText);
  }
  pU::writeFile(path, content);
}

void Eu4Parser::writeClimate(const std::string path,
                             const std::vector<GameProvince> &provinces) {
  Logger::logLine("EU4 Parser: Map: Writing climate");
  /* climate types:
   * tropical, arid, arctic, mild_winter, normal_winter, severe_winter,
   * impassable, mild_monsoon, normal_monsoon, severe_monsoon
   */
  auto content = pU::readFile("resources\\eu4\\map\\climate.txt");
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
    const auto provID = std::to_string(province.ID);
    auto minTemp = 400.0;
    auto maxTemp = -200.0;
    auto maxPrecipitation = 0.0;
    for (const auto &temp : province.baseProvince->weatherMonths) {
      // find min and max temperatures
      UtilLib::switchIfComparator(temp[1], minTemp, std::less());
      UtilLib::switchIfComparator(temp[1], maxTemp, std::greater());
      UtilLib::switchIfComparator(temp[2], maxPrecipitation, std::greater());
    }
    std::cout << minTemp << std::endl;
    if (minTemp < 0.1)
      severe_winter.append(provID + " ");
    else if (minTemp < 0.25)
      normal_winter.append(provID + " ");
    else if (minTemp < 0.4)
      mild_winter.append(provID + " ");

    if (minTemp < 0.1 && maxTemp < 0.4)
      arctic.append(provID + " ");
    if (province.terrainType == "mountains")
      impassable.append(provID + " ");

    if (province.terrainType == "jungle")
      tropical.append(provID + " ");
    if (province.terrainType == "desert")
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
  pU::replaceOccurences(content, "templateTropical", tropical);
  pU::replaceOccurences(content, "templateArid", arid);
  pU::replaceOccurences(content, "templateArctic", arctic);
  pU::replaceOccurences(content, "templateMildWinter", mild_winter);
  pU::replaceOccurences(content, "templateNormalWinter", normal_winter);
  pU::replaceOccurences(content, "templateSevereWinter", severe_winter);
  pU::replaceOccurences(content, "templateImpassable", impassable);
  pU::replaceOccurences(content, "templateMildMonsoon", mild_monsoon);
  pU::replaceOccurences(content, "templateNormalMonsoon", normal_monsoon);
  pU::replaceOccurences(content, "templateSevereMonsoon", severe_monsoon);
  pU::writeFile(path, content);
}

void Eu4Parser::writeDefaultMap(const std::string path,
                                const std::vector<GameProvince> &provinces) {
  Logger::logLine("EU4 Parser: Map: Writing default map");
  auto content = pU::readFile("resources\\eu4\\map\\default.map");
  pU::replaceOccurences(content, "templateWidth",
                        std::to_string(Env::Instance().width));
  pU::replaceOccurences(content, "templateHeight",
                        std::to_string(Env::Instance().height));
  pU::replaceOccurences(content, "templateProvinces",
                        std::to_string(provinces.size()));
  std::string seaStarts{""};
  std::string lakes{""};
  for (const auto &province : provinces) {
    if (province.baseProvince->sea) {
      seaStarts.append(std::to_string(province.ID) + " ");
      if (seaStarts.size() % 76 < 10 && seaStarts.size() >= 10)
        seaStarts.append("\n\t\t\t\t");
    } else if (province.baseProvince->isLake) {
      lakes.append(std::to_string(province.ID) + " ");
      if (lakes.size() % 76 < 10 && lakes.size() >= 10)
        lakes.append("\n\t\t\t");
    }
  }
  pU::replaceOccurences(content, "templateSeaStarts", seaStarts);
  pU::replaceOccurences(content, "templateLakes", lakes);
  pU::writeFile(path, content);
}

void Eu4Parser::writeDefinition(const std::string path,
                                const std::vector<GameProvince> &provinces) {
  Logger::logLine("EU4 Parser: Map: Defining Provinces");
  std::string content{"province;red;green;blue;x;x\n"};
  for (const auto &prov : provinces) {
    std::vector<std::string> arguments{
        std::to_string(prov.baseProvince->ID + 1),
        std::to_string(prov.baseProvince->colour.getRed()),
        std::to_string(prov.baseProvince->colour.getGreen()),
        std::to_string(prov.baseProvince->colour.getBlue())};
    content.append(pU::csvFormat(arguments, ';', false));
  }
  pU::writeFile(path, content);
}

void Eu4Parser::writePositions(const std::string path,
                               const std::vector<GameProvince> &provinces) {
  Logger::logLine("EU4 Parser: Map: Writing Positions");

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
      pU::readFile("resources\\eu4\\map\\positions.txt");
  for (const auto &prov : provinces) {
    std::string provincePositions{templateProvince};
    pU::replaceOccurences(provincePositions, "templateID",
                          std::to_string(prov.baseProvince->ID + 1));
    const auto &baseProv = prov.baseProvince;
    ;
    const std::string centerString = {
        std::to_string((double)baseProv->position.widthCenter) + " " +
        std::to_string((double)baseProv->position.heightCenter)};
    // slowly replace by more sensible arguments
    std::vector<std::string> arguments{centerString, centerString, centerString,
                                       centerString, centerString, centerString,
                                       centerString};
    pU::replaceOccurences(provincePositions, "templatePositions",
                          pU::csvFormat(arguments, ' ', false));
    content.append(provincePositions);
  }
  pU::writeFile(path, content);
}

void Eu4Parser::writeRegions(const std::string path,
                             const std::vector<GameRegion> &regions) {
  Logger::logLine("EU4 Parser: Map: Writing Regions");
  // TODO
  pU::writeFile(path, "");
}

void Eu4Parser::writeSuperregion(const std::string path,
                                 const std::vector<GameRegion> &regions) {
  Logger::logLine("EU4 Parser: Map: Writing Superregions");
  // not really necessary
  pU::writeFile(path, pU::readFile("resources\\eu4\\map\\superregion.txt"));
}

void Eu4Parser::writeTerrain(const std::string path,
                             const std::vector<GameProvince> &provinces) {
  Logger::logLine("EU4 Parser: Map: Writing Terrain");
  // copying for now, as overwrites of terrain type are not a necessity
  pU::writeFile(path, pU::readFile("resources\\eu4\\map\\terrain.txt"));
}

void Eu4Parser::writeTradewinds(const std::string path,
                                const std::vector<GameProvince> &provinces) {
  Logger::logLine("EU4 Parser: Map: Writing Tradewinds");
  // empty for now, as tradewinds are not a necessity
  pU::writeFile(path, pU::readFile("resources\\eu4\\map\\trade_winds.txt"));
}
