#include "vic3/Vic3Parsing.h"
using namespace Fwg;
namespace Logging = Fwg::Utils::Logging;
namespace pU = Fwg::Parsing;
namespace Scenario::Vic3::Parsing {
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
                const std::vector<std::shared_ptr<GameProvince>> &provinces) {
  auto templateContent = pU::readFile("resources\\vic3\\map_data\\default.map");
  Logging::logLine("VIC3 Parser: Map: Default Map");
  std::string content = templateContent;
  std::string seaStarts{""};
  std::string lakes{""};
  auto seaFormatCounter = 1;
  auto lakeFormatCounter = 1;
  for (const auto &province : provinces) {
    if (province->baseProvince->sea) {
      seaStarts.append(province->toHexString() + " ");
      if (seaFormatCounter++ % 5 == 0) {
        seaStarts.append("\n\t");
      }
    } else if (province->baseProvince->isLake) {
      lakes.append(province->toHexString() + " ");
      if (lakeFormatCounter++ % 5 == 0) {
        lakes.append("\n\t");
      }
    }
  }
  pU::Scenario::replaceOccurences(content, "TEMPLATE_SEA_STARTS", seaStarts);
  pU::Scenario::replaceOccurences(content, "TEMPLATE_LAKES", lakes);
  pU::writeFile(path, content);
}

void heightmap(const std::string &path, const Fwg::Gfx::Bitmap &heightMap) {
  auto content = pU::readFile("resources\\vic3\\map_data\\heightmap.heightmap");
  Logging::logLine("VIC3 Parser: Map: heightmap.heightmap");
  pU::Scenario::replaceOccurences(
      content, "template_map_x", std::to_string(heightMap.bInfoHeader.biWidth));
  pU::Scenario::replaceOccurences(
      content, "template_map_y",
      std::to_string(heightMap.bInfoHeader.biHeight));
  pU::writeFile(path, content);
}

void stateFiles(const std::string &path,
                const std::vector<std::shared_ptr<Region>> &regions) {
  const auto templateFile =
      pU::readFile("resources\\vic3\\map_data\\state_template.txt");
  std::string file = "";
  for (const auto &region : regions) {
    auto content = templateFile;
    pU::Scenario::replaceOccurences(content, "template_name", region->name);
    pU::Scenario::replaceOccurences(content, "template_id",
                                    std::to_string(region->ID + 1));
    std::string provinceString{""};
    for (auto prov : region->gameProvinces) {
      provinceString.append("\"" + prov->toHexString() + "\" ");
    }
    pU::Scenario::replaceOccurences(content, "template_provinces",
                                    provinceString);
    // don't write these details for ocean regions
    if (!region->sea) {
      int counter = 0;
      pU::Scenario::replaceOccurences(
          content, "template_city",
          region
              ->gameProvinces[std::clamp(counter++, 0,
                                         (int)region->provinces.size() - 1)]
              ->toHexString());
      pU::Scenario::replaceOccurences(
          content, "template_port",
          region
              ->gameProvinces[std::clamp(counter++, 0,
                                         (int)region->provinces.size() - 1)]
              ->toHexString());
      pU::Scenario::replaceOccurences(
          content, "template_farm",
          region
              ->gameProvinces[std::clamp(counter++, 0,
                                         (int)region->provinces.size() - 1)]
              ->toHexString());
      pU::Scenario::replaceOccurences(
          content, "template_mine",
          region
              ->gameProvinces[std::clamp(counter++, 0,
                                         (int)region->provinces.size() - 1)]
              ->toHexString());
      pU::Scenario::replaceOccurences(
          content, "template_wood",
          region
              ->gameProvinces[std::clamp(counter++, 0,
                                         (int)region->provinces.size() - 1)]
              ->toHexString());

      // check if we are a coastal region
      for (auto &prov : region->gameProvinces) {
        if (prov->baseProvince->sea) {
          pU::Scenario::replaceOccurences(content, "template_naval_exit",
                                          prov->toHexString());
        }
      }
      pU::Scenario::replaceOccurences(content, "template_naval_exit", "");

    } else {
      pU::Scenario::replaceOccurences(content, "template_city", "");
      pU::Scenario::replaceOccurences(content, "template_port", "");
      pU::Scenario::replaceOccurences(content, "template_farm", "");
      pU::Scenario::replaceOccurences(content, "template_mine", "");
      pU::Scenario::replaceOccurences(content, "template_wood", "");
      pU::Scenario::replaceOccurences(content, "template_naval_exit", "");
    }

    file.append(content);
  }
  pU::writeFile(path, file, true);
}

