#include "hoi4/Hoi4Parsing.h"
#include "hoi4/NationalFocus.h"
using namespace Fwg;
namespace Logging = Fwg::Utils::Logging;
namespace pU = Scenario::ParserUtils;
namespace Scenario::Hoi4::Parsing {
namespace Writing {
void adj(const std::string &path) {
  Logging::logLine("HOI4 Parser: Map: Writing Adjacencies");
  // From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment
  // empty file for now
  std::string content;
  content.append("From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_"
                 "rule_name;Comment");
  pU::writeFile(path, content);
}

void airports(const std::string &path,
              const std::vector<Fwg::Region> &regions) {
  Logging::logLine("HOI4 Parser: Map: Building Airfields");
  std::string content;
  for (const auto &region : regions) {
    if (region.sea)
      continue;
    content.append(std::to_string(region.ID + 1));
    content.append("={");
    for (const auto &prov : region.provinces) {
      if (!prov->isLake) {
        content.append(std::to_string(prov->ID + 1));
        break;
      }
    }
    content.append(" }\n");
  }
  pU::writeFile(path, content);
}

// places building positions
void buildings(const std::string &path,
               const std::vector<std::shared_ptr<Region>> &regions,
               const Fwg::Gfx::Bitmap &heightMap) {
  Logging::logLine("HOI4 Parser: Map: Constructing Factories");
  std::string content;
  for (const auto &region : regions) {
    for (const auto &building : region->buildings) {
      std::vector<std::string> arguments{
          std::to_string(region->ID + 1),
          building.name,
          std::to_string(building.position.x),
          std::to_string(building.position.y),
          std::to_string(building.position.z),
          std::to_string(building.position.rotation),
          std::to_string(building.relativeID ? building.relativeID + 1 : 0)};
      content.append(pU::csvFormat(arguments, ';', false));
    }
  }
  pU::writeFile(path, content);
}

void continents(const std::string &path,
                const std::vector<Continent> &continents) {
  Logging::logLine("HOI4 Parser: Map: Writing Continents");
  std::string content{"continents = {\n"};

  for (const auto &continent : continents) {
    content.append("\t");
    content.append(std::to_string(continent.ID + 1));
    content.append("\n");
  }
  content.append("}\n");
  pU::writeFile(path, content);
}

void definition(const std::string &path,
                const std::vector<std::shared_ptr<GameProvince>> &provinces) {
  Logging::logLine("HOI4 Parser: Map: Defining Provinces");
  // province id; r value; g value; b value; province type (land/sea/lake);
  // coastal (true/false); terrain (plains/hills/urban/etc. Defined for land or
  // sea provinces in common/terrain); continent (int)
  // 0;0;0;0;land;false;unknown;0

  // terraintypes: ocean, lakes, forest, hills, mountain, plains, urban, jungle,
  // marsh, desert, water_fjords, water_shallow_sea, water_deep_ocean TO DO:
  // properly map terrain types from climate
  // Bitmap typeMap(512, 512, 24);
  std::string content{"0;0;0;0;land;false;unknown;0\n"};
  for (const auto &prov : provinces) {
    auto seaType = prov->baseProvince->sea ? "sea" : "land";
    auto coastal = prov->baseProvince->coastal ? "true" : "false";
    if (prov->baseProvince->sea) {
      for (auto prov2 : prov->baseProvince->neighbours) {
        if (!prov2->sea)
          coastal = "true";
      }
    }
    std::string terraintype;
    if (prov->baseProvince->sea)
      terraintype = "ocean";
    else
      terraintype = prov->terrainType;
    if (prov->baseProvince->isLake) {
      terraintype = "lakes";
      seaType = "lake";
    }
    std::vector<std::string> arguments{
        std::to_string(prov->baseProvince->ID + 1),
        std::to_string(prov->baseProvince->colour.getRed()),
        std::to_string(prov->baseProvince->colour.getGreen()),
        std::to_string(prov->baseProvince->colour.getBlue()),
        seaType,
        coastal,
        terraintype,
        std::to_string(prov->baseProvince->sea || prov->baseProvince->isLake
                           ? 0
                           : prov->baseProvince->continentID +
                                 1) // 0 is for sea, no continent
    };
    content.append(pU::csvFormat(arguments, ';', false));
  }
  pU::writeFile(path, content);
}

void rocketSites(const std::string &path,
                 const std::vector<Fwg::Region> &regions) {
  Logging::logLine("HOI4 Parser: Map: Launching Rockets");
  std::string content;
  // regionId={provId }
  for (const auto &region : regions) {
    if (region.sea)
      continue;
    content.append(std::to_string(region.ID + 1));
    content.append("={");
    for (const auto &prov : region.provinces) {
      if (!prov->isLake) {
        content.append(std::to_string(prov->ID + 1));
        break;
      }
    }
    content.append(" }\n");
  }
  pU::writeFile(path, content);
}

void unitStacks(const std::string &path,
                const std::vector<Province *> provinces,
                const Fwg::Gfx::Bitmap &heightMap) {
  Logging::logLine("HOI4 Parser: Map: Remilitarizing the Rhineland");
  // 1;0;3359.00;9.50;1166.00;0.00;0.08
  // provID, neighbour?, xPos, zPos yPos, rotation(3=north,
  // 0=south, 1.5=east,4,5=west), ?? provID, xPos, ~10, yPos, ~0, 0,5 for each
  // neighbour add move state in the direction of the neighbour. 0 might be
  // stand still
  std::string content{""};
  for (const auto &prov : provinces) {
    int position = 0;
    auto pix = Fwg::Utils::selectRandom(prov->pixels);
    auto widthPos = pix % Cfg::Values().width;
    auto heightPos = pix / Cfg::Values().width;
    std::vector<std::string> arguments{
        std::to_string(prov->ID + 1),
        std::to_string(position),
        std::to_string(widthPos),
        std::to_string((double)heightMap[pix].getRed() / 10.0),
        std::to_string(heightPos),
        std::to_string(0.0),
        "0.0"};
    content.append(pU::csvFormat(arguments, ';', false));
    for (const auto &neighbour : prov->neighbours) {
      position++;
      double angle;
      auto nextPos = prov->getPositionBetweenProvinces(
          *neighbour, Cfg::Values().width, angle);
      angle += 1.57;
      auto widthPos = nextPos % Cfg::Values().width;
      auto heightPos = nextPos / Cfg::Values().width;
      std::vector<std::string> arguments{
          std::to_string(prov->ID + 1),
          std::to_string(position),
          std::to_string(widthPos),
          std::to_string((double)heightMap[pix].getRed() / 10.0),
          std::to_string(heightPos),
          std::to_string(angle),
          "0.0"};
      content.append(pU::csvFormat(arguments, ';', false));
    }
  }
  pU::writeFile(path, content);
}

void weatherPositions(const std::string &path,
                      const std::vector<Fwg::Region> &regions,
                      const std::vector<strategicRegion> &strategicRegions) {
  Logging::logLine("HOI4 Parser: Map: Creating Storms");
  // 1; 2781.24; 9.90; 1571.49; small
  std::string content{""};
  // stateId; pixelX; rotation??; pixelY; rotation??; size
  // 1; arms_factory; 2946.00; 11.63; 1364.00; 0.45; 0

  for (auto i = 0; i < strategicRegions.size(); i++) {
    const auto &region =
        Fwg::Utils::selectRandom(strategicRegions[i].gameRegionIDs);
    const auto prov = Fwg::Utils::selectRandom(regions[region].provinces);
    const auto pix = Fwg::Utils::selectRandom(prov->pixels);
    auto widthPos = pix % Cfg::Values().width;
    auto heightPos = pix / Cfg::Values().width;
    std::vector<std::string> arguments{
        std::to_string(i + 1), std::to_string(widthPos), std::to_string(9.90),
        std::to_string(heightPos), "small"};
    content.append(pU::csvFormat(arguments, ';', false));
  }
  pU::writeFile(path, content);
}

void adjacencyRules(const std::string &path) {
  Logging::logLine("HOI4 Parser: Map: Writing Adjacency Rules");
  std::string content{""};
  // empty for now
  pU::writeFile(path, content);
}

void strategicRegions(const std::string &path,
                      const std::vector<Fwg::Region> &regions,
                      const std::vector<strategicRegion> &strategicRegions) {
  constexpr std::array<int, 12> daysInMonth{30, 27, 30, 29, 30, 29,
                                            30, 30, 29, 30, 29, 30};
  Logging::logLine("HOI4 Parser: Map: Drawing Strategic Regions");
  auto templateContent =
      pU::readFile("resources\\hoi4\\map\\strategic_region.txt");
  const auto templateWeather = pU::getBracketBlock(templateContent, "period");
  for (auto i = 0; i < strategicRegions.size(); i++) {
    std::string provString{""};
    for (const auto &region : strategicRegions[i].gameRegionIDs) {
      for (const auto prov : regions[region].provinces) {
        provString.append(std::to_string(prov->ID + 1));
        provString.append(" ");
      }
    }
    auto content{templateContent};
    pU::replaceOccurences(content, "templateID", std::to_string(i + 1));
    pU::replaceOccurences(content, "template_provinces", provString);

    // weather
    std::string weather{""};
    for (auto mo = 0; mo < 12; mo++) {
      auto month{templateWeather};
      pU::replaceOccurences(month, "templateDateRange",
                            "0." + std::to_string(mo) + " " +
                                std::to_string(daysInMonth[mo]) + "." +
                                std::to_string(mo));
      pU::replaceOccurences(
          month, "templateTemperatureRange",
          std::to_string(round((float)strategicRegions[i].weatherMonths[mo][3]))
                  .substr(0, 5) +
              " " +
              std::to_string(
                  round((float)strategicRegions[i].weatherMonths[mo][4]))
                  .substr(0, 5));
      pU::replaceOccurences(
          month, "templateRainLightChance",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][5]));
      pU::replaceOccurences(
          month, "templateRainHeavyChance",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][6]));
      pU::replaceOccurences(
          month, "templateMud",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][7]));
      pU::replaceOccurences(
          month, "templateBlizzard",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][8]));
      pU::replaceOccurences(
          month, "templateSandStorm",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][9]));
      pU::replaceOccurences(
          month, "templateSnow",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][10]));
      pU::replaceOccurences(
          month, "templateNoPhenomenon",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][11]));
      // pU::replaceOccurences(month, "templateDateRange", "0." +
      // std::to_string(i) + " 30." + std::to_string(i));
      // pU::replaceOccurences(month, "templateDateRange", "0." +
      // std::to_string(i) + " 30." + std::to_string(i));
      weather.append(month + "\n\t\t");
    }
    pU::replaceOccurences(content, templateWeather, weather);
    pU::replaceOccurences(content, "template_provinces", provString);
    pU::writeFile(Fwg::Utils::varsToString(path, "\\", (i + 1), ".txt"),
                  content);
  }
}

