#include "vic3/Vic3Parsing.h"
using namespace Fwg;
namespace Logging = Fwg::Utils::Logging;
namespace pU = Fwg::Parsing;
namespace Rpx::Vic3::Parsing {
namespace Writing {
void adj(const std::string &path) {
  Logging::logLine("Vic3 Parser: Map: Writing Adjacencies");
  // empty file for now
  std::string content;
  content.append(
      "#From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_"
      "rule_name;Comment\n");
  pU::writeFile(path, content);
}

void defaultMap(const std::string &path,
                const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  auto templateContent = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                      "vic3//map_data//default.map");
  Logging::logLine("Vic3 Parser: Map: Default Map");
  std::string content = templateContent;
  std::string seaStarts{""};
  std::string lakes{""};
  auto seaFormatCounter = 1;
  auto lakeFormatCounter = 1;
  for (const auto &province : provinces) {
    if (province->isSea()) {
      seaStarts.append(province->toHexString(true, true) + " ");
      if (seaFormatCounter++ % 5 == 0) {
        seaStarts.append("\n\t");
      }
    } else if (province->isLake()) {
      lakes.append(province->toHexString(true, true) + " ");
      if (lakeFormatCounter++ % 5 == 0) {
        lakes.append("\n\t");
      }
    }
  }
  Rpx::Parsing::replaceOccurences(content, "TEMPLATE_SEA_STARTS", seaStarts);
  Rpx::Parsing::replaceOccurences(content, "TEMPLATE_LAKES", lakes);
  pU::writeFile(path, content);
}

void defines(const std::string &pathOut) {
  const auto &cfg = Cfg::Values();
  auto templateContent = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                      "vic3//common//defines//01_defines.txt");
  Rpx::Parsing::replaceOccurences(templateContent, "template_mapX",
                                  std::to_string(cfg.width));
  Rpx::Parsing::replaceOccurences(templateContent, "template_mapY",
                                  std::to_string(cfg.height));
  pU::writeFile(pathOut, templateContent);
}

void heightmap(const std::string &path, const Fwg::Gfx::Bitmap &heightMap,
               const Fwg::Gfx::Bitmap &packedHeightmap) {
  auto content = pU::readFile(Fwg::Cfg::Values().resourcePath +
                              "vic3//map_data//heightmap.heightmap");
  Logging::logLine("Vic3 Parser: Map: Writing heightmap.heightmap");
  Rpx::Parsing::replaceOccurences(content, "template_mapX",
                                  std::to_string(heightMap.width()));
  Rpx::Parsing::replaceOccurences(content, "template_mapY",
                                  std::to_string(heightMap.height()));
  Rpx::Parsing::replaceOccurences(content, "template_packedX",
                                  std::to_string(packedHeightmap.height() - 5));
  pU::writeFile(path, content);
}

