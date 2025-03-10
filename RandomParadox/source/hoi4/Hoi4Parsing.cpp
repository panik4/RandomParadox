#include "hoi4/Hoi4Parsing.h"
#include "hoi4/NationalFocus.h"
using namespace Fwg;
namespace Logging = Fwg::Utils::Logging;
namespace pU = Fwg::Parsing;
namespace Scenario::Hoi4::Parsing {
namespace Writing {
namespace Map {
void adj(const std::string &path) {
  Logging::logLine("HOI4 Parser: Map: Writing Adjacencies");
  // From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment
  // empty file for now
  std::string content;
  content.append("From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_"
                 "rule_name;Comment");
  pU::writeFile(path, content);
}
void adjacencyRules(const std::string &path) {
  Logging::logLine("HOI4 Parser: Map: Writing Adjacency Rules");
  std::string content{""};
  // empty for now
  pU::writeFile(path, content);
}
void ambientObjects(const std::string &path,
                    const Fwg::Gfx::Bitmap &heightMap) {
  Logging::logLine("HOI4 Parser: Map: editing ambient objects to ",
                   Fwg::Utils::userFilter(path, Fwg::Cfg::Values().username));
  auto templateContent = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                      "hoi4//map//ambient_object.txt");

  pU::Scenario::replaceOccurences(templateContent, "template_yresolution_top",
                                  std::to_string(heightMap.height() + 142));
  pU::Scenario::replaceOccurences(templateContent, "template_yresolution_logo",
                                  std::to_string(heightMap.height() + 82));
  // place in middle of map xres
  pU::Scenario::replaceOccurences(templateContent, "template_xpos_logo",
                                  std::to_string(heightMap.width() / 2));
  pU::writeFile(path, templateContent);
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
                const std::vector<ScenarioContinent> &continents,
                const std::string &hoiPath,
                const std::string &localisationPath) {
  Logging::logLine("HOI4 Parser: Map: Writing Continents");
  // copy continents file from cfg::Values().resourcePath + "/hoi4//map// to
  // path//map//
  std::string content = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                     "hoi4//map//continent.txt");
  try {
    std::string continentList;
    for (auto &continent : continents) {
      continentList.append(continent.name + "\n\t");
    }
    pU::replaceOccurences(content, "templateContinents", continentList);
    pU::writeFile(path, content);
  } catch (const std::filesystem::filesystem_error &e) {
    Logging::logLine("HOI4 Parser: Error writing continents.txt: " +
                     std::string(e.what()));
  }
  // read the localisation file  from the gamePath+ "/localisation// to
  // localisation//
  auto continentLocalisation = pU::readFile(
      hoiPath + "//localisation//english//province_names_l_english.yml");
  // add the continents to the localisation file
  for (auto &continent : continents) {
    continentLocalisation.append(" " + continent.name + ":0 \"" +
                                 continent.name + "\"\n");
    continentLocalisation.append(" " + continent.name + "_adj:0 \"" +
                                 continent.adjective + "\"\n");
  }
  pU::writeFile(localisationPath, continentLocalisation);
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

void unitStacks(const std::string &path,
                const std::vector<std::shared_ptr<Province>> provinces,
                const std::vector<std::shared_ptr<Region>> regions,
                const Fwg::Gfx::Bitmap &heightMap) {
  Logging::logLine("HOI4 Parser: Map: Remilitarizing the Rhineland");
  // 1;0;3359.00;9.50;1166.00;0.00;0.08
  // provID, neighbour?, xPos, zPos yPos, rotation(3=north,
  // 0=south, 1.5=east,4,5=west), ?? provID, xPos, ~10, yPos, ~0, 0,5 for each
  // neighbour add move state in the direction of the neighbour. 0 might be
  // stand still
  std::string content{""};
  for (const auto &prov : provinces) {
    int unitstackIndex = 0;
    auto pix = Fwg::Utils::selectRandom(prov->pixels);
    auto widthPos = pix % Cfg::Values().width;
    auto heightPos = pix / Cfg::Values().width;
    std::vector<std::string> arguments{
        std::to_string(prov->ID + 1),
        std::to_string(unitstackIndex),
        std::to_string(widthPos),
        std::to_string((double)heightMap[pix].getRed() / 10.0),
        std::to_string(heightPos),
        std::to_string(0.0),
        "0.0"};
    content.append(pU::csvFormat(arguments, ';', false));
    // attacking
    arguments[1] = "9";
    content.append(pU::csvFormat(arguments, ';', false));
    // defending
    arguments[1] = "10";
    content.append(pU::csvFormat(arguments, ';', false));
    // standstill RG
    arguments[1] = "21";
    content.append(pU::csvFormat(arguments, ';', false));
    unitstackIndex++;
    for (const auto &neighbour : prov->neighbours) {
      if (unitstackIndex < 9) {
        double angle;
        auto nextPos = prov->getPositionBetweenProvinces(
            *neighbour, Cfg::Values().width, angle);
        angle += 1.57;
        auto widthPos = nextPos % Cfg::Values().width;
        auto heightPos = nextPos / Cfg::Values().width;
        std::vector<std::string> arguments{
            std::to_string(prov->ID + 1),
            std::to_string(unitstackIndex),
            std::to_string(widthPos),
            std::to_string((double)heightMap[pix].getRed() / 10.0),
            std::to_string(heightPos),
            std::to_string(angle),
            "0.0"};
        content.append(pU::csvFormat(arguments, ';', false));
        // regroup equivalent
        arguments[1] = std::to_string(21 + unitstackIndex);
        content.append(pU::csvFormat(arguments, ';', false));
        unitstackIndex++;
      }
    }

    if (prov->coastal) {
      unitstackIndex = 11;
      for (const auto &neighbour : prov->neighbours) {
        if (prov->sea) {
          if (unitstackIndex < 19) {
            double angle;
            auto nextPos = prov->getPositionBetweenProvinces(
                *neighbour, Cfg::Values().width, angle);
            angle += 1.57;
            auto widthPos = nextPos % Cfg::Values().width;
            auto heightPos = nextPos / Cfg::Values().width;
            std::vector<std::string> arguments{
                std::to_string(prov->ID + 1),
                std::to_string(unitstackIndex),
                std::to_string(widthPos),
                std::to_string((double)heightMap[pix].getRed() / 10.0),
                std::to_string(heightPos),
                std::to_string(angle),
                "0.0"};
            content.append(pU::csvFormat(arguments, ';', false));
            unitstackIndex++;
          }
        }
      }
    }
  }
  for (auto &region : regions) {
    for (auto &vp : region->victoryPointsMap) {
      std::vector<std::string> arguments{
          std::to_string(vp.first + 1),
          std::to_string(38),
          std::to_string(vp.second.position.widthCenter),
          std::to_string(
              (double)heightMap[vp.second.position.weightedCenter].getRed() /
              10.0),
          std::to_string(vp.second.position.heightCenter),
          std::to_string(0.0),
          "0.0"};
      content.append(pU::csvFormat(arguments, ';', false));
    }
  }
  pU::writeFile(path, content);
}

void strategicRegions(const std::string &path,
                      const std::vector<Fwg::Region> &regions,
                      const std::vector<StrategicRegion> &strategicRegions) {
  constexpr std::array<int, 12> daysInMonth{30, 27, 30, 29, 30, 29,
                                            30, 30, 29, 30, 29, 30};
  Logging::logLine("HOI4 Parser: Map: Drawing Strategic Regions");
  auto templateContent = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                      "hoi4//map//strategic_region.txt");
  // first clear target path folder
  Fwg::IO::Utils::clearFilesOfType(path, ".txt");
  const auto templateWeather =
      pU::Scenario::getBracketBlock(templateContent, "period");
  for (auto i = 0; i < strategicRegions.size(); i++) {
    std::string provString{""};
    for (const auto &region : strategicRegions[i].gameRegions) {
      for (const auto prov : region->provinces) {
        provString.append(std::to_string(prov->ID + 1));
        provString.append(" ");
      }
    }
    auto content{templateContent};
    pU::Scenario::replaceOccurences(content, "templateID",
                                    std::to_string(i + 1));
    pU::Scenario::replaceOccurences(content, "template_provinces", provString);

    // weather
    std::string weather{""};
    for (auto mo = 0; mo < 12; mo++) {
      auto month{templateWeather};
      pU::Scenario::replaceOccurences(month, "templateDateRange",
                                      "0." + std::to_string(mo) + " " +
                                          std::to_string(daysInMonth[mo]) +
                                          "." + std::to_string(mo));
      pU::Scenario::replaceOccurences(
          month, "templateTemperatureRange",
          std::to_string(round((float)strategicRegions[i].weatherMonths[mo][3]))
                  .substr(0, 5) +
              " " +
              std::to_string(
                  round((float)strategicRegions[i].weatherMonths[mo][4]))
                  .substr(0, 5));
      pU::Scenario::replaceOccurences(
          month, "templateRainLightChance",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][5]));
      pU::Scenario::replaceOccurences(
          month, "templateRainHeavyChance",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][6]));
      pU::Scenario::replaceOccurences(
          month, "templateMud",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][7]));
      pU::Scenario::replaceOccurences(
          month, "templateBlizzard",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][8]));
      pU::Scenario::replaceOccurences(
          month, "templateSandStorm",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][9]));
      pU::Scenario::replaceOccurences(
          month, "templateSnow",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][10]));
      pU::Scenario::replaceOccurences(
          month, "templateNoPhenomenon",
          std::to_string((float)strategicRegions[i].weatherMonths[mo][11]));
      // pU::Scenario::replaceOccurences(month, "templateDateRange", "0." +
      // std::to_string(i) + " 30." + std::to_string(i));
      // pU::Scenario::replaceOccurences(month, "templateDateRange", "0." +
      // std::to_string(i) + " 30." + std::to_string(i));
      weather.append(month + "\n\t\t");
    }
    pU::Scenario::replaceOccurences(content, templateWeather, weather);
    pU::Scenario::replaceOccurences(content, "template_provinces", provString);
    pU::writeFile(Fwg::Utils::varsToString(path, "//", (i + 1), ".txt"),
                  content);
  }
}
void weatherPositions(const std::string &path,
                      const std::vector<Fwg::Region> &regions,
                      std::vector<StrategicRegion> &strategicRegions) {
  Logging::logLine("HOI4 Parser: Map: Creating Storms");
  // 1; 2781.24; 9.90; 1571.49; small
  std::string content{""};
  // stateId; pixelX; rotation??; pixelY; rotation??; size
  // 1; arms_factory; 2946.00; 11.63; 1364.00; 0.45; 0

  // delete strategic regions that have no gameRegions
  for (auto i = 0;
       i < strategicRegions.size();) { // Removed increment from here
    if (strategicRegions[i].gameRegions.size() == 0) {
      // do the erase
      strategicRegions.erase(strategicRegions.begin() + i);
      // No need to decrement i since we're not incrementing it in the loop
      // header
    } else {
      ++i; // Increment i only if an element is not erased
    }
  }

  for (auto i = 0; i < strategicRegions.size(); i++) {
    const auto &region =
        Fwg::Utils::selectRandom(strategicRegions[i].gameRegions);
    const auto prov = Fwg::Utils::selectRandom(region->provinces);
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

  pU::writeFile(path + "supply_nodes.txt", supplyNodes);
  pU::writeFile(path + "railways.txt", railways);
}

} // namespace Map
namespace Countries {
void commonCountries(const std::string &path, const std::string &hoiPath,
                     const CountryMap &countries) {
  Logging::logLine("HOI4 Parser: Common: Writing Countries");
  Fwg::IO::Utils::clearFilesOfType(path, ".txt");
  const auto content = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                    "hoi4//common//country_default.txt");
  const auto colorsTxtTemplate = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                              "hoi4//common//colors.txt");
  std::string colorsTxt = pU::readFile(hoiPath);
  for (const auto &country : countries) {
    auto tempPath = path + country->name + ".txt";
    auto countryText{content};
    auto col = Fwg::Utils::varsToString(country->colour);
    auto colourString = pU::Scenario::replaceOccurences(col, ";", " ");
    pU::Scenario::replaceOccurences(countryText, "templateCulture",
                                    country->gfxCulture);
    pU::Scenario::replaceOccurences(countryText, "templateColour",
                                    colourString);
    pU::writeFile(tempPath, countryText);
    auto templateCopy{colorsTxtTemplate};
    pU::Scenario::replaceOccurences(templateCopy, "templateTag", country->tag);
    pU::Scenario::replaceOccurences(templateCopy, "templateColour",
                                    colourString);
    colorsTxt.append(templateCopy);
  }
  pU::writeFile(path + "colors.txt", colorsTxt);
}