void supply(const std::string &path,
            const std::vector<std::vector<int>> &supplyNodeConnections) {
  std::string supplyNodes = "";
  std::string railways = "";
  std::set<int> nodes;
  for (const auto &connection : supplyNodeConnections) {
    if (connection.size() <= 1)
      continue;
    nodes.insert(connection[0]);
    nodes.insert(connection.back());
    railways += "1 ";
    railways += std::to_string(connection.size());
    railways += " ";
    for (auto prov : connection) {
      railways += std::to_string(prov + 1);
      railways += " ";
    }
    railways += "\n";
  }
  for (const auto &node : nodes) {
    supplyNodes.append("1 " + std::to_string(node + 1) + "\n");
  }
  ParserUtils::writeFile(path + "supply_nodes.txt", supplyNodes);
  ParserUtils::writeFile(path + "railways.txt", railways);
}

void states(const std::string &path, const hoiMap &countries) {
  Logging::logLine("HOI4 Parser: History: Drawing State Borders");
  auto templateContent = pU::readFile("resources\\hoi4\\history\\state.txt");
  std::vector<std::string> stateCategories{
      "wasteland",  "small_island", "pastoral",   "rural",      "town",
      "large_town", "city",         "large_city", "metropolis", "megalopolis"};
  for (const auto &country : countries) {
    for (const auto &region : country.second.hoi4Regions) {
      if (region->sea)
        continue;
      std::string provString{""};
      for (const auto &prov : region->provinces) {
        provString.append(std::to_string(prov->ID + 1));
        provString.append(" ");
      }
      auto content{templateContent};
      pU::replaceOccurences(content, "templateID",
                            std::to_string(region->ID + 1));
      pU::replaceOccurences(content, "template_provinces", provString);
      pU::replaceOccurences(content, "templateOwner", country.first);
      pU::replaceOccurences(
          content, "templateInfrastructure",
          std::to_string(1 + (int)(region->development * 4.0)));
      pU::replaceOccurences(content, "templateAirbase", std::to_string(0));
      pU::replaceOccurences(content, "templateCivilianFactory",
                            std::to_string((int)region->civilianFactories));
      pU::replaceOccurences(content, "templateArmsFactory",
                            std::to_string((int)region->armsFactories));

      pU::replaceOccurences(content, "templatePopulation",
                            std::to_string((int)region->population));
      pU::replaceOccurences(content, "templateStateCategory",
                            stateCategories[(int)region->stateCategory]);
      std::string navalBaseContent = "";
      for (const auto &gameProv : region->gameProvinces) {
        if (gameProv->attributeDoubles.at("naval_bases") > 0) {
          navalBaseContent +=
              std::to_string(gameProv->ID + 1) + " = {\n\t\t\t\tnaval_base = " +
              std::to_string(
                  (int)gameProv->attributeDoubles.at("naval_bases")) +
              "\n\t\t\t}\n\t\t\t";
        }
      }
      pU::replaceOccurences(content, "templateNavalBases", navalBaseContent);
      if (region->dockyards > 0)
        pU::replaceOccurences(content, "templateDockyards",
                              std::to_string((int)region->dockyards));
      else
        pU::replaceOccurences(content, "dockyard = templateDockyards", "");

      // resources
      for (const auto &resource : std::vector<std::string>{
               "aluminium", "chromium", "oil", "rubber", "steel", "tungsten"}) {
        pU::replaceOccurences(
            content, "template" + resource,
            std::to_string((int)region->resources.at(resource)));
      }
      pU::writeFile(path + "\\" + std::to_string(region->ID + 1) + ".txt",
                    content);
    }
  }
}
void flags(const std::string &path, const hoiMap &countries) {
  Logging::logLine("HOI4 Parser: Gfx: Printing Flags");
  using namespace Gfx::Textures;
  for (const auto &country : countries) {
    writeTGA(country.second.flag.width, country.second.flag.height,
             country.second.flag.getFlag(), path + country.first + ".tga");
    writeTGA(country.second.flag.width / 2, country.second.flag.height / 2,
             country.second.flag.resize(country.second.flag.width / 2,
                                        country.second.flag.height / 2),
             path + "\\medium\\" + country.first + ".tga");
    writeTGA(10, 7, country.second.flag.resize(10, 7),
             path + "\\small\\" + country.first + ".tga");
  }
}