void stateFiles(const std::string &path,
                const std::vector<std::shared_ptr<Region>> &regions) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: History: Writing state files");

  const auto templateFile = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                         "vic3//map_data//state_template.txt");
  const auto seaTemplateFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//map_data//sea_state_template.txt");
  std::string file = "";
  for (const auto &region : regions) {
    auto content = region->isSea() ? seaTemplateFile : templateFile;
    Rpx::Parsing::replaceOccurences(content, "template_name", region->name);
    Rpx::Parsing::replaceOccurences(content, "template_id",
                                    std::to_string(region->ID + 1));
    std::string provinceString{""};
    for (auto prov : region->ardaProvinces) {
      provinceString.append("\"" + prov->toHexString(true, true) + "\" ");
    }
    Rpx::Parsing::replaceOccurences(content, "template_provinces",
                                    provinceString);
    // don't write these details for ocean regions
    if (!region->isSea()) {
      int counter = 0;
      Rpx::Parsing::replaceOccurences(
          content, "template_city",
          region
              ->ardaProvinces[std::clamp(counter++, 0,
                                         (int)region->provinces.size() - 1)]
              ->toHexString(true, true));
      Rpx::Parsing::replaceOccurences(
          content, "template_port",
          region
              ->ardaProvinces[std::clamp(counter++, 0,
                                         (int)region->provinces.size() - 1)]
              ->toHexString(true, true));
      Rpx::Parsing::replaceOccurences(
          content, "template_farm",
          region
              ->ardaProvinces[std::clamp(counter++, 0,
                                         (int)region->provinces.size() - 1)]
              ->toHexString(true, true));
      Rpx::Parsing::replaceOccurences(
          content, "template_mine",
          region
              ->ardaProvinces[std::clamp(counter++, 0,
                                         (int)region->provinces.size() - 1)]
              ->toHexString(true, true));
      Rpx::Parsing::replaceOccurences(
          content, "template_wood",
          region
              ->ardaProvinces[std::clamp(counter++, 0,
                                         (int)region->provinces.size() - 1)]
              ->toHexString(true, true));

      // check if we are a coastal region
      for (auto &prov : region->ardaProvinces) {
        if (prov->isSea()) {
          Rpx::Parsing::replaceOccurences(content, "template_naval_exit",
                                          prov->toHexString(true, true));
        }
      }
      if (region->navalExit != -1) {
        Rpx::Parsing::replaceOccurences(content, "template_naval_exit",
                                        "naval_exit_id = " +
                                            std::to_string(region->navalExit));
      } else {
        Rpx::Parsing::replaceOccurences(content, "template_naval_exit", "");
      }

      std::string agriResString = "";
      std::string cappedResString = "";
      for (auto &res : region->resources) {
        if (res.second.amount > 1.0) {
          if (res.second.capped) {
            cappedResString.append(
                res.first + " = " +
                std::to_string(static_cast<int>(res.second.amount)));
            cappedResString.append("\n\t\t");
          } else {
            agriResString.append("\"" + res.first + +"\"");
            agriResString.append(" ");
          }
        }
      }
      Rpx::Parsing::replaceOccurences(content, "template_arable_resources",
                                      agriResString);
      Rpx::Parsing::replaceOccurences(content, "template_capped_resources",
                                      cappedResString);
      Rpx::Parsing::replaceOccurences(
          content, "template_arable_land",
          std::to_string(static_cast<int>(region->arableLand)));
    } else {
      Rpx::Parsing::replaceOccurences(content, "template_city", "");
      Rpx::Parsing::replaceOccurences(content, "template_port", "");
      Rpx::Parsing::replaceOccurences(content, "template_farm", "");
      Rpx::Parsing::replaceOccurences(content, "template_mine", "");
      Rpx::Parsing::replaceOccurences(content, "template_wood", "");
      Rpx::Parsing::replaceOccurences(content, "template_naval_exit", "");
    }

    file.append(content);
  }
  pU::writeFile(path, file, true);
}