void commonCountryTags(const std::string &path, const CountryMap &countries) {
  Logging::logLine("HOI4 Parser: Common: Writing Country Tags");
  std::string content = "";
  for (const auto &country : countries)
    content.append(country->tag + " = \"countries/" + country->name +
                   ".txt\"\n");
  pU::writeFile(path, content);
}

void commonCharacters(const std::string &path, const CountryMap &countries) {
  Logging::logLine("HOI4 Parser: Common: Writing Characters");
  Fwg::IO::Utils::clearFilesOfType(path, ".txt");
  const auto characterTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//common//characters//characterTemplate.txt");
  const auto advisorTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//common//characters//advisorTemplate.txt");
  const auto leaderTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//common//characters//leaderTemplate.txt");
  std::map<Type, std::string> slotTypes = {
      {Type::Politician, "political_advisor"},
      {Type::ArmyChief, "army_chief"},
      {Type::NavyChief, "navy_chief"},
      {Type::AirForceChief, "air_chief"},
      {Type::Theorist, "theorist"},
      {Type::HighCommand, "high_command"}};
  // map to randomly get one of the ideologies
  std::map<Ideology, std::vector<std::string>> ideologyMap{
      {Ideology::Fascist,
       {"rexism", "falangism", "fascism_ideology", "nazism"}},
      {Ideology::Communist,
       {"marxism", "leninism", "stalinism", "anti_revisionism",
        "anarchist_communism"}},
      {Ideology::Democratic, {"conservatism", "liberalism", "socialism"}},
      {Ideology::Neutral,
       {"despotism", "oligarchism", "moderatism", "centrism"}}};

  for (const auto &country : countries) {
    std::string content = "characters = {\n";

    auto tempPath = path + country->tag + ".txt";
    for (const auto &character : country->characters) {
      auto characterText = characterTemplate;
      if (character.type == Type::Leader) {
        pU::Scenario::replaceOccurences(characterText, "templateCharacterType",
                                        leaderTemplate);
      } else if (character.type != Type::ArmyGeneral &&
                 character.type != Type::FleetAdmiral) {
        pU::Scenario::replaceOccurences(characterText, "templateCharacterType",
                                        advisorTemplate);
      }
      pU::Scenario::replaceOccurences(characterText, "templateCountryTag",
                                      country->tag);
      pU::Scenario::replaceOccurences(characterText, "templateName",
                                      character.name);
      pU::Scenario::replaceOccurences(characterText, "templateLastName",
                                      character.surname);
      pU::Scenario::replaceOccurences(characterText, "templateType",
                                      slotTypes[character.type]);
      pU::Scenario::replaceOccurences(
          characterText, "templateIdeology",
          Fwg::Utils::selectRandom(ideologyMap[character.ideology]));

      std::string traits = "";
      for (const auto &trait : character.traits) {
        traits.append(trait + " ");
      }
      pU::Scenario::replaceOccurences(characterText, "templateTraits", traits);

      // pU::Scenario::replaceOccurences(characterText, "templateIdeology",
      //                                 character.ideology);
      // pU::Scenario::replaceOccurences(characterText, "templateTraits",
      //                                 character->traits);
      content.append(characterText);
    }
    content.append("}");
    pU::writeFile(tempPath, content);
  }
}
void commonNames(const std::string &path, const CountryMap &countries) {

  Logging::logLine("HOI4 Parser: Common: Naming people");
  const auto countryNamesTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//common//names//countryNamesTemplate.txt");
  auto content = pU::readFile(Fwg::Cfg::Values().resourcePath +
                              "hoi4//common//names//00_names.txt");
  // gather a list of male, female and surnames, dependent on the cultures
  // and their share in the country
  std::map<std::string, std::vector<std::string>> names;
  std::string maleNames = "";
  std::string femaleNames = "";
  std::string surnames = "";
  for (auto &country : countries) {

    auto nameTemplate = countryNamesTemplate;
    for (auto &culture : country->cultures) {
      // get the share of the culture in the country
      auto share = culture.second / country->populationFactor;
      auto language = culture.first->language;
      // get the names for the culture
      for (int i = 1; i < share * (double)language->maleNames.size(); i++) {
        maleNames.append("\"" + language->maleNames[i] + "\"" + " ");
        if (i % 10 == 0)
          maleNames.append("\n\t\t\t");
      }
      for (int i = 0; i < share * (double)language->femaleNames.size(); i++) {
        femaleNames.append("\"" + language->femaleNames[i] + "\"" + " ");
        if (i % 10 == 0)
          femaleNames.append("\n\t\t\t");
      }
      for (int i = 0; i < share * (double)language->surnames.size(); i++) {
        surnames.append("\"" + language->surnames[i] + "\"" + " ");
        if (i % 10 == 0)
          surnames.append("\n\t\t\t");
      }
      // now replace templateCountryTag
      pU::Scenario::replaceOccurences(nameTemplate, "templateCountryTag",
                                      country->tag);
      pU::Scenario::replaceOccurences(nameTemplate, "templateMaleNames",
                                      maleNames);
      pU::Scenario::replaceOccurences(nameTemplate, "templateFemaleNames",
                                      femaleNames);
      pU::Scenario::replaceOccurences(nameTemplate, "templateSurnames",
                                      surnames);
    }
    content.append(nameTemplate);
  }
  pU::writeFile(path, content);
}