void historyCountries(const std::string &path, const hoiMap &countries) {
  Logging::logLine("HOI4 Parser: History: Writing Country History");
  const auto content =
      pU::readFile("resources\\hoi4\\history\\country_template.txt");
  for (const auto &country : countries) {
    auto tempPath = path + country.first + " - " + country.second.name + ".txt";
    auto countryText{content};
    auto capitalID = 1;
    if (country.second.hoi4Regions.size())
      capitalID =
          (Fwg::Utils::selectRandom(country.second.hoi4Regions))->ID + 1;
    pU::replaceOccurences(countryText, "templateCapital",
                          std::to_string(capitalID));
    pU::replaceOccurences(countryText, "templateTag", country.first);
    pU::replaceOccurences(countryText, "templateParty",
                          country.second.rulingParty);
    std::string electAllowed = country.second.allowElections ? "yes" : "no";
    pU::replaceOccurences(countryText, "templateAllowElections", electAllowed);
    pU::replaceOccurences(countryText, "templateFasPop",
                          std::to_string(country.second.parties[0]));
    pU::replaceOccurences(countryText, "templateDemPop",
                          std::to_string(country.second.parties[1]));
    pU::replaceOccurences(countryText, "templateComPop",
                          std::to_string(country.second.parties[2]));
    pU::replaceOccurences(countryText, "templateNeuPop",
                          std::to_string(country.second.parties[3]));
    pU::writeFile(tempPath, countryText);
  }
}