void provinceTerrains(
    const std::string &path,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: Map: Writing Areas::Province Terrains");
  std::string content{""};
  for (const auto &province : provinces) {
    content.append(province->toHexString(true, true));
    content.append("=\"");
    std::string terraintype;
    if (province->isSea())
      terraintype = "ocean";
    else
      terraintype = province->terrainType;
    if (province->isLake()) {
      terraintype = "lakes";
    }

    content.append(terraintype);
    content.append("\"\n");
  }
  pU::writeFile(path, content);
}
void writeMetadata(const std::string &path) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: Mod: Writing metadata.json");
  const auto templateFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath + "vic3//metadata.json");

  pU::writeFile(path, templateFile);
}
void strategicRegions(const std::string &path,
    const std::vector<std::shared_ptr<Arda::SuperRegion>> &strategicRegions,
                      const std::vector<std::shared_ptr<Region>> &regions) {

  Fwg::Utils::Logging::logLine("Vic3 Parser: Map: Writing Strategig Regions");
  const auto templateFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//common//strategic_regions//template.txt");
  std::string file = "";
  for (const auto &region : strategicRegions) {
    auto content = templateFile;
    std::string states{""};
    std::string capital;
    bool capitalSelected = false;
    for (const auto &state : region->ardaRegions) {
      states.append(" STATE_" + state->name);
      if (!capitalSelected) {
        capitalSelected = true;
        capital = state->ardaProvinces[0]->toHexString(true, true);
      }
    }
    Rpx::Parsing::replaceOccurences(content, "template_name", region->name);
    Rpx::Parsing::replaceOccurences(content, "template_states", states);
    Rpx::Parsing::replaceOccurences(content, "template_capital", capital);

    file.append(content);
  }
  pU::writeFile(path, file, true);
}
void cultureCommon(const std::string &path,
    const std::vector<std::shared_ptr<Arda::Culture>> &cultures) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: Common: Writing cultures");
  const auto culturesTemplate = pU::readFile(
      Fwg::Cfg::Values().resourcePath + "vic3//common//cultureTemplate.txt");
  std::string cultureFile{""};
  for (const auto &culture : cultures) {
    auto cultString = culturesTemplate;

    Rpx::Parsing::replaceOccurences(cultString, "templateCulture",
                                    culture->name);
    std::string colour = Fwg::Utils::varsToString(
        (int)culture->colour.getRed(), " ", (int)culture->colour.getGreen(),
        " ", (int)culture->colour.getBlue());
    Rpx::Parsing::replaceOccurences(cultString, "templateColour", colour);
    // Rpx::Parsing::replaceOccurences(cultString, "templateReligion",
    //                                 culture->primaryReligion->name);
    cultureFile.append(cultString);
  }

  pU::writeFile(path, cultureFile, true);
}
void religionCommon(const std::string &path,
    const std::vector<std::shared_ptr<Arda::Religion>> &religions) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: History: Writing religions");

  const auto religionTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//common//singleReligionTemplate.txt");
  auto religionFile = pU::readFile(Fwg::Cfg::Values().resourcePath +
                                   "vic3//common//religionTemplate.txt");
  for (const auto &religion : religions) {
    auto relString = religionTemplate;

    Rpx::Parsing::replaceOccurences(relString, "templateReligion",
                                    religion->name);
    religionFile.append(relString);
  }
  pU::writeFile(path, religionFile, true);
}
void countryCommon(
    const std::string &path,
    const std::map<std::string, std::shared_ptr<Country>> &countries,
    const std::vector<std::shared_ptr<Region>> &regions) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: Common: Writing countries");
  const auto countryTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//common//countryDefinitionTemplate.txt");
  std::string countryDefinition{""};
  for (const auto &country : countries) {
    auto cString = countryTemplate;

    Rpx::Parsing::replaceOccurences(cString, "templateTag",
                                    country.second->tag);
    std::string colour =
        Fwg::Utils::varsToString((int)country.second->colour.getRed(), " ",
                                 (int)country.second->colour.getGreen(), " ",
                                 (int)country.second->colour.getBlue());
    Rpx::Parsing::replaceOccurences(cString, "templateColour", colour);
    auto capitalRegion = regions[country.second->capitalRegionID];
    Rpx::Parsing::replaceOccurences(cString, "templateCapital",
                                    capitalRegion->name);
    auto regCultures = capitalRegion->gatherCultures();
    using pair_type = decltype(regCultures)::value_type;
    auto pr = std::max_element(std::begin(regCultures), std::end(regCultures),
                               [](const pair_type &p1, const pair_type &p2) {
                                 return p1.second < p2.second;
                               });

    Rpx::Parsing::replaceOccurences(cString, "templateCultures",
                                    pr->first->name);
    countryDefinition.append(cString);
  }
  pU::writeFile(path, countryDefinition, true);
}
void popsHistory(const std::string &path,
                 const std::vector<std::shared_ptr<Region>> &regions) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: History: Writing pops");
  auto popsFile = pU::readFile(Fwg::Cfg::Values().resourcePath +
                               "vic3//common//history//popsTemplate.txt");
  // std::string statePops = "";
  const auto popsSingleTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//common//history//popsSingleTemplate.txt");
  const auto popsStateTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//common//history//popsStateTemplate.txt");
  std::string listOfStates{""};
  for (const auto &region : regions) {
    if (!region->isLand())
      continue;
    auto statePops = popsStateTemplate;
    Rpx::Parsing::replaceOccurences(statePops, "templateName", region->name);
    // TODO: real country
    Rpx::Parsing::replaceOccurences(statePops, "templateTag",
                                    region->owner->tag);

    std::string listOfPops{""};
    for (auto &culture : region->gatherCultures()) {
      std::string pop = popsSingleTemplate;
      Rpx::Parsing::replaceOccurences(pop, "templateCulture",
                                      culture.first->name);
      Rpx::Parsing::replaceOccurences(pop, "templatePopSize",
                                      std::to_string(region->totalPopulation));
      listOfPops.append(pop);
    }

    Rpx::Parsing::replaceOccurences(statePops, "templatePopList", listOfPops);
    listOfStates.append(statePops);
  }
  Rpx::Parsing::replaceOccurences(popsFile, "templatePopsData", listOfStates);

  pU::writeFile(path, popsFile, true);
}
void stateHistory(const std::string &path,
                  const std::vector<std::shared_ptr<Region>> &regions) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: History: Writing state history");
  auto file = pU::readFile(Fwg::Cfg::Values().resourcePath +
                           "vic3//common//history//states.txt");
  std::string stateContent = "";
  const auto stateTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//common//history//stateTemplate.txt");
  for (const auto &region : regions) {
    if (!region->isLand())
      continue;
    auto content = stateTemplate;
    Rpx::Parsing::replaceOccurences(content, "templateName", region->name);
    Rpx::Parsing::replaceOccurences(content, "templateCountry",
                                    region->owner->tag);
    std::string provinceString{""};
    for (auto prov : region->ardaProvinces) {
      provinceString.append("\"" + prov->toHexString(true, true) + "\" ");
    }
    Rpx::Parsing::replaceOccurences(content, "templateProvinces",
                                    provinceString);
    std::string cultures;
    for (auto &culture : region->gatherCultures()) {
      cultures.append("add_homeland = cu:" + culture.first->name + "\n\t\t");
    }
    Rpx::Parsing::replaceOccurences(content, "templateCulture", cultures);
    stateContent.append(content);
  }
  Rpx::Parsing::replaceOccurences(file, "templateStateData", stateContent);

  pU::writeFile(path, file, true);
}
void countryHistory(
    const std::string &path,
    const std::map<std::string, std::shared_ptr<Country>> &countries) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: Common: Writing country history");
  const auto countryTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//common//history//countryHistoryTemplate.txt");
  for (const auto &country : countries) {
    auto cString = countryTemplate;

    Rpx::Parsing::replaceOccurences(cString, "templateTag",
                                    country.second->tag);
    Rpx::Parsing::replaceOccurences(cString, "template_techlevel",
                                    country.second->techLevel);
    std::string filename =
        country.second->tag + " - " + country.second->name + ".txt";
    pU::writeFile(path + "//" + filename, cString, true);
  }
}
void staticModifiers(const std::string &path,
    const std::vector<std::shared_ptr<Arda::Culture>> &cultures,
    const std::vector<std::shared_ptr<Arda::Religion>> &religions) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: Common: Writing static modifiers");
  const auto cultureTemplateFile = pU::readFile(
      Fwg::Cfg::Values().resourcePath + "vic3//common//static_modifiers//"
                                        "culture_standard_of_living.txt");
  std::string cultureContent = "";
  for (const auto &culture : cultures) {
    auto content = cultureTemplateFile;
    Rpx::Parsing::replaceOccurences(content, "templateCulture", culture->name);
    cultureContent.append(content);
  }
  pU::writeFile(path + "//07_culture_standard_of_living.txt", cultureContent);
  // now the same for the religions
  const auto religionTemplateFile = pU::readFile(
      Fwg::Cfg::Values().resourcePath + "vic3//common//static_modifiers//"
                                        "religion_standard_of_living.txt");
  std::string religionContent = "";
  for (const auto &religion : religions) {
    auto content = religionTemplateFile;
    Rpx::Parsing::replaceOccurences(content, "templateReligion",
                                    religion->name);
    religionContent.append(content);
  }
  pU::writeFile(path + "//08_religion_standard_of_living.txt", religionContent);
}
void splineNetwork(const std::string &path) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: Gfx: Writing splines");
  pU::writeFile(path + "//spline_network.splnet", "");
}