void foci(const std::string &path, const CountryMap &countries,
          const NameGeneration::NameData &nData) {
  Logging::logLine("HOI4 Parser: History: Demanding Danzig");
  Fwg::IO::Utils::clearFilesOfType(path, ".txt");
  const auto focusTypes =
      pU::getLines(Fwg::Cfg::Values().resourcePath +
                   "hoi4//ai//national_focus//baseFiles//foci.txt");
  std::string baseTree =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//ai//national_focus//baseFiles//focusBase.txt");
  std::vector<std::string> focusTemplates;
  for (const auto &focusType : focusTypes)
    focusTemplates.push_back(pU::readFile(
        Fwg::Cfg::Values().resourcePath +
        "hoi4//ai//national_focus//focusTypes//" + focusType + "Focus.txt"));

  for (const auto &country : countries) {

    pU::writeFile(path + country->name + ".txt", country->focusTree);
  }
}

void ideas(const std::string &path, const CountryMap &countries) {
  Fwg::IO::Utils::clearFilesOfType(path, ".txt");
  for (const auto &country : countries) {

    pU::writeFile(path + country->name + ".txt", country->ideas);
  }
}

void flags(const std::string &path, const CountryMap &countries) {
  Logging::logLine("HOI4 Parser: Gfx: Printing Flags");
  using namespace Gfx::Textures;
  Fwg::IO::Utils::clearFilesOfType(path, ".tga");
  for (const auto &country : countries) {
    writeTGA(country->flag.width, country->flag.height, country->flag.getFlag(),
             path + country->tag + ".tga");
    writeTGA(
        country->flag.width / 2, country->flag.height / 2,
        country->flag.resize(country->flag.width / 2, country->flag.height / 2),
        path + "//medium//" + country->tag + ".tga");
    writeTGA(10, 7, country->flag.resize(10, 7),
             path + "//small//" + country->tag + ".tga");
  }
}