void historyUnits(const std::string &path, const hoiMap &countries) {
  Logging::logLine("HOI4 Parser: History: Deploying the Troops");
  const auto defaultTemplate =
      pU::readFile("resources\\hoi4\\history\\default_unit_template.txt");
  const auto unitBlock =
      pU::readFile("resources\\hoi4\\history\\unit_block.txt");

  const auto unitTemplateFile =
      ParserUtils::readFile("resources\\hoi4\\history\\divisionTemplates.txt");
  // now tokenize by : character to get single
  const auto unitTemplates = ParserUtils::getTokens(unitTemplateFile, ':');
  // auto IDMapFile =
  // pU::getLines("resources\\hoi4\\history\\divisionIDMapper.txt");
  std::map<int, std::string> IDMap;
  /*for (const auto& line : IDMapFile) {
          if (line.size()) {
                  auto lineTokens = pU::getTokens(line, ';');
                  IDMap[stoi(lineTokens[0])] = lineTokens[1];
          }
  }*/
  for (const auto &country : countries) {
    // get the template file
    std::string unitFile = defaultTemplate;
    std::string divisionTemplates = "";
    // now insert all the unit templates for this country
    for (const auto ID : country.second.units) {
      divisionTemplates.append(unitTemplates[ID]);
      // we need to buffer the names of the templates for use in later unit
      // generationm
      auto requirements = ParserUtils::getBracketBlockContent(unitTemplates[ID],
                                                              "requirements");
      auto value =
          ParserUtils::getBracketBlockContent(requirements, "templateName");
      IDMap[ID] = value;
      // remove requirements line
      ParserUtils::replaceLine(divisionTemplates, "requirements", "");
    }
    ParserUtils::replaceOccurences(unitFile, "templateTemplateBlock",
                                   divisionTemplates);

    // now that we have the templates written down, we deploy units of these
    // templates under the "divisions" key in the unitFile
    std::string totalUnits = "";
    // for every entry in unitCount vector
    for (int i = 0; i < country.second.unitCount.size(); i++) {
      // run unit generation ("unitCount")[i] times
      for (int x = 0; x < country.second.unitCount[i]; x++) {
        // copy the template unit file
        auto tempUnit{unitBlock};
        // replace division name with the generic division name
        ParserUtils::replaceOccurences(tempUnit, "templateDivisionName",
                                       "\"" + IDMap.at(i) + "\"");
        // now deploy the unit in a random province
        ParserUtils::replaceOccurences(
            tempUnit, "templateLocation",
            std::to_string(country.second.hoi4Regions[0]->gameProvinces[0]->ID +
                           1));
        totalUnits += tempUnit;
      }
    }

    // for (int i = 0; i < country.second.attributeVectors.at("units").size();
    // i++) {
    //
    //	for (int x = 0; x < country.second.attributeVectors.at("unitCount")[i];
    // x++) {
    // Logging::logLine(country.second.attributeVectors.at("units")[i]);
    // auto
    // tempUnit{ unitBlock }; 		ParserUtils::replaceOccurences(tempUnit,
    //"templateDivisionName", IDMap.at(i));
    // Logging::logLine(IDMap.at(i));
    // ParserUtils::replaceOccurences(tempUnit,
    //"templateLocation",
    // std::to_string(country.second.ownedRegions[0].gameProvinces[0].ID + 1));
    //		totalUnits += tempUnit;
    //	}
    //}
    ParserUtils::replaceOccurences(unitFile, "templateUnitBlock", totalUnits);
    // units
    auto tempPath = path + country.first + "_1936.txt";
    pU::writeFile(tempPath, unitFile);

    // navies
    tempPath = path + country.first + "_1936_naval.txt";
    pU::writeFile(tempPath, "");
    tempPath = path + country.first + "_1936_naval_mtg.txt";
    pU::writeFile(tempPath, "");
  }
}

void commonBookmarks(
    const std::string &path, const hoiMap &countries,
    const std::map<int, std::vector<std::string>> &strengthScores) {
  auto bookmarkTemplate = pU::readFile(
      "resources\\hoi4\\common\\bookmarks\\the_gathering_storm.txt");
  int count = 0;
  const auto majorTemplate =
      pU::getBracketBlock(bookmarkTemplate, "templateMajorTAG") + "\n\t\t";
  const auto minorTemplate =
      pU::getBracketBlock(bookmarkTemplate, "templateMinorTAG") + "\n\t\t";
  pU::removeBracketBlockFromKey(bookmarkTemplate, "templateMajorTAG");
  pU::removeBracketBlockFromBracket(bookmarkTemplate, "templateMinorTAG");
  std::string bookmarkCountries{""};
  for (auto iter = strengthScores.rbegin(); iter != strengthScores.rend();
       ++iter) {
    if (count == 0) {
      pU::replaceOccurences(bookmarkTemplate, "templateDefaultTAG",
                            iter->second[0]);
    }
    if (count < 7) {
      // major power:
      for (const auto &country : iter->second) {
        auto majorString{majorTemplate};
        pU::replaceOccurences(majorString, "templateIdeology",
                              countries.at(country).rulingParty);
        bookmarkCountries.append(
            pU::replaceOccurences(majorString, "templateMajorTAG", country));
        count++;
      }
    } else if (count < 14) {
      // regional power:
      for (const auto &country : iter->second) {
        auto minorString{minorTemplate};
        pU::replaceOccurences(minorString, "templateIdeology",
                              countries.at(country).rulingParty);
        bookmarkCountries.append(
            pU::replaceOccurences(minorString, "templateMinorTAG", country));
        count++;
      }
    }
  }
  pU::replaceOccurences(bookmarkTemplate,
                        "templateMinorTAG=", bookmarkCountries);
  pU::writeFile(path + "the_gathering_storm.txt", bookmarkTemplate);
}