void provinceTerrains(
    const std::string &path,
    const std::vector<std::shared_ptr<GameProvince>> &provinces) {
  // x48E2A5 = "desert"
  std::string content{""};
  for (const auto &province : provinces) {
    content.append(province->toHexString());
    content.append("=\"");
    std::string terraintype;
    if (province->baseProvince->sea)
      terraintype = "ocean";
    else
      terraintype = province->terrainType;
    if (province->baseProvince->isLake) {
      terraintype = "lakes";
    }

    content.append(terraintype);
    content.append("\"\n");
  }
  pU::writeFile(path, content);
}
void writeMetadata(const std::string &path) {
  const auto templateFile = pU::readFile("resources\\vic3\\metadata.json");

  pU::writeFile(path, templateFile);
}
void strategicRegions(const std::string &path,
                      const std::vector<strategicRegion> &strategicRegions,
                      const std::vector<std::shared_ptr<Region>> &regions) {

  const auto templateFile =
      pU::readFile("resources\\vic3\\common\\strategic_regions\\template.txt");
  std::string file = "";
  for (const auto &region : strategicRegions) {
    auto content = templateFile;
    std::string states{""};
    std::string capital;
    bool capitalSelected = false;
    for (auto stateID : region.gameRegionIDs) {
      const auto &state = regions[stateID];
      states.append(" STATE_" + state->name);
      if (!capitalSelected) {
        capitalSelected = true;
        capital = state->gameProvinces[0]->toHexString();
      }
    }
    pU::Scenario::replaceOccurences(content, "template_name", region.name);
    pU::Scenario::replaceOccurences(content, "template_states", states);
    pU::Scenario::replaceOccurences(content, "template_capital", capital);

    file.append(content);
  }
  pU::writeFile(path, file, true);
}
void cultureCommon(const std::string &path,
                   const std::vector<std::shared_ptr<Culture>> &cultures) {

  const auto culturesTemplate =
      pU::readFile("resources\\vic3\\common\\cultureTemplate.txt");
  std::string cultureFile{""};
  for (const auto &culture : cultures) {
    auto cultString = culturesTemplate;

    pU::Scenario::replaceOccurences(cultString, "templateCulture",
                                    culture->name);
    std::string colour = Fwg::Utils::varsToString(
        (int)culture->colour.getRed(), " ", (int)culture->colour.getGreen(),
        " ", (int)culture->colour.getBlue());
    pU::Scenario::replaceOccurences(cultString, "templateColour", colour);
    pU::Scenario::replaceOccurences(cultString, "templateReligion",
                                    culture->primaryReligion->name);
    cultureFile.append(cultString);
  }

  pU::writeFile(path, cultureFile, true);
}
void religionCommon(const std::string &path,
                    const std::vector<std::shared_ptr<Religion>> &religions) {

  const auto religionTemplate =
      pU::readFile("resources\\vic3\\common\\singleReligionTemplate.txt");
  auto religionFile =
      pU::readFile("resources\\vic3\\common\\religionTemplate.txt");
  for (const auto &religion : religions) {
    auto relString = religionTemplate;

    pU::Scenario::replaceOccurences(relString, "templateReligion",
                                    religion->name);
    religionFile.append(relString);
  }
  pU::writeFile(path, religionFile, true);
}
void countryCommon(const std::string &path,
                   const std::map<std::string, Country> &countries,
                   const std::vector<std::shared_ptr<Region>> &regions) {
  const auto countryTemplate =
      pU::readFile("resources\\vic3\\common\\countryDefinitionTemplate.txt");
  std::string countryDefinition{""};
  for (const auto &country : countries) {
    auto cString = countryTemplate;

    pU::Scenario::replaceOccurences(cString, "templateTag", country.second.tag);
    std::string colour =
        Fwg::Utils::varsToString((int)country.second.colour.getRed(), " ",
                                 (int)country.second.colour.getGreen(), " ",
                                 (int)country.second.colour.getBlue());
    pU::Scenario::replaceOccurences(cString, "templateColour", colour);
    auto capitalRegion = regions[country.second.capitalRegionID];
    pU::Scenario::replaceOccurences(cString, "templateCapital",
                                    capitalRegion->name);

    using pair_type = decltype(capitalRegion->cultures)::value_type;
    auto pr = std::max_element(std::begin(capitalRegion->cultures),
                               std::end(capitalRegion->cultures),
                               [](const pair_type &p1, const pair_type &p2) {
                                 return p1.second < p2.second;
                               });

    pU::Scenario::replaceOccurences(cString, "templateCultures",
                                    pr->first->name);
    countryDefinition.append(cString);
  }
  pU::writeFile(path, countryDefinition, true);
}
void popsHistory(const std::string &path,
                 const std::vector<std::shared_ptr<Region>> &regions) {
  auto popsFile =
      pU::readFile("resources\\vic3\\common\\history\\popsTemplate.txt");
  // std::string statePops = "";
  const auto popsSingleTemplate =
      pU::readFile("resources\\vic3\\common\\history\\popsSingleTemplate.txt");
  const auto popsStateTemplate =
      pU::readFile("resources\\vic3\\common\\history\\popsStateTemplate.txt");
  std::string listOfStates{""};
  for (const auto &region : regions) {
    if (region->sea)
      continue;
    auto statePops = popsStateTemplate;
    pU::Scenario::replaceOccurences(statePops, "templateName", region->name);
    // TODO: real country
    pU::Scenario::replaceOccurences(statePops, "templateTag", region->owner);

    std::string listOfPops{""};
    for (auto &culture : region->cultures) {
      std::string pop = popsSingleTemplate;
      pU::Scenario::replaceOccurences(pop, "templateCulture",
                                      culture.first->name);
      pU::Scenario::replaceOccurences(
          pop, "templatePopSize",
          std::to_string((int)(culture.second * 10000.0)));
      listOfPops.append(pop);
    }

    pU::Scenario::replaceOccurences(statePops, "templatePopList", listOfPops);
    listOfStates.append(statePops);
  }
  pU::Scenario::replaceOccurences(popsFile, "templatePopsData", listOfStates);

  pU::writeFile(path, popsFile, true);
}
void stateHistory(const std::string &path,
                  const std::vector<std::shared_ptr<Region>> &regions) {
  auto file = pU::readFile("resources\\vic3\\common\\history\\states.txt");
  std::string stateContent = "";
  const auto stateTemplate =
      pU::readFile("resources\\vic3\\common\\history\\stateTemplate.txt");
  for (const auto &region : regions) {
    if (region->sea)
      continue;
    auto content = stateTemplate;
    pU::Scenario::replaceOccurences(content, "templateName", region->name);
    pU::Scenario::replaceOccurences(content, "templateCountry", region->owner);
    std::string provinceString{""};
    for (auto prov : region->gameProvinces) {
      provinceString.append("\"" + prov->toHexString() + "\" ");
    }
    pU::Scenario::replaceOccurences(content, "templateProvinces",
                                    provinceString);

    pU::Scenario::replaceOccurences(content, "templateCulture", "dixie");
    stateContent.append(content);
  }
  pU::Scenario::replaceOccurences(file, "templateStateData", stateContent);

  pU::writeFile(path, file, true);
}
void countryHistory(const std::string &path,
                    const std::map<std::string, Country> &countries) {
  const auto countryTemplate =
      pU::readFile("resources\\vic3\\common\\countryHistoryTemplate.txt");
  for (const auto &country : countries) {
    auto cString = countryTemplate;

    pU::Scenario::replaceOccurences(cString, "templateTag", country.second.tag);
    std::string filename =
        country.second.tag + " - " + country.second.name + ".txt";
    pU::writeFile(path + "\\" + filename, cString, true);
  }
}
void splineNetwork(const std::string &path) {
  pU::writeFile(path + "//spline_network.splnet", "");
}