void historyCountries(const std::string &path, const CountryMap &countries,
                      const std::string &gamePath,
                      const std::vector<Fwg::Region> &regions) {
  Logging::logLine("HOI4 Parser: History: Writing Country History");
  Fwg::IO::Utils::clearFilesOfType(path, ".txt");
  // now compat countries
  compatibilityHistory(path, gamePath, regions);

  const auto content = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                    "hoi4//history//country_template.txt");

  const auto navyTemplateFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//history//navy//baseVariantFile.txt");
  const auto navyTechFile = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                         "hoi4//history//navy//navyTechs.txt");
  const auto airTemplateFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//history//airforce//baseVariantFile.txt");
  const auto armorTemplateFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//history//army//baseVariantFile.txt");

  for (const auto &country : countries) {
    auto tempPath = path + country->tag + " - " + country->name + ".txt";
    auto countryText{content};

    pU::Scenario::replaceOccurences(
        countryText, "templateCapital",
        std::to_string(country->capitalRegionID + 1));

    // technologies
    std::string techs = "";
    for (auto &techEra : country->infantryTechs) {
      for (auto &tech : techEra.second) {
        techs += tech.name + "= 1\n\t";
      }
    }
    for (auto &techEra : country->industryElectronicTechs) {
      for (auto &tech : techEra.second) {
        techs += tech.name + "= 1\n\t";
      }
    }
    // replace
    pU::Scenario::replaceOccurences(countryText, "templateGenericTechBlock",
                                    techs);

    // variants for armor, airforce and navy are predetermined strings, as their
    // assembly is rather difficult due to vanilla/dlc variations

    pU::Scenario::replaceOccurences(countryText, "templateNavalBlock",
                                    navyTemplateFile);
    pU::Scenario::replaceOccurences(countryText, "templateAirBlock",
                                    airTemplateFile);
    pU::Scenario::replaceOccurences(countryText, "templateArmorBlock",
                                    armorTemplateFile);

    pU::Scenario::replaceOccurences(countryText, "templateResearchSlots",
                                    std::to_string(country->researchSlots));
    pU::Scenario::replaceOccurences(countryText, "templateConvoys",
                                    std::to_string(country->convoyAmount));

    pU::Scenario::replaceOccurences(countryText, "templateStability",
                                    std::to_string(country->stability));
    pU::Scenario::replaceOccurences(countryText, "templateWarSupport",
                                    std::to_string(country->warSupport));
    pU::Scenario::replaceOccurences(countryText, "templateTag", country->tag);
    pU::Scenario::replaceOccurences(countryText, "templateParty",
                                    country->ideology);

    pU::Scenario::replaceOccurences(countryText, "templateAllowElections",
                                    country->allowElections ? "yes" : "no");
    pU::Scenario::replaceOccurences(countryText, "templateLastElection",
                                    country->lastElection);
    pU::Scenario::replaceOccurences(countryText, "templateFasPop",
                                    std::to_string(country->parties[0]));
    pU::Scenario::replaceOccurences(countryText, "templateDemPop",
                                    std::to_string(country->parties[1]));
    pU::Scenario::replaceOccurences(countryText, "templateComPop",
                                    std::to_string(country->parties[2]));
    pU::Scenario::replaceOccurences(countryText, "templateNeuPop",
                                    std::to_string(country->parties[3]));

    // to map from bba techs to vanilla air techs
    std::map<std::string, std::string> airTechMap{
        {"bba_early_transport_plane", "early_transport_plane"},
        {"iw_small_airframe", "early_fighter"},
        {"iw_medium_airframe", "early_bomber"},
        {"iw_large_airframe", "strategic_bomber1"},
        {"air_torpedoe_1", "naval_bomber1"},
        {"photo_reconnaisance", "scout_plane1"},
        {"early_bombs", "CAS1"}};
    std::string vanillaAirTechs = "";
    std::string bbaAirTechs = "";
    for (auto &techEra : country->airTechs) {
      for (auto &tech : techEra.second) {
        bbaAirTechs += tech.name + " = 1\n\t\t\t";
        if (airTechMap.find(tech.name) != airTechMap.end()) {
          vanillaAirTechs += airTechMap[tech.name] + " = 1\n\t\t\t";
        }
      }
    }
    pU::Scenario::replaceOccurences(countryText, "templateVanillaAirTechs",
                                    vanillaAirTechs);
    pU::Scenario::replaceOccurences(countryText, "templateBbaAirTechs",
                                    bbaAirTechs);

    // air variants
    const auto airVariantsTemplate =
        pU::readFile(Fwg::Cfg::Values().resourcePath +
                     "hoi4//history//airforce//baseVariant.txt");
    std::string airVariants = "";
    for (const auto &airVariant : country->planeVariants) {
      std::string variantString = airVariantsTemplate;
      std::string moduleString;
      for (auto &airModule : airVariant.bbaModules) {
        moduleString.append("\t\t\t" + airModule.first + " = " +
                            airModule.second + "\n");
      }
      pU::Scenario::replaceOccurences(variantString, "templateModules",
                                      moduleString);
      pU::Scenario::replaceOccurences(variantString, "templateVariantName",
                                      airVariant.name);
      pU::Scenario::replaceOccurences(variantString, "templateFrameType",
                                      airVariant.bbaFrameName);
      airVariants.append(variantString);
    }
    pU::Scenario::replaceOccurences(countryText, "templateAirVariants",
                                    airVariants);

    // same for armor
    std::map<std::string, std::string> armorTechMap{
        {"gwtank_chassis", "gwtank"},
        {"basic_light_tank_chassis", "basic_light_tank"},
        {"basic_heavy_tank_chassis", "basic_heavy_tank"},
        {"improved_light_tank_chassis", "improved_light_tank"}};
    std::string vanillaArmorTechs = "";
    std::string nsbArmorTechs = "";
    for (auto &techEra : country->armorTechs) {
      for (auto &tech : techEra.second) {
        nsbArmorTechs += tech.name + " = 1\n\t\t";
        if (armorTechMap.find(tech.name) != armorTechMap.end()) {
          vanillaArmorTechs += armorTechMap[tech.name] + " = 1\n\t\t";
        }
      }
    }
    pU::Scenario::replaceOccurences(countryText, "templateVanillaArmorTechs",
                                    vanillaArmorTechs);
    pU::Scenario::replaceOccurences(countryText, "templateNsbArmorTechs",
                                    nsbArmorTechs);

    const auto armorVariantTemplate =
        pU::readFile(Fwg::Cfg::Values().resourcePath +
                     "hoi4//history//army//baseVariant.txt");
    std::string armorVariants = "";
    for (const auto &tankVariant : country->tankVariants) {
      std::string variantString = armorVariantTemplate;
      std::string moduleString;
      for (auto &tankModule : tankVariant.nsbModules) {
        moduleString.append("\t\t\t" + tankModule.first + " = " +
                            tankModule.second + "\n");
      }
      pU::Scenario::replaceOccurences(variantString, "templateModules",
                                      moduleString);
      pU::Scenario::replaceOccurences(variantString, "templateVariantName",
                                      tankVariant.name);
      pU::Scenario::replaceOccurences(variantString, "templateChassisType",
                                      tankVariant.bbaArmorName);
      armorVariants.append(variantString);
    }
    pU::Scenario::replaceOccurences(countryText, "templateTankVariants",
                                    armorVariants);

    // map from shipclassType to string
    std::map<ShipClassType, std::string> shipClassTypeMap{
        {ShipClassType::Destroyer, "Destroyer"},
        {ShipClassType::LightCruiser, "LightCruiser"},
        {ShipClassType::HeavyCruiser, "HeavyCruiser"},
        {ShipClassType::BattleCruiser, "BattleCruiser"},
        {ShipClassType::BattleShip, "BattleShip"},
        {ShipClassType::Carrier, "Carrier"},
        {ShipClassType::Submarine, "Submarine"}};

    std::string navyTechs = "";
    std::string mtgNavyTechs = "";
    std::set<std::string> ownedVanillaTechs;
    std::set<std::string> ownedMtgTechs;
    for (auto &navyTechEra : country->navyTechs) {
      for (auto &navyTech : navyTechEra.second)
        ownedMtgTechs.insert(navyTech.name);
    }
    std::vector<std::string> vanillaVariants;
    std::vector<std::string> mtgVariants;
    // now replace all class names
    for (auto &eraShipClasses : country->shipClasses) {
      for (auto &shipClass : eraShipClasses.second) {

        // search key build: first for vanilla: vanilla,0 for Interwar, 1 for
        // Buildup, shipClassTypeMap value
        std::string searchKey = ",";
        if (shipClass.era == TechEra::Interwar) {
          searchKey += "0,";
        } else if (shipClass.era == TechEra::Buildup) {
          searchKey += "1,";
        }
        searchKey += shipClassTypeMap[eraShipClasses.first];
        std::string vanillaHullType =
            pU::getValue(navyTechFile, "vanilla" + searchKey);
        ownedVanillaTechs.insert(vanillaHullType);
        std::string mtgHullType = pU::getValue(navyTechFile, "mtg" + searchKey);
        ownedMtgTechs.insert(mtgHullType);
        std::string vanillaVariant =
            "\t\tcreate_equipment_variant = {\n\t\t\tname = \"" +
            shipClass.name + "\"\n";
        std::string mtgVariant = vanillaVariant;
        vanillaVariant += "\t\t\ttype = " + shipClass.vanillaShipType + "\n";
        vanillaVariant += "\t\t\tupgrades = {\n";
        if (shipClass.type != ShipClassType::Destroyer) {
          vanillaVariant += "\t\t\t\tship_reliability_upgrade = 1\n";
        }
        if (shipClass.type != ShipClassType::Destroyer &&
            shipClass.type != ShipClassType::Submarine &&
            shipClass.type != ShipClassType::LightCruiser &&
            shipClass.type != ShipClassType::Carrier) {
          vanillaVariant += "\t\t\t\tship_armor_upgrade = 1\n";
        }
        if (shipClass.type == ShipClassType::LightCruiser ||
            shipClass.type == ShipClassType::Destroyer) {
          vanillaVariant += "\t\t\t\tship_anti_air_upgrade = 1\n";
        }
        if (shipClass.type == ShipClassType::Destroyer) {
          vanillaVariant += "\t\t\t\tship_torpedo_upgrade = 1\n";
        }
        if (shipClass.type == ShipClassType::Submarine) {
          vanillaVariant += "\t\t\t\tsub_torpedo_upgrade = 1\n";
        }
        if (shipClass.type != ShipClassType::Submarine &&
            shipClass.type != ShipClassType::Destroyer &&
            shipClass.type != ShipClassType::Carrier) {
          vanillaVariant += "\t\t\t\tship_gun_upgrade = 1\n";
        }
        if (shipClass.type == ShipClassType::Destroyer) {
          vanillaVariant += "\t\t\t\tdestroyer_engine_upgrade = 1\n";
        } else if (shipClass.type == ShipClassType::Submarine) {
          vanillaVariant += "\t\t\t\tsub_engine_upgrade = 1\n";
        } else {
          vanillaVariant += "\t\t\t\tship_engine_upgrade = 1\n";
        }
        // stealth for submarines only
        if (shipClass.type == ShipClassType::Submarine) {
          vanillaVariant += "\t\t\t\tsub_stealth_upgrade = 1\n";
        }
        // ASW for destroyers only
        if (shipClass.type == ShipClassType::Destroyer) {
          vanillaVariant += "\t\t\t\tship_ASW_upgrade = 1\n";
        }
        // deck space for carriers only
        if (shipClass.type == ShipClassType::Carrier) {
          vanillaVariant += "\t\t\t\tship_deckspace_upgrade = 1\n";
        }
        vanillaVariant += "\t\t\t}\n\t\t}\n";

        mtgVariant += "\t\t\ttype = " + shipClass.mtgHullname + "\n";
        mtgVariant += "\t\t\tparent_version = 0\n";

        mtgVariant += "\t\t\tmodules = {\n";
        for (auto &mtgModule : shipClass.mtgModules) {
          mtgVariant +=
              "\t\t\t\t" + mtgModule.first + " = " + mtgModule.second + "\n";
        }
        mtgVariant += "\t\t\t}\n\t\t}\n";
        vanillaVariants.push_back(vanillaVariant);
        mtgVariants.push_back(mtgVariant);
      }
    }
    for (auto &tech : ownedVanillaTechs) {
      navyTechs += tech + " = 1\n\t";
    }
    for (auto &tech : ownedMtgTechs) {
      mtgNavyTechs += tech + " = 1\n\t";
    }
    std::string vanillaVariantsString = "";
    std::string mtgVariantsString = "";
    for (auto &variant : vanillaVariants) {
      vanillaVariantsString += variant;
    }
    for (auto &variant : mtgVariants) {
      mtgVariantsString += variant;
    }

    pU::Scenario::replaceOccurences(countryText, "templateNavyTech", navyTechs);
    pU::Scenario::replaceOccurences(countryText, "templateMtgNavyTech",
                                    mtgNavyTechs);
    pU::Scenario::replaceOccurences(countryText, "templateVariants",
                                    vanillaVariantsString);
    pU::Scenario::replaceOccurences(countryText, "templateMtgVariants",
                                    mtgVariantsString);

    // now gather characters, with a line of "recruit_character = id"
    std::string characters = "";
    for (auto &character : country->characters) {
      characters += "recruit_character = " + country->tag + "_" +
                    character.name + "_" + character.surname + "\n";
    }
    pU::Scenario::replaceOccurences(countryText, "templateCharacters",
                                    characters);

    pU::writeFile(tempPath, countryText);
  }
}
void historyUnits(const std::string &path, const CountryMap &countries) {
  Logging::logLine("HOI4 Parser: History: Deploying the Troops");
  Fwg::IO::Utils::clearFilesOfType(path, ".txt");
  const auto defaultTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//history//army//defaultUnitTemplate.txt");
  const auto unitBlock = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                      "hoi4//history//army//unitBlock.txt");

  const auto divisionTemplateFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//history//army//divisionTemplate.txt");
  const auto baseAirFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//history//airforce//baseAirFile.txt");
  const auto baseWingFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//history//airforce//baseWingFile.txt");
  const auto baseAirbaseFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//history//airforce//baseAirbaseFile.txt");
  // map from regiment type to string
  std::map<CombatRegimentType, std::string> regimentTypeMap{
      {CombatRegimentType::Infantry, "infantry"},
      {CombatRegimentType::Irregulars, "infantry"},
      {CombatRegimentType::Cavalry, "cavalry"},
      {CombatRegimentType::Motorized, "motorized"},
      {CombatRegimentType::AntiTank, "anti_tank_brigade"},
      {CombatRegimentType::AntiAir, "anti_air_brigade"},
      {CombatRegimentType::Artillery, "artillery_brigade"},
      {CombatRegimentType::Marines, "marine"},
      {CombatRegimentType::Mountaineers, "mountaineers"},
      {CombatRegimentType::Paratroopers, "paratrooper"},
      {CombatRegimentType::MotorizedAntiAir, "mot_anti_air_brigade"},
      {CombatRegimentType::MotorizedAntiTank, "mot_anti_tank_brigade"},
      {CombatRegimentType::MotorizedArtillery, "mot_artillery_brigade"},
      {CombatRegimentType::LightArmor, "light_armor"}};
  std::map<SupportRegimentType, std::string> supportRegimentTypeMap = {
      {SupportRegimentType::Engineer, "engineer"},
      {SupportRegimentType::Recon, "recon"},
      {SupportRegimentType::FieldHospital, "field_hospital"},
      {SupportRegimentType::Logistics, "logistics"},
      {SupportRegimentType::Maintenance, "maintenance_company"},
      {SupportRegimentType::AntiAir, "anti_air"},
      {SupportRegimentType::Artillery, "artillery"},
      {SupportRegimentType::AntiTank, "anti_tank"}};

  for (const auto &country : countries) {
    std::vector<int> allowedProvinces;
    for (auto &region : country->hoi4Regions) {
      for (auto &prov : region->gameProvinces) {
        if (!prov->baseProvince->isLake)
          allowedProvinces.push_back(prov->ID);
      }
    }

    // get the template file
    std::string unitFile = defaultTemplate;
    std::string divisionTemplates = "";
    // now insert all the unit templates for this country
    for (const auto divisionTemplate : country->divisionTemplates) {
      auto tempDivisionTemplate = divisionTemplateFile;
      Fwg::Parsing::Scenario::replaceOccurences(tempDivisionTemplate,
                                                "templateDivisionTemplateName",
                                                divisionTemplate.name);
      // start adding the regiments
      std::string regiments = "";
      for (int i = 0; i < divisionTemplate.regiments.size(); i++) {
        const auto &regimentColumn = divisionTemplate.regiments[i];
        for (int x = 0; x < regimentColumn.size(); x++) {
          regiments += "\t\t\t" + regimentTypeMap.at(regimentColumn[x]) +
                       " = {  x = " + std::to_string(i) +
                       " y = " + std::to_string(x) + " }\n";
        }
      }
      Fwg::Parsing::Scenario::replaceOccurences(tempDivisionTemplate,
                                                "templateRegiments", regiments);
      std::string supports = "";
      for (int i = 0; i < divisionTemplate.supportRegiments.size(); i++) {
        supports +=
            "\t\t\t" +
            supportRegimentTypeMap.at(divisionTemplate.supportRegiments[i]) +
            " = {  x = " + std::to_string(i) + " y = 0 }\n";
      }
      Fwg::Parsing::Scenario::replaceOccurences(
          tempDivisionTemplate, "templateSupportRegiments", supports);
      divisionTemplates += tempDivisionTemplate;
    }

    Fwg::Parsing::Scenario::replaceOccurences(unitFile, "templateTemplateBlock",
                                              divisionTemplates);

    // now that we have the templates written down, we deploy units of
    // these templates under the "divisions" key in the unitFile
    std::string totalUnits = "";
    for (auto &division : country->divisions) {
      auto tempDivision = unitBlock;
      Fwg::Parsing::Scenario::replaceOccurences(
          tempDivision, "templateDivisionName", division.name);
      Fwg::Parsing::Scenario::replaceOccurences(
          tempDivision, "templateLocation",
          std::to_string(division.location->ID + 1));
      Fwg::Parsing::Scenario::replaceOccurences(tempDivision,
                                                "templateDivisionTemplateName",
                                                division.divisionTemplate.name);
      Fwg::Parsing::Scenario::replaceOccurences(
          tempDivision, "templateStartExperience",
          std::to_string(division.startingExperienceFactor));
      Fwg::Parsing::Scenario::replaceOccurences(
          tempDivision, "templateStartEquipment",
          std::to_string(division.startingEquipmentFactor));

      totalUnits += tempDivision;
    }

    Fwg::Parsing::Scenario::replaceOccurences(unitFile, "templateUnitBlock",
                                              totalUnits);
    // units
    auto tempPath = path + country->tag + "_1936.txt";
    pU::writeFile(path + country->tag + "_1936.txt", unitFile);
    pU::writeFile(path + country->tag + "_1936_nsb.txt", unitFile);

    // ############# AIRFORCES #############
    std::string airforce = baseAirFile;
    std::string airbases = "";
    // for every airbase, for every wing, write the wing file
    for (auto &airbase : country->airBases) {
      if (airbase.second->wings.empty())
        continue;
      std::string airbaseFile = baseAirbaseFile;
      std::string wings = "";
      Fwg::Parsing::Scenario::replaceOccurences(
          airbaseFile, "templateStateID",
          std::to_string(airbase.first->ID + 1));
      for (auto &wing : airbase.second->wings) {
        std::string wingFile = baseWingFile;
        Fwg::Parsing::Scenario::replaceOccurences(wingFile, "templateName",
                                                  wing.variant.name);
        Fwg::Parsing::Scenario::replaceOccurences(
            wingFile, "templateCountryTag", country->tag);
        Fwg::Parsing::Scenario::replaceOccurences(wingFile, "templateAirFrame",
                                                  wing.variant.bbaFrameName);
        Fwg::Parsing::Scenario::replaceOccurences(wingFile, "templateAmount",
                                                  std::to_string(wing.amount));
        wings += wingFile;
      }
      Fwg::Parsing::Scenario::replaceOccurences(airbaseFile, "templateAirWings",
                                                wings);
      airbases += airbaseFile;
    }

    Fwg::Parsing::Scenario::replaceOccurences(airforce, "templateAirBases",
                                              airbases);
    tempPath = path + country->tag + "_1936_air_bba.txt";
    pU::writeFile(tempPath, airforce);
    // now replace the bba frames with vanilla frames
    tempPath = path + country->tag + "_1936_air_legacy.txt";
    for (auto &variant : country->planeVariants) {
      Fwg::Parsing::Scenario::replaceOccurences(airforce, variant.bbaFrameName,
                                                variant.vanillaFrameName);
    }
    pU::writeFile(tempPath, airforce);

    // ############# NAVIES #############
    const auto baseNavyFile =
        pU::readFile(Fwg::Cfg::Values().resourcePath +
                     "hoi4//history//navy//baseNavyFile.txt");
    const auto baseFleetFile =
        pU::readFile(Fwg::Cfg::Values().resourcePath +
                     "hoi4//history//navy//baseFleetFile.txt");
    const std::string baseShipString =
        "\t\t\tship = { name = \"templateShipName\" definition = "
        "templateShipType "
        "equipment = { templateShipEquipment = { amount = 1 owner = "
        "templateCountryTag version_name = "
        "\"templateClassName\" } }	}\n";
    const std::string mtgShipString =
        "\t\t\tship = { name = \"templateShipName\" definition = "
        "templateShipType "
        "equipment = { templateShipEquipment = { amount = 1 owner = "
        "templateCountryTag version_name = "
        "\"templateClassName\" } }	}\n";

    std::map<ShipClassType, std::string> ShipClassTypeDefinitions = {
        {ShipClassType::Destroyer, "destroyer"},
        {ShipClassType::LightCruiser, "light_cruiser"},
        {ShipClassType::HeavyCruiser, "heavy_cruiser"},
        {ShipClassType::BattleCruiser, "battle_cruiser"},
        {ShipClassType::BattleShip, "battleship"},
        {ShipClassType::Carrier, "carrier"},
        {ShipClassType::Submarine, "submarine"}};
    // for mtg
    std::map<ShipClassType, std::string> shipHullDefinitions = {
        {ShipClassType::Destroyer, "ship_hull_light_1"},
        {ShipClassType::LightCruiser, "ship_hull_light_1"},
        {ShipClassType::HeavyCruiser, "heavy_cruiser_1"},
        {ShipClassType::BattleCruiser, "heavy_cruiser_1"},
        {ShipClassType::BattleShip, "battleship_1"},
        {ShipClassType::Carrier, "carrier_1"},
        {ShipClassType::Submarine, "submarine_1"}};

    auto navyFile = baseNavyFile;
    auto mtgNavyFile = baseNavyFile;
    std::string fleets = "";
    std::string mtgFleets = "";
    for (auto &fleet : country->fleets) {
      std::string fleetString = baseFleetFile;
      std::string ships = "";
      std::string mtgShips = "";
      for (int i = 0; i < 2; i++) {
        for (auto &ship : fleet.ships) {
          auto shipString = i ? mtgShipString : baseShipString;
          Fwg::Parsing::Scenario::replaceOccurences(
              shipString, "templateShipName", ship->name);
          Fwg::Parsing::Scenario::replaceOccurences(
              shipString, "templateShipType",
              ShipClassTypeDefinitions.at(ship->shipClass.type));
          Fwg::Parsing::Scenario::replaceOccurences(
              shipString, "templateCountryTag", country->tag);
          Fwg::Parsing::Scenario::replaceOccurences(
              shipString, "templateClassName", ship->shipClass.name);

          // legacy
          if (i == 0) {
            Fwg::Parsing::Scenario::replaceOccurences(
                shipString, "templateShipEquipment",
                ship->shipClass.vanillaShipType);
            ships.append(shipString);
          }
          // mtg
          else {
            Fwg::Parsing::Scenario::replaceOccurences(
                shipString, "templateShipEquipment",
                ship->shipClass.mtgHullname);
            mtgShips.append(shipString);
          }
        }
      }
      Fwg::Parsing::Scenario::replaceOccurences(
          fleetString, "templateFleetName", fleet.name);
      Fwg::Parsing::Scenario::replaceOccurences(
          fleetString, "templateTaskForceName", fleet.name);
      Fwg::Parsing::Scenario::replaceOccurences(
          fleetString, "templateLocation",
          std::to_string(fleet.startingPort->ID + 1));
      auto mtgFleetString = fleetString;
      Fwg::Parsing::Scenario::replaceOccurences(fleetString, "templateShips",
                                                ships);
      Fwg::Parsing::Scenario::replaceOccurences(mtgFleetString, "templateShips",
                                                mtgShips);
      fleets.append(fleetString);
      mtgFleets.append(mtgFleetString);
    }
    Fwg::Parsing::Scenario::replaceOccurences(navyFile, "templateFleets",
                                              fleets);
    Fwg::Parsing::Scenario::replaceOccurences(mtgNavyFile, "templateFleets",
                                              mtgFleets);
    tempPath = path + country->tag + "_1936_naval.txt";
    pU::writeFile(tempPath, navyFile);
    tempPath = path + country->tag + "_1936_naval_mtg.txt";
    pU::writeFile(tempPath, mtgNavyFile);
  }
}
void states(const std::string &path,
            const std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: History: Drawing State Borders");
  Fwg::IO::Utils::clearFilesOfType(path, ".txt");
  auto templateContent = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                      "hoi4//history//state.txt");
  std::vector<std::string> stateCategories{
      "wasteland",  "small_island", "pastoral",   "rural",      "town",
      "large_town", "city",         "large_city", "metropolis", "megalopolis"};
  for (const auto &region : regions) {
    // skip both sea and land regions
    if (region->sea || region->lake) {
      continue;
    }
    std::string victoryPoints{""};
    std::string provString{""};
    for (const auto &prov : region->provinces) {
      provString.append(std::to_string(prov->ID + 1));
      provString.append(" ");
    }
    for (auto &vp : region->victoryPointsMap) {
      victoryPoints.append("victory_points = { " +
                           std::to_string(vp.first + 1) + " " +
                           std::to_string(vp.second.amount) + "}\n\t\t");
    }
    auto content{templateContent};
    pU::Scenario::replaceOccurences(content, "templateID",
                                    std::to_string(region->ID + 1));
    pU::Scenario::replaceOccurences(content, "template_provinces", provString);
    pU::Scenario::replaceOccurences(content, "templateVictoryPoints",
                                    victoryPoints);
    if (region->owner)
      pU::Scenario::replaceOccurences(content, "templateOwner",
                                      region->owner->tag);
    else {
      pU::Scenario::replaceOccurences(content, "owner = templateOwner", "");
      pU::Scenario::replaceOccurences(content, "add_core_of = templateOwner",
                                      "");
    }
    pU::Scenario::replaceOccurences(
        content, "templateInfrastructure",
        std::to_string(std::clamp(region->infrastructure, 1, 5)));
    pU::Scenario::replaceOccurences(
        content, "templateCivilianFactory",
        std::to_string((int)region->civilianFactories));
    pU::Scenario::replaceOccurences(content, "templateArmsFactory",
                                    std::to_string((int)region->armsFactories));

    pU::Scenario::replaceOccurences(
        content, "templatePopulation",
        std::to_string((int)region->totalPopulation));
    pU::Scenario::replaceOccurences(
        content, "templateStateCategory",
        stateCategories[(int)region->stateCategory]);
    std::string navalBaseContent = "";
    for (auto &[provID, navalBase] : region->navalBases) {
      navalBaseContent +=
          std::to_string(provID + 1) +
          " = {\n\t\t\t\tnaval_base = " + std::to_string(navalBase) +
          "\n\t\t\t}\n\t\t\t";
    }
    pU::Scenario::replaceOccurences(content, "templateNavalBases",
                                    navalBaseContent);
    if (region->dockyards > 0)
      pU::Scenario::replaceOccurences(content, "templateDockyards",
                                      std::to_string((int)region->dockyards));
    else
      pU::Scenario::replaceOccurences(content, "dockyard = templateDockyards",
                                      "");
    if (region->airBase)
      pU::Scenario::replaceOccurences(content, "templateAirBase",
                                      std::to_string(region->airBase->level));
    else
      pU::Scenario::replaceOccurences(content, "air_base = templateAirBase",
                                      "");

    // resources
    for (const auto &resource : std::vector<std::string>{
             "aluminium", "chromium", "oil", "rubber", "steel", "tungsten"}) {
      pU::Scenario::replaceOccurences(
          content, "template" + resource,
          std::to_string((int)region->resources.at(resource).amount));
    }
    pU::writeFile(path + "//" + std::to_string(region->ID + 1) + ".txt",
                  content);
  }
}