void commonCountries(const std::string &path, const std::string &hoiPath,
                     const hoiMap &countries) {
  Logging::logLine("HOI4 Parser: Common: Writing Countries");
  const auto content =
      pU::readFile("resources\\hoi4\\common\\country_default.txt");
  const auto colorsTxtTemplate =
      pU::readFile("resources\\hoi4\\common\\colors.txt");
  std::string colorsTxt = pU::readFile(hoiPath);
  for (const auto &country : countries) {
    auto tempPath = path + country.second.name + ".txt";
    auto countryText{content};
    auto colourString = pU::replaceOccurences(
        Fwg::Utils::varsToString(country.second.colour), ";", " ");
    pU::replaceOccurences(countryText, "templateCulture",
                          country.second.gfxCulture);
    pU::replaceOccurences(countryText, "templateColour", colourString);
    pU::writeFile(tempPath, countryText);
    auto templateCopy{colorsTxtTemplate};
    pU::replaceOccurences(templateCopy, "templateTag", country.first);
    pU::replaceOccurences(templateCopy, "templateColour", colourString);
    colorsTxt.append(templateCopy);
  }
  pU::writeFile(path + "colors.txt", colorsTxt);
}

void commonCountryTags(const std::string &path, const hoiMap &countries) {
  Logging::logLine("HOI4 Parser: Common: Writing Country Tags");
  std::string content = "";
  for (const auto &country : countries)
    content.append(country.first + " = countries/" + country.second.name +
                   ".txt\n");
  pU::writeFile(path, content);
}

void countryNames(const std::string &path, const hoiMap &countries,
                  const NameGeneration::NameData &nData) {
  Logging::logLine("HOI4 Parser: Localisation: Writing Country Names");
  std::string content = "l_english:\n";
  std::vector<std::string> ideologies{"fascism", "communism", "neutrality",
                                      "democratic"};

  for (const auto &c : countries) {
    for (const auto &ideology : ideologies) {
      auto ideologyName = NameGeneration::modifyWithIdeology(
          ideology, c.second.name, c.second.adjective, nData);
      content +=
          " " + c.first + "_" + ideology + ":0 \"" + ideologyName + "\"\n";
      content +=
          " " + c.first + "_" + ideology + "_DEF:0 \"" + ideologyName + "\"\n";
      ;
      content += " " + c.first + "_" + ideology + "_ADJ:0 \"" +
                 c.second.adjective + "\"\n";
      ;
    }
  }
  pU::writeFile(path + "countries_l_english.yml", content, true);
}

void stateNames(const std::string &path, const hoiMap &countries) {
  Logging::logLine("HOI4 Parser: Localisation: Writing State Names");
  std::string content = "l_english:\n";

  for (const auto &c : countries) {
    for (const auto &region : c.second.hoi4Regions)
      content += " STATE_" + std::to_string(region->ID + 1) + ":0 \"" +
                 region->name + "\"\n";
  }
  pU::writeFile(path + "state_names_l_english.yml", content, true);
}

void strategicRegionNames(
    const std::string &path,
    const std::vector<strategicRegion> &strategicRegions) {
  Logging::logLine("HOI4 Parser: Map: Naming the Regions");
  std::string content = "l_english:\n";
  for (auto i = 0; i < strategicRegions.size(); i++) {
    content += Fwg::Utils::varsToString(" STRATEGICREGION_", i, ":0 \"",
                                        strategicRegions[i].name, "\"\n");
  }
  pU::writeFile(path + "\\strategic_region_names_l_english.yml", content, true);
}

void tutorials(const std::string &path) {
  pU::writeFile(path, "tutorial = { }");
}

