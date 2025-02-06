#include "hoi4/Hoi4Parsing.h"
#include "hoi4/NationalFocus.h"
using namespace Fwg;
namespace Logging = Fwg::Utils::Logging;
namespace pU = Fwg::Parsing;
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
    if (region.sea || region.lake)
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

void aiStrategy(const std::string &path, const CountryMap &countries) {
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

void events(const std::string &path, const CountryMap &countries) {

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
                const std::vector<Continent> &continents) {
  Logging::logLine("HOI4 Parser: Map: Writing Continents");
  // copy continents file from cfg::Values().resourcePath + "/hoi4//map// to
  // path//map//
  std::filesystem::path source =
      Fwg::Cfg::Values().resourcePath + "hoi4//map//continent.txt";
  try {
    // Copy continents file
    std::filesystem::copy(source, path);
  } catch (const std::filesystem::filesystem_error &e) {
    Logging::logLine("HOI4 Parser: Error copying Continents: " +
                     std::string(e.what()));
  }
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
    if (region.sea || region.lake)
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

void adjacencyRules(const std::string &path) {
  Logging::logLine("HOI4 Parser: Map: Writing Adjacency Rules");
  std::string content{""};
  // empty for now
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

void states(const std::string &path,
            const std::vector<std::shared_ptr<Region>> &regions) {
  Logging::logLine("HOI4 Parser: History: Drawing State Borders");
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
    pU::Scenario::replaceOccurences(content, "templateAirbase",
                                    std::to_string(0));
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

    // resources
    for (const auto &resource : std::vector<std::string>{
             "aluminium", "chromium", "oil", "rubber", "steel", "tungsten"}) {
      pU::Scenario::replaceOccurences(
          content, "template" + resource,
          std::to_string((int)region->resources.at(resource)));
    }
    pU::writeFile(path + "//" + std::to_string(region->ID + 1) + ".txt",
                  content);
  }
}
void flags(const std::string &path, const CountryMap &countries) {
  Logging::logLine("HOI4 Parser: Gfx: Printing Flags");
  using namespace Gfx::Textures;
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

void historyCountries(const std::string &path, const CountryMap &countries) {
  Logging::logLine("HOI4 Parser: History: Writing Country History");
  const auto content = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                    "hoi4//history//country_template.txt");

  const auto navyTemplateFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//history//navy//baseVariantFile.txt");
  const auto navyTechFile = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                         "hoi4//history//navy//navyTechs.txt");
  for (const auto &country : countries) {
    auto tempPath = path + country->tag + " - " + country->name + ".txt";
    auto countryText{content};

    pU::Scenario::replaceOccurences(
        countryText, "templateCapital",
        std::to_string(country->capitalRegionID + 1));

    pU::Scenario::replaceOccurences(countryText, "templateResearchSlots",
                                    std::to_string(country->researchSlots));
    pU::Scenario::replaceOccurences(countryText, "templateConvoys",
                                    std::to_string(country->convoyAmount));

    pU::Scenario::replaceOccurences(countryText, "templateStability",
                                    std::to_string(country->stability));
    pU::Scenario::replaceOccurences(countryText, "templateWarSupport",
                                    std::to_string(country->warSupport));

    pU::Scenario::replaceOccurences(countryText, "templateNavalBlock",
                                    navyTemplateFile);
    pU::Scenario::replaceOccurences(countryText, "templateTag", country->tag);
    pU::Scenario::replaceOccurences(countryText, "templateParty",
                                    country->ideology);

    std::string electAllowed = country->allowElections ? "yes" : "no";
    pU::Scenario::replaceOccurences(countryText, "templateAllowElections",
                                    electAllowed);
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
        vanillaVariant += "\t\t\t\tship_reliability_upgrade = 1\n";
        vanillaVariant += "\t\t\t\t\ship_engine_upgrade = 1\n";
        vanillaVariant += "\t\t\t\t\ship_gun_upgrade = 1\n";
        vanillaVariant +=
            "\t\t\t\t\ship_anti_air_upgrade = 1\n\t\t\t}\n\t\t}\n";

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

    pU::writeFile(tempPath, countryText);
  }
}
void historyUnits(const std::string &path, const CountryMap &countries) {
  Logging::logLine("HOI4 Parser: History: Deploying the Troops");
  const auto defaultTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "hoi4//history//default_unit_template.txt");
  const auto unitBlock = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                      "hoi4//history//unit_block.txt");

  const auto unitTemplateFile = pU::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4//history//divisionTemplates.txt");
  // now tokenize by : character to get single
  const auto unitTemplates = pU::getTokens(unitTemplateFile, ':');

  // auto IDMapFile =
  // pU::getLines(Fwg::Cfg::Values().resourcePath +
  // "hoi4//history//divisionIDMapper.txt");
  std::map<int, std::string> IDMap;
  /*for (const auto& line : IDMapFile) {
          if (line.size()) {
                  auto lineTokens = pU::getTokens(line, ';');
                  IDMap[stoi(lineTokens[0])] = lineTokens[1];
          }
  }*/
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
    for (const auto ID : country->units) {
      divisionTemplates.append(unitTemplates[ID]);
      // we need to buffer the names of the templates for use in later
      // unit generationm
      auto requirements = Fwg::Parsing::Scenario::getBracketBlockContent(
          unitTemplates[ID], "requirements");
      auto value = Fwg::Parsing::Scenario::getBracketBlockContent(
          requirements, "templateName");
      IDMap[ID] = value;
      // remove requirements line
      Fwg::Parsing::Scenario::replaceLine(divisionTemplates, "requirements",
                                          "");
    }
    Fwg::Parsing::Scenario::replaceOccurences(unitFile, "templateTemplateBlock",
                                              divisionTemplates);

    // now that we have the templates written down, we deploy units of
    // these templates under the "divisions" key in the unitFile
    std::string totalUnits = "";
    // for every entry in unitCount vector
    for (int i = 0; i < country->unitCount.size(); i++) {
      // run unit generation ("unitCount")[i] times
      for (int x = 0; x < country->unitCount[i]; x++) {
        // copy the template unit file
        auto tempUnit{unitBlock};
        // replace division name with the generic division name
        Fwg::Parsing::Scenario::replaceOccurences(
            tempUnit, "templateDivisionName", "\"" + IDMap.at(i) + "\"");
        // now deploy the unit in a random province
        Fwg::Parsing::Scenario::replaceOccurences(
            tempUnit, "templateLocation",
            std::to_string(Fwg::Utils::selectRandom(allowedProvinces) + 1));
        totalUnits += tempUnit;
      }
    }

    // for (int i = 0; i < country->attributeVectors.at("units").size();
    // i++) {
    //
    //	for (int x = 0; x <
    // country->attributeVectors.at("unitCount")[i];
    // x++) {
    // Logging::logLine(country->attributeVectors.at("units")[i]);
    // auto
    // tempUnit{ unitBlock };
    // Fwg::Parsing::Scenario::replaceOccurences(tempUnit,
    //"templateDivisionName", IDMap.at(i));
    // Logging::logLine(IDMap.at(i));
    // Fwg::Parsing::Scenario::replaceOccurences(tempUnit,
    //"templateLocation",
    // std::to_string(country->ownedRegions[0].gameProvinces[0].ID +
    // 1));
    //		totalUnits += tempUnit;
    //	}
    //}
    Fwg::Parsing::Scenario::replaceOccurences(unitFile, "templateUnitBlock",
                                              totalUnits);
    // units
    auto tempPath = path + country->tag + "_1936.txt";
    pU::writeFile(path + country->tag + "_1936.txt", unitFile);
    pU::writeFile(path + country->tag + "_1936_nsb.txt", unitFile);

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

void commonCountries(const std::string &path, const std::string &hoiPath,
                     const CountryMap &countries) {
  Logging::logLine("HOI4 Parser: Common: Writing Countries");
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

void tutorials(const std::string &path) {
  pU::writeFile(path, "tutorial = { }");
}

void foci(const std::string &path, const CountryMap &countries,
          const NameGeneration::NameData &nData) {
  Logging::logLine("HOI4 Parser: History: Demanding Danzig");
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
  for (const auto &country : countries) {

    pU::writeFile(path + country->name + ".txt", country->ideas);
  }
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

void portraits(const std::string &path, const CountryMap &countries) {
  // get all the resources from the portraits resources folder
  auto portraitsTemplate = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                        "hoi4//portraits//00_portraits.txt");

  // scientists
  std::string scientistTemplate = "";

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
    scientistTemplate.append(scientistTemplate);
  }
  pU::writeFile(path + "00_portraits.txt", portraitsTemplate);
  pU::writeFile(path + "998_scientist_portraits.txt", scientistTemplate);
}
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
      Fwg::Province *p = new Fwg::Province();
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

} // namespace Scenario::Hoi4::Parsing