void portraits(const std::string &path, const CountryMap &countries) {
  // get all the resources from the portraits resources folder
  auto portraitsTemplate = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                        "hoi4//portraits//00_portraits.txt");

  // scientists
  std::string scientistsTemplate = "";

  const auto africanTemplate = pU::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4//portraits//templateAfrican.txt");
  const auto asianTemplate = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                          "hoi4//portraits//templateAsian.txt");
  const auto caucasianTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//portraits//templateCaucasian.txt");
  const auto arabicTemplate = pU::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4//portraits//templateArabic.txt");
  const auto southAmericanTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//portraits//templateSouthAmerican.txt");

  const auto africanScientistTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//portraits//templateAfricanScientist.txt");
  const auto asianScientistTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//portraits//templateAsianScientist.txt");
  const auto caucasianScientistTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//portraits//templateCaucasianScientist.txt");
  const auto arabicScientistTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//portraits//templateArabicScientist.txt");
  const auto southAmericanScientistTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//portraits//templateSouthAmericanScientist.txt");

  for (auto &country : countries) {
    if (!country->ownedRegions.size())
      continue;
    auto culture = country->getPrimaryCulture();
    std::string portraitTemplate;
    std::string scientistTemplate;
    if (culture->visualType == VisualType::AFRICAN) {
      portraitTemplate = africanTemplate;
      scientistTemplate = africanScientistTemplate;
    } else if (culture->visualType == VisualType::ASIAN) {
      portraitTemplate = asianTemplate;
      scientistTemplate = asianScientistTemplate;
    } else if (culture->visualType == VisualType::CAUCASIAN) {
      portraitTemplate = caucasianTemplate;
      scientistTemplate = caucasianScientistTemplate;
    } else if (culture->visualType == VisualType::ARABIC) {
      portraitTemplate = arabicTemplate;
      scientistTemplate = arabicScientistTemplate;
    } else if (culture->visualType == VisualType::SOUTH_AMERICAN) {
      portraitTemplate = southAmericanTemplate;
      scientistTemplate = southAmericanScientistTemplate;
    }
    // replace the tag
    pU::Scenario::replaceOccurences(portraitTemplate, "templateTag",
                                    country->tag);
    pU::Scenario::replaceOccurences(scientistTemplate, "templateTag",
                                    country->tag);
    // attach to templates
    portraitsTemplate.append(portraitTemplate);
    scientistsTemplate.append(scientistTemplate);
  }
  pU::writeFile(path + "00_portraits.txt", portraitsTemplate);
  pU::writeFile(path + "998_scientist_portraits.txt", scientistsTemplate);
}

} // namespace Countries
void aiStrategy(const std::string &path) {
  // copy folders ai_areas and ai_strategy from cfg::Values().resourcePath +
  // "/hoi4//common// to path//common//
  Logging::logLine("HOI4 Parser: Map: Writing AI Strategies");

  std::filesystem::path sourceAiAreas =
      Fwg::Cfg::Values().resourcePath + "hoi4//common//ai_areas";
  std::filesystem::path sourceAiStrategy =
      Fwg::Cfg::Values().resourcePath + "hoi4//common//ai_strategy";

  try {

    // Copy ai_areas folder
    std::filesystem::copy(sourceAiAreas, path + "//ai_areas",
                          std::filesystem::copy_options::recursive);

    // Copy ai_strategy folder
    std::filesystem::copy(sourceAiStrategy, path + "//ai_strategy",
                          std::filesystem::copy_options::recursive);
  } catch (const std::filesystem::filesystem_error &e) {
    Logging::logLine("HOI4 Parser: Error copying AI Strategies: " +
                     std::string(e.what()));
  }
}