void foci(const std::string &path, const hoiMap &countries,
          const NameGeneration::NameData &nData) {
  Logging::logLine("HOI4 Parser: History: Demanding Danzig");
  const auto focusTypes = ParserUtils::getLines(
      "resources\\hoi4\\ai\\national_focus\\baseFiles\\foci.txt");
  std::string baseTree = ParserUtils::readFile(
      "resources\\hoi4\\ai\\national_focus\\baseFiles\\focusBase.txt");
  std::vector<std::string> focusTemplates;
  for (const auto &focusType : focusTypes)
    focusTemplates.push_back(ParserUtils::readFile(
        "resources\\hoi4\\ai\\national_focus\\focusTypes\\" + focusType +
        "Focus.txt"));

  for (const auto &c : countries) {
    std::string treeContent = baseTree;
    std::string tempContent = "";
    for (const auto &focusChain : c.second.foci) {
      for (const auto &countryFocus : focusChain) {
        tempContent += focusTemplates[(size_t)countryFocus.fType];

        // build available from available keys
        std::string available = "";
        for (const auto &availKey : countryFocus.available) {
          available += NationalFocus::availableMap.at(availKey);
        }
        available += "if = { date > " + std::to_string(countryFocus.date.year) +
                     "." + std::to_string(countryFocus.date.day) + "." +
                     std::to_string(countryFocus.date.month) + "}\n";
        // build bypasses from bypass keys
        std::string bypasses = "";
        for (const auto &bypassKey : countryFocus.bypasses) {
          bypasses += NationalFocus::bypassMap.at(bypassKey);
        }
        // build completion rewards from completion reward keys
        std::string completionReward = "";
        for (const auto &rewardKey : countryFocus.completionRewards) {
          completionReward += NationalFocus::rewardMap.at(rewardKey);
        }
        ParserUtils::replaceOccurences(tempContent, "templateAvailable",
                                       available);
        ParserUtils::replaceOccurences(tempContent, "templateBypasses",
                                       bypasses);
        ParserUtils::replaceOccurences(tempContent, "templateCompletionRewards",
                                       completionReward);

        ParserUtils::replaceOccurences(tempContent, "templateStepID",
                                       std::to_string(countryFocus.stepID));
        ParserUtils::replaceOccurences(tempContent, "templateChainID",
                                       std::to_string(countryFocus.chainID));
        ParserUtils::replaceOccurences(tempContent, "templateSourceTag",
                                       c.first);
        ParserUtils::replaceOccurences(tempContent, "templateSourcename",
                                       c.second.name);
        ParserUtils::replaceOccurences(tempContent, "templateDestTag",
                                       countryFocus.destTag);
        // need a faction name
        if (tempContent.find("templateFactionname") != std::string::npos) {
          auto facName = NameGeneration::generateFactionName(
              c.second.rulingParty, c.second.name, c.second.adjective, nData);
          ParserUtils::replaceOccurences(tempContent, "templateFactionname",
                                         facName);
        }
        ParserUtils::replaceOccurences(tempContent, "templateDestTag",
                                       countryFocus.destTag);
        ParserUtils::replaceOccurences(
            tempContent, "templateXPosition",
            std::to_string(countryFocus.position[0]));
        ParserUtils::replaceOccurences(
            tempContent, "templateYPosition",
            std::to_string(countryFocus.position[1]));
        // now collect all prerequisites
        std::string preString = "";
        std::vector<std::vector<int>> andBlocks;
        std::set<int> usedF;
        for (const auto &prerequisite : countryFocus.precedingFoci) {
          // for every prerequisite, resolve and/or dependencies
          // first, resolve ands, and put them together
          // then, resolve or, and put them together
          // for every or, check if it has an and
          // if it has an and, resolve it and put in the group
          for (const auto &foc : focusChain) {
            // we found the prerequisite
            // iterate over and block from prerequisites
            std::vector<int> andBlock;
            if (foc.stepID == prerequisite) {
              for (const auto andF : foc.andFoci) {
                // now, we have found the focus. Now, resolve its and
                // dependencies
                if (usedF.find(andF) == usedF.end())
                  andBlock.push_back(andF);
                if (usedF.find(foc.stepID) == usedF.end())
                  andBlock.push_back(foc.stepID);
                usedF.insert(andF);
                usedF.insert(foc.stepID);
              }
              usedF.insert(foc.stepID);
            }
            if (andBlock.size())
              andBlocks.push_back(andBlock);
          }
        }

        for (auto &andBlock : andBlocks)
          std::sort(andBlock.begin(), andBlock.end());
        std::vector<std::vector<int>> preRequisiteBlocks;
        if (andBlocks.size() > 1) {
          int counter = 0;
          for (const auto &aBlock : andBlocks) {
            if (aBlock.size()) {
              preRequisiteBlocks.push_back(std::vector<int>{});
              preRequisiteBlocks[counter++].push_back(aBlock[0]);
            }
            std::string preName = Fwg::Utils::varsToString(
                c.first, focusChain[0].chainID, ".", aBlock[0]);

            preString += "prerequisite = {";
            preString += " focus = " + preName + " }\n\t\t";
          }
        } else {
          // no and cases, so just list all potential predecessors
          preString += "prerequisite = {";
          for (const auto &elem : usedF) {
            std::string preName = Fwg::Utils::varsToString(
                c.first, focusChain[0].chainID, ".", elem);
            preString += " focus = " + preName + " ";
          }
          preString += "}\n";
        }
        /* we now have the main prerequisites. Those prerequisiteBlocks will
        be placed in the file as prerequisite = { block1.focus1 ... }
        Now, these might have an OR relation
        * Now check for alternatives
        * this means that we check the rest of the and blocks

        */

        ParserUtils::replaceOccurences(tempContent, "templatePrerequisite",
                                       preString);
        // now make exclusive
        preString.clear();
        preString += "mutually_exclusive = {";
        for (const auto &exclusive : countryFocus.xorFoci) {
          for (const auto &foc : focusChain) {
            if (foc.stepID == exclusive) {
              // derive the name of the preceding focus
              std::string preName = Fwg::Utils::varsToString(
                  c.first, focusChain[0].chainID, ".", exclusive);
              preString += " focus = " + preName;
            }
          }
        }
        preString += " }\n\t\t";
        ParserUtils::replaceOccurences(tempContent, "templateExclusive",
                                       preString);
      }
    }
    ParserUtils::replaceOccurences(treeContent, "templateFocusTree",
                                   tempContent);
    ParserUtils::replaceOccurences(treeContent, "templateSourceTag", c.first);
    ParserUtils::writeFile(path + c.second.name + ".txt", treeContent);
  }
}

void compatibilityHistory(const std::string &path, const std::string &hoiPath,
                          const std::vector<Fwg::Region> &regions) {
  const std::filesystem::path hoiDir{hoiPath + "\\history\\countries\\"};
  const std::filesystem::path modDir{path};
  for (auto const &dir_entry : std::filesystem::directory_iterator{hoiDir}) {
    std::string pathString = dir_entry.path().string();

    std::string filename =
        pathString.substr(pathString.find_last_of("\\") + 1,
                          pathString.back() - pathString.find_last_of("\\"));
    auto content = pU::readFile(pathString);
    while (content.find("start_resistance = yes") != std::string::npos) {
      pU::removeSurroundingBracketBlockFromLineBreak(content,
                                                     "start_resistance = yes");
    }
    pU::replaceLine(content, "capital =", "capital = " + std::to_string(1));
    pU::writeFile(path + filename, content);
  }
}

} // namespace Writing