void compatFile(const std::string &path) {
  Fwg::Utils::Logging::logLine(
      "Vic3 Parser: Mod: Writing empty file for compatibility to ", path);
  pU::writeFile(path, "", true);
}

std::string compatRegions(const std::string &inFolder,
                          const std::string &outPath,
                          const std::vector<std::shared_ptr<Region>> &regions) {
  return "";
  Fwg::Utils::Logging::logLine(
      "Vic3 Parser: Map: Reading compatibility Regions from ", inFolder);
  int counter = regions.size() + 1;
  std::string foundRegionNames = "";
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;

    std::string filename =
        pathString.substr(pathString.find_last_of("//") + 1,
                          pathString.back() - pathString.find_last_of("//"));
    std::string content = "";
    auto lines = pU::getLines(pathString);
    for (auto &line : lines) {
      if (line.find("STATE_") != std::string::npos) {
        content.append(line);
        auto stateName = line.substr(0, line.find("=") - 1);
        foundRegionNames.append(stateName);
        content.append("\n\tid = " + std::to_string(counter++) + "\n");
        content.append("\tprovinces = { }\n");
        content.append("}\n");
      }
    }
    pU::writeFile(outPath + filename, content);
  }
  return foundRegionNames;
}
void compatStratRegions(const std::string &inFolder, const std::string &outPath,
                        const std::vector<std::shared_ptr<Region>> &regions,
                        std::string &baseArdaRegions) {
  return;
  Fwg::Utils::Logging::logLine(
      "Vic3 Parser: Map: Generating compatibility Strategic Regions from ",
      inFolder);
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    auto filePath = dir_entry.path();
    std::string filename = filePath.filename().string();
    if (filename.find(".txt") == std::string::npos)
      continue;
    std::string content = "";
    auto lines = pU::getLines(filePath.string());
    auto hexID = regions[0]->ardaProvinces[0]->toHexString(true, true);
    auto blocks = Rpx::Parsing::getOuterBlocks(lines);
    for (auto &block : blocks) {
      Rpx::Parsing::removeLines(block.content, "capital_province");
      content.append(block.name + " = {\n");
      content.append("\tcapital_province = " + hexID + "\n");
      content.append(block.content);
      content.append("}\n");
    }
    // for (auto &line : lines) {
    //   if (line.find("region_") != std::string::npos) {
    //     content.append(line);
    //     content.append("\n\tgraphical_culture = \"arabic\"\n");
    //     content.append("\tcapital_province = " + hexID + "\n");
    //     content.append("\tmap_color = { 0.5 0 0 }\n");
    //     content.append("\tstates = { }\n");
    //     content.append("}\n");
    //   }
    // }
    pU::writeFile(outPath + filename, content, true);
  }
}