void events(const std::string &path) {

  Logging::logLine("HOI4 Parser: Map: Writing events");
  std::filesystem::path sourceEvents =
      Fwg::Cfg::Values().resourcePath + "hoi4//events";
  try {

    // Copy ai_areas folder
    std::filesystem::copy(sourceEvents, path + "//events",
                          std::filesystem::copy_options::recursive);
  } catch (const std::filesystem::filesystem_error &e) {
    Logging::logLine("HOI4 Parser: Error copying Events: " +
                     std::string(e.what()));
  }
}

void commonBookmarks(const std::string &path, const CountryMap &countries,
                     const std::map<int, std::vector<std::shared_ptr<Country>>>
                         &strengthScores) {
  auto bookmarkTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//common//bookmarks//the_gathering_storm.txt");
  int count = 0;
  const auto majorTemplate =
      pU::Scenario::getBracketBlock(bookmarkTemplate, "templateMajorTAG") +
      "\n\t\t";
  const auto minorTemplate =
      pU::Scenario::getBracketBlock(bookmarkTemplate, "templateMinorTAG") +
      "\n\t\t";
  pU::Scenario::removeBracketBlockFromKey(bookmarkTemplate, "templateMajorTAG");
  pU::Scenario::removeBracketBlockFromBracket(bookmarkTemplate,
                                              "templateMinorTAG");
  std::string bookmarkCountries{""};
  if (strengthScores.size()) {

    for (auto iter = strengthScores.rbegin(); iter != strengthScores.rend();
         ++iter) {
      if (count < 7) {
        // major power:
        for (const auto &country : iter->second) {
          // reinterpret this country as a Hoi4Country
          auto hoi4Country = std::dynamic_pointer_cast<Hoi4Country>(country);
          auto majorString{majorTemplate};
          pU::Scenario::replaceOccurences(majorString, "templateIdeology",
                                          hoi4Country->ideology);
          bookmarkCountries.append(pU::Scenario::replaceOccurences(
              majorString, "templateMajorTAG", hoi4Country->tag));
          count++;
        }
      } else if (count < 14) {
        // regional power:
        for (const auto &country : iter->second) {
          auto hoi4Country = std::dynamic_pointer_cast<Hoi4Country>(country);
          auto minorString{minorTemplate};
          pU::Scenario::replaceOccurences(minorString, "templateIdeology",
                                          hoi4Country->ideology);
          bookmarkCountries.append(pU::Scenario::replaceOccurences(
              minorString, "templateMinorTAG", hoi4Country->tag));
          count++;
        }
      }
    }
  }
  pU::Scenario::replaceOccurences(bookmarkTemplate,
                                  "templateMinorTAG=", bookmarkCountries);
  pU::writeFile(path + "the_gathering_storm.txt", bookmarkTemplate);
}