void compatCanals(const std::string &path) { pU::writeFile(path, "", true); }

std::string compatRegions(const std::string &inFolder, const std::string &outPath,
                   const std::vector<std::shared_ptr<Region>> &regions) {
  int counter = regions.size() + 1;
  std::string foundRegionNames = "";
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;

    std::string filename =
        pathString.substr(pathString.find_last_of("\\") + 1,
                          pathString.back() - pathString.find_last_of("\\"));
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
                        std::string &baseGameRegions) {
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;
    std::string filename =
        pathString.substr(pathString.find_last_of("\\") + 1,
                          pathString.back() - pathString.find_last_of("\\"));
    std::string content = "";
    auto lines = pU::getLines(pathString);
    auto hexID = regions[0]->gameProvinces[0]->toHexString();
    for (auto &line : lines) {
      if (line.find("region_") != std::string::npos) {
        content.append(line);
        content.append("\n\tgraphical_culture = \"arabic\"\n");
        content.append("\tcapital_province = " + hexID + "\n");
        content.append("\tmap_color = { 0.5 0 0 }\n");
        content.append("\tstates = { }\n");
        content.append("}\n");
      }
    }
    pU::writeFile(outPath + filename, content, true);
  }
}

void compatReleasable(const std::string &inFolder, const std::string &outPath) {
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;
    std::string filename =
        pathString.substr(pathString.find_last_of("\\") + 1,
                          pathString.back() - pathString.find_last_of("\\"));
    std::string content = pU::readFile(pathString);
    while (
        pU::Scenario::removeBracketBlockFromBracket(content, "provinces = {")) {
    }
    pU::Scenario::replaceLines(content, "\tprovinces =", "provinces = { }\n");
    pU::writeFile(outPath + filename, content);
  }
}

} // namespace Writing

namespace Reading {}
} // namespace Scenario::Vic3::Parsing