void compatReleasable(const std::string &inFolder, const std::string &outPath) {
  Fwg::Utils::Logging::logLine(
      "Vic3 Parser: History: Compatibility Releasable Countries from ",
      inFolder);
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;
    std::string filename =
        pathString.substr(pathString.find_last_of("//") + 1,
                          pathString.back() - pathString.find_last_of("//"));
    Fwg::Utils::Logging::logLine("Determined filename: ", filename);
    std::string content = pU::readFile(pathString);
    while (
        Rpx::Parsing::removeBracketBlockFromBracket(content, "provinces = {")) {
    }
    Rpx::Parsing::replaceLines(content, "\tprovinces =", "provinces = { }\n");
    pU::writeFile(outPath + filename, content);
  }
}

void compatTriggers(const std::string &inFolder, const std::string &outPath) {
  Fwg::Utils::Logging::logLine(
      "Vic3 Parser: Common: Compatibility scripted Triggers ", inFolder);
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;
    std::string filename =
        pathString.substr(pathString.find_last_of("//") + 1,
                          pathString.back() - pathString.find_last_of("//"));
    std::string content = pU::readFile(pathString);
    Rpx::Parsing::removeLines(content, "sr:region");
    Rpx::Parsing::removeLines(content, "STATE_");
    pU::writeFile(outPath + filename, content);
  }
}