void scriptedTriggers(std::string gamePath, std::string modPath) {
  Fwg::Utils::Logging::logLine("HOI4 Parser: Scripted Triggers: Copying Files");
  // copy files from gamePath to modPath
  std::vector<std::string> filenames{"00_diplo_action_valid_triggers.txt",
                                     "00_resistance_initiate_triggers.txt",
                                     "00_scripted_triggers.txt",
                                     "debug_triggers.txt",
                                     "diplomacy_scripted_triggers.txt",
                                     "Elections_scripted_triggers.txt",
                                     "ideology_scripted_triggers.txt",
                                     "laws_war_support.txt",
                                     "unit_medals_scripted_triggers.txt"};
  for (const auto &filename : filenames) {
    std::filesystem::copy(gamePath + filename, modPath + filename,
                          std::filesystem::copy_options::overwrite_existing);
  }
}
// filter out simple to filter blocks from the common folder, removing
// potential error sources from vanilla countries
void commonFiltering(const std::string &gamePath, const std::string &modPath) {
  Logging::logLine("HOI4 Parser: Common: Filtering Files");
  std::vector<std::string> filenames{
      "//common//scripted_triggers//00_scripted_triggers.txt"};
  for (const auto &filename : filenames) {
    auto content = pU::readFile(gamePath + filename);
    auto blocks =
        pU::Scenario::getOuterBlocks(pU::getLines(gamePath + filename));
    for (const auto &block : blocks) {
      // if (block.content.contains("JAP")) {
      //   pU::Scenario::removeSurroundingBracketBlockFromLineBreak(content,
      //                                                            block.content);
      //   std::cout << "Removing: " << block.content << std::endl;
      // }
    }
    pU::writeFile(modPath + filename, content);
  }
}

void tutorials(const std::string &path) {
  pU::writeFile(path, "tutorial = { }");
}

void compatibilityHistory(const std::string &path, const std::string &hoiPath,
                          const std::vector<Fwg::Region> &regions) {
  Logging::logLine("HOI4 Parser: History: Writing Compatibility Files");
  const std::filesystem::path hoiDir{hoiPath + "//history//countries//"};
  Logging::logLine("HOI4 Parser: History: Reading Files from " +
                   hoiDir.string());
  const std::filesystem::path modDir{path};
  for (auto const &dir_entry : std::filesystem::directory_iterator{hoiDir}) {
    std::string pathString = dir_entry.path().string();
    std::string filename = dir_entry.path().filename().string();
    if (filename[0] == '.')
      continue;
    auto content = pU::readFile(pathString);
    while (content.find("start_resistance = yes") != std::string::npos) {
      pU::Scenario::removeSurroundingBracketBlockFromLineBreak(
          content, "start_resistance = yes");
    }
    pU::Scenario::replaceLine(content,
                              "capital =", "capital = " + std::to_string(1));
    pU::Scenario::replaceLine(content,
                              "SWI_find_biggest_fascist_neighbor = yes", "");
    auto blocks = pU::Scenario::getOuterBlocks(pU::getLines(pathString));
    for (auto &block : blocks) {
      if (block.content.contains("declare_war_on")) {
        pU::Scenario::removeSurroundingBracketBlockFromLineBreak(content,
                                                                 block.content);
      }
      if (block.content.contains("random_list")) {
        pU::Scenario::removeSurroundingBracketBlockFromLineBreak(content,
                                                                 block.content);
      }
    }
    std::smatch m;
    do {
      if (std::regex_search(
              content, m,
              std::regex("\\s([1-9]|[1-9][0-9]|[1-9][0-9][0-9]|[1-9][0-9][0-9]["
                         "0-9])\\s?\\s?=\\s?\\s?\\{")))
        pU::Scenario::removeBracketBlockFromKey(content, m[0]);
    } while (m.size());
    // remove tokens that crash the mod, as in country history states are
    // referenced by IDs. If there is no state with such an ID in game,
    // the game crashes otherwise
    auto lines = pU::getTokens(content, '\n');
    for (auto &line : lines) {
      auto tokens = pU::getTokens(line, '=');
      if (tokens.size()) {
        pU::Scenario::removeCharacter(tokens[0], ' ');
        if (Fwg::Utils::isInt(tokens[0])) {
          auto tokenRemove = tokens[0];
          pU::Scenario::removeBracketBlockFromKey(content, tokenRemove);
        }
      }
    }

    pU::writeFile(path + filename, content);
  }
}
void copyDescriptorFile(const std::string &sourcePath,
                        const std::string &destPath,
                        const std::string &modsDirectory,
                        const std::string &modName) {
  auto descriptorText = pU::readFile(sourcePath);
  pU::Scenario::replaceOccurences(descriptorText, "templateName", modName);
  auto modText{descriptorText};
  pU::Scenario::replaceOccurences(descriptorText, "templatePath", "");
  pU::writeFile(destPath + "//descriptor.mod", descriptorText);
  pU::Scenario::replaceOccurences(
      modText, "templatePath",
      Fwg::Utils::varsToString("path=\"", destPath, "\""));
  pU::writeFile(modsDirectory + "//" + modName + ".mod", modText);
}

namespace Localisation {

void countryNames(const std::string &path, const CountryMap &countries,
                  const NameGeneration::NameData &nData) {
  Logging::logLine("HOI4 Parser: Localisation: Writing Country Names");
  std::string content = "l_english:\n";
  std::vector<std::string> ideologies{"fascism", "communism", "neutrality",
                                      "democratic"};

  for (const auto &country : countries) {
    for (const auto &ideology : ideologies) {
      auto ideologyName = NameGeneration::modifyWithIdeology(
          ideology, country->name, country->adjective, nData);
      content +=
          " " + country->tag + "_" + ideology + ":0 \"" + ideologyName + "\"\n";
      content += " " + country->tag + "_" + ideology + "_DEF:0 \"" +
                 ideologyName + "\"\n";
      ;
      content += " " + country->tag + "_" + ideology + "_ADJ:0 \"" +
                 country->adjective + "\"\n";
      ;
    }
  }
  pU::writeFile(path + "countries_l_english.yml", content, true);
}

void stateNames(const std::string &path, const CountryMap &countries) {
  Logging::logLine("HOI4 Parser: Localisation: Writing State Names");
  std::string content = "l_english:\n";

  for (const auto &country : countries) {
    for (const auto &region : country->hoi4Regions)
      content += " STATE_" + std::to_string(region->ID + 1) + ":0 \"" +
                 region->name + "\"\n";
  }
  pU::writeFile(path + "state_names_l_english.yml", content, true);
}

void strategicRegionNames(
    const std::string &path,
    const std::vector<StrategicRegion> &strategicRegions) {
  Logging::logLine("HOI4 Parser: Map: Naming the Regions");
  std::string content = "l_english:\n";
  for (auto i = 0; i < strategicRegions.size(); i++) {
    content += Fwg::Utils::varsToString(" STRATEGICREGION_", i, ":0 \"",
                                        strategicRegions[i].name, "\"\n");
  }
  pU::writeFile(path + "//strategic_region_names_l_english.yml", content, true);
}

void victoryPointNames(const std::string &path,
                       const std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: Map: Naming the Regions");
  std::string content = "l_english:\n";
  for (auto region : regions) {
    for (auto vp : region->victoryPointsMap) {
      content += Fwg::Utils::varsToString(" VICTORY_POINTS_", vp.first + 1,
                                          ":0 \"", vp.second.name, "\"\n");
    }
  }
  pU::writeFile(path + "//victory_points_l_english.yml", content, true);
}

} // namespace Localisation

} // namespace Writing