namespace Reading {
// reads a text file containing colour->tag relations
// reads a bmp containing colours
Fwg::Utils::ColourTMap<std::string> readColourMapping(const std::string &path) {
  using namespace Scenario::ParserUtils;
  Fwg::Utils::ColourTMap<std::string> colourMap;
  auto mappings = readFile(path + "//common/countries/colors.txt");
  std::string countryColour;
  do {
    countryColour =
        removeSurroundingBracketBlockFromLineBreak(mappings, "color");
    if (countryColour.size()>10) {
      auto tag = countryColour.substr(1, 3);
      auto colourString = getValue(countryColour, "color_ui");
      auto hsv = getBracketBlockContent(colourString, "hsv");
      std::vector<int> rgb(3);
      if (colourString.find("rgb") != std::string::npos) {
        rgb = getNumberBlock(colourString, "rgb");
      } else if (hsv.size()) {
        auto hsvd = getTokens(hsv, ' ');
        for (int i = 0; i < hsvd.size(); i++) {
          if (!hsvd[i].size())
            hsvd.erase(hsvd.begin() + i);
        }

        std::vector<double> hsvv;
        hsvv.push_back(std::stod(hsvd[0]) * 360.0);
        hsvv.push_back(std::stod(hsvd[1]));
        hsvv.push_back(std::stod(hsvd[2]));
        auto C = hsvv[2] * hsvv[1];
        // C × (1 - |(H / 60°) mod 2 - 1|)
        auto X = C * (1.0 - abs(std::fmod((hsvv[0] / 60), 2.0) - 1.0));
        auto m = hsvv[2] - C;
        //  ((R'+m)×255, (G'+m)×255, (B'+m)×255)
        rgb[0] = static_cast<int>((C + m) * 255.0) % 255;
        rgb[1] = static_cast<int>((X + m) * 255.0) % 255;
        rgb[2] = static_cast<int>((0.0 + m) * 255.0) % 255;

        if (hsvv[0] < 60) {
        } else if (hsvv[0] < 120) {
          std::swap(rgb[1], rgb[0]);
        } else if (hsvv[0] < 180) {
          std::swap(rgb[2], rgb[0]);
          std::swap(rgb[2], rgb[1]);
        } else if (hsvv[0] < 240) {
          std::swap(rgb[2], rgb[0]);
        } else if (hsvv[0] < 300) {
          std::swap(rgb[0], rgb[1]);
          std::swap(rgb[1], rgb[2]);
        } else {
          std::swap(rgb[1], rgb[2]);
        }
      }
      Fwg::Gfx::Colour colour{std::vector<int>{rgb[2], rgb[1], rgb[0]}};
      while (colourMap.find(colour)) {
        // duplicate country colour
        colour++;
      }
      colourMap.setValue(colour, tag);
    }
  } while (countryColour.size());
  return colourMap;
}
// states are where tags are written down, expressing ownership of the map
// read them in from path, map province IDs against states
void readStates(const std::string &path, Generator &hoi4Gen) {
  using namespace Scenario::ParserUtils;
  // std::vector<Scenario::Region> regions;
  auto states = readFilesInDirectory(path + "/history/states");

  for (auto &state : states) {
    Scenario::Region r;
    auto reg = std::make_shared<Scenario::Region>(r);
    auto tag = getValue(state, "owner");
    reg->ID = std::stoi(getValue(state, "id")) - 1;
    removeCharacter(tag, ' ');
    reg->owner = tag;
    auto readIDs = getNumberBlock(state, "provinces");
    for (auto id : readIDs)
      reg->gameProvinces.push_back(hoi4Gen.gameProvinces[id - 1]);
    hoi4Gen.gameRegions.push_back(reg);
  }

  std::sort(hoi4Gen.gameRegions.begin(), hoi4Gen.gameRegions.end(),
            [](auto l, auto r) { return *l < *r; });
  for (auto &region : hoi4Gen.gameRegions) {
    if (hoi4Gen.countries.find(region->owner) != hoi4Gen.countries.end()) {
      hoi4Gen.countries.at(region->owner).ownedRegions.push_back(region->ID);
    } else {
      PdoxCountry c;
      c.tag = region->owner;
      c.ownedRegions.push_back(region->ID);
      hoi4Gen.countries.insert({c.tag, c});
    }
  }
}
// get the bmp file info and extract the respective IDs from definition.csv
std::vector<Fwg::Province> readProvinceMap(const std::string &path) {
  using namespace Scenario::ParserUtils;
  auto provMap =
      Fwg::Gfx::Bmp::load24Bit(path + "map/provinces.bmp", "provinces");
  auto definition = getLines(path + "map/definition.csv");
  Fwg::Utils::ColourTMap<Fwg::Province> provinces;
  for (const auto &line : definition) {
    auto nums = getNumbers(line, ';', {0, 1, 2, 3});
    provinces.setValue({static_cast<unsigned char>(nums[1]),
                        static_cast<unsigned char>(nums[2]),
                        static_cast<unsigned char>(nums[3])},
                       {nums[0],
                        {static_cast<unsigned char>(nums[1]),
                         static_cast<unsigned char>(nums[2]),
                         static_cast<unsigned char>(nums[3])},
                        false});
  }
  std::vector<Fwg::Province> retProvs(definition.size());
  for (auto i = 0; i < provMap.imageData.size(); i++) {
    const auto colour = provMap[i];
    provinces[colour].pixels.push_back(i);
    retProvs[provinces[colour].ID].pixels.push_back(i);
  }
  return retProvs;
}
void readAirports(const std::string &path,
                  std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: Map: Watching Planes");
  auto list = ParserUtils::getLines(path + "//map//airports.txt");
  for (const auto &entry : list) {
    auto tokens = ParserUtils::getTokens(entry, '=');
    if (tokens.size() == 2) {
      auto ID = std::stoi(tokens[0]);
      ParserUtils::removeSpecials(tokens[1]);
      auto provID = std::stoi(tokens[1]);
      regions[ID - 1]->airport = provID - 1;
    }
  }
}

void readBuildings(const std::string &path,
                   std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: Map: Observing Infrastructure");
  auto content = pU::getLines(path + "//map//buildings.txt");
  for (const auto &line : content) {
    Scenario::Utils::Building building;
    auto tokens = ParserUtils::getTokens(line, ';');
    building.name = tokens[1];
    building.relativeID = std::stoi(tokens[6]);
    building.position = Scenario::Utils::strToPos(tokens, {2, 3, 4, 5});
    regions[std::stoi(tokens[0]) - 1]->buildings.push_back(building);
  }
}

std::vector<std::vector<std::string>> readDefinitions(const std::string &path) {
  auto list = ParserUtils::getLinesByID(path);
  return list;
}
void readProvinces(const std::string &inPath, const std::string &mapName,
                   Fwg::Areas::AreaData &areaData) {
  Logging::logLine("HOI4 Parser: Map: Studying the land");
  auto provMap =
      Fwg::Gfx::Bmp::load24Bit(inPath + "map//" + mapName, "provinces");
  auto heightMap = Fwg::Gfx::Bmp::load24Bit(
      inPath + "map//" + "heightmap" + ".bmp", "heightmap");
  auto list = readDefinitions(inPath + "map//definition.csv");
  // now map definitions to read in IDs
  for (auto line : list) {
    if (line.size()) {
      auto tokens = ParserUtils::getTokens(line[0], ';');
      auto ID = std::stoi(tokens[0]) - 1;
      if (ID == 0)
        continue;
      auto r = static_cast<unsigned char>(std::stoi(tokens[1]));
      auto g = static_cast<unsigned char>(std::stoi(tokens[2]));
      auto b = static_cast<unsigned char>(std::stoi(tokens[3]));
      Fwg::Province *p = new Fwg::Province();
      p->ID = ID;
      p->colour = {r, g, b};
      areaData.provinceColourMap.setValue(p->colour, p);
      areaData.provinces.push_back(p);
    }
  }
  Fwg::Areas::Provinces::readProvinceBMP(provMap, heightMap, areaData.provinces,
                                         areaData.provinceColourMap);
}
void readRocketSites(const std::string &path,
                     std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: Map: Scanning for rockets");
  auto list = ParserUtils::getLines(path + "//map//rocketsites.txt");
  for (const auto &entry : list) {
    auto tokens = ParserUtils::getTokens(entry, '=');
    if (tokens.size() == 2) {
      auto ID = std::stoi(tokens[0]);
      ParserUtils::removeSpecials(tokens[1]);
      auto provID = std::stoi(tokens[1]);
      regions[ID - 1]->rocketsite = provID - 1;
    }
  }
}
void readSupplyNodes(const std::string &path,
                     std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: Map: Stealing from logistics hub");
  auto list = ParserUtils::getLines(path + "//map//supply_nodes.txt");
  for (const auto &entry : list) {
    auto tokens = ParserUtils::getTokens(entry, '=');
    if (tokens.size() == 2) {
      auto ID = std::stoi(tokens[0]);
      ParserUtils::removeSpecials(tokens[1]);
      auto provID = std::stoi(tokens[1]);
      regions[ID - 1]->supplyNode = provID - 1;
    }
  }
}
void readWeatherPositions(const std::string &path,
                          std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: Map: Observing the Weather");
  auto content = pU::getLines(path + "//map//weatherpositions.txt");
  for (const auto &line : content) {
    Scenario::Utils::WeatherPosition weather;
    auto tokens = ParserUtils::getTokens(line, ';');
    weather.position = Scenario::Utils::strToPos(tokens, {1, 2, 3, 3});
    weather.effectSize = tokens[4];
    regions[std::stoi(tokens[0]) - 1]->weatherPosition = weather;
  }
}
} // namespace Reading