void locators(const std::string &path,
              const std::vector<std::shared_ptr<Region>> &regions) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: History: Writing locators");
  const auto locatorsTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//gfx//generated_map_object_locators.txt");
  const auto singeLocatorTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//gfx//single_locator_instance.txt");
  std::string cityContent = locatorsTemplate;
  pU::replaceOccurence(cityContent, "templateWaterClamp", "no");
  std::string farmContent = cityContent;
  std::string mineContent = cityContent;
  std::string portContent = locatorsTemplate;
  std::string woodContent = cityContent;
  pU::replaceOccurence(portContent, "templateWaterClamp", "yes");
  using namespace Fwg::Civilization;
  std::map<LocationType, std::string> locatorContent;
  std::map<LocationType, int> locatorCount;

  for (const auto &region : regions) {
    for (const auto &locator : region->significantLocations) {
      auto content = singeLocatorTemplate;
      pU::replaceOccurence(content, "templateID",
                           std::to_string(1 + region->ID));
      pU::replaceOccurence(content, "templateX",
                           std::to_string(locator->position.widthCenter));
      pU::replaceOccurence(content, "templateY",
                           std::to_string(locator->position.heightCenter));
      locatorContent[locator->type].append(content);
    }
  }
  pU::replaceOccurence(cityContent, "templateType", "city");
  pU::replaceOccurence(farmContent, "templateType", "farm");
  pU::replaceOccurence(mineContent, "templateType", "mine");
  pU::replaceOccurence(portContent, "templateType", "port");
  pU::replaceOccurence(woodContent, "templateType", "wood");

  pU::replaceOccurence(cityContent, "templateLocators",
                       locatorContent[LocationType::City]);
  pU::replaceOccurence(farmContent, "templateLocators", "");
  /* Until figured out why farm locators close to the coast crash the game, this
  is removed */
  // pU::replaceOccurence(farmContent, "templateLocators",
  //                      locatorContent[LocationType::Farm]);
  pU::replaceOccurence(mineContent, "templateLocators",
                       locatorContent[LocationType::Mine]);
  pU::replaceOccurence(portContent, "templateLocators",
                       locatorContent[LocationType::Port]);
  pU::replaceOccurence(woodContent, "templateLocators",
                       locatorContent[LocationType::Forest]);

  pU::writeFile(path + "generated_map_object_locators_city.txt", cityContent,
                true);
  pU::writeFile(path + "generated_map_object_locators_farm.txt", farmContent,
                true);
  pU::writeFile(path + "generated_map_object_locators_mine.txt", mineContent,
                true);
  pU::writeFile(path + "generated_map_object_locators_port.txt", portContent,
                true);
  pU::writeFile(path + "generated_map_object_locators_wood.txt", woodContent,
                true);
}

} // namespace Writing

namespace Reading {}
} // namespace Rpx::Vic3::Parsing

void Rpx::Vic3::Parsing::History::writeBuildings(
    const std::string &path,
    const std::vector<std::shared_ptr<Region>> &regions) {

  Fwg::Utils::Logging::logLine("Vic3 Parser: History: Constructing economy");
  auto buildingsTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//common//history//buildingsTemplate.txt");
  const auto buildingsStateTemplate =
      pU::readFile(Fwg::Cfg::Values().resourcePath +
                   "vic3//common//history//buildingsStateTemplate.txt");
  const auto buildingsSingleBuildingTemplate = pU::readFile(
      Fwg::Cfg::Values().resourcePath +
      "vic3//common//history//buildingsSingleBuildingTemplate.txt");
  std::string allStateString;
  for (auto &region : regions) {
    if (!region->isLand())
      continue;
    auto stateString = buildingsStateTemplate;
    pU::replaceOccurence(stateString, "templateStateName",
                         "STATE_" + region->name);

    pU::replaceOccurence(stateString, "templateTag", region->owner->tag);
    std::string allBuildings;
    for (auto &building : region->buildings) {
      allBuildings.append(buildingsSingleBuildingTemplate);
      pU::replaceOccurence(allBuildings, "templateBuildingName",
                           building.first);
      pU::replaceOccurence(allBuildings, "templateBuildingLevel",
                           std::to_string(building.second.level));
      pU::replaceOccurence(allBuildings, "templateProductionMethods",
                           "\"" + building.second.prodMethod.name + "\"");
    }
    pU::replaceOccurence(stateString, "templateBuildings", allBuildings);
    allStateString.append(stateString);
  }
  pU::replaceOccurence(buildingsTemplate, "templateStateBuildings",
                       allStateString);
  pU::writeFile(path, buildingsTemplate, true);
}