namespace Reading {
// reads a text file containing colour->tag relations
// reads a bmp containing colours
Fwg::Utils::ColourTMap<std::string> readColourMapping(const std::string &path) {
  Fwg::Utils::ColourTMap<std::string> colourMap;
  auto mappings = pU::readFile(path + "//common/countries/colors.txt");
  std::string countryColour;
  do {
    countryColour = pU::Scenario::removeSurroundingBracketBlockFromLineBreak(
        mappings, "color =");
    if (countryColour.size() > 10) {
      auto tag = countryColour.substr(1, 3);
      auto colourString = pU::getValue(countryColour, "color_ui");
      auto hsv = pU::Scenario::getBracketBlockContent(colourString, "hsv");
      std::vector<int> rgb(3);
      if (colourString.find("rgb") != std::string::npos) {
        rgb = pU::Scenario::getNumberBlock(colourString, "rgb");
      } else if (hsv.size()) {
        auto hsvd = pU::getTokens(hsv, ' ');
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
void readStates(const std::string &path, std::shared_ptr<Generator> &hoi4Gen) {
  using namespace Fwg::Parsing::Scenario;
  auto states = pU::readFilesInDirectory(path + "/history/states");

  Fwg::Utils::ColourTMap<Fwg::Region> stateColours;
  hoi4Gen->gameRegions.clear();
  hoi4Gen->countries.clear();
  stateColours.clear();

  for (auto &state : states) {
    Fwg::Region reg;
    auto tag = pU::getValue(state, "owner");
    reg.ID = std::stoi(pU::getValue(state, "id")) - 1;
    removeCharacter(tag, ' ');
    // reg->owner = tag;
    auto readIDs = getNumberBlockMultiDelim(state, "provinces");
    for (auto id : readIDs) {
      // reg->gameProvinces.push_back(hoi4Gen.gameProvinces[id - 1]);
      reg.provinces.push_back(hoi4Gen->gameProvinces[id - 1]->baseProvince);
      // hoi4Gen.gameProvinces[id - 1]->baseProvince->regionID = reg.ID;
    }

    Fwg::Gfx::Colour colour;
    // pick a random, but unique colour
    do {
      colour.randomize();
    } while (stateColours.find(colour));
    reg.colour = colour;
    // hoi4Gen.gameRegions.push_back(reg);
    stateColours.setValue(reg.colour, reg);
    hoi4Gen->areas.regions.push_back(reg);
  }

  std::sort(hoi4Gen->areas.regions.begin(), hoi4Gen->areas.regions.end(),
            [](auto l, auto r) { return l < r; });
  Fwg::Gfx::regionMap(hoi4Gen->areas.regions, hoi4Gen->areas.provinces,
                      hoi4Gen->regionMap);
  // for (auto &region : hoi4Gen.gameRegions) {
  //   if (hoi4Gen.countries.find(region->owner) != hoi4Gen.countries.end())
  //   {
  //     hoi4Gen.countries.at(region->owner)->ownedRegions.push_back(region);
  //   } else {
  //     Country c;
  //     c.tag = region->owner;
  //     c.ownedRegions.push_back(region);
  //     hoi4Gen.countries.insert({c.tag, std::make_shared<Country>(c)});
  //   }
  // }
  // Fwg::Gfx::Bitmap regionMap(5632, 2048, 24);
  // Fwg::Gfx::regionMap(hoi4Gen->areas.regions, hoi4Gen->areas.provinces,
  //                    regionMap);
  // Fwg::Gfx::Bmp::save(regionMap, path + "/map/regions.bmp");
}
// get the bmp file info and extract the respective IDs from definition.csv
std::vector<Fwg::Province> readProvinceMap(const std::string &path) {
  using namespace Fwg::Parsing::Scenario;
  auto &cfg = Fwg::Cfg::Values();
  auto provMap =
      Fwg::IO::Reader::readGenericImage(path + "map/provinces.bmp", cfg);
  auto definition = pU::getLines(path + "map/definition.csv");
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
  auto list = pU::getLines(path + "//map//airports.txt");
  for (const auto &entry : list) {
    auto tokens = pU::getTokens(entry, '=');
    if (tokens.size() == 2) {
      auto ID = std::stoi(tokens[0]);
      // ignore all entries if the region doesn't exist
      if (ID < regions.size() + 1) {
        Fwg::Parsing::Scenario::removeSpecials(tokens[1]);
        auto provID = std::stoi(tokens[1]);
        regions[ID - 1]->airport = provID - 1;
      }
    }
  }
}

void readBuildings(const std::string &path,
                   std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: Map: Observing Infrastructure");
  auto content = pU::getLines(path + "//map//buildings.txt");
  for (const auto &line : content) {
    Scenario::Utils::Building building;
    auto tokens = pU::getTokens(line, ';');
    auto stateID = std::stoi(tokens[0]) - 1;

    if (stateID < regions.size()) {
      building.name = tokens[1];
      building.relativeID = std::stoi(tokens[6]);
      building.position = Scenario::Utils::strToPos(tokens, {2, 3, 4, 5});
      regions[stateID]->buildings.push_back(building);
    }
  }
}

std::vector<std::shared_ptr<Hoi4Country>>
readCountries(const std::string &path) {
  std::vector<std::shared_ptr<Hoi4Country>> countries;
  auto countryList =
      pU::getLines(path + "//common//country_tags//00_countries.txt");
  auto bList = pU::getLines(path + "//common//country_tags//01_countries.txt");
  countryList.insert(countryList.end(), bList.begin(), bList.end());
  for (auto &line : countryList) {
    if (line.size() > 3) {
      auto tag = line.substr(0, 3);
      auto name = pU::getValue(line, "=");
      // Hoi4Country hc;
      // hc.tag = tag;
      // hc.name = name;
    }
  }

  return countries;
}

std::vector<std::vector<std::string>> readDefinitions(const std::string &path) {
  auto list = pU::getLinesByID(path);
  return list;
}
void readProvinces(ClimateGeneration::ClimateData &climateData,
                   const std::string &inPath, const std::string &mapName,
                   Fwg::Areas::AreaData &areaData) {
  Logging::logLine("HOI4 Parser: Map: Studying the land");
  auto provMap = Fwg::IO::Reader::readGenericImage(inPath + "map//" + mapName,
                                                   Fwg::Cfg::Values());
  auto heightMap = Fwg::IO::Reader::readGenericImage(
      inPath + "map//" + "heightmap" + ".bmp", Fwg::Cfg::Values());
  auto list = readDefinitions(inPath + "map//definition.csv");
  // now map definitions to read in IDs
  for (auto &line : list) {
    if (line.size()) {
      auto tokens = pU::getTokens(line[0], ';');
      auto ID = std::stoi(tokens[0]) - 1;
      if (ID == -1)
        continue;
      auto r = static_cast<unsigned char>(std::stoi(tokens[1]));
      auto g = static_cast<unsigned char>(std::stoi(tokens[2]));
      auto b = static_cast<unsigned char>(std::stoi(tokens[3]));
      std::shared_ptr<Fwg::Province> p = std::make_shared<Fwg::Province>();
      p->ID = ID;
      p->colour = {r, g, b};
      p->isLake = tokens[4] == "lake";
      if (p->isLake) {
        p->coastal = false;
        p->sea = false;
      }
      p->continentID = stoi(tokens[7]) - 1;
      areaData.provinceColourMap.setValue(p->colour, p);
      areaData.provinces.push_back(p);
    }
  }
  // call it with special idsort bool to make sure we sort by ID only this
  // time
  Fwg::Areas::Provinces::readProvinceBMP(
      climateData, provMap, heightMap, areaData.provinces,
      areaData.provinceColourMap, areaData.segments, true);
}
void readRocketSites(const std::string &path,
                     std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: Map: Scanning for rockets");
  auto list = pU::getLines(path + "//map//rocketsites.txt");
  for (const auto &entry : list) {
    auto tokens = pU::getTokens(entry, '=');
    if (tokens.size() == 2) {
      auto ID = std::stoi(tokens[0]);
      if (ID < regions.size() + 1) {
        Fwg::Parsing::Scenario::removeSpecials(tokens[1]);
        auto provID = std::stoi(tokens[1]);
        regions[ID - 1]->rocketsite = provID - 1;
      }
    }
  }
}
void readSupplyNodes(const std::string &path,
                     std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: Map: Stealing from logistics hub");
  auto list = pU::getLines(path + "//map//supply_nodes.txt");
  for (const auto &entry : list) {
    auto tokens = pU::getTokens(entry, '=');
    if (tokens.size() == 2) {
      auto ID = std::stoi(tokens[0]);
      if (ID < regions.size() + 1) {
        Fwg::Parsing::Scenario::removeSpecials(tokens[1]);
        auto provID = std::stoi(tokens[1]);
        regions[ID - 1]->supplyNode = provID - 1;
      }
    }
  }
}
void readWeatherPositions(const std::string &path,
                          std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: Map: Observing the Weather");
  auto content = pU::getLines(path + "//map//weatherpositions.txt");
  for (const auto &line : content) {
    Scenario::Utils::WeatherPosition weather;
    auto tokens = pU::getTokens(line, ';');
    weather.position = Scenario::Utils::strToPos(tokens, {1, 2, 3, 3});
    weather.effectSize = tokens[4];
    auto stateID = std::stoi(tokens[0]) - 1;
    if (stateID < regions.size())
      regions[stateID]->weatherPosition = weather;
  }
}
} // namespace Reading

std::vector<std::string> readTypeMap() {
  return pU::getLines(Fwg::Cfg::Values().resourcePath +
                      "hoi4//ai//national_focus//baseFiles//foci.txt");
}

std::map<std::string, std::string> readRewardMap(const std::string &path) {
  auto file = pU::readFile(path);
  auto split = pU::getTokens(file, ';');
  std::map<std::string, std::string> rewardMap;
  for (const auto &elem : split) {
    auto key = Fwg::Parsing::Scenario::getBracketBlockContent(elem, "key");
    auto value = Fwg::Parsing::Scenario::getBracketBlockContent(elem, "value");
    rewardMap[key] = value;
  }
  return {rewardMap};
}

} // namespace Scenario::Hoi4::Parsing