std::vector<std::string> readTypeMap() {
  return ParserUtils::getLines(
      "resources\\hoi4\\ai\\national_focus\\baseFiles\\foci.txt");
}

std::map<std::string, std::string> readRewardMap(const std::string &path) {
  auto file = ParserUtils::readFile(path);
  auto split = ParserUtils::getTokens(file, ';');
  std::map<std::string, std::string> rewardMap;
  for (const auto &elem : split) {
    auto key = ParserUtils::getBracketBlockContent(elem, "key");
    auto value = ParserUtils::getBracketBlockContent(elem, "value");
    rewardMap[key] = value;
  }
  return {rewardMap};
}
void copyDescriptorFile(const std::string &sourcePath,
                        const std::string &destPath,
                        const std::string &modsDirectory,
                        const std::string &modName) {
  auto descriptorText = pU::readFile(sourcePath);
  pU::replaceOccurences(descriptorText, "templateName", modName);
  auto modText{descriptorText};
  pU::replaceOccurences(descriptorText, "templatePath", "");
  pU::writeFile(destPath + "//descriptor.mod", descriptorText);
  pU::replaceOccurences(modText, "templatePath",
                        Fwg::Utils::varsToString("path=\"", destPath, "\""));
  pU::writeFile(modsDirectory + "//" + modName + ".mod", modText);
}

} // namespace Scenario::Hoi4::Parsing